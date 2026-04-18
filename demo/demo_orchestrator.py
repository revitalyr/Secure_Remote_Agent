#!/usr/bin/env python3
"""
Demo Orchestrator - Coordinates the demo flow:
1. Starts pseudo-service (demo_simulator)
2. Starts agent
3. Periodically requests System Metrics from agent
4. Sends metrics to backend server
"""

import subprocess
import time
import requests
import json
import sys
import os
from pathlib import Path

class DemoOrchestrator:
    def __init__(self):
        self.project_root = Path(__file__).parent.parent
        self.build_dir = self.project_root / "build" / "src" / "components" / "service" / "Release"
        self.agent_dir = self.project_root / "build" / "src" / "components" / "agent" / "Release"
        self.backend_url = "http://localhost:5000"
        self.service_process = None
        self.agent_process = None
        self.running = False

    def start_service(self):
        """Start the pseudo-service (simulator)"""
        service_path = self.build_dir / "demo_simulator.exe"
        if not service_path.exists():
            print(f"ERROR: Service not found at {service_path}")
            return False
        
        print(f"Starting service: {service_path}")
        self.service_process = subprocess.Popen(
            [str(service_path)],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE
        )
        time.sleep(2)
        print("Service started")
        return True

    def start_agent(self):
        """Start the agent"""
        agent_path = self.agent_dir / "agent.exe"
        if not agent_path.exists():
            print(f"ERROR: Agent not found at {agent_path}")
            return False
        
        # Copy plugins to agent directory
        plugins_source = self.project_root / "build" / "src" / "plugins"
        plugins_dest = self.agent_dir / "plugins"
        if plugins_source.exists() and not plugins_dest.exists():
            os.makedirs(plugins_dest, exist_ok=True)
            # Copy from proxy and scraper plugin directories
            for plugin_dir in ["proxy", "scraper"]:
                plugin_path = plugins_source / plugin_dir / "Release"
                if plugin_path.exists():
                    for dll in plugin_path.glob("*.dll"):
                        import shutil
                        shutil.copy(dll, plugins_dest)
            print("Plugins copied to agent directory")
        
        print(f"Starting agent: {agent_path}")
        self.agent_process = subprocess.Popen(
            [str(agent_path)],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE
        )
        time.sleep(2)
        print("Agent started")
        return True

    def get_metrics_from_agent(self):
        """
        Request metrics from agent.
        For now, this is a placeholder since agent doesn't have HTTP interface.
        In the real implementation, the agent would expose an HTTP endpoint or use IPC.
        """
        # For now, we'll simulate getting metrics from the service via IPC
        # The agent should be the one doing this, but for simplicity in the demo,
        # we'll request directly from the backend's agents endpoint
        # which receives metrics from the service via /api/metrics

        try:
            # Get agent metrics from backend (which receives from service)
            response = requests.get(f"{self.backend_url}/api/agents", timeout=5)
            if response.status_code == 200:
                agents_data = response.json()
                # Extract metrics from the first service
                for service_id, service_data in agents_data.items():
                    return {
                        "cpu_percent": service_data.get("cpu_usage", 0.0),
                        "memory_percent": service_data.get("memory_usage", 0.0)
                    }
        except Exception as e:
            print(f"Error getting metrics: {e}")

        return {}

    def send_metrics_to_backend(self, metrics):
        """Send metrics to backend server"""
        if not metrics:
            return
        
        try:
            response = requests.post(
                f"{self.backend_url}/api/metrics",
                json={
                    "service_id": "SecureRemoteAgent",
                    "cpu_usage": metrics.get("cpu_percent", 0),
                    "memory_usage": metrics.get("memory_percent", 0),
                    "uptime": int(time.time()),
                    "requests_handled": 0,
                    "timestamp": int(time.time())
                },
                timeout=5
            )
            if response.status_code == 200:
                print(f"Metrics sent to backend: CPU={metrics.get('cpu_percent', 0):.1f}%, Memory={metrics.get('memory_percent', 0):.1f}%")
        except Exception as e:
            print(f"Error sending metrics to backend: {e}")

    def run(self):
        """Main orchestrator loop"""
        print("=== Demo Orchestrator Starting ===")
        
        # Start service
        if not self.start_service():
            print("Failed to start service")
            return
        
        # Start agent
        if not self.start_agent():
            print("Failed to start agent")
            self.stop()
            return
        
        self.running = True
        print("Orchestrator running. Press Ctrl+C to stop.")
        
        try:
            while self.running:
                # Get metrics from backend (which gets from service via IPC)
                metrics = self.get_metrics_from_agent()
                
                # Send metrics to backend
                self.send_metrics_to_backend(metrics)
                
                # Wait before next iteration
                time.sleep(5)
                
        except KeyboardInterrupt:
            print("\nStopping orchestrator...")
            self.running = False
        finally:
            self.stop()

    def stop(self):
        """Stop all processes"""
        print("Stopping orchestrator...")
        
        if self.agent_process:
            self.agent_process.terminate()
            try:
                self.agent_process.wait(timeout=5)
            except subprocess.TimeoutExpired:
                self.agent_process.kill()
            print("Agent stopped")
        
        if self.service_process:
            self.service_process.terminate()
            try:
                self.service_process.wait(timeout=5)
            except subprocess.TimeoutExpired:
                self.service_process.kill()
            print("Service stopped")
        
        print("Orchestrator stopped")

if __name__ == "__main__":
    orchestrator = DemoOrchestrator()
    orchestrator.run()
