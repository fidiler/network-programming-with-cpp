#include <iostream>
#include <cstring>
#include <cctype>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

enum PARSE_STATE {REQ_LINE = 0, REQ_HEADER, REQ_BAD};

enum LINE_STATUS {LINE_OK = 0, LINE_BAD, LINE_OPEN };

enum PARSE_STATUS {NO = 0, YES};

class HttpParser
{

public:
    HttpParser();
    ~HttpParser();
    HttpParser(char* httpContent): __httpContent(httpContent) {};
    enum PARSE_STATUS parse();
    inline void setReadIndex(const size_t& readIndex) { __readIndex = readIndex; };
    inline char* getBuffer() const { return __httpContent; };
    inline void resetBuffer() {
        char* newbuf = new char[getBufferSize() * 2 + 1];
        strcpy(newbuf, __httpContent);
        delete []__httpContent;
        __httpContent = newbuf;
    };
    inline size_t getBufferSize() const { return 8; };
private:
    size_t __checkIndex;
    size_t __readIndex;
    size_t __readLineIndex;
    char* __httpContent;
    char* __requestMethod;
    char* __requestPath;
    char* __requestVersion;
    const char* readHttpLine(enum LINE_STATUS& state);
    void parseRequestLine(const char* line, size_t lineSize);
    void parseRequestHeader(const char *line);
    enum PARSE_STATE __parseState;
};

HttpParser::HttpParser() {
    __checkIndex = 0;
    __readIndex = 0;
    __readLineIndex = 0;
    __parseState = PARSE_STATE::REQ_LINE;
    __httpContent = new char[8];
}

HttpParser::~HttpParser() {
    delete []__httpContent;
}

enum PARSE_STATUS HttpParser::parse() {
    LINE_STATUS lineStatus;
    const char* line;
    for(;;) {
        line = readHttpLine(lineStatus);
        if (line == nullptr && lineStatus != LINE_STATUS::LINE_OK) {
            break;
        }
        switch (__parseState) {
        case PARSE_STATE::REQ_LINE:
            parseRequestLine(line, strlen(line));
            break;
        case PARSE_STATE::REQ_HEADER:
            parseRequestHeader(line);
            break;
        case PARSE_STATE::REQ_BAD:
            std::cout << "parse http request bad" << std::endl;
            break;
        }
    }

    if (lineStatus == LINE_OPEN) {
        std::cout << "line open" << std::endl;
        return PARSE_STATUS::YES;
    } else {
        return PARSE_STATUS::NO;
    }
}

const char* HttpParser::readHttpLine(enum LINE_STATUS& state) {
    for (;__checkIndex < __readIndex; __checkIndex++) {
        if (__httpContent[__checkIndex] == '\r') {
            // 表示读取完本次的缓冲区, 但没有读到一个完整到http line报文
            if (__checkIndex + 1 == __readIndex) {
                state = LINE_STATUS::LINE_OPEN;
                return nullptr;

                 // \r\n, 完整到读取到了一个http line 报文
            }else if (__httpContent[__checkIndex+1] == '\n') {
                __httpContent[__checkIndex++] = '\0';
                __httpContent[__checkIndex++] = '\0';
                state = LINE_STATUS::LINE_OK;
                // 根据当前读取到的偏移值获取line
                const char* line = __httpContent + __readLineIndex;
                // 更新偏移
                __readLineIndex = __checkIndex;
                return line;
            }

            state = LINE_STATUS::LINE_BAD;
            return nullptr;

        } else if(__httpContent[__checkIndex] == '\n') {
            if (__checkIndex > 1 && __httpContent[__checkIndex-1] == '\r') {
                __httpContent[__checkIndex++] = '\0';
                __httpContent[__checkIndex++] = '\0';
                state = LINE_STATUS::LINE_OK;
                // 根据当前读取到的偏移值获取line
                const char* line = __httpContent + __readLineIndex;
                // 更新偏移
                __readLineIndex = __checkIndex;
                return line;
            }

            state = LINE_STATUS::LINE_BAD;
            return nullptr;
        }
    }


    state = LINE_STATUS::LINE_OPEN;
    return nullptr;
}

void HttpParser::parseRequestLine(const char* line, size_t lineSize) {
    std::cout << line << std::endl;
    char method[255];
    size_t i = 0, j = 0;
    while (!isspace(line[i]) && i < lineSize) {
        method[j++] = line[i++];
    }

    method[j] = '\0';

    while(isspace(line[i]) && i < lineSize) {
        i++;
    }


    char path[255];
    j = 0;
    while(!isspace(line[i]) && i < lineSize) {
        path[j++] = line[i++];
    }
    path[j] = '\0';

    while(isspace(line[i]) && i < lineSize) {
        i++;
    }

    char version[255];
    j = 0;
    while(i < lineSize) {
        version[j++] = line[i++];
    }
    version[j] = '\0';
    std::cout << "method: " << method << std::endl;
    std::cout << "path: " << path<< std::endl;
    std::cout << "version: " << version << std::endl;

    __parseState = PARSE_STATE::REQ_HEADER;

//    // parse method
//    char* path = strpbrk(__httpContent, " \t");
//    if (!path) {

//        return;
//    }
//    *path++ = '\0';
//    this->__requestMethod = new char[strlen(__httpContent)+1];
//    strcpy(this->__requestMethod, __httpContent);

//    // parse path
//    char* version = strpbrk(path, " \t");
//    if (!version) {
//        std::cout << "bad request" << std::endl;
//    }

//    *version++ = '\0';
//    this->__requestPath = new char[strlen(path)+1];
//    strcpy(this->__requestPath, path);


//    this->__requestVersion = new char(strlen(version)+1);
//    strcpy(this->__requestVersion, version);

//    std::cout << "Request Method: " << __requestMethod << std::endl;
//    std::cout << "Request Path: " << __requestPath << std::endl;
//    std::cout << "version: " << __requestVersion << std::endl;
}

void HttpParser::parseRequestHeader(const char *line) {
    std::cout << line << std::endl;
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
       enum PARSE_STATUS parseStatus = parser.parse();
       std::cout << parseStatus << std::endl;
       if (parseStatus == NO) {
           break;
       } else {
           parser.resetBuffer();

           continue;
       }
    }
}
