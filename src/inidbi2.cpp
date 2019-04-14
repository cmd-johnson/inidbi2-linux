#include "inidbi2.h"

#include <ctime>
#include <cstdio>
#include <iostream>
#include <iomanip>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/transform_width.hpp>

namespace pt = boost::property_tree;

#define VERSION "2.05 (linux)"
#define BASE_DIR "./@inidbi2/db/"

Inidbi2* instance = nullptr;

extern "C" void RVExtension(char* output, int outputSize, const char* function)
{
  if (instance == nullptr) {
    instance = new Inidbi2(BASE_DIR);
  }

  std::string result = instance->Invoke(function);
  strncpy(output, result.c_str(), outputSize);
  output[outputSize - 1] = '\0';
}

Inidbi2::Inidbi2(std::string const& basePath) : separator("|"), basePath(basePath)
{
}

std::string Inidbi2::Invoke(std::string const& parameters)
{
  std::vector<std::string> lines;
  boost::split(lines, parameters, boost::is_any_of(this->separator));

  if (lines.empty())
  {
    this->Log("Error: No Function Name Provided");
    return "";
  }

  std::string function = lines[0];

  if (function == "version") {
    return VERSION;
  } else if (function == "write" && this->SufficientParams(lines, 4)) {
    return this->Write(lines[1], lines[2], lines[3], lines[4]);
  } else if (function == "read" && this->SufficientParams(lines, 3)) {
    return this->Read(lines[1], lines[2], lines[3]);
  } else if (function == "deletesection" && this->SufficientParams(lines, 2)) {
    return this->DeleteSection(lines[1], lines[2]);
  } else if (function == "deletekey" && this->SufficientParams(lines, 3)) {
    return this->DeleteKey(lines[1], lines[2], lines[3]);
  } else if (function == "delete" && this->SufficientParams(lines, 1)) {
    return this->Delete(lines[1]);
  } else if (function == "exists" && this->SufficientParams(lines, 1)) {
    return this->Exists(lines[1]) ? "true" : "false";
  } else if (function == "gettimestamp") {
    return this->GetTimestamp();
  } else if (function == "decodebase64" && this->SufficientParams(lines, 1)) {
    return this->DecodeBase64(lines[1]);
  } else if (function == "encodebase64" && this->SufficientParams(lines, 1)) {
    return this->EncodeBase64(lines[1]);
  } else if (function == "setseparator" && this->SufficientParams(lines, 1)) {
    this->separator = lines[1];
    return "";
  } else if (function == "getseparator") {
    return this->separator;
  } else if (function == "getsections" && this->SufficientParams(lines, 1)) {
    return this->GetSections(lines[1]);
  } else {
    this->Log("Invalid function: " + function);
    return "";
  }
}

bool Inidbi2::SufficientParams(std::vector<std::string> const& params, unsigned int expected)
{
  // > and not >= because params[0] is the function name to be called.
  bool enoughParams = params.size() > expected;
  if (!enoughParams) {
    this->Log("Error: Insufficient Parameters");
  }
  return enoughParams;
}

void Inidbi2::Log(std::string const& message)
{
  std::cerr << "Inidbi2: " << message << std::endl;
}

std::string Inidbi2::GetFullPath(std::string const& file) const
{
  return this->basePath + file;
}

std::string Inidbi2::Write(std::string const& file, std::string const& section, std::string const& key, std::string const& value)
{
  // Because of *reasons*, the input string is always wrapped in quotes, leading to errors in Arma when trying to read the values back.
  std::string strippedValue = value.substr(1, value.length() - 2);

  try {
    using namespace boost::property_tree;

    std::string path = this->GetFullPath(file);
    ptree tree;
    std::fstream iniFile;
    iniFile.open(path, std::fstream::in);
    read_ini(iniFile, tree);
    iniFile.close();

    tree.put(section + "." + key, strippedValue);
    write_ini(path, tree);

    return "true";
  } catch (const std::exception& e) {
    this->Log(std::string("Error accessing ini: ") + e.what());
  }
  return "false";
}

std::string Inidbi2::Read(std::string const& file, std::string const& section, std::string const& key)
{
  if (this->Exists(file)) {
    try {
      using namespace boost::property_tree;
      ptree tree;
      read_ini(this->GetFullPath(file), tree);
      boost::optional<std::string> value = tree.get_optional<std::string>(section + "." + key);
      if (value) {
        std::ostringstream s;
        s << "[true, " << value.get() << "]";
        return s.str();
      }
    } catch (const std::exception& e) {
      this->Log(std::string("Error reading ini: ") + e.what());
    }
  }
  return "[false, \"\"]";
}

std::string Inidbi2::DeleteSection(std::string const& file, std::string const& section)
{
  try {
    using namespace boost::property_tree;

    std::string path = this->GetFullPath(file);
    ptree tree;
    read_ini(path, tree);
    tree.erase(section);
    write_ini(path, tree);

    return "true";
  } catch (const std::exception& e) {
    this->Log(std::string("Error accessing ini: ") + e.what());
  }
  return "false";
}

std::string Inidbi2::DeleteKey(std::string const& file, std::string const& section, std::string const& key)
{
  try {
    using namespace boost::property_tree;

    std::string path = this->GetFullPath(file);
    ptree tree;
    read_ini(path, tree);
    auto s = tree.get_child_optional(section);
    if (s) {
      s.get().erase(key);
    }
    write_ini(path, tree);

    return "true";
  } catch (const std::exception& e) {
    this->Log(std::string("Error accessing ini: ") + e.what());
  }
  return "false";
}

std::string Inidbi2::Delete(std::string const& file)
{
  int errorCode = remove(this->GetFullPath(file).c_str());
  if (errorCode != 0) {
    this->Log("Error deleting file: " + file);
  }
  return errorCode == 0 ? "true" : "false";
}

bool Inidbi2::Exists(std::string const& file)
{
  std::string path = this->GetFullPath(file);
  std::ifstream infile(path);
  return infile.good();
}

std::string Inidbi2::GetTimestamp()
{
  std::ostringstream ts;
  std::time_t t = std::time(nullptr);
  ts << std::put_time(std::localtime(&t), "[%Y,%m,%d,%H,%M,%S]");
  return ts.str();
}

std::string Inidbi2::EncodeBase64(std::string const& plaintext)
{
  // Taken from https://stackoverflow.com/a/28471421/7483565
  using namespace boost::archive::iterators;
  using It = base64_from_binary<transform_width<std::string::const_iterator, 6, 8>>;
  auto tmp = std::string(It(std::begin(plaintext)), It(std::end(plaintext)));
  return tmp.append((3 - plaintext.size() % 3) % 3, '=');
}

std::string Inidbi2::DecodeBase64(std::string const& encoded)
{
  // Taken from https://stackoverflow.com/a/28471421/7483565
  using namespace boost::archive::iterators;
  using It = transform_width<binary_from_base64<std::string::const_iterator>, 8, 6>;
  return boost::algorithm::trim_right_copy_if(std::string(It(std::begin(encoded)), It(std::end(encoded))), [](char c) {
    return c == '\0';
  });
}

std::string Inidbi2::GetSections(std::string const& file)
{
  if (this->Exists(file)) {
    try {
      using namespace boost::property_tree;
      ptree tree;
      read_ini(this->GetFullPath(file), tree);
      std::ostringstream sections;
      sections << "[";
      bool first = true;
      for (const auto& section: tree) {
        if (!first) {
          sections << ", ";
        }
        sections << "\"" << section.first << "\"";
        first = false;
      }
      sections << "]";
      return sections.str();
    } catch (const std::exception& e) {
      this->Log(std::string("Error reading ini: ") + e.what());
    }
  }
  return "[]";
}
