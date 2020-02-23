#include <iostream>
#include <string>
#include <sstream>
#include <cstring>
#include <cassert>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#ifndef BUFSIZE
#define BUFSIZE 2048
#endif // BUFSIZE;

void runServer(const std::string& host, const in_port_t port) {
    int serverfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(serverfd != -1);

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, host.c_str(), &addr.sin_addr);

    assert(bind(serverfd, (struct sockaddr* )&addr, sizeof(struct sockaddr_in)) != -1);
    assert(listen(serverfd, 1) != -1);
    char buffer[BUFSIZE];
    for (;;) {
        struct sockaddr clientAddr;
        socklen_t clientAddrLen = sizeof(struct sockaddr);
        int connfd = accept(serverfd, &clientAddr, &clientAddrLen);

        for (;;) {
            int ret = recv(connfd, buffer, BUFSIZE, 0);
            std::cout << "got " << ret << " bytes of normal data " << buffer << std::endl;
            memset(buffer, '\0', BUFSIZE);

            // 接收带外数据
            ret = recv(connfd, buffer, BUFSIZE, MSG_OOB);
            if (ret == 1) {
                std::cout << "got " << ret << " bytes of oob data " << buffer << std::endl;
                memset(buffer, '\0', BUFSIZE);
            }
        }
    }
}

void runClient(const std::string& host, const in_port_t port) {
    int serverfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(serverfd != -1);

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, host.c_str(), &addr.sin_addr);
    assert(connect(serverfd, (struct sockaddr*)&addr, sizeof(struct sockaddr_in)) != -1);
    std::string ISMSG_OOB;
    std::string sendData;
    for(;;) {
        std::cout << "send oop data[Y/N]: ";
        std::cin >> ISMSG_OOB;
        if (ISMSG_OOB == "Y") {
            std::cout << "please input send oob data:";
            std::cin >> sendData;

            int ret = send(serverfd, sendData.c_str(), sendData.length(), MSG_OOB);
            std::cout << "client send "<< ret << " bytes oob data: "<< sendData << std::endl;
            sendData.clear();

        } else {
            std::cout << "please input send normal data: ";
            std::cin >> sendData;

            int ret = send(serverfd, sendData.c_str(), sendData.length(), 0);
            std::cout << "client send "<< ret << " bytes normal data: "<< sendData << std::endl;
            sendData.clear();
        }
    }
}
int main(int argc, const char* argv[]) {
    if (argc < 2) {
        std::cout << "select client or server" << std::endl;
        return 0;
    }

    if (strcmp(argv[1], "server") == 0) {
       if (argc < 3) {
           std::cout << "server need argument address:port" << std::endl;
           return 0;
       }

       in_port_t port;
       std::stringstream(std::string(argv[2])) >> port;
       runServer(std::string(argv[1]), port);

    } else if (strcmp(argv[1], "client") == 0){
        if (argc < 3) {
            std::cout << "client need argument server(host:port)" << std::endl;
            return 0;
        }

        in_port_t port;
        std::stringstream(std::string(argv[2])) >> port;
        runClient(std::string(argv[1]), port);

    } else {
        std::cout << "invalid argument" << std::endl;
    }
}
