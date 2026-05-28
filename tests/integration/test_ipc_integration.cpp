#include <catch2/catch_all.hpp>
#include <catch2/catch_all.hpp>
#include <QCoreApplication>
#include "core/ipc_client.h"

TEST_CASE("IpcClient Basic Initialization", "[ipc]") {
    int argc = 0;
    char* argv[] = { nullptr };
    QCoreApplication app(argc, argv);

    IpcClient client;
    REQUIRE(client.isConnected() == false);
    app.exit();
}
