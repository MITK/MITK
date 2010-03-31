/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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
  if (m_TrackingDevice->GetState()==mitk::TrackingDevice::Tracking)
  {
    StopTracking();
  }
  if (m_TrackingDevice->GetState()==mitk::TrackingDevice::Ready)
  {
    Disconnect();
  }
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
    //check this: TODO:
    ////this might happen if a tool is plugged into an aurora during tracking.
    //this->CreateOutputs();

    std::stringstream ss;
    ss << "mitk::TrackingDeviceSource: not enough outputs available for all tools. " 
      << this->GetNumberOfOutputs() << " outputs available, but "
      << m_TrackingDevice->GetToolCount() << " tools available in the tracking device.";
    throw std::out_of_range(ss.str());
  }
  /* update outputs with tracking data from tools */
  unsigned int toolCount = m_TrackingDevice->GetToolCount();
  for (unsigned int i = 0; i < toolCount; ++i)
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
    t->GetOrientation(o);
    nd->SetOrientation(o);
    nd->SetOrientationAccuracy(t->GetTrackingError());
    nd->SetPositionAccuracy(t->GetTrackingError());
    nd->SetTimeStamp( mitk::TimeStamp::GetInstance()->GetElapsed() );
  }
}


void mitk::TrackingDeviceSource::SetTrackingDevice( mitk::TrackingDevice* td )
{
  itkDebugMacro("setting TrackingDevice to " << td );
  if (this->m_TrackingDevice.GetPointer() != td)
  {
    this->m_TrackingDevice = td;
    this->CreateOutputs();
  }
}

void mitk::TrackingDeviceSource::CreateOutputs(){
  //if outputs are set then delete them
  if (this->GetNumberOfOutputs() > 0)
  {
    for (unsigned int numOP = this->GetNumberOfOutputs(); numOP>0; numOP--)
      this->RemoveOutput(this->GetOutput(numOP));
    this->Modified();
  }
  
  //fill the outputs if a valid tracking device is set
  if (m_TrackingDevice.IsNull())
    return;

  this->SetNumberOfOutputs(m_TrackingDevice->GetToolCount());  // create outputs for all tools
  unsigned int numberOfOutputs = this->GetNumberOfOutputs();
  for (unsigned int idx = 0; idx < numberOfOutputs; ++idx)
  {
    if (this->GetOutput(idx) == NULL)
    {
      DataObjectPointer newOutput = this->MakeOutput(idx);
      static_cast<mitk::NavigationData*>(newOutput.GetPointer())->SetName(m_TrackingDevice->GetTool(idx)->GetToolName()); // set NavigationData name to ToolName
      this->SetNthOutput(idx, newOutput);
      this->Modified();
    }    
  }
}

void mitk::TrackingDeviceSource::Connect()
{
  if (m_TrackingDevice.IsNull())
    throw std::invalid_argument("mitk::TrackingDeviceSource: No tracking device set");
  if (this->IsConnected())
    return;
  if (m_TrackingDevice->OpenConnection() == false)
    throw std::runtime_error(std::string("mitk::TrackingDeviceSource: Could not open connection to tracking device. Error: ") + m_TrackingDevice->GetErrorMessage());
  
  /* NDI Aurora needs a connection to discover tools that are connected to it. 
     Therefore we need to create outputs for these tools now */
  //if (m_TrackingDevice->GetType() == mitk::NDIAurora)
    //this->CreateOutputs();
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


//unsigned int mitk::TrackingDeviceSource::GetToolCount()
//{
//  if (m_TrackingDevice) 
//    return m_TrackingDevice->GetToolCount(); 
//  return 0;
//}


bool mitk::TrackingDeviceSource::IsConnected()
{
  if (m_TrackingDevice.IsNull())
    return false;
  
  return (m_TrackingDevice->GetState() == mitk::TrackingDevice::Ready) || (m_TrackingDevice->GetState() == mitk::TrackingDevice::Tracking);
}


bool mitk::TrackingDeviceSource::IsTracking()
{
  if (m_TrackingDevice.IsNull())
    return false;

  return m_TrackingDevice->GetState() == mitk::TrackingDevice::Tracking;
}
