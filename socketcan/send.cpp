
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include <linux/can.h>
#include <linux/can/raw.h>

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>

int main(int argc, char **argv)
{
 int FdSocket;
 struct sockaddr_can addr;//接口索引结构体
 struct ifreq ifr;
 struct can_frame frame;
 frame.can_id = 0x11;
 frame.can_dlc = 1;
 frame.data[0] = 'X';
 int rtn;
 FdSocket = socket(PF_CAN, SOCK_RAW, CAN_RAW);
 if (FdSocket < 0)
 {
  perror("socket");
  close(FdSocket);
  return 1;
 }
 strcpy(ifr.ifr_name, argv[1]);
 if(ioctl(FdSocket,SIOCGIFINDEX,&ifr) < 0)//指定接口索引
 {
  perror("ioctl");
  return 1;
 }
 addr.can_family  = AF_CAN;
 addr.can_ifindex = ifr.ifr_ifindex;
 int rc = bind(FdSocket, (struct sockaddr *)&addr,sizeof(addr));//绑定套接字和can接口。
 if (rc < 0)
 {
  perror("bind");
 }
 for(int i=0;i < 5;i++)
 {
  rtn = write(FdSocket,&frame,sizeof(frame));
  sleep(1);
  if (rtn != sizeof(frame))
  {
 	printf("send error!\n");
 	break;
  }else
  printf("send the data length is %d\n",rtn);
 }
 
 close(FdSocket);
 return 0;

}
