#define WIN32_LEAN_AND_MEAN
#include <iostream>
#include <windows.h>
#include <ws2tcpip.h>
#include <string>

#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

int main()
{
    system("title КЛІЄНТСЬКА СТОРОНА");

    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        std::cout << "WSAStartup не вдалося: " << iResult << "\n";
        return 1;
    }

    addrinfo hints{}, * result = NULL;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    iResult = getaddrinfo("127.0.0.1", DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        std::cout << "getaddrinfo не вдалося: " << iResult << "\n";
        WSACleanup();
        return 2;
    }

    SOCKET ConnectSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ConnectSocket == INVALID_SOCKET) {
        std::cout << "socket не вдалося створити: " << WSAGetLastError() << "\n";
        freeaddrinfo(result);
        WSACleanup();
        return 3;
    }

    iResult = connect(ConnectSocket, result->ai_addr, (int)result->ai_addrlen);
    freeaddrinfo(result);

    if (iResult == SOCKET_ERROR) {
        std::cout << "неможливо підключитися до сервера!\n";
        closesocket(ConnectSocket);
        WSACleanup();
        return 4;
    }

    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;

    while (true) {
        std::string command;
        std::cout << "Введіть команду (TIME, DATE, WEATHER <city>, EUR, BTC): ";
        std::getline(std::cin, command);

        iResult = send(ConnectSocket, command.c_str(), (int)command.size(), 0);
        if (iResult == SOCKET_ERROR) {
            std::cout << "send не вдалося: " << WSAGetLastError() << "\n";
            closesocket(ConnectSocket);
            WSACleanup();
            return 5;
        }

        iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
        if (iResult > 0) {
            recvbuf[iResult] = '\0';
            std::cout << "Відповідь сервера: " << recvbuf << "\n";
        }
        else if (iResult == 0) {
            std::cout << "З'єднання закрито.\n";
            break;
        }
        else {
            std::cout << "recv не вдалося: " << WSAGetLastError() << "\n";
            break;
        }
    }

    closesocket(ConnectSocket);
    WSACleanup();
    return 0;
}