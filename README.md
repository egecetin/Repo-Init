<div align="center">

# 🚀 Repo-Init

<img src="doc/logo.png" alt="Repo-Init Logo" width="200">

### ⚡ **Modern C++ Application Template with Enterprise-Grade Features** ⚡

<p align="center">
  <img src="https://img.shields.io/github/languages/top/egecetin/Repo-Init?style=for-the-badge&logo=cplusplus&logoColor=white" alt="Top Language">
  <img src="https://img.shields.io/github/license/egecetin/Repo-Init?style=for-the-badge&color=brightgreen" alt="License">
</p>

<p align="center">
  <a href="https://github.com/egecetin/Repo-Init/actions/workflows/build_and_test.yml">
    <img src="https://img.shields.io/github/actions/workflow/status/egecetin/Repo-Init/build_and_test.yml?label=Build%20%26%20Test&branch=master&logo=github&style=for-the-badge" alt="Build Status">
  </a>
  <a href="https://sonarcloud.io/summary/new_code?id=egecetin_Repo-Init">
    <img src="https://img.shields.io/sonar/coverage/egecetin_Repo-Init?server=https%3A%2F%2Fsonarcloud.io&style=for-the-badge&logo=sonarqubecloud" alt="Coverage">
  </a>
  <a href="https://sonarcloud.io/summary/new_code?id=egecetin_Repo-Init">
    <img src="https://img.shields.io/sonar/quality_gate/egecetin_Repo-Init?server=https%3A%2F%2Fsonarcloud.io&style=for-the-badge&logo=sonarqubecloud" alt="Quality Gate">
  </a>
  <a href="https://scorecard.dev/viewer/?uri=github.com/egecetin/Repo-Init">
    <img src="https://img.shields.io/ossf-scorecard/github.com/egecetin/Repo-Init?label=OpenSSF%20Score&style=for-the-badge&logo=data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgdmlld0JveD0iMCAwIDI0IDI0IiBmaWxsPSJub25lIiBzdHJva2U9IndoaXRlIiBzdHJva2Utd2lkdGg9IjIiIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCI+PHJlY3QgeD0iMyIgeT0iMTEiIHdpZHRoPSIxOCIgaGVpZ2h0PSIxMSIgcng9IjIiIHJ5PSIyIj48L3JlY3Q+PHBhdGggZD0ibTcgMTFWN0E1IDUgMCAwIDEgMTcgN3Y0Ij48L3BhdGg+PC9zdmc+" alt="OpenSSF Score">
  </a>
</p>

---

</div>

## 🎯 What is Repo-Init?

**Repo-Init** is a powerful, production-ready CMake template designed to kickstart your C++ applications with enterprise-grade features built-in. Say goodbye to boilerplate code and hello to modern, robust software development!

> 🔧 **Requires**: C++17 supported compiler

> 💡 **Perfect for**: Microservices, CLI tools, system applications, and performance-critical software

<div align="center">

### 🌟 **Key Features at a Glance** 🌟

</div>

<table align="center">
<tr>
<td align="center" width="33%">

### 🛡️ **Crash Protection**
Automatic minidump generation with [**Crashpad**](https://chromium.googlesource.com/crashpad/crashpad/)
*Never lose critical debugging information*

</td>
<td align="center" width="33%">

### 📊 **Advanced Logging**
Multi-output logging with **Spdlog**
*File rotation • Syslog • Colorized console*

</td>
<td align="center" width="33%">

### 🔗 **Service Integration**
[**Sentry**](https://sentry.io/) • [**Grafana Loki**](https://grafana.com/oss/loki/) • [**Prometheus**](https://prometheus.io/)
*Real-time monitoring & alerting*

</td>
</tr>
<tr>
<td align="center">

### ⚡ **Performance Metrics**
- ✅ Success/failure counters
- ⏱️ Min/max timing analysis
- 📈 Quantile-based bottleneck detection

</td>
<td align="center">

### 🌐 **Network Ready**
- 🖥️ Built-in Telnet server
- ⚡ [**ZeroMQ**](https://zeromq.org/) messaging
- 🔌 HTTP connectivity

</td>
<td align="center">

### 🚀 **Developer Experience**
- 📦 One-command setup
- 🧪 Comprehensive testing
- 📋 Auto-generated docs

</td>
</tr>
</table>

---

> 💬 **Questions? Issues? Contributions?**
> Feel free to ask, use, and report any bugs you encounter! We ❤️ community feedback.

<div align="center">

[![SonarCloud](https://sonarcloud.io/images/project_badges/sonarcloud-orange.svg)](https://sonarcloud.io/summary/new_code?id=egecetin_Repo-Init)

---

</div>

## 📋 Table of Contents

- [🎯 What is Repo-Init?](#-what-is-repo-init)
- [📋 Table of Contents](#-table-of-contents)
- [🔧 CMake Modules](#-cmake-modules)
- [📜 Utility Scripts](#-utility-scripts)
- [📦 Dependencies](#-dependencies)
- [🧪 Testing Setup](#-testing-setup)
- [🎯 Build Targets](#-build-targets)
- [📊 Grafana Integration](#-grafana-integration)

## 🔧 CMake Modules

Our carefully crafted CMake modules provide powerful build automation:

| Module | 🎯 Purpose | ✨ Benefits |
|--------|------------|-------------|
| **CodeCoverage** | Detects and enables `gcovr` | 📊 Automatic test coverage reports |
| **CompilerSecurityOptions** | Enables/Disables secure compiler flags | 🛡️ Hardened binary security |
| **Doxy** | Finds Doxygen package and prepares docs | 📖 Auto-generated documentation |
| **GenerateSymbols** | Adds target for symbol file generation | 🔍 Enhanced debugging with minidumps |
| **GitVersion** | Gets SHA1 hash of current commit | 🏷️ Version tracking and build reproducibility |
| **GraphViz** | Finds GraphViz and dot executable | 🎨 Visual dependency graphs |

---

## 📜 Utility Scripts

> 💡 **Tip**: All scripts should be executed from the top-level directory

| Script | 🚀 Function | 📝 Description |
|--------|-------------|----------------|
| `firstName.sh` | 🏷️ **Name Changer** | Replaces placeholder names throughout the project |
| `dump_syms.py` | 🔧 **Symbol Dumper** | Generates symbol files for crash analysis |

---

## 📦 Dependencies

<div align="center">

### 🏗️ **Core Runtime Libraries**

</div>

<table>
<tr>
<td width="50%">

#### 🔥 **Integrated Dependencies**
*Built and bundled automatically*

- 🔌 [**CppZMQ**](https://github.com/zeromq/cppzmq.git) - Modern C++ ZeroMQ bindings
- 💥 [**Crashpad**](https://chromium.googlesource.com/crashpad/crashpad/) - Crash reporting system
- 📅 [**Date**](https://github.com/HowardHinnant/date.git) - C++ date/time library
- 🌐 [**Http-status-codes**](https://github.com/j-ulrich/http-status-codes-cpp.git) - HTTP utilities
- 📊 [**Prometheus-cpp**](https://github.com/jupp0r/prometheus-cpp.git) - Metrics collection
- ⚡ [**RapidJSON**](https://github.com/Tencent/rapidjson.git) - Ultra-fast JSON parser
- 🔐 [**Sentry**](https://github.com/getsentry/sentry-native.git) - Error monitoring
- 🖥️ [**TelnetServLib**](https://github.com/lukemalcolm/TelnetServLib.git) - *Modified & embedded*

</td>
<td width="50%">

#### 🛠️ **Development Dependencies**
*For building and testing only*

- 🔧 [**Breakpad**](https://chromium.googlesource.com/breakpad/breakpad/) - Symbol dumping
- ✅ [**GoogleTest**](https://github.com/google/googletest.git) - Unit testing framework
- 🔍 [**MemPlumber**](https://github.com/seladb/MemPlumber.git) - Memory leak detection
- 📦 **ZLIB** - Compression (required by Breakpad)

#### 🌐 **System Dependencies**
*Install via package manager (apt/dnf/brew)*

- 🌍 [**cURL**](https://github.com/curl/curl) - HTTP client library
- 📝 [**Spdlog**](https://github.com/gabime/spdlog.git) - Fast logging library
- ⚡ [**ZeroMQ**](https://github.com/zeromq/libzmq.git) - High-performance messaging

</td>
</tr>
</table>

<div align="center">

> 📊 **Want to see the full picture?** Check out our complete [**dependency graph**](doc/XXX-tree.svg)!

---

</div>

## 🧪 Testing Setup

<div align="center">

### 🐍 **Python Test Dependencies** 🐍

</div>

Our test suite requires some Python dependencies for comprehensive testing. Here's how to set them up:

<table>
<tr>
<td width="50%">

#### 🚀 **Quick Setup**

```bash
# Create virtual environment
python3 -m venv .venv

# Activate virtual environment
source .venv/bin/activate  # Linux/macOS
# OR
.venv\Scripts\activate     # Windows

# Install test dependencies
pip install -r tests/data/requirements.txt
```

</td>
<td width="50%">

#### 📋 **Required Dependencies**

- 🔌 **pyzmq** - Python ZeroMQ bindings for testing messaging functionality

#### ⚠️ **Important Notes**

- Virtual environment **must be activated** before running tests
- Dependencies are automatically detected by the test suite
- Deactivate with `deactivate` when done

</td>
</tr>
</table>

---

## 🎯 Build Targets

<div align="center">

### ⚡ **One Command, Multiple Possibilities** ⚡

</div>

| Target | 🚀 Command | 📋 Description |
|--------|------------|----------------|
| **🏗️ all** | `cmake --build .` | Builds the complete project with all components |
| **📊 coverage** | `cmake --build . --target coverage` | Generates comprehensive test coverage reports |
| **📖 docs** | `cmake --build . --target docs` | Creates beautiful documentation with Doxygen |
| **🎨 dependency-graph** | `cmake --build . --target dependency-graph` | Visualizes project dependencies with GraphViz |
| **📦 package** | `cmake --build . --target package` | Creates distribution packages (DEB/RPM + systemd service) |
| **✅ test** | `ctest . --parallel` | Runs the complete test suite with GoogleTest |

<div align="center">

> 💡 **Pro Tips**:
> - For packages, specify your preferred format with `-DCPACK_GENERATOR="DEB"` or `"RPM"`
> - Ensure Python virtual environment is activated before running tests!

---

</div>

### 🎛️ **Build Options**

<div align="center">

**Customize your build with powerful CMake configuration options**

</div>

> 💡 **Important**: Re-run CMake configuration (`cmake -B build`) after changing any options

<table>
<tr>
<td width="60%">

#### 🧪 **Testing & Quality Assurance**

| Option | Description | Default | 🎯 Use Case |
|--------|-------------|---------|-------------|
| `XXX_BUILD_TESTS` | 🔧 Build all test suites | `ON` | Complete testing pipeline |
| `XXX_BUILD_UNITTESTS` | ✅ Build unit tests only | `ON` | Fast development feedback |
| `XXX_BUILD_FUZZTESTS` | 🎲 Build fuzz testing suite | `OFF` | Security & robustness testing |
| `XXX_ENABLE_COVERAGE` | 📊 Generate test coverage reports | `OFF` | Code quality metrics |
| `XXX_ENABLE_MEMLEAK_CHECK` | 🔍 Memory leak detection with MemPlumber | `OFF` | Debug memory issues |

#### 🚀 **Release & Distribution**

| Option | Description | Default | 🎯 Use Case |
|--------|-------------|---------|-------------|
| `XXX_ENABLE_SYMBOL_GENERATION` | 🔧 Generate debug symbols for crash dumps | `OFF` | Production debugging |
| `XXX_ENABLE_PACKAGING` | 📦 Enable DEB/RPM packaging with systemd | `OFF` | Distribution & deployment |

</td>
<td width="40%">

#### 💡 **Quick Examples**

```bash
# Development build with all tests
cmake -B build -DXXX_BUILD_TESTS=ON \
                -DXXX_ENABLE_COVERAGE=ON

# Production build with packaging
cmake -B build -DXXX_BUILD_TESTS=OFF \
                -DXXX_ENABLE_PACKAGING=ON \
                -DXXX_ENABLE_SYMBOL_GENERATION=ON

# Security testing build
cmake -B build -DXXX_BUILD_FUZZTESTS=ON \
                -DXXX_ENABLE_MEMLEAK_CHECK=ON

# Minimal build (fastest)
cmake -B build -DXXX_BUILD_TESTS=OFF
```

#### ⚡ **Pro Tips**

- 🏃 **Fast Iteration**: Disable tests for quick builds during development
- 🔒 **Security Focus**: Enable fuzz tests and memory checks for critical code
- 📈 **CI/CD**: Use coverage reports in your automated pipelines
- 🚀 **Production**: Always enable symbol generation for crash analysis

</td>
</tr>
</table>


## 📊 Grafana Integration

<div align="center">

### 🎯 **Real-Time Monitoring Made Beautiful** 🎯

Thanks to our integrated **Prometheus server**, monitoring your application has never been easier! Get instant insights into your application's performance, health, and behavior.

</div>

<table>
<tr>
<td width="50%">

#### 🚀 **Quick Setup**

1. 📥 **Import Dashboard**: Use our pre-built [**Grafana template**](scripts/GrafanaDashboard-1730032129887.json)
2. 🔗 **Connect Prometheus**: Point to your app's metrics endpoint
3. 📊 **Monitor**: Watch real-time metrics flow in!

#### ✨ **What You Get**

- 📈 **Performance Metrics**: Response times, throughput, resource usage
- 🚨 **Error Tracking**: Real-time error rates and alerting
- 💾 **Resource Monitoring**: CPU, memory, and system metrics
- 🔍 **Custom Metrics**: Track your application-specific KPIs

</td>
<td width="50%">

<div align="center">

#### 📸 **Live Dashboard Preview**

<img src="doc/GrafanaDashboard.png" alt="Grafana Dashboard" width="100%">

*Beautiful, responsive, and information-rich monitoring*

</div>

</td>
</tr>
</table>

<div align="center">

---

## 🤝 **Contributing & Support**

<p>
  <a href="https://github.com/egecetin/Repo-Init/issues">🐛 Report Issues</a> •
  <a href="https://github.com/egecetin/Repo-Init/discussions">💬 Discussions</a> •
  <a href="https://github.com/egecetin/Repo-Init/pulls">🔧 Pull Requests</a>
</p>

**Made with ❤️ for the C++ community**

*Star ⭐ this repo if you find it useful!*

---

</div>
