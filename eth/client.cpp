//
//  main.cpp
//  linux_socket_api_client
//
//  Created by bikang on 16/11/2.
//  Copyright (c) 2016年 bikang. All rights reserved.
//
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


void tserver(int argc, const char * argv[]);

int main(int argc, const char * argv[]) {
    tserver(argc,argv);
    return 0;
}
void tserver(int argc, const char * argv[]){

    const char* ip = "127.0.0.1";
    int port = 5231;

    std::cout << "ip=" << ip << " port="<<port << std::endl;

    int fd;
    int check_ret;

    fd = socket(PF_INET,SOCK_DGRAM , 0);
    assert(fd >= 0);

    struct sockaddr_in address;
    bzero(&address,sizeof(address));

    //转换成网络地址
    address.sin_port = htons(port);
    address.sin_family = AF_INET;
    //地址转换
    inet_pton(AF_INET, ip, &address.sin_addr);
    //发送数据
    const char* normal_data = "my boy!";
    if(sendto(fd, normal_data, strlen(normal_data),0,(struct sockaddr*)&address,sizeof(address)) < 0) 
    { 
      perror("Send File Name Failed:"); 
      exit(1); 
    }
    close(fd);
}