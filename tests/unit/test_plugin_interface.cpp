// tests/unit/test_plugin_interface.cpp
#include <catch2/catch.hpp>
#include "../../src/plugins/loader/plugin_interface.h"
#include <QCoreApplication>
#include <QPluginLoader>
#include <QDir>

// Mock plugin for testing
class MockPlugin : public QObject, public IPlugin {
    Q_OBJECT

public:
    MockPlugin() : m_status(PluginStatus::Unloaded) {}

    QString name() const override { return "MockPlugin"; }
    QString version() const override { return "1.0.0"; }
    QString description() const override { return "Mock plugin for testing"; }
    
    bool initialize(const QVariantMap& config = QVariantMap()) override {
        m_config = config;
        m_status = PluginStatus::Loaded;
        return true;
    }
    
    void start() override {
        if (m_status == PluginStatus::Loaded) {
            m_status = PluginStatus::Running;
        }
    }
    
    void stop() override {
        if (m_status == PluginStatus::Running) {
            m_status = PluginStatus::Loaded;
        }
    }
    
    void cleanup() override {
        m_status = PluginStatus::Unloaded;
        m_config.clear();
    }
    
    void execute() override {
        // Mock execution
    }
    
    QVariantMap executeWithParams(const QVariantMap& params) override {
        QVariantMap result;
        result["status"] = "success";
        result["params"] = params;
        return result;
    }
    
    PluginStatus status() const override { return m_status; }
    QVariantMap getMetrics() const override {
        QVariantMap metrics;
        metrics["executions"] = 0;
        metrics["uptime"] = 0;
        return metrics;
    }
    
    QString getLastError() const override { return m_lastError; }
    
    bool setConfig(const QVariantMap& config) override {
        m_config = config;
        return true;
    }
    
    QVariantMap getConfig() const override { return m_config; }
    
    void setCallback(std::function<void(const QString&, const QVariantMap&)> callback) override {
        m_callback = callback;
    }

private:
    PluginStatus m_status;
    QVariantMap m_config;
    QString m_lastError;
    std::function<void(const QString&, const QVariantMap&)> m_callback;
};

TEST_CASE("Plugin Lifecycle", "[plugin]") {
    int argc = 0;
    char** argv = nullptr;
    QCoreApplication app(argc, argv);
    MockPlugin mockPlugin;

    // Test initial state
    REQUIRE(mockPlugin.status() == PluginStatus::Unloaded);
    REQUIRE(mockPlugin.name() == "MockPlugin");
    REQUIRE(mockPlugin.version() == "1.0.0");
    
    // Test initialization
    QVariantMap config;
    config["test_param"] = "test_value";
    
    bool initResult = mockPlugin.initialize(config);
    REQUIRE(initResult);
    REQUIRE(mockPlugin.status() == PluginStatus::Loaded);
    REQUIRE(mockPlugin.getConfig()["test_param"].toString() == "test_value");
    
    // Test start
    mockPlugin.start();
    REQUIRE(mockPlugin.status() == PluginStatus::Running);
    
    // Test stop
    mockPlugin.stop();
    REQUIRE(mockPlugin.status() == PluginStatus::Loaded);
    
    // Test cleanup
    mockPlugin.cleanup();
    REQUIRE(mockPlugin.status() == PluginStatus::Unloaded);
    REQUIRE(mockPlugin.getConfig().isEmpty());
}

TEST_CASE("Plugin Execution", "[plugin]") {
    int argc = 0;
    char** argv = nullptr;
    QCoreApplication app(argc, argv);
    MockPlugin mockPlugin;

    // Initialize plugin
    mockPlugin.initialize();
    mockPlugin.start();
    
    // Test execute
    mockPlugin.execute();
    
    // Test execute with params
    QVariantMap params;
    params["action"] = "test";
    params["value"] = 42;
    
    QVariantMap result = mockPlugin.executeWithParams(params);
    
    REQUIRE(result.value("status").toString() == "success");
    REQUIRE(result.value("params").toMap().value("action").toString() == "test");
    REQUIRE(result.value("params").toMap().value("value").toInt() == 42);
}

TEST_CASE("Plugin Metrics", "[plugin]") {
    int argc = 0;
    char** argv = nullptr;
    QCoreApplication app(argc, argv);
    MockPlugin mockPlugin;

    // Test metrics
    QVariantMap metrics = mockPlugin.getMetrics();
    
    REQUIRE(metrics.contains("executions"));
    REQUIRE(metrics.contains("uptime"));
    REQUIRE(metrics.value("executions").toInt() == 0);
    REQUIRE(metrics.value("uptime").toInt() == 0);
}

TEST_CASE("Plugin Configuration", "[plugin]") {
    int argc = 0;
    char** argv = nullptr;
    QCoreApplication app(argc, argv);
    MockPlugin mockPlugin;

    // Test configuration
    QVariantMap config;
    config["setting1"] = "value1";
    config["setting2"] = 123;
    
    bool setResult = mockPlugin.setConfig(config);
    REQUIRE(setResult);
    
    QVariantMap retrievedConfig = mockPlugin.getConfig();
    REQUIRE(retrievedConfig.value("setting1").toString() == "value1");
    REQUIRE(retrievedConfig.value("setting2").toInt() == 123);
}

TEST_CASE("Plugin Error Handling", "[plugin]") {
    int argc = 0;
    char** argv = nullptr;
    QCoreApplication app(argc, argv);
    MockPlugin mockPlugin;

    // Test error handling
    QString error = mockPlugin.getLastError();
    REQUIRE(error.isEmpty() || error.isNull());
}

#include "test_plugin_interface.moc"
