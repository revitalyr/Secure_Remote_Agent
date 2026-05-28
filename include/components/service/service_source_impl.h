#pragma once

#include "rep_ServiceInterface_merged.h"

class ServiceSourceImpl : public ServiceInterfaceSimpleSource
{
    Q_OBJECT
public:
    using ServiceInterfaceSimpleSource::ServiceInterfaceSimpleSource;

    void ping() override { emit pongReceived("pong"); }
    void requestShutdown() override { emit logMessage("INFO", "Shutdown requested via IPC"); }
};
