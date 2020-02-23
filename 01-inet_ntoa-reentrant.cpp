#include <arpa/inet.h>
#include <netinet/in.h>
#include <iostream>
#include <cstdlib>
int main() {
    struct in_addr addr1 = {inet_addr("192.168.0.0")};
    struct in_addr addr2 = {inet_addr("192.168.0.1")};
    // inet_ntoa属于不可重入的函数, 返回值内部由一个static的char * 存储
    char* v1 = inet_ntoa(addr1);
    char* v2 = inet_ntoa(addr2);
    std::cout << v1 << std::endl; // v1 的值会被改变
    std::cout << v2 << std::endl;

    // 使用inet_ntop解决该问题
    char* v3 = new char[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr1, v3, INET_ADDRSTRLEN);
    char* v4 = new char[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr2, v4, INET_ADDRSTRLEN);

    std::cout << v3 << std::endl;
    std::cout << v4 << std::endl;

    delete []v3;
    delete []v4;
    return 0;
}
