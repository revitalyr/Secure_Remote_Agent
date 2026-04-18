#!/bin/bash

# Secure Remote Agent Demo Runner
# This script sets up and runs the complete demo

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Demo configuration
BUILD_DIR="build"
DEMO_BACKEND_DIR="demo/backend"
SERVICE_NAME="SecureRemoteAgentDemo"

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}Secure Remote Agent Demo Setup${NC}"
echo -e "${BLUE}========================================${NC}"

# Function to print colored output
print_status() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if running as Administrator (for Windows service operations)
check_admin() {
    if [[ "$OSTYPE" == "msys" || "$OSTYPE" == "win32" ]]; then
        # Windows - check if running as Administrator
        net session > /dev/null 2>&1
        if [ $? -ne 0 ]; then
            print_warning "This script should be run as Administrator for Windows Service operations"
            read -p "Continue anyway? (y/N): " -n 1 -r
            echo
            if [[ ! $REPLY =~ ^[Yy]$ ]]; then
                exit 1
            fi
        fi
    fi
}

# Build the project
build_project() {
    print_status "Building the project..."
    
    if [ ! -d "$BUILD_DIR" ]; then
        mkdir -p "$BUILD_DIR"
    fi
    
    cd "$BUILD_DIR"
    
    # Configure with CMake
    print_status "Configuring with CMake..."
    cmake .. -DCMAKE_BUILD_TYPE=Release
    
    # Build
    print_status "Building project..."
    cmake --build . --config Release
    
    cd ..
    print_status "Build completed successfully!"
}

# Run tests
run_tests() {
    print_status "Running unit and integration tests..."
    
    cd "$BUILD_DIR"
    
    # Run unit tests
    print_status "Running unit tests..."
    ctest --output-on-failure -R "unit_" || print_warning "Some unit tests failed"
    
    # Run integration tests
    print_status "Running integration tests..."
    ctest --output-on-failure -R "integration_" || print_warning "Some integration tests failed"
    
    cd ..
    print_status "Tests completed!"
}

# Setup Python backend
setup_backend() {
    print_status "Setting up Python backend..."
    
    cd "$DEMO_BACKEND_DIR"
    
    # Check if Python is available
    if ! command -v python3 &> /dev/null; then
        print_error "Python 3 is required but not installed"
        exit 1
    fi
    
    # Install requirements
    print_status "Installing Python dependencies..."
    pip3 install -r requirements.txt || pip install -r requirements.txt
    
    cd ..
    print_status "Backend setup completed!"
}

# Start backend server
start_backend() {
    print_status "Starting demo backend server..."
    
    cd "$DEMO_BACKEND_DIR"
    
    # Start backend in background
    python3 demo_server.py &
    BACKEND_PID=$!
    
    # Wait for backend to start
    sleep 3
    
    # Check if backend is running
    if curl -s http://localhost:5000/api/system > /dev/null; then
        print_status "Backend server started successfully (PID: $BACKEND_PID)"
        echo $BACKEND_PID > backend.pid
    else
        print_error "Failed to start backend server"
        exit 1
    fi
    
    cd ..
}

# Install Windows Service
install_service() {
    print_status "Installing Windows Service..."
    
    SERVICE_PATH="$(pwd)/$BUILD_DIR/service/Release/service_main.exe"
    
    if [ ! -f "$SERVICE_PATH" ]; then
        print_error "Service executable not found at $SERVICE_PATH"
        exit 1
    fi
    
    # Stop and delete existing service if it exists
    sc query "$SERVICE_NAME" > /dev/null 2>&1
    if [ $? -eq 0 ]; then
        print_status "Stopping existing service..."
        sc stop "$SERVICE_NAME"
        sleep 2
        sc delete "$SERVICE_NAME"
    fi
    
    # Install new service
    sc create "$SERVICE_NAME" binPath="$SERVICE_PATH --demo" start=auto
    if [ $? -eq 0 ]; then
        print_status "Windows Service installed successfully"
    else
        print_error "Failed to install Windows Service"
        exit 1
    fi
}

# Start Windows Service
start_service() {
    print_status "Starting Windows Service..."
    
    sc start "$SERVICE_NAME"
    if [ $? -eq 0 ]; then
        print_status "Windows Service started successfully"
        
        # Wait for service to initialize
        sleep 3
        
        # Check service status
        SERVICE_STATUS=$(sc query "$SERVICE_NAME" | grep "STATE" | awk '{print $4}')
        print_status "Service status: $SERVICE_STATUS"
    else
        print_error "Failed to start Windows Service"
        exit 1
    fi
}

# Launch UI
launch_ui() {
    print_status "Launching Demo UI..."
    
    UI_PATH="$(pwd)/$BUILD_DIR/app/Release/main.exe"
    
    if [ ! -f "$UI_PATH" ]; then
        print_error "UI executable not found at $UI_PATH"
        exit 1
    fi
    
    # Start UI in background
    "$UI_PATH" &
    UI_PID=$!
    
    print_status "Demo UI launched (PID: $UI_PID)"
    echo $UI_PID > ui.pid
}

# Open web dashboard
open_dashboard() {
    print_status "Opening web dashboard..."
    
    # Wait a moment for backend to be fully ready
    sleep 2
    
    # Open dashboard in default browser
    if command -v start &> /dev/null; then
        start http://localhost:5000/dashboard
    elif command -v xdg-open &> /dev/null; then
        xdg-open http://localhost:5000/dashboard
    else
        print_status "Open http://localhost:5000/dashboard in your browser"
    fi
}

# Show demo information
show_demo_info() {
    print_status "Demo is now running!"
    echo
    echo -e "${BLUE}Demo Components:${NC}"
    echo -e "  Backend Server: ${GREEN}http://localhost:5000${NC}"
    echo -e "  Web Dashboard: ${GREEN}http://localhost:5000/dashboard${NC}"
    echo -e "  Windows Service: ${GREEN}$SERVICE_NAME${NC}"
    echo -e "  Desktop UI: ${GREEN}Running in background${NC}"
    echo
    echo -e "${BLUE}Demo Features:${NC}"
    echo -e "  1. Service Control - Start/Stop/Restart service from UI"
    echo -e "  2. Plugin Management - Load and test Proxy/Scraper plugins"
    echo -e "  3. Real-time Metrics - Monitor CPU, Memory, and agent status"
    echo -e "  4. Log Streaming - View real-time logs from all components"
    echo -e "  5. TLS Security - Test secure HTTPS connections"
    echo -e "  6. IPC Communication - Test Qt RemoteObjects messaging"
    echo
    echo -e "${BLUE}Demo URLs:${NC}"
    echo -e "  Main Dashboard: ${GREEN}http://localhost:5000/dashboard${NC}"
    echo -e "  API Endpoints: ${GREEN}http://localhost:5000/api/${NC}"
    echo -e "  System Info:    ${GREEN}http://localhost:5000/api/system${NC}"
    echo
    echo -e "${YELLOW}Press Ctrl+C to stop the demo${NC}"
}

# Cleanup function
cleanup() {
    print_status "Cleaning up demo..."
    
    # Stop backend
    if [ -f "$DEMO_BACKEND_DIR/backend.pid" ]; then
        BACKEND_PID=$(cat "$DEMO_BACKEND_DIR/backend.pid")
        kill $BACKEND_PID 2>/dev/null || true
        rm -f "$DEMO_BACKEND_DIR/backend.pid"
        print_status "Backend server stopped"
    fi
    
    # Stop UI
    if [ -f "ui.pid" ]; then
        UI_PID=$(cat "ui.pid")
        kill $UI_PID 2>/dev/null || true
        rm -f "ui.pid"
        print_status "Demo UI stopped"
    fi
    
    # Stop Windows Service
    sc query "$SERVICE_NAME" > /dev/null 2>&1
    if [ $? -eq 0 ]; then
        sc stop "$SERVICE_NAME"
        print_status "Windows Service stopped"
    fi
    
    print_status "Demo cleanup completed!"
}

# Set up signal handlers
trap cleanup EXIT INT TERM

# Main execution
main() {
    check_admin
    build_project
    run_tests
    setup_backend
    start_backend
    install_service
    start_service
    launch_ui
    open_dashboard
    show_demo_info
    
    # Keep script running
    while true; do
        sleep 10
        # Check if backend is still running
        if ! curl -s http://localhost:5000/api/system > /dev/null; then
            print_warning "Backend server is not responding, restarting..."
            start_backend
        fi
    done
}

# Run main function
main "$@"
