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
  
  //create tracking device source
  if (m_TrackingDevice->GetType()==mitk::NDIAurora) {returnValue = CreateNDIAuroraTrackingDeviceSource(m_TrackingDevice,m_NavigationTools);}
  else if (m_TrackingDevice->GetType()==mitk::NDIPolaris) {returnValue = CreateNDIPolarisTrackingDeviceSource(m_TrackingDevice,m_NavigationTools);}
  else if (m_TrackingDevice->GetType()==mitk::ClaronMicron) {returnValue = CreateMicronTrackerTrackingDeviceSource(m_TrackingDevice,m_NavigationTools);}
    //TODO: insert other tracking systems? 
  if (returnValue.IsNull()) return NULL;

  //create visualization filter
  visualizationFilter = CreateNavigationDataObjectVisualizationFilter(returnValue,m_NavigationTools);
  if (visualizationFilter.IsNull()) return NULL;
  
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
  mitk::TrackingDeviceSource::Pointer returnValue = mitk::TrackingDeviceSource::New();
  mitk::NDITrackingDevice::Pointer thisDevice = dynamic_cast<mitk::NDITrackingDevice*>(trackingDevice.GetPointer());

  //connect to aurora to dectect tools automatically
  thisDevice->OpenConnection();
  thisDevice->StartTracking();
  //thisDevice->StopTracking();
  //thisDevice->CloseConnection();
  
  //now search for automatically detected tools in the tool storage and save them
  mitk::NavigationToolStorage::Pointer newToolStorageInRightOrder = mitk::NavigationToolStorage::New();
  for (int i=0; i<thisDevice->GetToolCount(); i++)
      {
      bool toolFound = false;
      for (int j=0; j<navigationTools->GetToolCount(); j++)
        {
          if ((dynamic_cast<mitk::NDIPassiveTool*>(thisDevice->GetTool(i)))->GetSerialNumber() == navigationTools->GetTool(j)->GetSerialNumber())
            {
            //add tool in right order
            newToolStorageInRightOrder->AddTool(navigationTools->GetTool(j));
            //adapt name of tool
            dynamic_cast<mitk::NDIPassiveTool*>(thisDevice->GetTool(i))->SetToolName(navigationTools->GetTool(j)->GetToolName());
            toolFound = true;
            break;
            }
        }
      if (!toolFound)
        {
        this->m_ErrorMessage = "Error: did not find every automatically detected tool in the loaded tool storage: aborting initialization.";
        return NULL;
        }
      }
  
  //delete all tools from the tool storage
  navigationTools->DeleteAllTools();

  //and add only the detected tools in the right order
  for (int i=0; i<newToolStorageInRightOrder->GetToolCount(); i++)
      {
      navigationTools->AddTool(newToolStorageInRightOrder->GetTool(i));   
      }
  returnValue->SetTrackingDevice(thisDevice);
  return returnValue;
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
    if (currentTool.IsNull())
      {
      this->m_ErrorMessage = "Error: did not find correspondig tool in tracking device after initialization.";
      return NULL;
      }
    returnValue->SetInput(i,trackingDeviceSource->GetOutput(i));
    returnValue->SetRepresentationObject(i,currentTool->GetDataNode()->GetData());
    }
  return returnValue;
  }
