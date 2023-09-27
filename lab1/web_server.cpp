#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

using namespace std;

const int DEFAULT_PORT = 8080;             // 服务器监听端口
const string DEFAULT_ROOT_DIR = "/var/www"; // 服务器主目录

int main(int argc, char* argv[]) {
    // 创建套接字
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        cerr << "create socket error!" << endl;
        return 1;
    }

    // 解析命令行参数，设置配置项
    int port = (argc > 1) ? atoi(argv[1]) : DEFAULT_PORT;
    string rootDir = (argc > 2) ? argv[2] : DEFAULT_ROOT_DIR;

    // 绑定服务器地址和端口
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = INADDR_ANY;
    if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
        cerr << "bind failed" << endl;
        return 1;
    }

    // 开始监听客户端连接
    if (listen(server_socket, 5) == -1) {
        cerr << "listen failed" << endl;
        return 1;
    }

    cout << "Web server is running on port " << port << "..." << endl;

    while (true) {
        // 等待客户端连接
        int client_socket = accept(server_socket, NULL, NULL);
        if (client_socket == -1) {
            cerr << "connect failed" << endl;
            continue;
        }
        
        char buffer[1024];
        ssize_t bytesRead = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytesRead == -1) {
            cerr << "Error reading from client" << endl;
            close(client_socket);
            continue;
        }

        string request(buffer, bytesRead);
        cout << "Received request:\n" << request << endl;

        // 检查请求是否为根路径"/"
        if (request.find("GET / ") != string::npos) {
            // 读取HTML文件内容
            ifstream htmlFile("/home/miles/Computer-Network/lab1/webroot/index.html");
            if (htmlFile.is_open()) {
                stringstream htmlContent;
                htmlContent << htmlFile.rdbuf();

                // 构建HTTP响应
                string response = "HTTP/1.1 200 OK\r\n"
                                  "Content-Type: text/html\r\n"
                                  "\r\n" + htmlContent.str();

                // 发送HTTP响应给客户端
                send(client_socket, response.c_str(), response.size(), 0);

                // 关闭HTML文件
                htmlFile.close();
            } else {
                // 文件打开失败，发送404错误
                string notFoundResponse = "HTTP/1.1 404 Not Found\r\n"
                                          "Content-Type: text/html\r\n"
                                          "\r\n"
                                          "<html><body><h1>404 Not Found</h1></body></html>";

                send(client_socket, notFoundResponse.c_str(), notFoundResponse.size(), 0);
            }
        } else {
            // 其他请求返回404错误
            string notFoundResponse = "HTTP/1.1 404 Not Found\r\n"
                                      "Content-Type: text/html\r\n"
                                      "\r\n"
                                      "<html><body><h1>404 Not Found</h1></body></html>";

            send(client_socket, notFoundResponse.c_str(), notFoundResponse.size(), 0);
        }

        // 关闭客户端连接
        close(client_socket);
    }

    // 关闭服务器Socket
    close(server_socket);

    return 0;
}
