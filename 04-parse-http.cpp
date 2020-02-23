#include <iostream>
#include <cstring>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>


enum LINE_STATUS { LINE_OK = 0, LINE_BAD, LINE_OPEN };

enum HTTP_PARSE_STATUS {NO = 0, YES};

class HttpParser
{

public:
    HttpParser();
    ~HttpParser();
    HttpParser(char* httpContent): __httpContent(httpContent) {};
    void parseRequestLine();
    enum HTTP_PARSE_STATUS parse();
    inline void setReadIndex(const size_t& readIndex) { __readIndex = readIndex; };
    inline char* getBuffer() const { return __httpContent; };
    inline void resetBuffer() { delete []__httpContent; __httpContent = new char[4096]; };
    inline size_t getBufferSize() const { return 4096; };
private:
    size_t __checkIndex;
    size_t __readIndex;
    char* __httpContent;
    char* __requestMethod;
    char* __requestPath;
    char* __requestVersion;
    enum LINE_STATUS readHttpLine();
};

HttpParser::HttpParser() {
    __checkIndex = 0;
    __readIndex = 0;
    __httpContent = new char[4096];
}

HttpParser::~HttpParser() {
    delete []__httpContent;
}

enum HTTP_PARSE_STATUS HttpParser::parse() {
    LINE_STATUS lineStatus;
    while((lineStatus = readHttpLine()) == LINE_OK) {
        std::cout << __httpContent << std::endl;
    }

    if (lineStatus == LINE_OPEN) {
        return HTTP_PARSE_STATUS::YES;
    } else {
        return HTTP_PARSE_STATUS::NO;
    }
}

enum LINE_STATUS HttpParser::readHttpLine() {
    for (;__checkIndex < __readIndex; __checkIndex++) {
        if (__httpContent[__checkIndex] == '\r') {
            if (__checkIndex + 1 == __readIndex) {
                return LINE_STATUS::LINE_OPEN;
            }

            if (__httpContent[__checkIndex+1] == '\n') {
                __httpContent[__checkIndex++] = '\0';
                __httpContent[__checkIndex++] = '\0';
                return LINE_STATUS::LINE_OK;
            }

            return LINE_STATUS::LINE_BAD;

        } else if(__httpContent[__checkIndex] == '\n') {
            if (__checkIndex > 1 && __httpContent[__checkIndex -1] == '\r') {
                __httpContent[__checkIndex++] = '\0';
                __httpContent[__checkIndex++] = '\0';
                return LINE_STATUS::LINE_OK;
            }

            return LINE_STATUS::LINE_BAD;
        }
    }

    std::cout << "check: " << __httpContent << std::endl;

    return LINE_STATUS::LINE_OPEN;
}

void HttpParser::parseRequestLine() {
    std::cout << __httpContent << std::endl;

    // parse method
    char* path = strpbrk(__httpContent, " \t");
    if (!path) {
        std::cout << "bad request" << std::endl;
        return;
    }
    *path++ = '\0';
    this->__requestMethod = new char[strlen(__httpContent)+1];
    strcpy(this->__requestMethod, __httpContent);


    // parse path
    char* version = strpbrk(path, " \t");
    if (!path) {
        std::cout << "bad request" << std::endl;
    }
    *version++ = '\0';
    this->__requestPath = new char[strlen(version)+1];
    strcpy(this->__requestPath, path);


    // parse version
    char* header = strpbrk(version, "\n");
    *header++ = '\0';
    this->__requestVersion = new char(strlen(version)+1);
    strcpy(this->__requestVersion, version);

    std::cout << "Request Method: " << __requestMethod << std::endl;
    std::cout << "Request Path: " << __requestPath << std::endl;
    std::cout << "version: " << __requestVersion << std::endl;
    std::cout << "header: " << header << std::endl;


}

int main()
{
    int serverfd = socket(AF_INET, SOCK_STREAM, 0);
    int reuse = 1;
    setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    struct sockaddr_in addr;
    bzero(&addr, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9090);
    addr.sin_addr.s_addr = INADDR_ANY;

    int ret = bind(serverfd, (struct sockaddr*)&addr, sizeof(struct sockaddr_in));
    if (ret == -1) {
        std::cout << "bind error" << std::endl;
        close(serverfd);
        return 0;
    }

    ret = listen(serverfd, 128);
    if (ret == -1) {
        std::cout << "listen error" << std::endl;
        close(serverfd);
        return 0;
    }

    struct sockaddr_in cliAddr;
    socklen_t cliAddrLen = sizeof(struct sockaddr_in);


    int connfd = accept(serverfd, (struct sockaddr*)&cliAddr, &cliAddrLen);
    if (connfd == -1) {
        std::cout << "listen error" << std::endl;
        close(serverfd);
        return 0;
    }

    HttpParser parser;
    size_t readIndex = 0;
    for(;;) {

       ret = recv(connfd, parser.getBuffer(), parser.getBufferSize(), 0);
       if (ret == 0) {
           std::cout << "peer closed" << std::endl;
           return 0;
       } else if(ret == -1) {
           std::cout << "recv error" << std::endl;
           return 0;
       }

       readIndex += ret;
       parser.setReadIndex(readIndex);
       enum HTTP_PARSE_STATUS parseStatus = parser.parse();
       std::cout << parseStatus << std::endl;
       if (parseStatus == YES) {
           break;
       } else {
           continue;
       }
       parser.resetBuffer();
    }
}
