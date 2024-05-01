<div align="center" width="50">
<img src=doc/logo.png>

![GitHub top language](https://img.shields.io/github/languages/top/egecetin/Repo-Init)
![GitHub](https://img.shields.io/github/license/egecetin/Repo-Init)
[![GitHub Workflow Status](https://img.shields.io/github/actions/workflow/status/egecetin/Repo-Init/build_and_test.yml?label=Actions&branch=master&logo=github)](https://github.com/egecetin/Repo-Init/actions/workflows/build_and_test.yml)
[![Coverage](https://sonarcloud.io/api/project_badges/measure?project=egecetin_Repo-Init&metric=coverage)](https://sonarcloud.io/summary/new_code?id=egecetin_Repo-Init)
[![OpenSSF Scorecard](https://api.scorecard.dev/projects/github.com/egecetin/Repo-Init/badge)](https://scorecard.dev/viewer/?uri=github.com/egecetin/Repo-Init)

</div>

## Project
CMake template to create new C++ applications with basic codes/interfaces are already defined. Requires a C++14 supported compiler.

It provides the following features
  - Tracing
    - Generates automatically minidump files using [Crashpad](https://chromium.googlesource.com/crashpad/crashpad/) for error signals.
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

Feel free to ask, use and report any bugs you encountered!

<div align="center" width="50">

[![SonarCloud](https://sonarcloud.io/images/project_badges/sonarcloud-orange.svg)](https://sonarcloud.io/summary/new_code?id=egecetin_Repo-Init)
</div>

## Contents

- [Project](#project)
- [Contents](#contents)
- [CMake](#cmake)
- [Scripts](#scripts)
- [Dependencies](#dependencies)
- [Targets](#targets)

## CMake

- CodeCoverage          : Detects and enables gcovr
- CompilerSecurityOption: Enables/Disables secure compiler flags
- Doxy                  : Find doxygen package and prepare doc environment
- GenerateSymbols       : Adds a target for generation of symbol files for minidump
- GitVersion            : Get SHA1 hash of current commit
- GraphViz              : Find graphviz and dot executable to create dependency graph

## Scripts

All scripts should be executed from top level directory

- firstName             : Script to change placeholder name
- dump_syms             : Dumps symbol files

## Dependencies

 - [CppZMQ](https://github.com/zeromq/cppzmq.git)
 - [crashpad](https://chromium.googlesource.com/crashpad/crashpad/)
 - [Date](https://github.com/HowardHinnant/date.git)
 - [Http-status-codes](https://github.com/j-ulrich/http-status-codes-cpp.git)
 - [Prometheus-cpp](https://github.com/jupp0r/prometheus-cpp.git)
 - [RapidJSON](https://github.com/Tencent/rapidjson.git)
 - [Sentry](https://github.com/getsentry/sentry-native.git)
 - [TelnetServLib](https://github.com/lukemalcolm/TelnetServLib.git) (Modified and embedded to source directory)

Developing dependencies not required for runtime,

 - [breakpad](https://chromium.googlesource.com/breakpad/breakpad/) (It is required for automatically dumping symbols)
 - [GoogleTest](https://github.com/google/googletest.git) (For testing)
 - [MemPlumber](https://github.com/seladb/MemPlumber.git) (For testing)
 - [ZLIB]() (It is required by breakpad)

These runtime dependencies should be installed to the system (for example via package managers like apt or dnf),

 - [cURL](https://github.com/curl/curl)
 - [Spdlog](https://github.com/gabime/spdlog.git)
 - [ZeroMQ](https://github.com/zeromq/libzmq.git)

Full dependency graph can be seen [here](doc/XXX-tree.svg)

## Targets

 - all              : Prepares all targets
 - coverage         : Prepares coverage report
 - docs             : Prepares documentation
 - dependency-graph : Prepares graphviz visualization of dependencies
 - package          : Prepares default packages
 - test             : Prepares gtest target
