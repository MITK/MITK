/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkTrackingDeviceSource.h"

#include "mitkTrackingDevice.h"
#include "mitkTrackingTool.h"

#include "mitkTimeStamp.h"


mitk::TrackingDeviceSource::TrackingDeviceSource() 
: mitk::NavigationDataSource(), m_TrackingDevice(NULL)
{
}


mitk::TrackingDeviceSource::~TrackingDeviceSource()
{
  StopTracking();
  Disconnect();
  m_TrackingDevice = NULL;
}


void mitk::TrackingDeviceSource::GenerateData()
{
  if (m_TrackingDevice.IsNull())
    return;

  if (m_TrackingDevice->GetToolCount() < 1)
    return;

  if (this->GetNumberOfOutputs() != m_TrackingDevice->GetToolCount()) // mismatch between tools and outputs. What should we do? Were tools added to the tracking device after SetTrackingDevice() was called?
  {
    std::stringstream ss;
    ss << "mitk::TrackingDeviceSource: not enough outputs available for all tools. " 
      << this->GetNumberOfOutputs() << " outputs available, but "
      << m_TrackingDevice->GetToolCount() << " tools available in the tracking device.";
    throw std::out_of_range(ss.str());
  }
  /* update outputs with tracking data from tools */
  for (unsigned int i = 0; i < m_TrackingDevice->GetToolCount(); ++i)
  {
    mitk::NavigationData* nd = this->GetOutput(i);
    assert(nd);
    mitk::TrackingTool* t = m_TrackingDevice->GetTool(i);
    assert(t);

    if ((t->IsEnabled() == false) || (t->IsDataValid() == false))
    {
      nd->SetDataValid(false);
      continue;
    }
    nd->SetDataValid(true);
    mitk::NavigationData::PositionType p;
    t->GetPosition(p);
    nd->SetPosition(p);

    mitk::NavigationData::OrientationType o;
    t->GetQuaternion(o);
    nd->SetOrientation(o);
    nd->SetError(t->GetTrackingError());
    //\TODO set timestamp from tool too.
    nd->SetTimeStamp( mitk::TimeStamp::GetInstance()->GetElapsed() );

  }
}


void mitk::TrackingDeviceSource::SetTrackingDevice( mitk::TrackingDevice* td )
{
  itkDebugMacro("setting TrackingDevice to " << td );
  if (this->m_TrackingDevice.GetPointer() != td)
  {
    this->m_TrackingDevice = td;
    this->SetNumberOfOutputs(m_TrackingDevice->GetToolCount());  // create outputs for all tools
    for (unsigned int idx = 0; idx < this->GetNumberOfOutputs(); ++idx)
      if (this->GetOutput(idx) == NULL)
      {
        DataObjectPointer newOutput = this->MakeOutput(idx);
        this->SetNthOutput(idx, newOutput);
      }
    this->Modified();
  }
}


void mitk::TrackingDeviceSource::Connect()
{
  if (m_TrackingDevice.IsNull())
    throw std::invalid_argument("mitk::TrackingDeviceSource: No tracking device set");
  if (m_TrackingDevice->OpenConnection() == false)
    throw std::runtime_error("mitk::TrackingDeviceSource: Could not open connection to tracking device");
}


void mitk::TrackingDeviceSource::StartTracking()
{
  if (m_TrackingDevice.IsNull())
    throw std::invalid_argument("mitk::TrackingDeviceSource: No tracking device set");
  if (m_TrackingDevice->StartTracking() == false)
    throw std::runtime_error("mitk::TrackingDeviceSource: Could not start tracking");
}


void mitk::TrackingDeviceSource::Disconnect()
{
  if (m_TrackingDevice.IsNull())
    throw std::invalid_argument("mitk::TrackingDeviceSource: No tracking device set");
  if (m_TrackingDevice->CloseConnection() == false)
    throw std::runtime_error("mitk::TrackingDeviceSource: Could not close connection to tracking device");
}


void mitk::TrackingDeviceSource::StopTracking()
{
  if (m_TrackingDevice.IsNull())
    throw std::invalid_argument("mitk::TrackingDeviceSource: No tracking device set");
  if (m_TrackingDevice->StopTracking() == false)
    throw std::runtime_error("mitk::TrackingDeviceSource: Could not stop tracking");
}


void mitk::TrackingDeviceSource::UpdateOutputInformation()
{
  this->Modified();  // make sure that we need to be updated
  Superclass::UpdateOutputInformation();
}
