/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "QmitkTrackingDeviceConfigurationWidgetConnectionWorker.h"

#include <mitkProgressBar.h>
#include <mitkNDITrackingDevice.h>
#include <mitkIGTException.h>

void QmitkTrackingDeviceConfigurationWidgetConnectionWorker::TestConnectionThreadFunc()
{
  MITK_INFO << "Testing Connection!";
  QString output;
  bool connected = false;
  mitk::ProgressBar::GetInstance()->AddStepsToDo(4);
  try
  {
    if (!m_TrackingDevice->IsDeviceInstalled())
    {
      output = "ERROR: Device is not installed!";
    }
    else
    {
      //test connection and start tracking, generate output
      output = "<br>testing connection <br>  ...";
      m_TrackingDevice->OpenConnection();
      output += "OK";
      mitk::ProgressBar::GetInstance()->Progress();

      //try start/stop tracking
      output += "<br>testing tracking <br>  ...";
      m_TrackingDevice->StartTracking();
      mitk::ProgressBar::GetInstance()->Progress();
      m_TrackingDevice->StopTracking();
      mitk::ProgressBar::GetInstance()->Progress();

      //try close connection
      m_TrackingDevice->CloseConnection();
      mitk::ProgressBar::GetInstance()->Progress();
      output += "OK";
      connected = true;
    }
  }
  catch (mitk::IGTException &e)
  {
    output += "ERROR!";
    MITK_WARN << "Error while testing connection / start tracking of the device: " << e.GetDescription();
  }
  mitk::ProgressBar::GetInstance()->Progress(4);
  emit ConnectionTested(connected, output);
}

void QmitkTrackingDeviceConfigurationWidgetConnectionWorker::SetTrackingDevice(mitk::TrackingDevice::Pointer t)
{
  m_TrackingDevice = t;
}