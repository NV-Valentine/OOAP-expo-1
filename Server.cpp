#define WIN32_LEAN_AND_MEAN
#include <iostream>
#include <windows.h>
#include <ws2tcpip.h>
#include <string>

#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

SOCKET ClientSocket = INVALID_SOCKET;

DWORD WINAPI ClientHandler(void* param)
{
    SOCKET client = (SOCKET)param;
    char buffer[DEFAULT_BUFLEN];

    while (true) {
        // оставляем место для '\0'
        int iResult = recv(client, buffer, DEFAULT_BUFLEN - 1, 0);
        if (iResult <= 0) break;

        buffer[iResult] = '\0';
        std::string command(buffer);
        std::string answer;

        if (command == "TIME") {
            SYSTEMTIME st; GetLocalTime(&st);
            char buf[50];
            sprintf_s(buf, sizeof(buf), "%02d:%02d:%02d", st.wHour, st.wMinute, st.wSecond);
            answer = buf;
        }
        else if (command == "DATE") {
            SYSTEMTIME st; GetLocalTime(&st);
            char buf[50];
            sprintf_s(buf, sizeof(buf), "%02d-%02d-%04d", st.wDay, st.wMonth, st.wYear);
            answer = buf;
        }
        else if (command.rfind("WEATHER", 0) == 0) {
            answer = "Weather service not implemented yet";
        }
        else if (command == "EUR") {
            answer = "EUR rate: stub";
        }
        else if (command == "BTC") {
            answer = "BTC rate: stub";
        }
        else {
            answer = "Unknown command";
        }

        send(client, answer.c_str(), (int)answer.size(), 0);
    }

    closesocket(client);
    return 0;
}

int main()
{
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        std::cout << "WSAStartup failed: " << iResult << "\n";
        return 1;
    }

    struct addrinfo hints, * result = NULL;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        std::cout << "getaddrinfo failed: " << iResult << "\n";
        WSACleanup();
        return 2;
    }

    SOCKET ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        std::cout << "socket failed: " << WSAGetLastError() << "\n";
        freeaddrinfo(result);
        WSACleanup();
        return 3;
    }

    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        std::cout << "bind failed: " << WSAGetLastError() << "\n";
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 4;
    }

    freeaddrinfo(result);

    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        std::cout << "listen failed: " << WSAGetLastError() << "\n";
        closesocket(ListenSocket);
        WSACleanup();
        return 5;
    }

    std::cout << "Server is running, waiting for connections...\n";

    while (true) {
        ClientSocket = accept(ListenSocket, NULL, NULL);
        if (ClientSocket == INVALID_SOCKET) {
            std::cout << "accept failed: " << WSAGetLastError() << "\n";
            closesocket(ListenSocket);
            WSACleanup();
            return 6;
        }

        CreateThread(NULL, 0, ClientHandler, (void*)ClientSocket, 0, NULL);
    }

    closesocket(ListenSocket);
    WSACleanup();
    return 0;
}

//- команды (TIME, DATE, EUR, BTC, WEATHER Kyiv).
