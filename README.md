<div align="center" width="50">
<img src=scripts/data/branding/logo.png>

![GitHub top language](https://img.shields.io/github/languages/top/egecetin/Repo-Init?style=for-the-badge)
![GitHub](https://img.shields.io/github/license/egecetin/Repo-Init?style=for-the-badge)
[![GitHub Workflow Status](https://img.shields.io/github/actions/workflow/status/egecetin/Repo-Init/build_and_test.yml?label=Actions&branch=master&logo=github&style=for-the-badge)](https://github.com/egecetin/Repo-Init/actions/workflows/build_and_test.yml)
[![Codecov](https://img.shields.io/codecov/c/github/egecetin/Repo-Init?logo=codecov&logoColor=white&style=for-the-badge)](https://app.codecov.io/gh/egecetin/Repo-Init)
[![Codacy grade](https://img.shields.io/codacy/grade/33df0cae00b84c25b09df2561c10fe3a?logo=codacy&style=for-the-badge)](https://app.codacy.com/gh/egecetin/Repo-Init/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_grade)
</div>

## Project
CMake template to create new C++ applications with basic codes/interfaces are already defined.

It provides the following features
  - Logging
    - Spdlog with rotating file, syslog and coloured stdout outputs
    - [Sentry](https://sentry.io/) and [Grafana Loki](https://grafana.com/oss/loki/) integration for automatic forwarding of logs to an external HTTP server
    - [Prometheus](https://prometheus.io/) client for track internal metrics like error counts, performance of functions including
      - Incremental counters to track success/fail rates and active processes,
      - Minimum and maximum timings,
      - Quantile based performance metrics to determine bottlenecks
  - User interactivity
    - Embedded Telnet Server
    - [ZeroMQ](https://zeromq.org/) socket

Periodically scanned with [Codacy](https://app.codacy.com/gh/egecetin/Repo-Init/dashboard) and [Sonarcloud](https://sonarcloud.io/project/overview?id=egecetin_Repo-Init)

## Contents

- [Project](#project)
- [Contents](#contents)
- [CMake](#cmake)
- [Scripts](#scripts)
- [Dependencies](#dependencies)
- [Targets](#targets)
- [Developer comments](#developer-comments)

## CMake

- AutoBuildNumber       : Auto increments the build number
- CodeCoverage          : Detects and enables gcovr
- Colorize              : Color commands for message outputs
- CompilerFlags         : Defines compiler flags for GCC and Intel Compiler for compile types
- CompilerOptions       : Enables/Disables compiler warnings
- CStandards            : Defines c standard
- CppStandards          : Defines cpp standard
- CustomFindSubversion  : Modified version of FindSubversion.cmake to ignore errors
- Doxy                  : Find doxygen package and prepare doc environment
- GitVersion            : Get SHA1 hash of current commit
- GraphViz              : Find graphviz and dot executable to create dependency graph
- PostBuild             : Post build action to prepare makeself package after CPack while calculating checksums for all binaries
- SvnVersion            : Get revision number of current commit

## Scripts

All scripts should be executed from top level directory

- create_installer      : Creates installer package using makeself
- firstName             : Script to change placeholder name
- generateKey           : Generates public/private key pairs
- ldd-copy-dependencies : Script to copy dynamic dependencies of a binary
- makeself              : [Makeself](https://github.com/megastep/makeself) self-extractable archive creator
- passwordless-ssh      : Script to enable passwordless ssh from Windows hosts
- system-setup          : Installs desired packages on RedHat based systems. Check scripts/SYSTEMSETUP.md for details

## Dependencies

 - [Backward-cpp](https://github.com/bombela/backward-cpp)
 - [Date](https://github.com/HowardHinnant/date.git)
 - [CppZMQ](https://github.com/zeromq/cppzmq.git)
 - [Http-status-codes](https://github.com/j-ulrich/http-status-codes-cpp.git)
 - [GoogleTest](https://github.com/google/googletest.git) (For tests only)
 - [MemPlumber](https://github.com/seladb/MemPlumber.git) (For tests only)
 - [Prometheus-cpp](https://github.com/jupp0r/prometheus-cpp.git)
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
 - package          : Prepares makeself package
 - test             : Prepares gtest target

## Developer comments

 - Update branding information after cloning. The folder should include three icons/logos and a css file. Look the branding.css or RHEL branding folder for recommended image sizes
   - logo.png
   - favicon.ico
   - apple-touch-icon.png
   - branding.css
