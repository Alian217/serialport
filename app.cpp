/////////////////////////////////  Includes  //////////////////////////////////

#include "stdafx.h"
#include "SerialPort.h"


///////////////////////////////// Defines /////////////////////////////////////

#ifdef CSERIALPORT_MFC_EXTENSIONS
#ifdef _DEBUG
#define new DEBUG_NEW
#endif //#ifdef _DEBUG
#endif //#ifdef CSERIALPORT_MFC_EXTENSIONS


//////////////////////////////// Implementation ///////////////////////////////


VOID WINAPI CompletionRoutine(_In_ DWORD dwErrorCode, _In_ DWORD dwNumberOfBytesTransfered, _Inout_ LPOVERLAPPED lpOverlapped)
{
  UNREFERENCED_PARAMETER(dwErrorCode);
  UNREFERENCED_PARAMETER(dwNumberOfBytesTransfered);
  UNREFERENCED_PARAMETER(lpOverlapped);
}



#ifdef CSERIALPORT_MFC_EXTENSIONS
class CSerialPortApp : public CWinApp
{
public:
  virtual BOOL InitInstance();
};

CSerialPortApp theApp;

BOOL CSerialPortApp::InitInstance()
#else
int CALLBACK _tWinMain(_In_ HINSTANCE /*hInstance*/, _In_opt_ HINSTANCE /*hPrevInstance*/, _In_ LPTSTR /*lpCmdLine*/, _In_ int /*nShowCmd*/)
#endif //#ifdef CSERIALPORT_MFC_EXTENSIONS
{
  //Please note that all the following is just test code to exercise the CSerialPort API. It assumes
  //that a COM1 is available on the local computer.

  BYTE* pBuf = new BYTE[10000];
  memset(pBuf, '%', 10000);
  HANDLE hEvent = NULL;
  int nPortToUse = 1;

  try
  {
    COMMCONFIG config;
    CSerialPort::GetDefaultConfig(nPortToUse, config);

    CSerialPort port;
    port.Open(nPortToUse, 1200, CSerialPort::NoParity, 8, CSerialPort::OneStopBit, CSerialPort::XonXoffFlowControl);

    HANDLE hPort = port.Detach();
    port.Attach(hPort);

    DWORD dwModemStatus;
    port.GetModemStatus(dwModemStatus);

    DCB dcb;
    port.GetState(dcb);

    dcb.BaudRate = 9600;
    port.SetState(dcb);    

    DWORD dwErrors;                      
    port.ClearError(dwErrors);

    port.SetBreak();
    port.ClearBreak();

    COMSTAT stat;
    port.GetStatus(stat);

    COMMTIMEOUTS timeouts;
    port.GetTimeouts(timeouts);

    port.Setup(10000, 10000);

    port.GetConfig(config);

    config.dcb.BaudRate = 9600;
    port.SetConfig(config);

    port.Set0WriteTimeout();
    port.Set0ReadTimeout();

    char sBuf[] = "This should appear on the serial port";
    port.Write(sBuf, static_cast<DWORD>(strlen(sBuf)));

    DWORD dwMask;
    port.GetMask(dwMask);

    port.SetMask(EV_TXEMPTY); 

    //port.WaitEvent(dwMask);

    port.TerminateOutstandingWrites();

    port.TransmitChar('p');

    port.Set0Timeout();

    char sRxBuf[10];
    DWORD dwRead = port.Read(sRxBuf, 10);
    UNREFERENCED_PARAMETER(dwRead);

    port.TerminateOutstandingReads();

    port.ClearDTR();

    port.ClearRTS();

    port.SetDTR();

    port.SetRTS();

    //port.SetXOFF();

    //port.SetXON();

    COMMPROP properties;
    memset(&properties, 0, sizeof(properties));
    port.GetProperties(properties);

    port.ClearWriteBuffer();

    port.ClearReadBuffer();

    port.Flush();

    port.CancelIo();

    port.Close();


    //Try out the overlapped functions
    CSerialPort port2;
    port2.Open(nPortToUse, 9600, CSerialPort::NoParity, 8, CSerialPort::OneStopBit, CSerialPort::XonXoffFlowControl, TRUE);

    OVERLAPPED overlapped;
    memset(&overlapped, 0, sizeof(overlapped));
    hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    ATLASSERT(hEvent != NULL);
    overlapped.hEvent = hEvent;

    try
    {
      port2.Write(pBuf, 10000, overlapped);
    }
  #ifdef CSERIALPORT_MFC_EXTENSIONS
    catch(CSerialException* pEx)
    {
      if (pEx->m_dwError == ERROR_IO_PENDING)
      {
        DWORD dwBytesTransferred = 0;
        port2.GetOverlappedResult(overlapped, dwBytesTransferred, TRUE);
        pEx->Delete();
      }
      else
      {
        DWORD dwError = pEx->m_dwError;
        pEx->Delete();
        CSerialPort::ThrowSerialException(dwError);
      }
    }
  #else
    catch(CSerialException& e)
    {
      if (e.m_dwError == ERROR_IO_PENDING)
      {
        DWORD dwBytesTransferred = 0;
        port2.GetOverlappedResult(overlapped, dwBytesTransferred, TRUE);
      }
      else
      {
        CSerialPort::ThrowSerialException(e.m_dwError);
      }
    }
  #endif
    try
    {
      port2.Read(pBuf, 10, overlapped);
    }
  #ifdef CSERIALPORT_MFC_EXTENSIONS
    catch(CSerialException* pEx)
    {
      if (pEx->m_dwError == ERROR_IO_PENDING)
      {
        DWORD dwBytesTransferred = 0;
        port2.GetOverlappedResult(overlapped, dwBytesTransferred, TRUE);
        pEx->Delete();
      }
      else
      {
        DWORD dwError = pEx->m_dwError;
        pEx->Delete();
        CSerialPort::ThrowSerialException(dwError);
      }
    }
  #else
    catch(CSerialException& e)
    {
      if (e.m_dwError == ERROR_IO_PENDING)
      {
        DWORD dwBytesTransferred = 0;
        port2.GetOverlappedResult(overlapped, dwBytesTransferred, TRUE);
      }
      else
      {
        CSerialPort::ThrowSerialException(e.m_dwError);
      }
    }
  #endif
          
    port2.SetMask(EV_TXEMPTY); 

    port2.WriteEx(sBuf, static_cast<DWORD>(strlen(sBuf)), &overlapped, CompletionRoutine);
    SleepEx(INFINITE, TRUE);
    port2.ReadEx(pBuf, 10, &overlapped, CompletionRoutine);
    SleepEx(INFINITE, TRUE);
  }
#ifdef CSERIALPORT_MFC_EXTENSIONS
  catch (CSerialException* pEx)
  {
    ATLTRACE(_T("Unexpected CSerialPort exception, Error:%u,%s\n"), pEx->m_dwError, pEx->GetErrorMessage().operator LPCTSTR());
    pEx->Delete();
  }
#else
  catch (CSerialException& e)
  {
    ATLTRACE(_T("Unexpected CSerialPort exception, Error:%u\n"), e.m_dwError);
    UNREFERENCED_PARAMETER(e);
  }
#endif //#ifdef CSERIALPORT_MFC_EXTENSIONS

  //Clean up the resources we have been using
  delete [] pBuf;
  if (hEvent != NULL)
  {
    CloseHandle(hEvent);
    hEvent = NULL;
  }

#ifdef CSERIALPORT_MFC_EXTENSIONS
  return FALSE;
#else
  return 0;
#endif
}
