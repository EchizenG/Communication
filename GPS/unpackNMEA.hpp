#ifndef _GPS_MONITER_HH_  
#define _GPS_MONITER_HH_  
  
BOOL InitSerailPort(CString csSerialPort,LPVOID pParent = NULL);  
DWORD WINAPI ReadNMEAThread(LPVOID lpParameter);  
void SetSystemTimeFormUTC(CString csDate,CString csUTCTime);  
  
void DeinitSerialPort(void);  
  
#endif