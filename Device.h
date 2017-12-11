//==============================================================================
//
// Title:		Device.h
// Purpose:		A short description of the interface.
//
// Created on:	2017-08-24 at 23:00:16 by Karol Rychter.
// Copyright:	Politechnika Warszawska. All Rights Reserved.
//
//==============================================================================

#ifndef __Device_H__
#define __Device_H__

#ifdef __cplusplus
    extern "C" {
#endif

//==============================================================================
// Include files

#include "cvidef.h"
#include <utility.h>
#include <rs232.h>
#include <cvirte.h>		
#include <userint.h>
#include <toolbox.h>

typedef struct DeviceStatus
{
	int				comPortNum;
	unsigned int	TCPHandle;
	int				comPortOpenedFlag;	
	int				measurementFlag;
	unsigned int	threadId;	
	int				threadFuncId;
	int				threadInited;
	unsigned int	stopFlag;
	
} DeviceStatus, *DeviceStatusPtr;

typedef struct MeasurementSettings
{
	int				calibChannel;
	int				calibResistance;
	float			calibPhase;
	int				calibFrequency;
	int				channelNumber;	
	int				frequency;	
	int				timeBetween;
	int				measurementNumber; 
 
} MeasurementSettings, *MeasurementSettingsPtr;


DeviceStatus GlobalDeviceStatus;   
MeasurementSettings GlobalMeasurementSettings; 

extern int GetDeviceStatus(char *data);
extern int GetMeasurementSettings(char *data);
extern void CVICALLBACK SetDeviceCommand (void *cbData);

static int StartMeasurement();
static int StopMeasurement();
static int StartCalibration(int channel, int resistance, int frequency, float phase);

extern int CVICALLBACK DeviceThreadFunction(void *data);


#ifdef __cplusplus
    }
#endif

#endif  /* ndef __Device_H__ */
