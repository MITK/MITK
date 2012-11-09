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

#include "mitkTrackingDeviceSourceConfigurator.h"

#include "mitkNDITrackingDevice.h"
#include "mitkClaronTrackingDevice.h"


mitk::TrackingDeviceSourceConfigurator::TrackingDeviceSourceConfigurator(mitk::NavigationToolStorage::Pointer NavigationTools, mitk::TrackingDevice::Pointer TrackingDevice)
{
//make a copy of the navigation tool storage because we will modify the storage
if (NavigationTools.IsNotNull())
  {
  m_NavigationTools = mitk::NavigationToolStorage::New();
  for (int i=0; i<NavigationTools->GetToolCount(); i++)
      {
      m_NavigationTools->AddTool(NavigationTools->GetTool(i));
      }
  }

m_TrackingDevice = TrackingDevice;
m_ToolCorrespondencesInToolStorage = std::vector<int>();
m_ErrorMessage = "";
}

mitk::NavigationToolStorage::Pointer mitk::TrackingDeviceSourceConfigurator::GetUpdatedNavigationToolStorage()
{
return m_NavigationTools;
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
  m_ToolCorrespondencesInToolStorage = std::vector<int>();
  //add the tools to the tracking device
  for (int i=0; i<navigationTools->GetToolCount(); i++)
      {
        mitk::NavigationTool::Pointer thisNavigationTool = m_NavigationTools->GetTool(i);
        m_ToolCorrespondencesInToolStorage.push_back(i);
        bool toolAddSuccess = thisDevice->AddTool(thisNavigationTool->GetToolName().c_str(),thisNavigationTool->GetCalibrationFile().c_str());
        if (!toolAddSuccess)
          {
          //todo: error handling
          this->m_ErrorMessage = "Can't add tool, is the SROM-file valid?";
          return NULL;
          }
        thisDevice->GetTool(i)->SetToolTip(thisNavigationTool->GetToolTipPosition(),thisNavigationTool->GetToolTipOrientation());
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

  //now search for automatically detected tools in the tool storage and save them
  mitk::NavigationToolStorage::Pointer newToolStorageInRightOrder = mitk::NavigationToolStorage::New();
  std::vector<int> alreadyFoundTools = std::vector<int>();
  m_ToolCorrespondencesInToolStorage = std::vector<int>();
  for (int i=0; i<thisDevice->GetToolCount(); i++)
      {
      bool toolFound = false;
      for (int j=0; j<navigationTools->GetToolCount(); j++)
        {
          //check if the serial number is the same to identify the tool
          if ((dynamic_cast<mitk::NDIPassiveTool*>(thisDevice->GetTool(i)))->GetSerialNumber() == navigationTools->GetTool(j)->GetSerialNumber())
            {
            //check if this tool was already added to make sure that every tool is only added once (in case of same serial numbers)
            bool toolAlreadyAdded = false;
            for(int k=0; k<alreadyFoundTools.size(); k++) if (alreadyFoundTools.at(k) == j) toolAlreadyAdded = true;

            if(!toolAlreadyAdded)
              {
              //add tool in right order
              newToolStorageInRightOrder->AddTool(navigationTools->GetTool(j));
              m_ToolCorrespondencesInToolStorage.push_back(j);
              //adapt name of tool
              dynamic_cast<mitk::NDIPassiveTool*>(thisDevice->GetTool(i))->SetToolName(navigationTools->GetTool(j)->GetToolName());
              //set tip of tool
              dynamic_cast<mitk::NDIPassiveTool*>(thisDevice->GetTool(i))->SetToolTip(navigationTools->GetTool(j)->GetToolTipPosition(),navigationTools->GetTool(j)->GetToolTipOrientation());
              //rember that this tool was already found
              alreadyFoundTools.push_back(j);

              toolFound = true;
              break;
              }
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
  m_ToolCorrespondencesInToolStorage = std::vector<int>();
  //add the tools to the tracking device
  for (int i=0; i<navigationTools->GetToolCount(); i++)
      {
        mitk::NavigationTool::Pointer thisNavigationTool = m_NavigationTools->GetTool(i);
        m_ToolCorrespondencesInToolStorage.push_back(i);
        bool toolAddSuccess = thisDevice->AddTool(thisNavigationTool->GetToolName().c_str(),thisNavigationTool->GetCalibrationFile().c_str());
        if (!toolAddSuccess)
          {
          //todo error handling
          this->m_ErrorMessage = "Can't add tool, is the toolfile valid?";
          return NULL;
          }
        thisDevice->GetTool(i)->SetToolTip(thisNavigationTool->GetToolTipPosition(),thisNavigationTool->GetToolTipOrientation());
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

int mitk::TrackingDeviceSourceConfigurator::GetToolNumberInToolStorage(int outputID)
  {
  if (outputID < m_ToolCorrespondencesInToolStorage.size()) return m_ToolCorrespondencesInToolStorage.at(outputID);
  else return -1;
  }

std::string mitk::TrackingDeviceSourceConfigurator::GetToolIdentifierInToolStorage(int outputID)
  {
  if (outputID < m_ToolCorrespondencesInToolStorage.size()) return m_NavigationTools->GetTool(m_ToolCorrespondencesInToolStorage.at(outputID))->GetIdentifier();
  else return "";
  }

std::vector<int> mitk::TrackingDeviceSourceConfigurator::GetToolNumbersInToolStorage()
  {
  return m_ToolCorrespondencesInToolStorage;
  }

std::vector<std::string> mitk::TrackingDeviceSourceConfigurator::GetToolIdentifiersInToolStorage()
  {
  std::vector<std::string> returnValue = std::vector<std::string>();
  for (int i=0; i<m_ToolCorrespondencesInToolStorage.size(); i++)
    {returnValue.push_back(m_NavigationTools->GetTool(m_ToolCorrespondencesInToolStorage.at(i))->GetIdentifier());}
  return returnValue;
  }
