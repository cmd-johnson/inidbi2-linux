# inidbi2 for Linux Servers
[inidbi2](https://github.com/code34/inidbi2/) is an extension for Arma 3 that allows to save data to .ini files that later can be read again.
This allows to persist data and allows for state that survives across server restarts, while not requiring a full-blown (SQL) database.

Unfortunately, inidbi2 (as opposed to inidbi) is written in C# and only provides a dll file for INI handling, so it does not work on Linux servers.

This implementation of inidbi2 aims to address this by re-implementing all C# code of inidbi2 in C++, allowing us to generate a Shared Object file, making the extension usable on Linux servers.

## Disclaimer
This is a quick & dirty re-implementation of the C# code of inidbi2 in C++.
It is not designed with efficiency in mind.

I replaced all Windows-specific function calls with stuff I found in the [boost](https://www.boost.org/) library.
On my server, this 'just works', but you might need to install additional dependencies that I am not (yet) aware of.
Open an issue if you find you were missing a dependency, then I'll add them here.

Also note that the `inidbi2.so` file is *huge* compared to the `inidbi2.dll`.
This is because the boost library is statically linked.
Dynamic linking might work just as well, but my aim was to get this done fast and to do the bare minimum to get it to work.

## Installation
To install the extension, first [download the inidbi2 extension](https://forums.bohemia.net/forums/topic/186131-inidbi2-save-and-load-data-to-the-server-or-your-local-computer-without-databases).
Put it into your server directory, e.g. `/usr/games/arma3/@inidbi2`.
Rename the `Addons` folder inside the `@inidbi2` folder to `addons` (The Arma server for Linux doesn't like the capitalized name).
Download the inidbi2.so file built from this project and place it inside the `@inidbi2` folder (e.g. `/usr/games/arma3/@inidbi2/inidbi2.so`).

Make sure you add `@inidbi2` to your server's startup command line (e.g. `-serverMod=@inidbi2`).

That's it!
You should now be able to use the extension on your Linux server.

You can try it by loading up a mission, enabling the admin debug console in the mission's properties, logging in as admin, entering the following snippet into your Arma console and running it on your server:
```sqf
_inidbi = ["new", "test"] call OO_INIDBI;
_version = "getVersion" call _inidbi;
diag_log format ["Inidbi version: %1", _version];
```

Look into your server's output, there should be a line reading "Inidbi version: Inidbi: 2.05 Dll: 2.05 (linux)".
