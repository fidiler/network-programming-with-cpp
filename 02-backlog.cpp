#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include <sstream>

int main(int argc, const char* argv[]) {
    if (argc < 3) {
        std::cout << "usage: localhost, port, backlog" << std::endl;
        return 0;
    }

    in_port_t port;
    std::stringstream (std::string(argv[2])) >> port;

    int backlog;
    std::stringstream (std::string(argv[3])) >> backlog;

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in* addr =  new sockaddr_in();
    addr->sin_family = AF_INET;
    addr->sin_addr = in_addr{inet_addr(argv[1])};
    addr->sin_port = htons(port);

    bind(sockfd, (struct sockaddr*)addr, sizeof(struct sockaddr));

    listen(sockfd, backlog);

    std::cout << "server listen: " << argv[1] << ":" << argv[2] << ", backlog = " << argv[3] << std::endl;

    for(;;) {
        usleep(1000);
    }
}
