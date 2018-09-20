
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

#define PER_CAN_ID 0x11

int main(int argc, char **argv)
{
 int FdSocket;
 struct sockaddr_can addr;
 struct ifreq ifr;
 struct can_frame frame;
 struct can_filter rfilter;
 int rtn;
 FdSocket = socket(PF_CAN, SOCK_RAW, CAN_RAW);
 if (FdSocket < 0)
 {
  printf("socket creat failed!\n");
  perror("socket");
  close(FdSocket);
  return 1;
 }
 strcpy(ifr.ifr_name, argv[1]);
 if(ioctl(FdSocket,SIOCGIFINDEX,&ifr) < 0)
 {
  printf("device set failed!\n");
  perror("ioctl");
  return 1;
 }
 addr.can_family  = AF_CAN;
 addr.can_ifindex = ifr.ifr_ifindex;
 int rc = bind(FdSocket, (struct sockaddr *)&addr,sizeof(addr));
 if (rc < 0)
 {
  printf("socket with can failed!\n");
  perror("bind");
  return 1;
 }

 //set recivies rules
 rfilter.can_id = PER_CAN_ID;
 rfilter.can_mask = CAN_SFF_MASK;
 setsockopt(FdSocket, SOL_CAN_RAW, CAN_RAW_FILTER, &rfilter,sizeof(rfilter));

 while(1)
 {
  rtn = read(FdSocket,&frame,sizeof(frame));

  if (rtn > 0)
  {
  printf("read success!\n");
  printf("can_id:0x%X  can_dlc:%d can_data:%c \n",frame.can_id,frame.can_dlc,frame.data[0]);
 
  }else
  printf("revice nothing!\n");
 }
 
 close(FdSocket);
 return 0;

}
