#include <string>
#include <vector>

class Inidbi2
{
public:
  Inidbi2(std::string const& basePath);

  std::string Invoke(std::string const& parameters);

private:
  std::string separator;
  std::string basePath;

  bool SufficientParams(std::vector<std::string> const& params, unsigned int expected);
  void Log(std::string const& message);

  std::string GetFullPath(std::string const& file) const;

  std::string Write(std::string const& file, std::string const& section, std::string const& key, std::string const& value);
  std::string Read(std::string const& file, std::string const& section, std::string const& key);
  std::string DeleteSection(std::string const& file, std::string const& section);
  std::string DeleteKey(std::string const& file, std::string const& section, std::string const& key);
  std::string Delete(std::string const& file);
  bool Exists(std::string  const&file);
  std::string GetTimestamp();
  std::string EncodeBase64(std::string const& plaintext);
  std::string DecodeBase64(std::string const& encoded);
  std::string GetSections(std::string const& file);
};
