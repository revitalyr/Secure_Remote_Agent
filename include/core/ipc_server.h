/**
 * @file ipc_server.h
 * @brief IPC server for inter-process communication
 */

#pragma once

/**
 * @brief Starts the IPC server for inter-process communication
 *
 * Initializes and starts a local IPC server that listens for incoming
 * connections from other processes. Uses Qt's QLocalServer for
 * Windows named pipes functionality.
 */
void startIpcServer();

/**
 * @brief Stops the IPC server
 *
 * Gracefully shuts down the IPC server and releases all resources.
 * All active connections are terminated before shutdown.
 */
void stopIpcServer();
