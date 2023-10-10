#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;

const int DEFAULT_PORT = 8080;             // 服务器监听端口
const string DEFAULT_ROOT_DIR = "/home/miles/Computer-Network/lab1/webroot"; // 服务器主目录

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
    string listenAddress = (argc > 3) ? argv[3] : "0.0.0.0"; // 默认监听所有网络接口

    // 绑定服务器地址和端口
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);

    // 将监听地址从字符串转换为网络地址
    if (inet_pton(AF_INET, listenAddress.c_str(), &(server_address.sin_addr)) <= 0) {
        cerr << "Invalid listen address" << endl;
        return 1;
    }

    if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
        cerr << "bind failed" << endl;
        return 1;
    }

    // 开始监听客户端连接
    if (listen(server_socket, 5) == -1) {
        cerr << "listen failed" << endl;
        return 1;
    }

    cout << "Web server is running on port " << port << "..." << endl<<endl;

    while (true) {
        // 等待客户端连接
        int client_socket = accept(server_socket, NULL, NULL);
        if (client_socket == -1) {
            cerr << "connect failed" << endl;
            continue;
        }
        
        // 获取客户端的IP地址和端口号
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        getpeername(client_socket, (struct sockaddr*)&client_addr, &client_addr_len);
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
        int client_port = ntohs(client_addr.sin_port);
        
        cout << "Connection from IP: " << client_ip << ", Port: " << client_port << endl;

        char buffer[1024];
        ssize_t bytesRead = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytesRead == -1) {
            cerr << "Error reading from client" << endl;
            close(client_socket);
            continue;
        }

        string request(buffer, bytesRead);
        cout << "Received request:\n" << request << endl;

        
        // string requestLine;
        // size_t pos = request.find("\r\n");
        // if (pos != string::npos) {
        //     requestLine = request.substr(0, pos); // Get the first line of the HTTP request
        //     // Now you can parse the requestLine to extract the URL
        //     istringstream iss(requestLine);
        //     string httpMethod, url, httpVersion;
        //     iss >> httpMethod >> url >> httpVersion;
            
        //     // 'url' now contains the requested URL
        //     cout << "Requested URL: " << url << endl;
        // }

        //处理请求
        if (request.find("GET /index.html") != string::npos)
        {
            cout<<"GET /index.html"<<endl;
            ifstream htmlFile(rootDir+"/index.html");
            if (htmlFile.is_open()) {
                stringstream htmlContent;
                htmlContent << htmlFile.rdbuf();
                string response = "HTTP/1.1 200 OK\r\n"
                                  "Content-Type: text/html\r\n"
                                  "\r\n" + htmlContent.str();

                // 发送HTTP响应给客户端
                send(client_socket, response.c_str(), response.size(), 0);
                cout << "sent html" << endl << endl;
                // 关闭HTML文件
                htmlFile.close();
            }
        }

        else if (request.find("GET /image.jpg") != string::npos) 
        {
            cout<<"GET /image.jpg"<<endl;
            ifstream imageFile(rootDir+"/image.jpg");
            if (imageFile.is_open()) {
                stringstream imageContent;
                imageContent << imageFile.rdbuf();
                
                // 根据图像文件扩展名确定内容类型
                string contentType = "image/jpeg"; // 根据实际的图像类型进行修改
                
                // 构建HTTP响应
                string response = "HTTP/1.1 200 OK\r\n"
                                "Content-Type: " + contentType + "\r\n"
                                "\r\n" + imageContent.str();
                
                send(client_socket, response.c_str(), response.size(), 0);
                cout << "sent image" << endl << endl;
                imageFile.close();
                }
        }

        else {
            // 其他请求返回404错误
            string notFoundResponse = "HTTP/1.1 404 Not Found\r\n"
                                      "Content-Type: text/html\r\n"
                                      "\r\n"
                                      "<html><body><h1>404 Not Found</h1></body></html>";

            send(client_socket, notFoundResponse.c_str(), notFoundResponse.size(), 0);
            cout << "404 not found" << endl;
        }

        // 关闭客户端连接
        close(client_socket);
    }

    // 关闭服务器Socket
    close(server_socket);

    return 0;
}
