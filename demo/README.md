# Secure Remote Agent - Demo

This demo showcases the complete Secure Remote Agent system with all features working together.

## Demo Overview

The demo demonstrates:
1. **Windows Service** running in the background
2. **Plugin System** with Proxy and Scraper plugins
3. **TLS/HTTPS** secure communications
4. **Qt/QML UI** with real-time monitoring
5. **IPC Communication** between UI and Service
6. **Backend Integration** with REST API

## Demo Architecture

```
Demo Environment:
+---------------------+     +---------------------+     +---------------------+
|   Demo UI           |     |   Demo Service      |     |   Demo Backend      |
|   (Qt/QML)          |<--->|   (Windows Service) |<--->|   (Python Flask)    |
|                     | IPC |                     | HTTPS|                     |
| - Live Demo Control |     | - Plugin Manager    |     | - Mock API Server   |
| - Metrics Display   |     | - Agent Simulator   |     | - Data Collector    |
| - Plugin Demo        |     | - TLS Client        |     | - Web Dashboard     |
+---------------------+     +---------------------+     +---------------------+
```

## Quick Start

### 1. Build the Project
```bash
# Build all components
cmake --build build --config Release

# Run tests to verify everything works
cmake --build build --target run_all_tests
```

### 2. Start Demo Backend
```bash
# Navigate to demo backend
cd demo/backend

# Install dependencies
pip install -r requirements.txt

# Start demo server
python demo_server.py
```

The backend will start on `http://localhost:5000`

### 3. Install and Start Windows Service
```bash
# Install the service (run as Administrator)
sc create "SecureRemoteAgentDemo" binPath="[build-dir]\service\Release\service_main.exe --demo"

# Start the service
sc start "SecureRemoteAgentDemo"

# Check service status
sc query "SecureRemoteAgentDemo"
```

### 4. Launch Demo UI
```bash
# Start the demo UI
[build-dir]\app\Release\main.exe
```

### 5. Open Web Dashboard
Navigate to: `http://localhost:5000/dashboard`

## Demo Features

### 1. Service Control Demo
- **Start/Stop Service**: Control the Windows Service from UI
- **Service Status**: Real-time service status monitoring
- **Ping Test**: Test IPC connectivity between UI and Service

### 2. Plugin System Demo
- **Proxy Plugin**: 
  - Intercepts HTTP/HTTPS requests
  - Shows traffic forwarding metrics
  - Demonstrates request/response handling
  
- **Scraper Plugin**:
  - Scrapes demo websites
  - Shows extracted data
  - Demonstrates scheduled scraping

### 3. TLS Security Demo
- **Certificate Management**: Load and verify certificates
- **TLS Connection**: Secure HTTPS connections to backend
- **Certificate Pinning**: MITM protection demonstration

### 4. Real-time Monitoring Demo
- **System Metrics**: Live CPU and RAM usage
- **Agent Status**: Connected agents simulation
- **Log Stream**: Real-time log display with filtering
- **Performance Graphs**: Visual metrics representation

### 5. IPC Communication Demo
- **Qt RemoteObjects**: Type-safe communication
- **Real-time Updates**: Live data synchronization
- **Command Execution**: Remote command demonstration

## Demo Scripts

### Automated Demo Script
```bash
# Run the complete automated demo
demo/run_demo.sh
```

This script will:
1. Build all components
2. Run unit and integration tests
3. Start backend server
4. Install and start service
5. Launch UI
6. Run demo scenarios
7. Generate demo report

### Manual Demo Steps

#### Scenario 1: Basic Service Operation
1. Start the service via UI "Start" button
2. Observe service status change to "Running"
3. Click "Ping" to test IPC connectivity
4. Monitor log stream for service events

#### Scenario 2: Plugin Demonstration
1. Load Proxy Plugin: `plugins/proxy.dll`
2. Execute proxy operations with test URLs
3. Observe proxy metrics and logs
4. Load Scraper Plugin: `plugins/scraper.dll`
5. Run scraping tasks on demo websites
6. Monitor extracted data and results

#### Scenario 3: TLS Security Demo
1. Configure TLS certificates
2. Enable certificate pinning
3. Test secure connections to backend
4. Verify certificate validation
5. Monitor TLS metrics and security events

#### Scenario 4: Performance Monitoring
1. Observe real-time CPU and RAM usage
2. Monitor agent connections and status
3. Track plugin performance metrics
4. Analyze log patterns and system health

## Demo Data

### Mock Backend API
The demo backend provides:
- `/api/heartbeat` - Agent heartbeat endpoint
- `/api/metrics` - Metrics collection endpoint
- `/api/plugins` - Plugin status endpoint
- `/api/logs` - Log aggregation endpoint
- `/dashboard` - Web dashboard interface

### Sample Configuration Files
- `config/service.json` - Service configuration
- `config/plugins.json` - Plugin settings
- `config/tls.json` - TLS certificates setup
- `config/demo.json` - Demo-specific settings

## Troubleshooting Demo

### Common Issues
1. **Service Won't Start**: Check permissions and dependencies
2. **UI Can't Connect**: Verify IPC server is running
3. **Plugin Loading Fails**: Check plugin paths and dependencies
4. **TLS Connection Errors**: Verify certificate configuration
5. **Backend Not Responding**: Check Python server status

### Demo Logs
- Service logs: Windows Event Viewer
- UI logs: Console output and `logs/ui.log`
- Plugin logs: `logs/plugins/` directory
- Backend logs: `logs/backend.log`

### Reset Demo
```bash
# Stop all demo components
demo/reset_demo.sh

# Clean demo data
demo/clean_demo.sh

# Restart from scratch
demo/run_demo.sh
```

## Performance Benchmarks

The demo includes performance benchmarks:
- **IPC Latency**: < 5ms average
- **Plugin Loading**: < 100ms per plugin
- **TLS Handshake**: < 200ms
- **UI Response Time**: < 50ms
- **Memory Usage**: < 100MB baseline

## Extending the Demo

### Adding New Plugins
1. Implement `IPlugin` interface
2. Build as DLL
3. Add to demo configuration
4. Test with demo UI

### Custom Backend Endpoints
1. Modify `demo/backend/demo_server.py`
2. Add new API routes
3. Update UI to use new endpoints
4. Test integration

### Additional Test Scenarios
1. Create new test files in `tests/`
2. Add demo scenarios
3. Update demo scripts
4. Document new features

## Demo Video

A video demonstration is available at:
`demo/video/secure_agent_demo.mp4`

The video shows:
- Complete system setup
- All demo scenarios
- Performance metrics
- Feature highlights

## Support

For demo issues:
1. Check troubleshooting section
2. Review demo logs
3. Verify configuration files
4. Run automated tests
5. Contact development team
