#ifndef NETWORK_HPP
#define NETWORK_HPP
#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOGDI
#define NOUSER
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <array>
#include <cstddef>
#include <cstdint>

#pragma comment(lib, "ws2_32.lib")

#define NET_PORT "27015"
#define NET_BUFSIZE 64

enum class Turn
{
    NONE,
    White,
    Black
};

enum class NetworkRole
{
    None,
    Host,
    Client
};

// Wire-safe fixed-size payload (NOT pointers)
struct NetMove
{
    char *fromSquare; // "A2" + '\0'
    char *toSquare;   // "A4" + '\0'
};

class Network
{
private:
    SOCKET listenSock = INVALID_SOCKET;
    SOCKET connSock   = INVALID_SOCKET;
    NetworkRole role  = NetworkRole::None;
    bool connected    = false;

    // Partial receive accumulators (non-blocking sockets)
    std::array<std::uint8_t, sizeof(NetMove)> moveRxBuf{};
    std::size_t moveRxBytes = 0;

    std::array<std::uint8_t, 1> turnRxBuf{};
    std::size_t turnRxBytes = 0;

public:
    Network() = default;
    ~Network();

    bool Init();
    bool Host();
    bool Connect(const std::string &ip);

    bool SendMove(const NetMove &move);
    bool SendTurn(Turn turn);

    bool RecvMove(NetMove &outMove);
    bool RecvTurn(Turn &outTurn);

    void Disconnect();

    bool IsConnected() const { return connected; }
    NetworkRole GetRole() const { return role; }
};

#endif // NETWORK_HPP
