#include "../Headers/Network.hpp"
#include <cstring>
#include <cstdio>

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
    role = NetworkRole::Host;

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
        return false;
    }
    freeaddrinfo(res);

    if (listen(listenSock, 1) == SOCKET_ERROR)
    {
        printf("[Network] listen() failed: %d\n", WSAGetLastError());
        closesocket(listenSock);
        return false;
    }

    printf("[Network] Waiting for opponent on port %s ...\n", NET_PORT);
    connSock = accept(listenSock, nullptr, nullptr);
    if (connSock == INVALID_SOCKET)
    {
        printf("[Network] accept() failed: %d\n", WSAGetLastError());
        closesocket(listenSock);
        return false;
    }

    // Set socket to non-blocking so RecvMove doesn't stall the game loop
    u_long mode = 1;
    ioctlsocket(connSock, FIONBIO, &mode);

    connected = true;
    printf("[Network] Opponent connected!\n");
    return true;
}

bool Network::Connect(const std::string &ip)
{
    role = NetworkRole::Client;

    struct addrinfo hints = {}, *res = nullptr;
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    if (getaddrinfo(ip.c_str(), NET_PORT, &hints, &res) != 0)
    {
        printf("[Network] getaddrinfo failed\n");
        return false;
    }

    connSock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (connSock == INVALID_SOCKET)
    {
        printf("[Network] socket() failed: %d\n", WSAGetLastError());
        freeaddrinfo(res);
        return false;
    }

    if (connect(connSock, res->ai_addr, (int)res->ai_addrlen) == SOCKET_ERROR)
    {
        printf("[Network] connect() failed: %d\n", WSAGetLastError());
        freeaddrinfo(res);
        closesocket(connSock);
        return false;
    }
    freeaddrinfo(res);

    // Set socket to non-blocking
    u_long mode = 1;
    ioctlsocket(connSock, FIONBIO, &mode);

    connected = true;
    printf("[Network] Connected to host at %s\n", ip.c_str());
    return true;
}

bool Network::SendMove(const NetMove &move)
{
    if (!connected) return false;

    int result = send(connSock, reinterpret_cast<const char *>(&move), sizeof(NetMove), 0);
    if (result == SOCKET_ERROR)
    {
        printf("[Network] send() failed: %d\n", WSAGetLastError());
        connected = false;
        return false;
    }
    return true;
}

bool Network::RecvMove(NetMove &outMove)
{
    if (!connected) return false;

    int result = recv(connSock, reinterpret_cast<char *>(&outMove), sizeof(NetMove), 0);
    if (result == sizeof(NetMove))
        return true;

    if (result == 0)
    {
        printf("[Network] Connection closed by remote.\n");
        connected = false;
    }
    // WSAEWOULDBLOCK means no data yet — that's normal in non-blocking mode
    return false;
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
    WSACleanup();
}

Network::~Network()
{
    Disconnect();
}
