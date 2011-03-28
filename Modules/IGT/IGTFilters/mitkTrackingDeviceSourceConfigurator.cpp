/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2011-01-18 13:22:38 +0100 (Di, 18 Jan 2011) $
Version:   $Revision: 28959 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkTrackingDeviceSourceConfigurator.h"

#include "mitkNDITrackingDevice.h"
#include "mitkClaronTrackingDevice.h"


mitk::TrackingDeviceSourceConfigurator::TrackingDeviceSourceConfigurator(mitk::NavigationToolStorage::Pointer NavigationTools, mitk::TrackingDevice::Pointer TrackingDevice)
{
m_NavigationTools = NavigationTools;
m_TrackingDevice = TrackingDevice;
m_ErrorMessage = "";
}


mitk::TrackingDeviceSourceConfigurator::~TrackingDeviceSourceConfigurator()
{
}

bool mitk::TrackingDeviceSourceConfigurator::IsCreateTrackingDeviceSourcePossible()
{
if (m_NavigationTools.IsNull())
  {
  m_ErrorMessage = "NavigationToolStorage is NULL!";
  return false;
  }
else if (m_TrackingDevice.IsNull())
  {
  m_ErrorMessage = "TrackingDevice is NULL!";
  return false;
  }
else
  {
  for (int i=0; i<m_NavigationTools->GetToolCount(); i++)
    {
    if (m_NavigationTools->GetTool(i)->GetTrackingDeviceType() != m_TrackingDevice->GetType()) 
      {
      m_ErrorMessage = "At least one tool is not of the same type like the tracking device.";
      return false;
      }
    }
  //TODO in case of Aurora: check if the tools are automatically detected by comparing the serial number
  return true;
  }
}

mitk::TrackingDeviceSource::Pointer mitk::TrackingDeviceSourceConfigurator::CreateTrackingDeviceSource()
{
mitk::NavigationDataObjectVisualizationFilter::Pointer dummy;
return this->CreateTrackingDeviceSource(dummy); 
}

mitk::TrackingDeviceSource::Pointer mitk::TrackingDeviceSourceConfigurator::CreateTrackingDeviceSource(mitk::NavigationDataObjectVisualizationFilter::Pointer &visualizationFilter)
{
  if (!this->IsCreateTrackingDeviceSourcePossible()) return NULL;
  mitk::TrackingDeviceSource::Pointer returnValue;
  if (m_TrackingDevice->GetType()==mitk::NDIAurora)
    {
    returnValue = CreateNDIAuroraTrackingDeviceSource(m_TrackingDevice,m_NavigationTools);
    visualizationFilter = CreateNavigationDataObjectVisualizationFilter(returnValue,m_NavigationTools); 
    }
  else if (m_TrackingDevice->GetType()==mitk::NDIPolaris)
    {
    returnValue = CreateNDIPolarisTrackingDeviceSource(m_TrackingDevice,m_NavigationTools);
    visualizationFilter = CreateNavigationDataObjectVisualizationFilter(returnValue,m_NavigationTools); 
    }
  else if (m_TrackingDevice->GetType()==mitk::ClaronMicron)
    {
    returnValue = CreateMicronTrackerTrackingDeviceSource(m_TrackingDevice,m_NavigationTools);
    visualizationFilter = CreateNavigationDataObjectVisualizationFilter(returnValue,m_NavigationTools); 
    }
  //TODO: insert other tracking systems?    
  return returnValue;
}
    
std::string mitk::TrackingDeviceSourceConfigurator::GetErrorMessage()
{
  return this->m_ErrorMessage;
}

//############################ internal help methods ########################################

mitk::TrackingDeviceSource::Pointer mitk::TrackingDeviceSourceConfigurator::CreateNDIPolarisTrackingDeviceSource(mitk::TrackingDevice::Pointer trackingDevice, mitk::NavigationToolStorage::Pointer navigationTools)
  {
  mitk::TrackingDeviceSource::Pointer returnValue = mitk::TrackingDeviceSource::New();
  mitk::NDITrackingDevice::Pointer thisDevice = dynamic_cast<mitk::NDITrackingDevice*>(trackingDevice.GetPointer());
  //add the tools to the tracking device
  for (int i=0; i<navigationTools->GetToolCount(); i++)
      {
        mitk::NavigationTool::Pointer thisNavigationTool = m_NavigationTools->GetTool(i);
        bool toolAddSuccess = thisDevice->AddTool(thisNavigationTool->GetToolName().c_str(),thisNavigationTool->GetCalibrationFile().c_str());
        if (!toolAddSuccess)
          {
          this->m_ErrorMessage = "Can't add tool, is the SROM-file valid?";
          return NULL;
          }
      } 
  returnValue->SetTrackingDevice(thisDevice);
  return returnValue;
  }

mitk::TrackingDeviceSource::Pointer mitk::TrackingDeviceSourceConfigurator::CreateNDIAuroraTrackingDeviceSource(mitk::TrackingDevice::Pointer trackingDevice, mitk::NavigationToolStorage::Pointer navigationTools)
  {
  mitk::NDITrackingDevice::Pointer thisDevice = dynamic_cast<mitk::NDITrackingDevice*>(trackingDevice.GetPointer());
  for (int i=0; i<navigationTools->GetToolCount(); i++)
      {
      //TODO here: create a tracking tool for every navigation tool
      //           check if some tools are already connected to the aurora
      //           also: new class, which handles the visualization of tool surfaces? => NavigationDataObjectVisual
    
      }
  return NULL;
  }

mitk::TrackingDeviceSource::Pointer mitk::TrackingDeviceSourceConfigurator::CreateMicronTrackerTrackingDeviceSource(mitk::TrackingDevice::Pointer trackingDevice, mitk::NavigationToolStorage::Pointer navigationTools)  
  {
  mitk::TrackingDeviceSource::Pointer returnValue = mitk::TrackingDeviceSource::New();
  mitk::ClaronTrackingDevice::Pointer thisDevice = dynamic_cast<mitk::ClaronTrackingDevice*>(trackingDevice.GetPointer());
  //add the tools to the tracking device
  for (int i=0; i<navigationTools->GetToolCount(); i++)
      {
        mitk::NavigationTool::Pointer thisNavigationTool = m_NavigationTools->GetTool(i);
        bool toolAddSuccess = thisDevice->AddTool(thisNavigationTool->GetToolName().c_str(),thisNavigationTool->GetCalibrationFile().c_str());
        if (!toolAddSuccess)
          {
          this->m_ErrorMessage = "Can't add tool, is the toolfile valid?";
          return NULL;
          }
      } 
  returnValue->SetTrackingDevice(thisDevice);
  return returnValue;
  }

mitk::NavigationDataObjectVisualizationFilter::Pointer mitk::TrackingDeviceSourceConfigurator::CreateNavigationDataObjectVisualizationFilter(mitk::TrackingDeviceSource::Pointer trackingDeviceSource, mitk::NavigationToolStorage::Pointer navigationTools)
  {
  mitk::NavigationDataObjectVisualizationFilter::Pointer returnValue = mitk::NavigationDataObjectVisualizationFilter::New();
  for (int i=0; i<trackingDeviceSource->GetNumberOfOutputs(); i++)
    {
    mitk::NavigationTool::Pointer currentTool = navigationTools->GetToolByName(trackingDeviceSource->GetOutput(i)->GetName());
    returnValue->SetInput(i,trackingDeviceSource->GetOutput(i)); //is this the right way to connect the filters?
    returnValue->SetRepresentationObject(i,currentTool->GetDataNode()->GetData());
    }
  return returnValue;
  }
