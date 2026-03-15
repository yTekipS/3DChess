#ifndef NETWORK_HPP
#define NETWORK_HPP
#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOGDI   // prevents Rectangle() conflict with raylib
#define NOUSER  // prevents CloseWindow() and ShowCursor() conflict with raylib
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>

#pragma comment(lib, "ws2_32.lib")

#define NET_PORT "27015"
#define NET_BUFSIZE 64

enum class NetworkRole
{
    None,
    Host,
    Client
};

// Represents a chess move sent over the network
struct NetMove
{
    const char *fromSquare; // e.g. "A2"
    const char *toSquare;   // e.g. "A4"
};

class Network
{
private:
    SOCKET listenSock = INVALID_SOCKET;
    SOCKET connSock   = INVALID_SOCKET;
    NetworkRole role  = NetworkRole::None;
    bool connected    = false;

public:
    Network() = default;
    ~Network();

    // Call once before using the network
    bool Init();

    // Start hosting – waits (blocks) for one client to connect
    bool Host();

    // Connect to a host at the given IP address
    bool Connect(const std::string &ip);

    // Send a move to the remote player. Returns false on error.
    bool SendMove(const NetMove &move);

    // Receive a move (non-blocking). Returns true if a move was received.
    bool RecvMove(NetMove &outMove);

    // Gracefully shut down the connection
    void Disconnect();

    bool IsConnected() const { return connected; }
    NetworkRole GetRole() const { return role; }
};

#endif // NETWORK_HPP
