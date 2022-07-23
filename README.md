## Project

![GitHub top language](https://img.shields.io/github/languages/top/egecetin/Repo-Init?style=plastic)
![GitHub](https://img.shields.io/github/license/egecetin/Repo-Init?style=plastic)
![GitHub Workflow Status](https://img.shields.io/github/workflow/status/egecetin/Repo-Init/Build%20and%20test?style=plastic)
![Codecov](https://img.shields.io/codecov/c/github/egecetin/Repo-Init?style=plastic&token=G65MG0J07F)
![LGTM Grade](https://img.shields.io/lgtm/grade/cpp/github/egecetin/Repo-Init?style=plastic)

## Contents

- [Project](#project)
- [Contents](#contents)
- [CMake](#cmake)
- [Scripts](#scripts)
- [Dependencies](#dependencies)
- [Targets](#targets)

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

 - [CppZMQ](https://github.com/zeromq/cppzmq.git)
 - [MemPlumber](https://github.com/seladb/MemPlumber.git)
 - [RapidJSON](https://github.com/Tencent/rapidjson.git)
 - [Spdlog](https://github.com/gabime/spdlog.git)
 - [Sentry](https://github.com/getsentry/sentry-native.git)
 - [TelnetServLib](https://github.com/lukemalcolm/TelnetServLib.git) (Modified and embedded to source directory)
 - [ZeroMQ](https://github.com/zeromq/libzmq.git)

## Targets

 - all              : Prepares all targets
 - coverage         : Prepares coverage report
 - docs             : Prepares documentation
 - dependency-graph : Prepares graphviz visualization of dependencies
 - package          : Prepares Debian, RPM and makeself packages
 - test             : Prepares gtest target
