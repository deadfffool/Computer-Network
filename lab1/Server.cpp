#include <iostream>
#include <fstream>
#include <string>
#include <WinSock2.h>
using namespace std;

#pragma comment(lib, "ws2_32.lib")

int DEFAULT_PORT = 8080;
string DEFAULT_ROOT_DIRECTORY = "C:\\Users\\Miles\\Desktop\\Computer-Network\\lab1\\VerySimpleServer\\webroot\\index.html";

void HandleClient(SOCKET clientSocket);

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "Failed to initialize Winsock" << endl;
        return 1;
    }

    // Create socket
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        cerr << "Failed to create socket" << endl;
        return 1;
    }

    // Bind socket
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    cout << "Enter the port number: ";
    cin >> DEFAULT_PORT;
    serverAddr.sin_port = htons(DEFAULT_PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cerr << "Failed to bind socket" << endl;
        closesocket(serverSocket);
        return 1;
    }

    // Listen for incoming connections
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        cerr << "Failed to listen on socket" << endl;
        closesocket(serverSocket);
        return 1;
    }

    cout << "Enter your webroot\n";
    cin >> DEFAULT_ROOT_DIRECTORY;
    cout << "Web server is listening on port " << DEFAULT_PORT << endl;

    while (true) {
        // Accept incoming connection
        sockaddr_in clientAddr;
        int clientAddrLen = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
        if (clientSocket == INVALID_SOCKET) {
            cerr << "Failed to accept connection" << endl;
            continue;
        }

        // Handle the client's request
        HandleClient(clientSocket);

        // Clean up the client socket
        closesocket(clientSocket);
    }

    // Clean up the server socket
    closesocket(serverSocket);
    WSACleanup();
    return 0;
}

void HandleClient(SOCKET clientSocket) {
    const int BUFFER_SIZE = 1024;
    char buffer[BUFFER_SIZE];

    // Receive HTTP request from the client
    int bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
    if (bytesRead == SOCKET_ERROR) {
        cerr << "Failed to receive data from client" << endl;
        return;
    }

    // Parse the HTTP request to determine the requested file
    string request(buffer, bytesRead);

    // Build the full path to the requested file
    string fullPath = DEFAULT_ROOT_DIRECTORY;

    // Open and read the requested file
    ifstream file(fullPath, ios::in | ios::binary);
    if (!file) {
        cerr << "Failed to open file: " << fullPath << endl;
        return;
    }

    // Read file contents
    string responseContent((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());

    // Build HTTP response
    string httpResponse = "HTTP/1.1 200 OK\r\n";
    httpResponse += "Content-Length: " + to_string(responseContent.size()) + "\r\n";
    httpResponse += "Content-Type: text/html\r\n\r\n";
    httpResponse += responseContent;

    // Send the HTTP response to the client
    send(clientSocket, httpResponse.c_str(), httpResponse.size(), 0);
}

