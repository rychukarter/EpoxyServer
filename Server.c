//==============================================================================
//
// Title:		Server.c
// Purpose:		A short description of the implementation.
//
// Created on:	2017-08-24 at 22:59:38 by Karol Rychter.
// Copyright:	Politechnika Warszawska. All Rights Reserved.
//
//==============================================================================
#include <toolbox.h>
#include <userint.h>
#include <tcpsupp.h>
#include <cvirte.h>
#include "Server.h"
#include "Device.h"
#include "EpoxyServer.h"


static int CVICALLBACK ClientThreadFunction (void *data)
{
	ClientInfoPtr clientInfoPtr = (ClientInfoPtr) data;
	
	clientInfoPtr->threadId = CmtGetCurrentThreadID ();
	clientInfoPtr->threadInited = 1;

	while (!clientInfoPtr->stopFlag)
		{
		ProcessSystemEvents ();
		}
		
	return 0;
}

extern int ConnectClient(unsigned int handle)
{
	int 			tcpErr = 0;
	int 			cmtErr = 0; 
	ClientInfoPtr	clientInfoPtr = 0;
	char			peerName[64], peerAddress[32];

	clientInfoPtr = calloc (1, sizeof (ClientInfo));
	if (clientInfoPtr == NULL)
		return -1;
	clientInfoPtr->handle = handle;
	GlobalDeviceStatus.TCPHandle = handle;
	
	if(clientList == 0)
		clientList = ListCreate(sizeof (ClientInfoPtr));
	
	if((tcpErr = GetTCPPeerName(handle, peerName, sizeof (peerName))) < 0)
		ReportTCPError (tcpErr);
	else
		sprintf (clientInfoPtr->name, peerName);		
	if((tcpErr = GetTCPPeerAddr(handle, peerAddress, sizeof (peerAddress))) < 0)
		ReportTCPError (tcpErr);	
	else
		sprintf (clientInfoPtr->address, peerAddress);
	
	if((cmtErr = CmtScheduleThreadPoolFunction (DEFAULT_THREAD_POOL_HANDLE, ClientThreadFunction, clientInfoPtr, &clientInfoPtr->threadFuncId)) < 0)
		ReportCMTError(cmtErr);  		

	ListInsertItem (clientList, &clientInfoPtr, END_OF_LIST);

	return 0;
}

extern int ResolveClientData(unsigned int handle)
{

	ClientInfo		clientInfo = {0};
	ClientInfoPtr	clientInfoPtr = &clientInfo;
	size_t			index;

	clientInfoPtr->handle = handle;
	index = ListFindItem (clientList, &clientInfoPtr, FRONT_OF_LIST, CompareClientInfoPtr);
	if (index > 0)
	{
		ListGetItem (clientList, &clientInfoPtr, index);
		if (clientInfoPtr->threadInited && !clientInfoPtr->readingData)
		{
			clientInfoPtr->readingData = 1;
			PostDeferredCallToThread (DeferredReceive, clientInfoPtr, clientInfoPtr->threadId);
		}
	}	

	return 0;	
}
				
static void CVICALLBACK DeferredReceive (void *data)
{
	ClientInfoPtr	clientInfoPtr = (ClientInfoPtr) data;
	char			dataBuf[256];
	char			sendBuf[256];
	int				bytesRead;
	
	assert (clientInfoPtr->readingData == 1);
	
	DisableBreakOnLibraryErrors ();
	while (1)
	{
		bytesRead = ServerTCPRead (clientInfoPtr->handle, dataBuf, sizeof (dataBuf) - 1, 100);
		if (bytesRead > 0)
		{
			dataBuf[bytesRead] = '\0';
			if(dataBuf[0] == 'D' && dataBuf[1] == 'E' && dataBuf[2] == 'V')
			{
				if(dataBuf[3] == 'C' && dataBuf[4] == 'M' && dataBuf[5] == 'D')
					PostDeferredCallToThread(SetDeviceCommand, dataBuf, GlobalDeviceStatus.threadId);
				
				else if(dataBuf[3] == 'S' && dataBuf[4] == 'T' && dataBuf[5] == 'A')
				{
					GetDeviceStatus(sendBuf);
					DeferredSend(sendBuf);
				}
				else if(dataBuf[3] == 'M' && dataBuf[4] == 'E' && dataBuf[5] == 'A')
				{
					GetMeasurementSettings(sendBuf);
					DeferredSend(sendBuf);
				}
			}
		}
		else
		{
			clientInfoPtr->readingData = 0;
			break;
		}
	}
	EnableBreakOnLibraryErrors ();	
}

static void CVICALLBACK DeferredSend (void *data)
{
	char			*dataBuf = (char*) data;
	int				dataBufSize = strlen(dataBuf);
	
	if (dataBufSize > 0)
	{
		if (dataBuf)
		{
			char *currData = dataBuf;
			while (dataBufSize > 0)
			{
				//printf("\n\nCURRENTDATA: %s\n\n", currData);
				int bytesSent = ServerTCPWrite (GlobalDeviceStatus.TCPHandle, currData, dataBufSize, 0);
				if (bytesSent >= 0)
				{
					dataBufSize -= bytesSent;
					currData += bytesSent;
				}
			}
		}
	}
}

extern int SendData(void *data, unsigned int handle)
{
	ClientInfo		clientInfo = {0}; 
	ClientInfoPtr	clientInfoPtr = &clientInfo;
	size_t			index;

	clientInfoPtr->handle = handle;
	index = ListFindItem (clientList, &clientInfoPtr, FRONT_OF_LIST, CompareClientInfoPtr);
	if (index > 0)
	{
		ListGetItem (clientList, &clientInfoPtr, index);
		if (clientInfoPtr->threadInited)
		{
			PostDeferredCallToThread (DeferredSend, data, clientInfoPtr->threadId);
		}
	}	
	return 0;	
}

static int Disconnect (ClientInfoPtr clientInfoPtr, size_t index)
{
	int 			tcpErr = 0;
	int 			cmtErr = 0;
	
	clientInfoPtr->stopFlag = 1;	
	if((cmtErr = CmtWaitForThreadPoolFunctionCompletion (DEFAULT_THREAD_POOL_HANDLE, clientInfoPtr->threadFuncId, OPT_TP_PROCESS_EVENTS_WHILE_WAITING)) < 0)
		ReportCMTError(cmtErr);
			
	if((cmtErr = CmtReleaseThreadPoolFunctionID (DEFAULT_THREAD_POOL_HANDLE, clientInfoPtr->threadFuncId)) < 0)
		ReportCMTError(cmtErr);
		
	if((tcpErr = DisconnectTCPClient (clientInfoPtr->handle)) < 0)
		ReportTCPError (tcpErr);
		
	ListRemoveItem (clientList, NULL, index);
	free (clientInfoPtr);
	
	return 0;
}

extern int DisconnectClient(unsigned int handle)
{
	ClientInfo 		clientInfo = {0};
	ClientInfoPtr 	clientInfoPtr = &clientInfo;
	size_t 			index;

	clientInfoPtr->handle = handle;
	index = ListFindItem (clientList, &clientInfoPtr, FRONT_OF_LIST, CompareClientInfoPtr);
	if (index > 0)
	{
		ListGetItem (clientList, &clientInfoPtr, index);
		Disconnect(clientInfoPtr, index);
	}
	
	return 0;
}

static int CVICALLBACK DisconnectClientListItem (int index, void *itemPtr, void *data)
{
	Disconnect (*(ClientInfo **) itemPtr, index);
	return 0;
}

extern void ReportTCPError(int error)
{
	char messageBuffer[1024];
	if (error < 0)
	{
		sprintf(messageBuffer,GetTCPErrorString(error),GetTCPSystemErrorString());
		MessagePopup("Error", messageBuffer);
	}
}

extern void ReportCMTError(int error)
{
	char messageBuffer[1024];
	if (error < 0)
	{
		CmtGetErrorMessage(error, messageBuffer);
		MessagePopup("Error", messageBuffer);
	}
}

extern int CVICALLBACK CompareClientInfoPtr (void *item1, void *item2)
{
	return ((*(ClientInfoPtr *) item1)->handle - (*(ClientInfoPtr *) item2)->handle);
}

extern void CleanUpTCPConnections()
{
	if (clientList)
	{
		ListApplyToEach (clientList, 0, DisconnectClientListItem, 0);
		ListDispose (clientList);
	}
}
