//==============================================================================
//
// Title:		Device.c
// Purpose:		A short description of the implementation.
//
// Created on:	2017-08-24 at 23:00:16 by Karol Rychter.
// Copyright:	Politechnika Warszawska. All Rights Reserved.
//
//==============================================================================

#include <ansi_c.h>
#include "Device.h"
void CVICALLBACK SetDeviceCommand (void *cbData)
{
	char	*resPtr;
	char	*data = (char*) cbData;
	
	if((resPtr=strstr(data,"DEVCMD_STARTCALIBRATION"))!=NULL)
	{
		int calibrationChannel, calibrationResistance, calibrationFrequency = 0;
		float calibrationPhase = 0;
		
		sscanf(resPtr, "DEVCMD_STARTCALIBRATION_%d_%d_%d_%fX", &calibrationChannel, &calibrationResistance, &calibrationFrequency,&calibrationPhase);
		StartCalibration(calibrationChannel, calibrationResistance, calibrationFrequency, calibrationPhase);
	}
	else if((resPtr=strstr(data,"DEVCMD_STARTMEASUREMENT"))!=NULL)
	{
		int frequency, timeBetween, measurementNumber, channelNumber = 0;
		
		sscanf(resPtr, "DEVCMD_STARTMEASUREMENT_%d_%d_%d_%d", &frequency, &timeBetween, &measurementNumber, &channelNumber);
		StartMeasurement(frequency, timeBetween, measurementNumber, channelNumber);
	}
	else if((resPtr=strstr(data,"DEVCMD_STOPMEASUREMENT"))!=NULL)
	{
		StopMeasurement();
	}
	
}

extern int GetDeviceStatus(char *data)
{
/*	printf("DeviceStatus:\ncomPortNum: %d\ncomPortOpenedFlag: %d\nTCPHandle: %d\nthreadFuncId: %d\nthreadId: %d\nthreadInited: %d\nmeasurementFlag: %d\nstopFlag: %d\n",
		   GlobalDeviceStatus.comPortNum,
		   GlobalDeviceStatus.comPortOpenedFlag,
		   GlobalDeviceStatus.TCPHandle,
		   GlobalDeviceStatus.threadFuncId,
		   GlobalDeviceStatus.threadId,
		   GlobalDeviceStatus.threadInited,
		   GlobalDeviceStatus.measurementFlag,
		   GlobalDeviceStatus.stopFlag
		  );*/
	sprintf(data, "DeviceStatus:\ncomPortNum: %d\ncomPortOpenedFlag: %d\nTCPHandle: %d\nthreadFuncId: %d\nthreadId: %d\nthreadInited: %d\nmeasurementFlag: %d\nstopFlag: %d\n",
		   GlobalDeviceStatus.comPortNum,
		   GlobalDeviceStatus.comPortOpenedFlag,
		   GlobalDeviceStatus.TCPHandle,
		   GlobalDeviceStatus.threadFuncId,
		   GlobalDeviceStatus.threadId,
		   GlobalDeviceStatus.threadInited,
		   GlobalDeviceStatus.measurementFlag,
		   GlobalDeviceStatus.stopFlag
		  );	
	return 0;
}

extern int GetMeasurementSettings(char *data)
{
/*	printf("MeasurementSettings:\ncalibChannel: %d\ncalibResistance: %d\ncalibPhase: %f\ncalibFrequency: %d\nchannelNumber: %d\nfrequency: %d\ntimeBetween: %d\nmeasurementNumber: %d\n",
		   GlobalMeasurementSettings.calibChannel,
		   GlobalMeasurementSettings.calibResistance,
		   GlobalMeasurementSettings.calibPhase,
		   GlobalMeasurementSettings.calibFrequency,
		   GlobalMeasurementSettings.channelNumber,
		   GlobalMeasurementSettings.frequency,
		   GlobalMeasurementSettings.timeBetween,
		   GlobalMeasurementSettings.measurementNumber
	);*/
	sprintf(data, "MeasurementSettings:\ncalibChannel: %d\ncalibResistance: %d\ncalibPhase: %f\ncalibFrequency: %d\nchannelNumber: %d\nfrequency: %d\ntimeBetween: %d\nmeasurementNumber: %d\n",
		   GlobalMeasurementSettings.calibChannel,
		   GlobalMeasurementSettings.calibResistance,
		   GlobalMeasurementSettings.calibPhase,
		   GlobalMeasurementSettings.calibFrequency,
		   GlobalMeasurementSettings.channelNumber,
		   GlobalMeasurementSettings.frequency,
		   GlobalMeasurementSettings.timeBetween,
		   GlobalMeasurementSettings.measurementNumber);	
	return 0;
}

static int StartMeasurement(int frequency, int timeBetween, int measurementNumber, channelNumber)
{
	char sendBuffer[40] = {0};
	int sendBufferLenght=0;
	
	sprintf(sendBuffer, "F%dT%dM%dC%dX\n", frequency, timeBetween, measurementNumber, channelNumber); 
	sendBufferLenght=strlen(sendBuffer);
	ComWrt(GlobalDeviceStatus.comPortNum ,sendBuffer, sendBufferLenght);
	
	GlobalDeviceStatus.measurementFlag = 1;
	GlobalMeasurementSettings.channelNumber = channelNumber;
	GlobalMeasurementSettings.frequency = frequency;
	GlobalMeasurementSettings.measurementNumber = measurementNumber;
	GlobalMeasurementSettings.timeBetween = timeBetween;
	
	return 0;	
}

static int StopMeasurement()
{
	char sendBuffer[10] = {0};
	int sendBufferLenght=0;
	
	sprintf(sendBuffer, "SX\n");  
	sendBufferLenght=strlen(sendBuffer);
	ComWrt(GlobalDeviceStatus.comPortNum ,sendBuffer, sendBufferLenght);
	return 0;	
}

static int StartCalibration(int channel, int resistance, int frequency, float phase)
{
	char sendBuffer[40] = {0};
	int sendBufferLenght=0;
	
	sprintf(sendBuffer, "R%dA%0.2fK%dX\n", resistance, phase, channel);

	sendBufferLenght=strlen(sendBuffer);
	ComWrt(GlobalDeviceStatus.comPortNum ,sendBuffer, sendBufferLenght);
	
	GlobalMeasurementSettings.calibChannel = channel;
	GlobalMeasurementSettings.calibResistance = resistance;
	GlobalMeasurementSettings.calibFrequency = frequency;
	GlobalMeasurementSettings.calibPhase = phase;
	
	return 0;
}

extern int CVICALLBACK DeviceThreadFunction(void *data)
{
	DeviceStatusPtr deviceStatusPtr = (DeviceStatusPtr) data;
	
	deviceStatusPtr->threadId = CmtGetCurrentThreadID ();
	deviceStatusPtr->threadInited = 1;

	while (!deviceStatusPtr->stopFlag)
		{
		ProcessSystemEvents ();
		}
		
	return 0;
}
