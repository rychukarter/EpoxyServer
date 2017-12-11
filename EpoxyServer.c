//#include <tcpsupp.h>
#include <cvirte.h>		
#include <userint.h>
#include "EpoxyServer.h"

#include "Server.h"
#include "Device.h"

#define SERVER_PORT_NUM 10000

/******************************************************
*  TODO:											  * 
*	Format code										  *
*	Auto COM detection								  *
*	Flag Pharse										  *
*	ComErr											  *
*	Blocking unsafe operations						  *
*	Propts for Port etc					       		  *
*												      *
*	M- QUIT problem									  *
*	M- Better printfs								  *
*	M- DeviceList insteead of global or single client *
******************************************************/
static int panelHandle;

static int CVICALLBACK ServerCallback(unsigned handle, int xType, int errCode, void *callbackData);
static int ConnectDevice(int comPort);
static int DisconnectDevice();
static void CVICALLBACK ComPortReciveCallback(int portNumber, int eventMask, void *callbackdata);

int main (int argc, char *argv[])
{
	int tcpErr = 0;
	
	if (InitCVIRTE (0, argv, 0) == 0)
		return -1;
	if ((panelHandle = LoadPanel (0, "EpoxyServer.uir", PANEL)) < 0)
		return -1;
	
	if(ConnectDevice(7) < 0)
		return -1;
	if((tcpErr = RegisterTCPServer(SERVER_PORT_NUM, ServerCallback, NULL)) < 0)
		ReportTCPError (tcpErr);

	DisplayPanel (panelHandle);
	RunUserInterface ();
	
	CleanUpTCPConnections();
	DisconnectDevice();
	DiscardPanel (panelHandle);
	return 0;
}

static int ConnectDevice(int comPort)
{
	int 					cmtErr = 0; 
	DeviceStatusPtr			deviceStatusPtr = 0;
	
	deviceStatusPtr = &GlobalDeviceStatus;
	memset(&GlobalDeviceStatus, 0, sizeof GlobalDeviceStatus);

	if(OpenComConfig(comPort, "", 256000, 0, 8, 1, 512, 512) >= 0)
	{
		deviceStatusPtr->comPortNum = comPort;
		deviceStatusPtr->comPortOpenedFlag = 1;
		
		if(InstallComCallback (comPort, (LWRS_RECEIVE | LWRS_ERR), 13, 0x44, ComPortReciveCallback, NULL) < 0)
			return -1;
		
		if((cmtErr=CmtScheduleThreadPoolFunction (DEFAULT_THREAD_POOL_HANDLE, DeviceThreadFunction, deviceStatusPtr, &deviceStatusPtr->threadFuncId)) < 0)
			ReportCMTError(cmtErr);
	}
	else
	{
		CloseCom(comPort);
		return -1;
	}
	return 0;
}

static int DisconnectDevice()
{
	int comPortError = 0;
	DeviceStatusPtr	deviceStatusPtr = 0;
	deviceStatusPtr = &GlobalDeviceStatus;	
	
	comPortError = CloseCom (deviceStatusPtr->comPortNum);
	if(comPortError >= 0)
	{
		deviceStatusPtr->comPortNum = 0;
		deviceStatusPtr->comPortOpenedFlag = 0;
	}
	return 0;	
}

static int CVICALLBACK ServerCallback(unsigned handle, int xType, int errCode, void *callbackData)
{
	if(xType == TCP_CONNECT)
	{
		ConnectClient(handle);
	}
	else if(xType == TCP_DATAREADY)
	{
		ResolveClientData(handle);
	}
	else if(xType == TCP_DISCONNECT)
	{
		DisconnectClient(handle);
	}
	
	return 0;
}

static void CVICALLBACK ComPortReciveCallback(int portNumber, int eventMask, void *callbackdata)
{
	char readBuf[256] = {0};
	int strLen;
	
	switch (eventMask){
		case LWRS_RECEIVE:
			
			strLen = GetInQLen (portNumber);
			
			ComRd (portNumber, readBuf, strLen);
			SendData(readBuf, GlobalDeviceStatus.TCPHandle);
			
			break;
		}

	return;
}

int CVICALLBACK Quit (int panel, int control, int event,
					  void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			QuitUserInterface(0);  
			break;
	}
	return 0;
}
