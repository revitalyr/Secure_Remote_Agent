// app/main.qml
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import SecureAgent 1.0

ApplicationWindow {
    id: window
    visible: true
    width: 1200
    height: 800
    title: "Secure Remote Agent - Control Panel"

    // uiManager is available as a context property set in main.cpp

    // Main layout
    RowLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        // Left panel - Service Control
        Rectangle {
            Layout.preferredWidth: 300
            Layout.fillHeight: true
            color: "#f0f0f0"
            border.color: "#ccc"
            border.width: 1
            radius: 5

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 15

                // Service Status
                GroupBox {
                    title: "Service Status"
                    Layout.fillWidth: true

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 10

                        Rectangle {
                            Layout.fillWidth: true
                            height: 40
                            color: uiManager && uiManager.serviceStatus === "Running" ? "#4CAF50" : 
                                  uiManager && uiManager.serviceStatus === "Stopped" ? "#F44336" : "#FF9800"
                            radius: 5

                            Text {
                                anchors.centerIn: parent
                                text: uiManager ? uiManager.serviceStatus : "Unknown"
                                color: "white"
                                font.bold: true
                                font.pixelSize: 14
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true

                            Button {
                                text: "Start"
                                Layout.fillWidth: true
                                enabled: uiManager && uiManager.serviceStatus !== "Running"
                                onClicked: uiManager.startService()
                            }

                            Button {
                                text: "Stop"
                                Layout.fillWidth: true
                                enabled: uiManager && uiManager.serviceStatus === "Running"
                                onClicked: uiManager.stopService()
                            }
                        }

                        Button {
                            text: "Restart"
                            Layout.fillWidth: true
                            onClicked: uiManager.restartService()
                        }

                        Button {
                            text: "Ping"
                            Layout.fillWidth: true
                            onClicked: uiManager.pingService()
                        }
                    }
                }

                // System Metrics
                GroupBox {
                    title: "System Metrics"
                    Layout.fillWidth: true

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 10

                        Text {
                            text: "CPU Usage: " + (uiManager ? uiManager.cpuUsage.toFixed(1) : "0.0") + "%"
                        }

                        ProgressBar {
                            Layout.fillWidth: true
                            value: uiManager ? uiManager.cpuUsage : 0
                            to: 100
                        }

                        Text {
                            text: "Memory Usage: " + (uiManager ? uiManager.memoryUsage.toFixed(1) : "0.0") + "%"
                        }

                        ProgressBar {
                            Layout.fillWidth: true
                            value: uiManager ? uiManager.memoryUsage : 0
                            to: 100
                        }

                        Text {
                            text: "Memory: " + (uiManager ? uiManager.memoryAvailable.toFixed(0) : "0") + " MB / " + (uiManager ? uiManager.memoryTotal.toFixed(0) : "0") + " MB"
                        }

                        Text {
                            text: "Uptime: " + formatUptime(uiManager ? uiManager.uptime : 0)
                        }
                    }
                }

                // Connected Agents
                GroupBox {
                    title: "Connected Agents"
                    Layout.fillWidth: true

                    ScrollView {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 150

                        Column {
                            width: parent.width
                            spacing: 5

                            Repeater {
                                model: uiManager ? uiManager.connectedAgents : []

                                Rectangle {
                                    width: parent.width
                                    height: 30
                                    color: "#e0e0e0"
                                    border.color: "#ccc"
                                    border.width: 1
                                    radius: 3

                                    Row {
                                        anchors.left: parent.left
                                        anchors.verticalCenter: parent.verticalCenter
                                        anchors.margins: 5
                                        spacing: 10

                                        Text {
                                            text: modelData
                                            font.pixelSize: 12
                                        }

                                        Button {
                                            text: "Stop"
                                            implicitWidth: 40
                                            implicitHeight: 20
                                            font.pixelSize: 10
                                            onClicked: uiManager.stopAgent(modelData)
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        // Right panel - Logs and Plugins
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#f8f8f8"
            border.color: "#ccc"
            border.width: 1
            radius: 5

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 15

                // Log Stream
                GroupBox {
                    title: "Log Stream"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 400

                    ScrollView {
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        ListView {
                            id: logView
                            model: uiManager ? uiManager.logEntries : []
                            delegate: Rectangle {
                                width: logView.width
                                height: 20
                                color: index % 2 === 0 ? "#ffffff" : "#f0f0f0"

                                Text {
                                    anchors.left: parent.left
                                    anchors.verticalCenter: parent.verticalCenter
                                    anchors.margins: 5
                                    text: modelData
                                    font.family: "Consolas"
                                    font.pixelSize: 10
                                    color: getLogLevelColor(modelData)
                                }

                                function getLogLevelColor(logEntry) {
                                    if (logEntry.includes("ERROR")) return "#F44336"
                                    if (logEntry.includes("WARNING")) return "#FF9800"
                                    if (logEntry.includes("INFO")) return "#2196F3"
                                    return "#000000"
                                }
                            }
                        }
                    }
                }

                // Plugin Management
                GroupBox {
                    title: "Plugin Management"
                    Layout.fillWidth: true

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 10

                        RowLayout {
                            Layout.fillWidth: true

                            TextField {
                                id: pluginPathField
                                Layout.fillWidth: true
                                placeholderText: "Plugin path (e.g., plugins/proxy.dll)"
                            }

                            Button {
                                text: "Load"
                                onClicked: {
                                    if (pluginPathField.text.length > 0) {
                                        uiManager.loadPlugin(pluginPathField.text)
                                        pluginPathField.text = ""
                                    }
                                }
                            }
                        }

                        ScrollView {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 150

                            ListView {
                                id: pluginView
                                model: ["ProxyPlugin", "ScraperPlugin"]
                                delegate: Rectangle {
                                    width: pluginView.width
                                    height: 30
                                    color: "#e0e0e0"
                                    border.color: "#ccc"
                                    border.width: 1
                                    radius: 3

                                    Row {
                                        anchors.left: parent.left
                                        anchors.verticalCenter: parent.verticalCenter
                                        anchors.margins: 5
                                        spacing: 10

                                        Text {
                                            text: modelData
                                            font.pixelSize: 12
                                        }

                                        Text {
                                            text: "Loaded"
                                            color: "#4CAF50"
                                            font.pixelSize: 10
                                        }

                                        Button {
                                            text: "Unload"
                                            implicitWidth: 50
                                            implicitHeight: 20
                                            font.pixelSize: 10
                                            onClicked: uiManager.unloadPlugin(modelData)
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // Status bar
    Rectangle {
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: 30
        color: "#333"

        Row {
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            anchors.margins: 10
            spacing: 20

            Text {
                text: "Status: " + (uiManager ? uiManager.serviceStatus : "Unknown")
                color: "white"
                font.pixelSize: 12
            }

            Text {
                text: "CPU: " + (uiManager ? uiManager.cpuUsage.toFixed(1) : "0.0") + "%"
                color: "white"
                font.pixelSize: 12
            }

            Text {
                text: "RAM: " + (uiManager ? uiManager.memoryUsage.toFixed(1) : "0.0") + "%"
                color: "white"
                font.pixelSize: 12
            }

            Text {
                text: new Date().toLocaleTimeString()
                color: "white"
                font.pixelSize: 12
            }
        }
    }

    function formatUptime(seconds) {
        if (seconds < 60) return seconds + "s"
        if (seconds < 3600) return Math.floor(seconds / 60) + "m " + (seconds % 60) + "s"
        return Math.floor(seconds / 3600) + "h " + Math.floor((seconds % 3600) / 60) + "m"
    }
}
