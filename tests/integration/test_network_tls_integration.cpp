#include <catch2/catch_all.hpp>
#include <QCoreApplication>
#include <QNetworkAccessManager>
#include <QSslCertificate>
#include <QSslKey>
#include <QException>
#include "core/network_client.h"

TEST_CASE("NetworkClient TLS Configuration", "[network_tls]") {
    // Initialize QCoreApplication
    int argc = 1;
    char* argv[] = { (char*)"integration_tests" };
    QCoreApplication app(argc, argv);

    QNetworkAccessManager manager;
    NetworkClient client(&manager);

    // Mock certificate data
    QSslCertificate cert(QByteArrayLiteral(R"(
        -----BEGIN CERTIFICATE-----
        MIIDazCCAl+gAwIBAgIJAKwXx9YWxXQMA0GCSqGSIb3DQEBBQUAMB4XDTA5MDkx
        MzIwMzIwMFowgZdMQswCQYDVQQGEwJVUzELMAkGA1UEBwwCQ29tZSBJbnRlcm5ldCBD
        QSBMaWJyYXJ5MB4XDTA5MDkxMzIwMzIwMFowgZdMQswCQYDVQQGEwJVUzELMAkGA1UE
        BwwCQ29tZSBJbnRlcm5ldCBDQSBMaWJyYXJ5MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA
        uQ5XJQZJQZJQZJQZJQZJQZJQZJQZJQZJQZJQZJQZJQZJQZJQZJQZJQZJQZJQZJQZJQZJ
        QZJQZJQZJQZJQZJQZJQZJQZJQZJQZJQZJQZJQZJQZJQZJQZJQZJQZJQZJQZJQZJQZJ
        -----END CERTIFICATE-----
        )")
    );

    QSslKey key(QByteArray(), QSsl::Rsa);

    // Test TLS configuration using exception handling
    bool success = true;
    try {
        client.setClientCertificate(cert, key);
        client.setCaCertificate(cert);
        client.setCertificateVerification(true);
    } catch (...) {
        success = false;
    }
    REQUIRE(success);
    app.exit();
}
