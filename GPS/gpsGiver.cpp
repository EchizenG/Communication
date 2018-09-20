#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include "gpsGiver.hpp"
#include "unpackNMEA.hpp"
#include "errno.h"

static void set_tty(void)
{
  char adj_GPS[]={
    0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x0A, 0x00, 0x00,0x00,0x00,0x00,0x00, 0x09, 0x69,//disable DTM
    0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x44, 0x00, 0x00,0x00,0x00,0x00,0x00, 0x43, 0xFF,//disable_GBQ
    0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x09, 0x00, 0x00,0x00,0x00,0x00,0x00, 0x08, 0x62,//disable_GBS
    0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x00, 0x00, 0x00,0x00,0x00,0x00,0x00, 0xFF, 0x23,//disable_GGA
    0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x43, 0x00, 0x00,0x00,0x00,0x00,0x00, 0x42, 0xF8,//disable_GLQ
    0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x42, 0x00, 0x00,0x00,0x00,0x00,0x00, 0x41, 0xF1,//disable_GNQ
    0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x0d, 0x00, 0x00,0x00,0x00,0x00,0x00, 0x0C, 0x7E,//disable_GNS
    0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x40, 0x00, 0x00,0x00,0x00,0x00,0x00, 0x3F, 0xE3,//disable_GPQ
    0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x06, 0x00, 0x00,0x00,0x00,0x00,0x00, 0x05, 0x4D,//disable_GRS
    0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x02, 0x00, 0x00,0x00,0x00,0x00,0x00, 0x01, 0x31,//disable_GSA
    0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x07, 0x00, 0x00,0x00,0x00,0x00,0x00, 0x06, 0x54,//disable_GST
    0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x03, 0x00, 0x00,0x00,0x00,0x00,0x00, 0x02, 0x38,//disable_GSV
    0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x41, 0x00, 0x00,0x00,0x00,0x00,0x00, 0x40, 0xEA,//disable_TXT
    0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x0F, 0x00, 0x00,0x00,0x00,0x00,0x00, 0x0E, 0x8C,//disable_VLW
    0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x05, 0x00, 0x00,0x00,0x00,0x00,0x00, 0x04, 0x46,//disable_VTG
    0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x08, 0x00, 0x00,0x00,0x00,0x00,0x00, 0x07, 0x5B,//disable_ZDA
    0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x01, 0x00, 0x00,0x00,0x00,0x00,0x00, 0x00, 0x2A,//disable_GLL
    0xB5, 0x62, 0x06, 0x08, 0x06, 0x00, 0x64, 0x00, 0x01, 0x00,0x01, 0x00,          0x7A, 0x12,//cfg_rate
    0xB5, 0x62, 0x06, 0x00, 0x14, 0x00, 0x01, 0x00, 0x00, 0x00, 0xD0, 0x08, 0x00, 0x00, 0x00, 0xC2, 0x01, 0x00, 0x07, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x7E//set baud rate to 115200
    }; 
  int fd = open("/dev/ttymxc1", O_RDWR);//read+write and block to open
  fd = write(fd, adj_GPS,sizeof(adj_GPS));
  printf("settty done: %d, errno: %d\n", fd, errno);
  close(fd);
}

namespace loc{
  GpsGiver::GpsGiver(void)
  {
    // set_tty();

    // system("stty -F /dev/ttymxc1 speed 115200");

    _ubloxHandle = open("/dev/ttymxc1", O_RDONLY);//O_RDONLY---ReadOnly;O_WRONLY---WriteOnly;O_RDWR---Read&Write;O_NONBLOCK---no block;

    if(_ubloxHandle<0)
    {
      printf("init error\n");
      exit(1);
    }

    tv.tv_sec = 1;
    tv.tv_usec = 0;
  }

  GpsGiver::~GpsGiver(void)
  {
    close(_ubloxHandle);
  }

  bool GpsGiver::readGPS(void)
  {
    memset(_buf, 0, sizeof(_buf));
    memset(_inbuf, 0, sizeof(_inbuf));
    int c = 0;

    do
    {
      FD_ZERO( &readset );
      if( _ubloxHandle >= 0 )
      FD_SET( _ubloxHandle, &readset );
      tv.tv_sec = 0;
      tv.tv_usec = 500000;
      do
      {
        z = select( _ubloxHandle + 1, &readset, 0, 0, &tv);
      }while( z==-1 && errno==EINTR );
      if( z == -1 )
        printf("select(2)\n");
      if( z == 0 )
      {
        _ubloxHandle = -1;
      }
  
      if( _ubloxHandle>=0 && FD_ISSET(_ubloxHandle, &readset) )
      {
        z = read( _ubloxHandle, _buf, sizeof(_buf) - c );
        c += z;
        if( z == -1 )
        {
          _ubloxHandle = -1;
        }
        if( z > 0 )
        {

          _buf[ z + 1 ] = '\0';
          strncat(_inbuf, _buf, sizeof(_buf) - 1);
          memset(_buf, 0x00, sizeof(_buf));
        }
        else
        {
          _ubloxHandle = -1;
        }
      }
   }while( _ubloxHandle >= 0 );
   memcpy( pData, _inbuf, c );

   printf("ublox gps data: %s\n", pData);
    

    // if(/*_buf[3] == 'R' && _buf[4] == 'M' && */_buf[5] == 'C')
    // {      
    //   memset(_latitude,0,sizeof(_latitude));
    //   memset(_longitude,0,sizeof(_longitude));
    //   memset(_bearing, 0, sizeof(_bearing));

    //   strncpy(_latitude, _buf+19, sizeof(_latitude));
    //   strncpy(_longitude, _buf+32, sizeof(_longitude));
    //   strncpy(_bearing, _buf+51, sizeof(_bearing));

    //   return true;
    // }
    // /*
    // else if(_buf[3] == 'G' && _buf[4] == 'G' && _buf[5] == 'A')
    // {
    //   memset(_accuracy, 0, sizeof(_accuracy));

    //   strncpy(_accuracy, _buf+49, sizeof(_accuracy));

    //   return true;
    // }*/
    // else
    // {
    //   return false;
    // }
    return true;

  }

  bool GpsGiver::unpackNMEA(void)
  {
    InitSerailPort("/dev/ttymxc1");
    ReadNMEAThread();
  }
/*
  bool GpsGiver::resoRMCGGAGLL(void)
  {
    if(_buf[3] == 'R' && _buf[4] == 'M' && _buf[5] == 'C')
    {      
      memset(_latitude,0,sizeof(_latitude));
      memset(_longitude,0,sizeof(_longitude));
      memset(_accuracy, 0, sizeof(_accuracy));
      memset(_bearing, 0, sizeof(_bearing));

      strncpy(_latitude, _buf+19, sizeof(_latitude));
      strncpy(_longitude, _buf+32, sizeof(_longitude));
      strncpy(_accuracy, _buf+45, sizeof(_accuracy));
      strncpy(_bearing, _buf+51, sizeof(_bearing));

      return true;
    }
    else
    {
      return false;
    }
   
  }
*/

  double GpsGiver::getLatitude(void)
  {
    double temp_point = 0.0;
    double rslt = 0.0;
    if(_latitude[0] == '3')
    {
      rslt = atof(_latitude) / 100.0;
      temp_point = rslt - (int)rslt;
      rslt = (int)rslt + temp_point/0.60;
      return rslt;
    }
    else
      return 0.0;
  }

  double GpsGiver::getLongitude(void)
  {
    double temp_point = 0.0;
    double rslt = 0.0;
    if(_longitude[0] == '1')
    {
      rslt = atof(_longitude) / 100.0;
      temp_point = rslt - (int)rslt;
      rslt = (int)rslt + temp_point/0.60;
      return rslt;
    }
    else
      return 0.0;
  }

  double GpsGiver::getBearing(void)
  {
    if(_bearing != NULL)
      return (atof(_bearing) / 100.0);
    else
      return 0.0;
  }

  double GpsGiver::getAccuracy(void)
  {
    if(_accuracy != NULL)
      return atof(_accuracy);
    else
      return 0.0;
  }
}