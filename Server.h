//==============================================================================
//
// Title:		Server.h
// Purpose:		A short description of the interface.
//
// Created on:	2017-08-24 at 22:59:38 by Karol Rychter.
// Copyright:	Politechnika Warszawska. All Rights Reserved.
//
//==============================================================================

#ifndef __Server_H__
#define __Server_H__

#ifdef __cplusplus
    extern "C" {
#endif

//==============================================================================
// Include files

#include "cvidef.h"
#include <toolbox.h>
#include <userint.h>
#include <tcpsupp.h>
		
typedef struct ClientInfo
{
	unsigned int	handle;		
	unsigned int	threadId;	
	int				threadFuncId;
	int				stopFlag;	
	char			name[64];	
	char			address[32];	
	int				readingData;	
	int				panel;			
	int             threadInited;   
} ClientInfo, *ClientInfoPtr;		

static ListType clientList = 0;

static int CVICALLBACK ClientThreadFunction (void *data);

extern int ConnectClient(unsigned int handle);
static int Disconnect (ClientInfoPtr clientInfoPtr, size_t index);
extern int DisconnectClient(unsigned int handle);
static int CVICALLBACK DisconnectClientListItem (int index, void *itemPtr, void *data);
extern void CleanUpTCPConnections();

extern int ResolveClientData(unsigned int handle);
extern int SendData(void *data, unsigned int handle);
static void CVICALLBACK DeferredReceive (void *data);
extern void CVICALLBACK DeferredSend (void *data); 

extern void ReportTCPError(int error);
extern void ReportCMTError(int error);
static int CVICALLBACK CompareClientInfoPtr (void *item1, void *item2);
extern void ButtonDis();



#ifdef __cplusplus
    }
#endif

#endif  /* ndef __Server_H__ */
