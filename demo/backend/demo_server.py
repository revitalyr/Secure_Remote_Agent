#!/usr/bin/env python3
"""
Demo Backend Server for Secure Remote Agent
Flask-based REST API with web dashboard
"""

import json
import time
import threading
from datetime import datetime
from flask import Flask, render_template, request, jsonify, redirect
from flask_cors import CORS
import psutil
import logging

app = Flask(__name__)
CORS(app)

# Configure logging
logging.basicConfig(level=logging.DEBUG)
logger = logging.getLogger(__name__)

# Configure werkzeug logger to ERROR to suppress access logs
logging.getLogger('werkzeug').setLevel(logging.ERROR)

# Global state for backend (receives real data from service)
agents = {}  # Real agent data from Windows Service
logs = []
plugins = {}  # Real plugin data from Windows Service
system_stats = {}  # Real system metrics from Windows Service

def initialize_backend():
    """Initialize backend - ready to receive real data"""
    # Add initial log entry
    logs.append({
        "timestamp": datetime.now().isoformat(),
        "level": "INFO",
        "source": "backend",
        "message": "Backend server started - ready to receive real data from service"
    })
    
    logger.info("Backend initialized - ready to receive real data from service")

def update_system_metrics():
    """Update system metrics - now comes from orchestrator via /api/metrics"""
    # System metrics are now sent by the demo orchestrator
    # This function is kept for compatibility but no longer uses psutil
    while True:
        try:
            # System stats are updated via /api/metrics POST requests from orchestrator
            # We'll just update the timestamp to show the system is alive
            system_stats["timestamp"] = time.time()
        except Exception as e:
            logger.error(f"Error updating system metrics: {e}")
        
        time.sleep(2)

# API Routes
@app.route('/api/heartbeat', methods=['POST'])
def heartbeat():
    """Receive heartbeat from Windows Service"""
    try:
        data = request.get_json()
        service_id = data.get('service_id', 'SecureRemoteAgent')
        
        if service_id not in agents:
            agents[service_id] = {
                "id": service_id,
                "status": "active",
                "last_heartbeat": time.time(),
                "start_time": time.time()
            }
            logs.append({
                "timestamp": datetime.now().isoformat(),
                "level": "INFO",
                "source": "backend",
                "message": f"Windows Service registered: {service_id}"
            })
        else:
            agents[service_id]["last_heartbeat"] = time.time()
            agents[service_id]["status"] = "active"
        
        return jsonify({"status": "success", "timestamp": time.time()})
    
    except Exception as e:
        logger.error(f"Heartbeat error: {e}")
        return jsonify({"status": "error", "message": str(e)}), 500

@app.route('/api/metrics', methods=['POST'])
def receive_metrics():
    """Receive metrics from Windows Service"""
    try:
        data = request.get_json()
        service_id = data.get('service_id', 'SecureRemoteAgent')
        
        if service_id in agents:
            # Update service with received real metrics
            agents[service_id]["cpu_usage"] = data.get('cpu_usage', 0.0)
            agents[service_id]["memory_usage"] = data.get('memory_usage', 0.0)
            agents[service_id]["uptime"] = data.get('uptime', 0.0)
            agents[service_id]["requests_handled"] = data.get('requests_handled', 0)
            agents[service_id]["timestamp"] = time.time()
        else:
            logger.warning(f"Received metrics from unregistered service: {service_id}")
        
        return jsonify({"status": "success"})
    
    except Exception as e:
        logger.error(f"Metrics error: {e}")
        return jsonify({"status": "error", "message": str(e)}), 500

@app.route('/api/plugins', methods=['GET', 'POST'])
def plugin_status():
    """Get or update plugin status"""
    if request.method == 'GET':
        return jsonify(plugins)
    else:
        try:
            data = request.get_json()
            
            # Handle multiple plugins sent as a dictionary
            if isinstance(data, dict):
                for plugin_name, plugin_data in data.items():
                    if plugin_name not in plugins:
                        plugins[plugin_name] = {}
                    
                    # Update plugin data
                    for key, value in plugin_data.items():
                        plugins[plugin_name][key] = value
                    
                    plugins[plugin_name]['last_activity'] = time.time()
                
                logger.info(f"Updated plugin status for {len(data)} plugins")
            else:
                # Handle single plugin (legacy format)
                plugin_name = data.get('plugin_name', 'unknown')
                plugin_status = data.get('status', 'unknown')
                
                if plugin_name not in plugins:
                    plugins[plugin_name] = {}
                
                plugins[plugin_name]['status'] = plugin_status
                plugins[plugin_name]['last_activity'] = time.time()
            
            return jsonify({"status": "success"})
        
        except Exception as e:
            logger.error(f"Plugin status error: {e}")
            return jsonify({"status": "error", "message": str(e)}), 500

@app.route('/api/logs', methods=['GET', 'POST'])
def log_management():
    """Get or add log entries"""
    if request.method == 'GET':
        limit = request.args.get('limit', 100, type=int)
        return jsonify(logs[-limit:])
    else:
        try:
            data = request.get_json()
            log_entry = {
                "timestamp": datetime.now().isoformat(),
                "level": data.get('level', 'INFO'),
                "source": data.get('source', 'agent'),
                "message": data.get('message', '')
            }
            
            logs.append(log_entry)
            
            # Keep only last 1000 log entries
            if len(logs) > 1000:
                logs.pop(0)
            
            return jsonify({"status": "success"})
        
        except Exception as e:
            logger.error(f"Log management error: {e}")
            return jsonify({"status": "error", "message": str(e)}), 500

@app.route('/api/scrape', methods=['POST'])
def scrape_endpoint():
    """Scrape URL using scraper plugin"""
    try:
        data = request.get_json()
        url = data.get('url', '')
        
        if not url:
            return jsonify({"status": "error", "message": "URL is required"}), 400
        
        # For now, perform simple scraping using requests
        import requests
        from bs4 import BeautifulSoup
        
        response = requests.get(url, timeout=10)
        soup = BeautifulSoup(response.text, 'html.parser')
        
        # Extract title and some basic information
        title = soup.title.string if soup.title else "No title"
        links = len(soup.find_all('a'))
        paragraphs = len(soup.find_all('p'))
        
        results = [
            f"Title: {title}",
            f"Links found: {links}",
            f"Paragraphs: {paragraphs}",
            f"Status code: {response.status_code}"
        ]
        
        # Update scraper plugin metrics
        if 'ScraperPlugin' in plugins:
            plugins['ScraperPlugin']['pages_scraped'] = plugins['ScraperPlugin'].get('pages_scraped', 0) + 1
            plugins['ScraperPlugin']['last_activity'] = time.time()
        
        logs.append({
            "timestamp": datetime.now().isoformat(),
            "level": "INFO",
            "source": "backend",
            "message": f"Scraped URL: {url}"
        })
        
        return jsonify({"status": "success", "results": results})
    
    except Exception as e:
        logger.error(f"Scrape error: {e}")
        return jsonify({"status": "error", "message": str(e)}), 500

@app.route('/api/agents', methods=['GET'])
def get_agents():
    """Get all connected services status"""
    return jsonify(agents)

@app.route('/api/system', methods=['GET'])
def get_system():
    """Get system metrics"""
    return jsonify(system_stats)

@app.route('/dashboard')
def dashboard():
    """Web dashboard interface"""
    return render_template('dashboard.html')

@app.route('/')
def index():
    """Main page - redirect to dashboard"""
    return redirect('/dashboard')

# HTML Templates
@app.template_filter('datetime')
def datetime_filter(timestamp):
    """Format timestamp for display"""
    if isinstance(timestamp, (int, float)):
        return datetime.fromtimestamp(timestamp).strftime('%Y-%m-%d %H:%M:%S')
    return timestamp

@app.template_filter('bytes')
def bytes_filter(value):
    """Format bytes for display"""
    for unit in ['B', 'KB', 'MB', 'GB']:
        if value < 1024:
            return f"{value:.1f} {unit}"
        value /= 1024
    return f"{value:.1f} TB"

def main():
    """Main function to start the backend server"""
    logger.info("Starting Secure Remote Agent Backend Server")
    
    # Initialize backend
    initialize_backend()
    
    # Start metrics update thread (local system metrics only)
    metrics_thread = threading.Thread(target=update_system_metrics, daemon=True)
    metrics_thread.start()
    
    # Start Flask server
    app.run(host='0.0.0.0', port=5000, debug=True)

if __name__ == '__main__':
    main()
