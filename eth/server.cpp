
#include <iostream>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

//using namespace std;

#define BUFFER_SIZE 1024


void tsocket(int argc, const char * argv[]);

int main(int argc, const char * argv[]) {
    tsocket(argc,argv);
    return 0;
}
void tsocket(int argc, const char * argv[]){

    const char* ip = "172.20.30.2";

    int port = 5231;

    std::cout << "ip=" << ip << " port="<<port << std::endl;

    int fd;
    int check_ret;

    fd = socket(AF_INET,SOCK_DGRAM , 0);
    assert(fd >= 0);

    struct sockaddr_in address;
    bzero(&address,sizeof(address));

    //转换成网络地址
    address.sin_port = htons(port);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    //地址转换
    //inet_pton(AF_INET, ip, &address.sin_addr);

    //绑定ip和端口
    check_ret = bind(fd,(struct sockaddr*)&address,sizeof(address));
    printf("bind return: %d, errno: %d\n", check_ret, errno);
    assert(check_ret >= 0);



    while(1){

        char buffer[BUFFER_SIZE];
        struct sockaddr_in addressClient;
        socklen_t clientLen = sizeof(addressClient);
        memset(buffer, '\0', BUFFER_SIZE);
        //获取信息
        if(recvfrom(fd, buffer, BUFFER_SIZE-1,0,(struct sockaddr*)&addressClient, &clientLen) == -1) 
        { 
           perror("Receive Data Failed:"); 
           exit(1); 
        } 
        printf("buffer=%s\n", buffer);
    }
    close(fd);
}