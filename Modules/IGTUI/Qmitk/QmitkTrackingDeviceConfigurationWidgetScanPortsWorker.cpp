/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkTrackingDeviceConfigurationWidgetScanPortsWorker.h"

#include <mitkProgressBar.h>
#include <mitkNDITrackingDevice.h>
#include <mitkIGTException.h>

#include "mitkUnspecifiedTrackingTypeInformation.h"

void QmitkTrackingDeviceConfigurationWidgetScanPortsWorker::ScanPortsThreadFunc()
{
  int Port = -1;
  int PortType = -1;

  QString result = "<br>Found Devices:";
  int resultSize = result.size(); //remember size of result: if it stays the same no device were found
  mitk::TrackingDeviceType scannedPort;
#ifdef WIN32
  mitk::ProgressBar::GetInstance()->AddStepsToDo(19);

  QString devName;
  for (unsigned int i = 1; i < 20; ++i)
  {
    QString statusOutput = "Scanning Port #" + QString::number(i);
    MITK_INFO << statusOutput.toStdString().c_str();
    if (i < 10) devName = QString("COM%1").arg(i);
    else devName = QString("\\\\.\\COM%1").arg(i); // prepend "\\.\ to COM ports >9, to be able to allow connection"
    scannedPort = ScanPort(devName);
    if (!(scannedPort == mitk::UnspecifiedTrackingTypeInformation::GetTrackingDeviceName()))
    {
      result += "<br>" + devName + ": " + QString::fromStdString(scannedPort);
      Port = i;
    }
    mitk::ProgressBar::GetInstance()->Progress();
  }
#else //linux systems
  for(unsigned int i = 1; i < 6; ++i)
  {
    QString devName = QString("/dev/ttyS%1").arg(i);
    mitk::TrackingDeviceType scannedPort = ScanPort(devName);
    if (!(scannedPort == mitk::UnspecifiedTrackingTypeInformation::GetTrackingDeviceName()))
    {
      result += "<br>" + devName + ": " + QString::fromStdString(scannedPort);
      Port = i;
      PortType = 1;
    }
  }

  for(unsigned int i = 0; i <7; ++i)
  {
    QString devName = QString("/dev/ttyUSB%1").arg(i);
    mitk::TrackingDeviceType scannedPort = ScanPort(devName);
    if (!(scannedPort == mitk::UnspecifiedTrackingTypeInformation::GetTrackingDeviceName()))
    {
      result += "<br>" + devName + ": " + QString::fromStdString(scannedPort);
      Port = i;
      PortType = 0;
    }
  }
#endif

  if (result.size() == resultSize) result += "<br>none";

  emit PortsScanned(Port, result, PortType);
}

mitk::TrackingDeviceType QmitkTrackingDeviceConfigurationWidgetScanPortsWorker::ScanPort(QString port)
{
  mitk::NDITrackingDevice::Pointer tracker = mitk::NDITrackingDevice::New();
  tracker->SetDeviceName(port.toStdString());
  mitk::TrackingDeviceType returnValue = mitk::UnspecifiedTrackingTypeInformation::GetTrackingDeviceName();
  try
  {
    returnValue = tracker->TestConnection();
  }
  catch (const mitk::IGTException&)
  {
  }//do nothing: there is simply no device on this port
  return returnValue;
}
