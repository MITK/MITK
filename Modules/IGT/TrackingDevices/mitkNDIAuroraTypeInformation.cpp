/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkNDIAuroraTypeInformation.h"

#include "mitkIGTHardwareException.h"
#include "mitkNDITrackingDevice.h"

namespace mitk
{
  std::string NDIAuroraTypeInformation::GetTrackingDeviceName()
  {
    return "NDI Aurora";
  }

  TrackingDeviceData NDIAuroraTypeInformation::GetDeviceDataAuroraCompact()
  {
    TrackingDeviceData data = { NDIAuroraTypeInformation::GetTrackingDeviceName(), "Aurora Compact", "NDIAuroraCompactFG_Dome.stl", "A" };
    return data;
  }

  TrackingDeviceData NDIAuroraTypeInformation::GetDeviceDataAuroraPlanarCube()
  {
    TrackingDeviceData data = { NDIAuroraTypeInformation::GetTrackingDeviceName(), "Aurora Planar (Cube)", "NDIAurora.stl", "9" };
    return data;
  }

  TrackingDeviceData NDIAuroraTypeInformation::GetDeviceDataAuroraPlanarDome()
  {
    TrackingDeviceData data = { NDIAuroraTypeInformation::GetTrackingDeviceName(), "Aurora Planar (Dome)", "NDIAuroraPlanarFG_Dome.stl", "A" };
    return data;
  }

  TrackingDeviceData NDIAuroraTypeInformation::GetDeviceDataAuroraTabletop()
  {
    TrackingDeviceData data = { NDIAuroraTypeInformation::GetTrackingDeviceName(), "Aurora Tabletop", "NDIAuroraTabletopFG_Dome.stl", "A" };
    return data;
  }

  NDIAuroraTypeInformation::NDIAuroraTypeInformation()
  {
    m_DeviceName = NDIAuroraTypeInformation::GetTrackingDeviceName();
    m_TrackingDeviceData.push_back(GetDeviceDataAuroraPlanarCube());
    m_TrackingDeviceData.push_back(GetDeviceDataAuroraPlanarDome());
    m_TrackingDeviceData.push_back(GetDeviceDataAuroraTabletop());
    m_TrackingDeviceData.push_back(GetDeviceDataAuroraCompact());
  }

  NDIAuroraTypeInformation::~NDIAuroraTypeInformation()
  {
  }

  mitk::TrackingDeviceSource::Pointer NDIAuroraTypeInformation::CreateTrackingDeviceSource(
    mitk::TrackingDevice::Pointer trackingDevice,
    mitk::NavigationToolStorage::Pointer navigationTools,
    std::string* errorMessage,
    std::vector<int>* toolCorrespondencesInToolStorage)
  {
    MITK_DEBUG << "Creating Aurora tracking device.";
    mitk::TrackingDeviceSource::Pointer returnValue = mitk::TrackingDeviceSource::New();
    mitk::NDITrackingDevice::Pointer thisDevice = dynamic_cast<mitk::NDITrackingDevice*>(trackingDevice.GetPointer());

    try
    {
      //connect to aurora to dectect tools automatically
      thisDevice->OpenConnection();
    }
    catch (mitk::IGTHardwareException& e)
    {
      errorMessage->append("Hardware error on opening the connection (");
      errorMessage->append(e.GetDescription());
      errorMessage->append(")");
      return nullptr;
    }
    catch (mitk::IGTException& e)
    {
      errorMessage->append("Error on opening the connection (");
      errorMessage->append(e.GetDescription());
      errorMessage->append(")");
      return nullptr;
    }

    //now search for automatically detected tools in the tool storage and save them
    mitk::NavigationToolStorage::Pointer newToolStorageInRightOrder = mitk::NavigationToolStorage::New();
    std::vector<unsigned int> alreadyFoundTools = std::vector<unsigned int>();
    *toolCorrespondencesInToolStorage = std::vector<int>();
    for (unsigned int i = 0; i < thisDevice->GetToolCount(); i++)
    {
      bool toolFound = false;
      for (unsigned int j = 0; j < navigationTools->GetToolCount(); j++)
      {
        //check if the serial number is the same to identify the tool
        if ((dynamic_cast<mitk::NDIPassiveTool*>(thisDevice->GetTool(i)))->GetSerialNumber() == navigationTools->GetTool(j)->GetSerialNumber())
        {
          //check if this tool was already added to make sure that every tool is only added once (in case of same serial numbers)
          bool toolAlreadyAdded = false;
          for (unsigned int k = 0; k < alreadyFoundTools.size(); k++)
          {
            if (alreadyFoundTools.at(k) == j)
            {
              toolAlreadyAdded = true;
            }
          }

          if (!toolAlreadyAdded)
          {
            //add tool in right order
            newToolStorageInRightOrder->AddTool(navigationTools->GetTool(j));
            toolCorrespondencesInToolStorage->push_back(j);
            //adapt name of tool
            dynamic_cast<mitk::NDIPassiveTool*>(thisDevice->GetTool(i))->SetToolName(navigationTools->GetTool(j)->GetToolName());
            //set tip of tool
            dynamic_cast<mitk::NDIPassiveTool*>(thisDevice->GetTool(i))->SetToolTipPosition(navigationTools->GetTool(j)->GetToolTipPosition(), navigationTools->GetTool(j)->GetToolAxisOrientation());
            //rember that this tool was already found
            alreadyFoundTools.push_back(j);

            toolFound = true;
            break;
          }
        }
      }
      if (!toolFound)
      {
        errorMessage->append("Error: did not find every automatically detected tool in the loaded tool storage: aborting initialization.");
        return nullptr;
      }
    }

    //And resort them (this was done in TrackingToolBoxWorker before).
    for (unsigned int i = 0; i < newToolStorageInRightOrder->GetToolCount(); i++)
    {
      navigationTools->AssignToolNumber(newToolStorageInRightOrder->GetTool(i)->GetIdentifier(), i);
    }

    returnValue->SetTrackingDevice(thisDevice);
    MITK_DEBUG << "Number of tools of created tracking device: " << thisDevice->GetToolCount();
    MITK_DEBUG << "Number of outputs of created source: " << returnValue->GetNumberOfOutputs();
    return returnValue;
  }
}
