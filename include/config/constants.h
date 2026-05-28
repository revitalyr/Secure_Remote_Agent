#pragma once

#include <QString>
#include <QByteArray>

namespace Constants {
    // Service Configuration
    constexpr int DEFAULT_LISTEN_PORT = 8080;
    constexpr int DEFAULT_TARGET_PORT = 5000;
    constexpr int DEFAULT_SCRAPE_INTERVAL = 300; // 5 minutes
    constexpr int DEFAULT_MAX_RETRIES = 3;
    constexpr int DEFAULT_INITIAL_DELAY = 1000; // 1 second
    constexpr int DEFAULT_MAX_DELAY = 30000; // 30 seconds
    constexpr int DEFAULT_METRICS_INTERVAL = 1000; // 1 second
    constexpr int DEFAULT_PLUGIN_STATUS_INTERVAL = 10000; // 10 seconds
    constexpr int DEFAULT_HEARTBEAT_INTERVAL = 30000; // 30 seconds
    constexpr int DEFAULT_WATCHDOG_INTERVAL = 15000; // 15 seconds

    // Service Names
    inline const QString SERVICE_NAME = QStringLiteral("SecureRemoteAgent");
    inline const QString PROXY_PLUGIN_NAME = QStringLiteral("ProxyPlugin");
    inline const QString SCRAPER_PLUGIN_NAME = QStringLiteral("ScraperPlugin");

    // Default URLs
    inline const QString DEFAULT_BACKEND_URL = QStringLiteral("http://localhost:5000");
    inline const QString DEFAULT_REGISTRATION_ENDPOINT = QStringLiteral("/api/register");
    inline const QString DEFAULT_HEARTBEAT_ENDPOINT = QStringLiteral("/api/heartbeat");
    inline const QString DEFAULT_METRICS_ENDPOINT = QStringLiteral("/api/metrics");
    inline const QString DEFAULT_PLUGINS_ENDPOINT = QStringLiteral("/api/plugins");

    // Default User Agent
    inline const QString DEFAULT_USER_AGENT = QStringLiteral("SecureAgent/1.0.0");

    // Default Configuration
    inline const QByteArray DEFAULT_TEST_CERTIFICATE = QByteArrayLiteral(R"(
        -----BEGIN CERTIFICATE-----
        MIIDazCCAl+gAwIBAgIJAKwXx9YWxXQMA0GCSqGSIb3DQEBBQUAMB4XDTA5MDkx
        MzIwMzIwMFoXDTI5MDkxMzIwMzIwMFowgZdMQswCQYDVQQGEwJVUzELMAkGA1UE
        BwwCQ29tZSBJbnRlcm5ldCBDQSBMaWJyYXJ5MB4XDTA5MDkxMzIwMzIwMFoXDTI5
        MDkxMzIwMzIwMFowgZdMQswCQYDVQQGEwJVUzELMAkGA1UEBwwCQ29tZSBJbnRl
        cm5ldCBDQSBMaWJyYXJ5MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA
        uQ5XJQZJQZJQZJQZJQZJQZJQZJQZJQZJQZJQZJQZJQZJQZJQZJQZJQZJQZJQZJQZJQZJ
        QZJQZJQZJQZJQZJQZJQZJQZJQZJQZJQZJQZJQZJQZJQZJQZJQZJQZJQZJQZJQZJQZJ
        -----END CERTIFICATE-----
        )");
}