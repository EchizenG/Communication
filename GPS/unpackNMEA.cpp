#include "stdafx.h"  
#include "unpackNMEA.hpp"  
  
#define SATTATOLNUMBER  32  
  
// 用于在 GPS 监控界面显示 NMEA 信息  
char gcBuff[4096];  
  
CString gcsTime;  
CString gcsDate;  
int gdSignalNumber[SATTATOLNUMBER];  
CString csLat;  
CString csLatdir;  
CString csLon;  
CString csLondir;  
CString csAltitude;  
CString csSpeed;  
CString csOrientation;  
int nNumDisplayed;  
int giGSVSatNumber;  
  
HANDLE ghCommHandle;  
HANDLE nmeathread_hand;                         // Global handle to the NMEA reading thread  
  
CString gcsGPSState;    //定位: A; 导航: V  
CString gcsTimeOp;  
CString gcsLatField;  
CString gcsLonField;  
int giResult;  
  
int giSalNumber;  
int giGSVCurrentPackage;  
  
int giHourDiff;  
  
BOOL InitSerailPort(CString csSerialPort,LPVOID pParent)  
{  
    DCB commDCB;  
    COMMTIMEOUTS timeouts;  
  
    ghCommHandle = CreateFile(csSerialPort, GENERIC_READ | GENERIC_WRITE, 0, NULL,   
        OPEN_EXISTING, FILE_ATTRIBUTE_SYSTEM, NULL);  
    if(INVALID_HANDLE_VALUE == ghCommHandle)  
    {  
        RETAILMSG(1, (TEXT("Opening GPS %s failed: %d!\r\n"),csSerialPort,(int)GetLastError()));  
        return FALSE;  
    }  
  
    commDCB.DCBlength = sizeof (DCB);  
    if(!GetCommState(ghCommHandle,&commDCB))  
    {  
        CloseHandle(ghCommHandle);  
        ghCommHandle = INVALID_HANDLE_VALUE;  
         RETAILMSG(1,(L"Failed in getting GPS %s DCB settings: %d!\r\n",csSerialPort,(int)GetLastError()));  
        return FALSE;  
    }  
    commDCB.DCBlength = sizeof(DCB);   
    commDCB.BaudRate = 9600;             // Current baud  
    commDCB.ByteSize = 8;                 // Number of bits/bytes, 4-8  
    commDCB.Parity = NOPARITY;            // 0-4=no,odd,even,mark,space   
    commDCB.StopBits = ONESTOPBIT;        // 0,1,2 = 1, 1.5, 2  
  
    // Setting serial port to Centrality speicifcations  
    if(!SetCommState(ghCommHandle,&commDCB))  
    {  
        CloseHandle(ghCommHandle);  
        ghCommHandle = INVALID_HANDLE_VALUE;  
        RETAILMSG(1,(L"Error in trying to set GPS %s DCB settings: %d!\r\n",csSerialPort,(int)GetLastError()));  
        return FALSE;  
    }  
  
    // Get the default timeout settings for port  
    if(!GetCommTimeouts(ghCommHandle, &timeouts))  
    {  
        CloseHandle(ghCommHandle);  
        ghCommHandle = INVALID_HANDLE_VALUE;  
        RETAILMSG(1,(L"Failed in getting GPS %s timeout settings: %d!\r\n",csSerialPort,(int)GetLastError()));  
        return FALSE;  
    }  
    // RETAILMSG(1,(L"%s DCB set successfully!\r\n",csSerialPort));  
  
    // Change the timeouts structure settings to Centrality settings  
    timeouts.ReadIntervalTimeout = 500;  
    timeouts.ReadTotalTimeoutMultiplier = 0;  
    timeouts.ReadTotalTimeoutConstant = 0;  
  
    // Set the time-out parameters for all read and write operations on the port.   
    if(!SetCommTimeouts(ghCommHandle,&timeouts))  
    {  
        CloseHandle(ghCommHandle);  
        ghCommHandle = INVALID_HANDLE_VALUE;  
        RETAILMSG(1,(L"Error in trying to set GPS %s timeout settings: %d!\r\n",csSerialPort,(int)GetLastError()));  
        return FALSE;  
    }  
  
    if(0 != SetCommMask(ghCommHandle,EV_RXCHAR))  
    {  
        RETAILMSG(1,(L"==Set %s mask OK!\r\n",csSerialPort));  
    }  
    else  
    {  
        RETAILMSG(1,(TEXT("==Set %s mask failure:%d!\r\n"),csSerialPort,GetLastError()));  
    }  
  
    nmeathread_hand = CreateThread(NULL,0,ReadNMEAThread,pParent,0,NULL);  
    //nmeathread_hand = CreateThread(NULL, 0, ReadNMEAThread, this, CREATE_SUSPENDED, NULL);  
    if(!nmeathread_hand)  
    {  
        RETAILMSG(1, (L"Could not create NMEA read thread.\r\n"));  
        return FALSE;  
    }  
    else  
    {  
        //SetThreadPriority(nmeathread_hand, THREAD_PRIORITY_BELOW_NORMAL);  
        //ResumeThread(nmeathread_hand);  
    }  
  
    SYSTEMTIME stUTC;  
    SYSTEMTIME stLocal;  
  
    GetLocalTime(&stLocal);  
    GetSystemTime(&stUTC);  
    giHourDiff = stLocal.wHour - stUTC.wHour;  
  
    return TRUE;  
}  
  
// nmeathread_hand = CreateThread(NULL, 0, ReadNMEAThread, this, 0, NULL);  
DWORD WINAPI ReadNMEAThread(LPVOID lpParameter)  
{  
    //DECLARE_USER_MESSAGE(UWM_NEW_NMEA);  
  
    int start, endline, onestart, oneend, linelen, degdig, iPos;  
    ULONG bytesRead;  
    DWORD EventMask = EV_RXCHAR;  
    CString field;  
    TCHAR *stopstring;  
  
    static int iCount = 0;  
  
    CWnd *mpNmea;  
    mpNmea = (CWnd*)lpParameter;  
  
    // Wait on the event  
    while(WaitCommEvent(ghCommHandle, &EventMask, NULL))  
    {  
        // RETAILMSG(1,(TEXT("---------------------------------------------ReadNMEAThread:%d,Tick: %d\r\n"),iCount++,GetTickCount()));  
        // Clear the buffer before you start reading  
        memset(gcBuff, 0, 4096);  
  
        // Read from serial port (4b)  
        if (ReadFile(ghCommHandle, gcBuff, 4096, &bytesRead, NULL))  
        {  
            if(bytesRead == 0)  
                continue;  
  
            CString dacstr(gcBuff); /*Leo:从串口读GPS卫星数据*/  
  
            start = 0;  
            endline = 0;  
  
            // Parse/Process the output (4c)  
            while(1)  
            {  
                int i = 0;  
                start = dacstr.Find(L"$G", start);  
                if(start < 0)  
                    break;  
                endline = dacstr.Find(L"\r\n", start);  
                if(endline < 0)  
                    break;  
  
                linelen = endline - start;  
                //DebugOutput(MSG_DEBUG, "GPSViewer msg: start = %d endline = %d length = %d", start, endline, linelen);      
  
                // Extract one line  
                CString oneline;  
  
                oneline = dacstr.Mid(start, linelen);  
#if _DEBUG  
                // RETAILMSG(1,(TEXT("*******************************GPSViewer msg: Oneline = %s\r\n"),oneline));  
#endif  
  
                onestart = 0;  
                oneend = 0;  
                i = 0;  
                //////////////////////////////////////////////////////////////////////////  
                //$GPRMC,075017.31,V,2232.6057,N,11356.3074,E,,,190708,,W,N*15  
                //$GPRMC,080525.82,A,2232.5196,N,11356.3719,E,,,190708,,W,A*08  
                if(oneline.Left(6) == L"$GPRMC")  
                {  
                    while((iPos = oneline.Find(L",")) >= 0)  
                    {  
                        field = oneline.Left(iPos);  
                        i ++;  
                        oneline = oneline.Mid(iPos + 1);  
  
                        if(3 == i)  
                        {  
                            gcsGPSState = field;  
                        }  
                        else if(10 == i)        // <9>当前UTC日期ddmmyy 格式 - 例如: 030222  
                        {  
                            gcsDate = field;  
                        }  
                    }  
                }  
                //////////////////////////////////////////////////////////////////////////  
                //$GPGGA,080514.82,2232.5203,N,11356.3719,E,1,6,1.327,239.386,M,,M,,*4D  
                else if(oneline.Left(6) == L"$GPGGA")  
                {  
                    while((iPos = oneline.Find(L",")) >= 0)  
                    {  
                        static int iOrientation = 0;  
  
                        field = oneline.Left(iPos);  
                        i ++;  
                        oneline = oneline.Mid(iPos + 1);  
                        if (i == 2)  
                        {  
                            //////////////////////////////////////////////////////////////////////////  
                            if(iOrientation != giResult)  
                            {  
                                RETAILMSG(1,(L"[GPS]Status of GPS is changed: %d(Old: %d)\r\n",giResult,iOrientation));  
                                iOrientation = giResult;  
                            }  
                            //////////////////////////////////////////////////////////////////////////  
                            gcsTimeOp = field;  
                            // 将格式从: --:--:--.-- 修改为: --:--:-- ,即不显示秒后的数据 ---Leo 2009-03-26  
                            CString csTmp = gcsTimeOp.Right(5);  
                            CString csHour = gcsTimeOp.Left(2);  
                            int iHour = _wtoi(csHour);  
  
                            if(iHour + giHourDiff < 24)  
                            {  
                                csHour.Format(L"%d",iHour + giHourDiff);  
                            }  
                            else  
                            {  
                                csHour.Format(L"%d",((iHour + giHourDiff) - 24));  
                            }  
                            // gcsTime = csHour + L":" + time.Mid(2,2) + L":" + csTmp.Left(2);  
                            gcsTime = csHour + gcsTimeOp.Mid(2,2) + csTmp.Left(2);  
                        }  
                        else if (i == 3)          
                        {   //Get Latitude from GGA - Value may not be valid.  Check flag to be sure.  
                            gcsLatField = field;  
                            degdig = gcsLatField.GetLength() - 2;  
                            csLat = gcsLatField.Left(2) + CString(" ") + gcsLatField.Right(degdig);  
                        }  
                        else if (i == 4)          
                        {   //Get Latitude Direction (N,S) from GGA - Value may not be valid.  Check flag to be sure.  
                            csLatdir = field;  
                        }             
                        else if (i == 5)  
                        {   //Get Longitude from GGA - Value may not be valid.  Check flag to be sure.  
                            gcsLonField = field;  
                            degdig = gcsLonField.GetLength() - 3;  
                            csLon = gcsLonField.Left(3) + CString(" ") + gcsLonField.Right(degdig);  
                        }  
                        else if (i == 6)  
                        {   //Get Longitude Direction (E,W) from GGA - Value may not be valid.  Check flag to be sure.  
                            csLondir = field;  
                        }  
                        else if (i == 7)    //<6>GPS状态批示0-未定位 1-无差分定位信息 2-带差分定位信息  
                        {   //Get Flag from GGA indicating position fix.  Position output from GGA is valid.  
                            giResult = atoi((const char*)((LPCTSTR)field));  
                            if(0 == giResult)  
                            {  
                                // RETAILMSG(1,(TEXT("===No orientation\r\n")));  
                            }  
                            else if(1 == giResult)  
                            {  
                                // RETAILMSG(1,(TEXT("===Orientation with no difference\r\n")));  
                            }  
                            else if(2 == giResult)  
                            {  
                                // RETAILMSG(1,(TEXT("===Orientation with difference\r\n")));  
                            }  
                        }  
                        else if (i == 10)  
                        {  
                            csAltitude = field;  
                        }  
                    }  
                }  
                //$GPGSV,3,3,11,29,21,93,36,30,33,40,36,31,49,324,*46  
                else if (oneline.Left(6) == L"$GPGSV")  
                {  
                    while((iPos = oneline.Find(L",")) >= 0)  
                    {  
                        field = oneline.Left(iPos);  
                        i ++;  
                        oneline = oneline.Mid(iPos + 1);  
  
                        if(3 == i)  
                        {  
                            if(_ttoi(field) > 0)  
                            {  
                                giGSVCurrentPackage = _ttoi(field);  
                                if (giGSVCurrentPackage == 1) // new GSV sentence  
                                {  
                                    nNumDisplayed = 0;  
                                    for(int j = 0;j < SATTATOLNUMBER;j++)  
                                    {  
                                        gdSignalNumber[j] = -1;  
                                    }  
                                }  
                            }  
                        }  
                        else if(4 == i)  
                        {  
                            if(_tcstod(field, &stopstring) > 0)  
                            {  
                                giGSVSatNumber = (int)_tcstod(field, &stopstring);  
                            }  
                        }  
                        else if(0 == (i - 5) % 4) //卫星的PRV号星号  
                        {  
                            if (_ttoi(field) > 0)  
                            {  
                                giSalNumber = _ttoi(field);  
                            }                 
                        }  
                        else if(3 == (i - 5) % 4) // SNR  
                        {  
                            if (_ttoi(field) > 0)  
                            {  
                                gdSignalNumber[giSalNumber-1] = _ttoi(field);  
                                nNumDisplayed++;  
                            }  
                        }  
                    }  
                    if ((iPos = oneline.Find(L"*")) >= 0) // last sat  
                    {  
                        field = oneline.Left(iPos);  
                        i ++;  
                        if(3 == (i - 5) % 4) // SNR  
                        {  
                            if (_ttoi(field) > 0)  
                            {  
                                gdSignalNumber[giSalNumber-1] = _ttoi(field);  
                                nNumDisplayed++;  
                            }  
                        }  
                    }  
                }  
                //$GPVTG,<1>,T,<2>,M,<3>,N,<4>,K,<5>*hh   
                // <1> 对地航向（单位：度）  
                // <2> 磁偏角（单位：度）  
                // <3> 对地航速（单位：哩/小时）  
                // <4> 地面速率(0000.0~1851.8公里/小时，前面的0也将被传输)  
                //如: $GPVTG,359.95,T,,M,15.15,N,28.0,K,A*04   
                else if(oneline.Left(6) == L"$GPVTG")  
                {  
                    while((iPos = oneline.Find(L",")) >= 0)  
                    {  
                        field = oneline.Left(iPos);  
                        i ++;  
                        oneline = oneline.Mid(iPos + 1);  
                        if(2 == i)  
                        {  
                            csOrientation = field;  
                        }  
                        else if(8 == i)  
                        {  
                            csSpeed = field;  
                        }  
                    }  
                }  
                start = endline + 2;  
            } // end of buffer processing  
        }   
        //Sleep(1000);// end of ReadFile  
    } // end of WaitCommEvent  
    {  
        // RETAILMSG(1,(TEXT("Exit GPS thread:%d"),GetLastError()));  
    }  
  
    return 0;  
}// end of ReadNMEAThread  
  
  
//csDate: 01-01-03  
//csUTCTime: 04:07:44.08  
void SetSystemTimeFormUTC(CString csDate,CString csUTCTime)  
{  
    SYSTEMTIME st;  
    int iHour = 0;  
    int iMinute = 0;  
    int iSecond = 0;  
    int iYear = 0;  
    int iMonth = 0;  
    int iDay = 0;  
    CString csSubString;  
    TCHAR *stopstring;  
    // RETAILMSG(1,(TEXT("====Set system time from UTC.Date: %s, Time: %s"),csDate,csUTCTime));  
  
    GetSystemTime(&st);  
  
    csSubString = csDate.Left(2);  
    iDay = (int)_tcstod(csSubString, &stopstring);  
  
    csSubString = csDate.Mid(3,2);  
    iMonth = (int)_tcstod(csSubString, &stopstring);  
  
    csSubString = csDate.Right(2);  
    iYear = 2000 + (int)_tcstod(csSubString, &stopstring);  
  
    st.wYear = iYear;  
    st.wMonth = iMonth;  
    st.wDay = iDay;  
  
    csSubString = csUTCTime.Left(2);  
    iHour = (int)_tcstod(csSubString, &stopstring);   
  
    csSubString = csUTCTime.Mid(3,2);  
    iMinute = (int)_tcstod(csSubString, &stopstring);  
  
    csSubString = csUTCTime.Mid(6,2);  
    iSecond = (int)_tcstod(csSubString, &stopstring);  
  
    st.wHour = iHour;  
    st.wMinute = iMinute;  
    st.wSecond = iSecond;  
  
    SetSystemTime(&st);  
}  
  
void DeinitSerialPort(void)  
{  
    SetCommMask(ghCommHandle,0);  
  
    if(nmeathread_hand)  
    {  
        TerminateThread(nmeathread_hand,1);  
        CloseHandle(nmeathread_hand);  
    }  
    if(INVALID_HANDLE_VALUE != ghCommHandle)  
    {  
        EscapeCommFunction(ghCommHandle,CLRDTR);  
        EscapeCommFunction(ghCommHandle,CLRRTS);  
        //清除驱动程序内部的发送和接收队列  
        PurgeComm(ghCommHandle,PURGE_TXCLEAR|PURGE_RXCLEAR);  
  
        CloseHandle(ghCommHandle);  
        ghCommHandle = INVALID_HANDLE_VALUE;  
    }  
}