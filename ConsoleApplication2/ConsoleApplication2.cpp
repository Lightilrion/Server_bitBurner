#undef UNICODE
#define _WIN32_WINNT 0x501
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <algorithm>
#include <iostream>
#include <vector>
#include <string>
#include <time.h>
#pragma comment(lib, "Ws2_32.lib")
#define DEFAULT_BUFLEN 2048
struct sa {
    SOCKET ListenSocket;
    struct addrinfo* result;
};
sa setups(const char* port) {
    int iResult;
    sa h;
    h.ListenSocket = 0;
    h.result = 0;
    WSADATA wsaData;
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult) {
        std::cout << "Failed 1 " << iResult;
        return h;
    }
    struct addrinfo* result = NULL, * ptr = NULL, hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;
    iResult = getaddrinfo(NULL, port, &hints, &result);
    if (iResult) {
        std::cout << "Failed 2 " << iResult;
        WSACleanup();
        return h;
    }
    SOCKET ListenSocket = INVALID_SOCKET;
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        std::cout << "Failed 3 " << iResult;
        freeaddrinfo(result);
        WSACleanup();
        return h;
    }
    sa ha;
    ha.ListenSocket = ListenSocket;
    ha.result = result;
    return ha;
}
std::string getData(const char * recvbuf) {
    std::string temp;
    for (int i = (int)strlen(recvbuf) - 1; i > 0; i--) {
        if (recvbuf[i] == '\n') {
            break;
        }
        temp += recvbuf[i];
    }
    std::reverse(temp.begin(), temp.end());
    return temp;
}
std::string response_base= "HTTP/1.1 202 OK\r\nAccess-Control-Allow-Origin: *\r\nAccess-Control-Allow-Credentials: true\r\nAccess-Control-Allow-Methods: POST\r\nAccess-Control-Allow-Headers: Content-Type";
void responseWithData(SOCKET ClientSocket, std::string temps) {
    std::string response = response_base;
    response += "Content-Length: ";
    response += std::to_string(temps.length());
    response += "\r\n";
    response += "Content-Type: text/html\r\nConnection: closed\r\n\r\n";
    response += temps;
    send(ClientSocket, response.c_str(), (int)strlen(response.c_str()), 0);
}
void sendTwoData(SOCKET ClientSocket, std::vector<double> moneys, std::vector<time_t> timesM) {
    std::string temps = "";
    std::string response = response_base;
    for (int i = 0; i < moneys.size(); i++) {
        temps += std::to_string(moneys[i]);
        temps += ",";
        temps += std::to_string(timesM[i]);
    }
    temps.pop_back();
    responseWithData(ClientSocket, temps);
}
void responseDefault(SOCKET ClientSocket) {
    std::string response = response_base;
    response += "Content-Type: text/html\r\nConnection: closed\r\n\r\n";
    send(ClientSocket, response.c_str(), (int)strlen(response.c_str()), 0);
}

int main() {
    sa ha = setups("27015");
    SOCKET ListenSocket = ha.ListenSocket;
    struct addrinfo* result = ha.result;
    std::vector<double> moneys;
    std::vector<time_t> timesM;
    if (ListenSocket == INVALID_SOCKET || result == 0) {
        std::cout << "hjs";
        return 1;
    }
    int iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        std::cout << "Failed 4 " << iResult;
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
    freeaddrinfo(result);

    char recvbuf[DEFAULT_BUFLEN];
    int iSendResult;
    int recvbuflen = DEFAULT_BUFLEN;
    int haha;
    while (true) {
        SOCKET ClientSocket = INVALID_SOCKET;
        listen(ListenSocket, 1);
        ClientSocket = accept(ListenSocket, NULL, NULL);
        iResult = 0;
        memset(recvbuf, 0, sizeof(recvbuf));
        if (ClientSocket == INVALID_SOCKET) {
            std::cout << "Failed 6 " << WSAGetLastError();
            closesocket(ListenSocket);
            WSACleanup();
            return 1;
        }
        do {
            iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
            if (iResult > 0) {
                //std::cout<<"recv: "<<recvbuf<<std::endl;
                std::string temp = getData(recvbuf);
                std::cout << temp << std::endl;
                memset(recvbuf, 0, sizeof(recvbuf));
                if (temp.find("Gm") != std::string::npos) {
                    temp.erase(0, 2);
                    moneys.push_back(std::stod(temp));
                    timesM.push_back(std::time(nullptr));
                    responseDefault(ClientSocket);
                }
                if (temp.find("AskM") != std::string::npos) sendTwoData(ClientSocket, moneys, timesM);
                iResult = shutdown(ClientSocket, SD_SEND);
                break;
            }
            else if (iResult == 0) std::cout << "closing connection";
            else {
                std::cout << "Failed 6 " << WSAGetLastError();
                closesocket(ClientSocket);
                WSACleanup();
                std::cin >> haha;
                return 1;
            }

        } while (iResult > 0);

    }
    return 0;
}
