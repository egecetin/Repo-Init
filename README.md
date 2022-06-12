
## CMake

- AutoBuildNumber       : Auto increments the build number
- Colorize              : Color commands for message outputs
- CodeCoverage          : Detects and enables gcovr
- CustomFindSubversion  : Modified version of FindSubversion.cmake to ignore errors
- Doxy                  : Find doxygen package and prepare doc environment
- GitVersion            : Get SHA1 hash of current commit
- SvnVersion            : Get revision number of current commit

## Scripts

All scripts should be executed from top level directory

- create_installer      : Creates installer package using makeself
- firstName             : Script to change placeholder name
- init                  : Script used initialization of this repo
- ldd-copy-dependencies : Script to copy dynamic dependencies of a binary
- makeself              : [Makeself](https://github.com/megastep/makeself) self-extractable archive creator
- prepare-release       : Prepares a release package
- system-setup          : Installs desired packages on RedHat based systems

## Dependencies

 - [ZeroMQ](https://github.com/zeromq/libzmq.git)
 - [CppZMQ](https://github.com/zeromq/cppzmq.git)
 - [RapidJSON](https://github.com/Tencent/rapidjson.git)
 - [Spdlog](https://github.com/gabime/spdlog.git)
 - [MemPlumber](https://github.com/seladb/MemPlumber.git)
 - [TelnetServLib](https://github.com/lukemalcolm/TelnetServLib.git) (Modified and embedded to source directory)
