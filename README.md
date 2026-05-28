# Secure Remote Agent

Windows Service-based multi-agent system. IPC via Qt RemoteObjects, plugin architecture with dynamic loading, TLS 1.3 encrypted backend communication, and Qt Quick real-time monitoring UI.

## Architecture

```
+---------------------+     +----------------------+     +---------------------+
| Desktop UI          |     | Windows Service      |     | Backend Server      |
| (Qt/QML)            |<--->| (C++23, Qt6)         |<--->| (Python/REST)       |
|                     | IPC |                      | HTTPS|                     |
| - Live metrics      |     | - Agent management   |     | - Data storage      |
| - Service control   |     | - Plugin loading     |     | - REST API          |
| - Log streaming     |     | - IPC server         |     | - Web dashboard     |
| - Plugin management |     | - TLS client         |     | - Analytics         |
+---------------------+     +----------------------+     +---------------------+
                                      |
                                      | plugins
                                      v
                            +-------------------------+
                            | Plugin Architecture     |
                            | +---------------------+ |
                            | | Proxy Plugin        | |
                            | | HTTP/HTTPS traffic  | |
                            | +---------------------+ |
                            | | Scraper Plugin      | |
                            | | Web data extraction | |
                            | +---------------------+ |
                            +-------------------------+
```

## Components

### Core Library (`src/core/`)
Static library providing shared functionality:
- **IPC**: Qt RemoteObjects host/client with typed interface (ping/pong, metrics streaming, log streaming, agent operations)
- **Network**: TLS 1.3 HTTPS client with certificate pinning, OpenSSL integration
- **Remote Execution**: Async command dispatch, payload serialization

### Windows Service (`src/components/service/`)
Windows service executable. StartServiceCtrlDispatcher-based implementation with:
- Service lifecycle: START, STOP, PAUSE, CONTINUE
- QtRO IPC server on `local:service_ipc`
- Plugin loader (runtime DLL discovery in `plugins/` directory)
- Periodic heartbeat and metric push to backend (REST)
- System metrics collection via Windows API (CPU, memory, network I/O)
- Watchdog timer (3 missed ticks triggers recovery heartbeat)

### Desktop UI (`src/components/app/`)
Qt Quick application:
- System metrics display: CPU, memory, uptime
- Service management: start/stop/restart, status visualization
- Agent management: connected agents list, individual control
- Log streaming: real-time display, levels, timestamps
- Plugin management: load/unload/status

### Agent (`src/components/agent/`)
Standalone agent executable. Connects to service via IPC, executes commands through plugin system.

### Plugins
Dynamic libraries loaded at runtime via Qt Plugin system.

| Plugin | Type | Description |
|--------|------|-------------|
| Proxy  | SHARED | HTTP/HTTPS traffic interception, request forwarding, metrics collection |
| Scraper | SHARED | Web scraping with configurable selectors, scheduled execution, data extraction |

### Backend Server
Python Flask server. Provides REST API endpoints for service registration, heartbeat, metrics collection, plugin status, and web dashboard.

## Build

### Dependencies
- Windows 10/11 SDK 10.0.26200
- Visual Studio 2022 (C++23)
- Qt 6.x (Core, Network, RemoteObjects, Quick, Gui)
- OpenSSL
- Python 3.9+ (backend server)
- CMake 3.21+
- vcpkg with x64-windows triplet

### Build Steps
```
cmake --preset default
cmake --build build --config Release
```

### Targets

| Target | Type | Output |
|--------|------|--------|
| `core` | STATIC | `core.lib` |
| `service_main` | EXECUTABLE | `service_main.exe` |
| `main` | EXECUTABLE | `main.exe` |
| `agent` | EXECUTABLE | `agent.exe` |
| `demo_simulator` | EXECUTABLE | `demo_simulator.exe` |
| `proxy_plugin` | SHARED | `proxy_plugin.dll` |
| `scraper_plugin` | SHARED | `scraper_plugin.dll` |
| `unit_tests` | EXECUTABLE | `unit_tests.exe` |

### Service Installation
```
sc create "SecureRemoteAgent" binPath="<build>/src/components/service/Release/service_main.exe"
sc start "SecureRemoteAgent"
```

## Project Structure
```
include/                  # Public headers (semantic aliases)
  core/                   # IPC server/client, network client, remote executor
  components/
    agent/                # Agent interface
    app/                  # UI manager
    service/              # Service controller, source impl
  config/                 # Constants
  plugins/
    loader/               # Plugin interface
    proxy/                # Proxy plugin header
    scraper/              # Scraper plugin header
src/
  core/                   # Core library source
  components/
    agent/                # Agent source
    app/                  # UI source (main.cpp, ui_manager.cpp, QML)
    service/              # Service source
  plugins/
    loader/               # Plugin loader source
    proxy/                # Proxy plugin source
    scraper/              # Scraper plugin source
tests/
  unit/                   # Unit tests (Catch2)
  integration/            # Integration tests (Catch2)
CMakeLists.txt            # Top-level build configuration
```

## IPC Interface

Defined in `src/core/ServiceInterface.rep`:
- Properties: `serviceStatus`, `cpuUsage`, `memoryUsage`, `memoryTotal`, `memoryAvailable`, `netRxKbps`, `netTxKbps`, `uptime`
- Slots: `ping()`, `requestShutdown()`
- Signals: `pongReceived(QString)`, `logMessage(QString, QString)`

Generated via `qt6_add_repc_merged()` into `rep_ServiceInterface_merged.h`.

## Tests

```
cmake --build build --target unit_tests
./build/tests/unit/Debug/unit_tests.exe
```

Requires Catch2. Enable with `-DBUILD_TESTS=ON`.

## License

MIT
