#include "../Headers/Network.hpp"
#include <cstring>
#include <cstdio>

namespace
{
    bool SendAllNonBlocking(SOCKET sock, const char* data, int len)
    {
        int sent = 0;
        while (sent < len)
        {
            int r = send(sock, data + sent, len - sent, 0);
            if (r > 0)
            {
                sent += r;
                continue;
            }
            if (r == 0) return false;

            int err = WSAGetLastError();
            if (err == WSAEWOULDBLOCK)
            {
                fd_set writeSet;
                FD_ZERO(&writeSet);
                FD_SET(sock, &writeSet);

                timeval tv{};
                tv.tv_sec = 0;
                tv.tv_usec = 200000; // 200ms

                int sel = select(0, nullptr, &writeSet, nullptr, &tv);
                if (sel <= 0) return false;
                continue;
            }
            return false;
        }
        return true;
    }
}   

bool Network::Init()
{
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0)
    {
        printf("[Network] WSAStartup failed: %d\n", result);
        return false;
    }
    return true;
}

bool Network::Host()
{
    if (connected) return true;

    role = NetworkRole::Host;

    // First call: create listening socket. Next calls: just poll accept().
    if (listenSock == INVALID_SOCKET)
    {
        struct addrinfo hints = {}, *res = nullptr;
        hints.ai_family   = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        hints.ai_flags    = AI_PASSIVE;

        if (getaddrinfo(nullptr, NET_PORT, &hints, &res) != 0)
        {
            printf("[Network] getaddrinfo failed\n");
            return false;
        }

        listenSock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (listenSock == INVALID_SOCKET)
        {
            printf("[Network] socket() failed: %d\n", WSAGetLastError());
            freeaddrinfo(res);
            return false;
        }

        if (bind(listenSock, res->ai_addr, (int)res->ai_addrlen) == SOCKET_ERROR)
        {
            printf("[Network] bind() failed: %d\n", WSAGetLastError());
            freeaddrinfo(res);
            closesocket(listenSock);
            listenSock = INVALID_SOCKET;
            return false;
        }
        freeaddrinfo(res);

        if (listen(listenSock, 1) == SOCKET_ERROR)
        {
            printf("[Network] listen() failed: %d\n", WSAGetLastError());
            closesocket(listenSock);
            listenSock = INVALID_SOCKET;
            return false;
        }

        // Make accept non-blocking so game loop never stalls.
        u_long listenMode = 1;
        ioctlsocket(listenSock, FIONBIO, &listenMode);

        printf("[Network] Hosting on port %s. Waiting for opponent...\n", NET_PORT);
    }

    connSock = accept(listenSock, nullptr, nullptr);
    if (connSock == INVALID_SOCKET)
    {
        int err = WSAGetLastError();
        if (err == WSAEWOULDBLOCK)
            return false; // no incoming client yet

        printf("[Network] accept() failed: %d\n", err);
        closesocket(listenSock);
        listenSock = INVALID_SOCKET;
        return false;
    }

    // Keep connected socket non-blocking so RecvMove doesn't stall.
    u_long connMode = 1;
    ioctlsocket(connSock, FIONBIO, &connMode);

    connected = true;
    printf("[Network] Opponent connected!\n");
    return true;
}

bool Network::Connect(const std::string &ip)
{
    role = NetworkRole::Client;

    if (ip.empty())
    {
        printf("[Network] Empty IP address.\n");
        return false;
    }

    struct addrinfo hints = {}, *res = nullptr;
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    if (getaddrinfo(ip.c_str(), NET_PORT, &hints, &res) != 0)
    {
        printf("[Network] getaddrinfo failed for host '%s'\n", ip.c_str());
        return false;
    }

    connSock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (connSock == INVALID_SOCKET)
    {
        printf("[Network] socket() failed: %d\n", WSAGetLastError());
        freeaddrinfo(res);
        return false;
    }

    // Set socket to non-blocking before connect so UI loop doesn't freeze
    u_long mode = 1;
    ioctlsocket(connSock, FIONBIO, &mode);

    int connectResult = connect(connSock, res->ai_addr, (int)res->ai_addrlen);
    if (connectResult == SOCKET_ERROR)
    {
        int err = WSAGetLastError();
        if (err != WSAEWOULDBLOCK && err != WSAEINPROGRESS && err != WSAEALREADY)
        {
            printf("[Network] connect() failed immediately: %d\n", err);
            freeaddrinfo(res);
            closesocket(connSock);
            connSock = INVALID_SOCKET;
            return false;
        }

        fd_set writeSet;
        fd_set errorSet;
        FD_ZERO(&writeSet);
        FD_ZERO(&errorSet);
        FD_SET(connSock, &writeSet);
        FD_SET(connSock, &errorSet);

        timeval timeout;
        timeout.tv_sec = 3;
        timeout.tv_usec = 0;

        int sel = select(0, nullptr, &writeSet, &errorSet, &timeout);
        if (sel == 0)
        {
            printf("[Network] connect() timed out to %s\n", ip.c_str());
            freeaddrinfo(res);
            closesocket(connSock);
            connSock = INVALID_SOCKET;
            return false;
        }
        if (sel == SOCKET_ERROR)
        {
            printf("[Network] select() failed during connect: %d\n", WSAGetLastError());
            freeaddrinfo(res);
            closesocket(connSock);
            connSock = INVALID_SOCKET;
            return false;
        }

        int soError = 0;
        int soLen = sizeof(soError);
        if (getsockopt(connSock, SOL_SOCKET, SO_ERROR, reinterpret_cast<char *>(&soError), &soLen) == SOCKET_ERROR || soError != 0)
        {
            printf("[Network] connect() failed after wait: %d\n", soError);
            freeaddrinfo(res);
            closesocket(connSock);
            connSock = INVALID_SOCKET;
            return false;
        }
    }
    freeaddrinfo(res);

    connected = true;
    printf("[Network] Connected to host at %s\n", ip.c_str());
    return true;
}

bool Network::SendMove(const NetMove &move)
{
    if (!connected) return false;

    const char* bytes = reinterpret_cast<const char*>(&move);
    if (!SendAllNonBlocking(connSock, bytes, static_cast<int>(sizeof(NetMove))))
    {
        printf("[Network] send(move) failed: %d\n", WSAGetLastError());
        connected = false;
        return false;
    }
    return true;
}

bool Network::SendTurn(Turn turn)
{
    if (!connected) return false;

    std::uint8_t wireTurn = static_cast<std::uint8_t>(turn);
    const char* bytes = reinterpret_cast<const char*>(&wireTurn);

    if (!SendAllNonBlocking(connSock, bytes, 1))
    {
        printf("[Network] send(turn) failed: %d\n", WSAGetLastError());
        connected = false;
        return false;
    }
    return true;
}

bool Network::RecvMove(NetMove &outMove)
{
    if (!connected) return false;

    while (moveRxBytes < sizeof(NetMove))
    {
        int r = recv(
            connSock,
            reinterpret_cast<char*>(moveRxBuf.data() + moveRxBytes),
            static_cast<int>(sizeof(NetMove) - moveRxBytes),
            0
        );

        if (r > 0)
        {
            moveRxBytes += static_cast<std::size_t>(r);
            continue;
        }

        if (r == 0)
        {
            printf("[Network] Connection closed by remote.\n");
            connected = false;
            return false;
        }

        int err = WSAGetLastError();
        if (err == WSAEWOULDBLOCK) return false;

        printf("[Network] recv(move) failed: %d\n", err);
        connected = false;
        return false;
    }

    std::memcpy(&outMove, moveRxBuf.data(), sizeof(NetMove));
    moveRxBytes = 0;
    return true;
}

bool Network::RecvTurn(Turn &outTurn)
{
    if (!connected) return false;

    while (turnRxBytes < 1)
    {
        int r = recv(
            connSock,
            reinterpret_cast<char*>(turnRxBuf.data() + turnRxBytes),
            static_cast<int>(1 - turnRxBytes),
            0
        );

        if (r > 0)
        {
            turnRxBytes += static_cast<std::size_t>(r);
            continue;
        }

        if (r == 0)
        {
            printf("[Network] Connection closed by remote.\n");
            connected = false;
            return false;
        }

        int err = WSAGetLastError();
        if (err == WSAEWOULDBLOCK) return false;

        printf("[Network] recv(turn) failed: %d\n", err);
        connected = false;
        return false;
    }

    const std::uint8_t wire = turnRxBuf[0];
    turnRxBytes = 0;

    if (wire > static_cast<std::uint8_t>(Turn::Black))
    {
        printf("[Network] Invalid turn value received: %u\n", static_cast<unsigned>(wire));
        return false;
    }

    outTurn = static_cast<Turn>(wire);
    return true;
}

void Network::Disconnect()
{
    if (connSock != INVALID_SOCKET)
    {
        shutdown(connSock, SD_BOTH);
        closesocket(connSock);
        connSock = INVALID_SOCKET;
    }
    if (listenSock != INVALID_SOCKET)
    {
        closesocket(listenSock);
        listenSock = INVALID_SOCKET;
    }
    connected = false;
    moveRxBytes = 0;
    turnRxBytes = 0;
    WSACleanup();
}

Network::~Network()
{
    Disconnect();
}
