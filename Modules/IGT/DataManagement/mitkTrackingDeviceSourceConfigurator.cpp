/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkTrackingDeviceSourceConfigurator.h"

#include "mitkNDITrackingDevice.h"
#include "mitkClaronTrackingDevice.h"
#include "mitkOptitrackTrackingDevice.h"
#include "mitkOpenIGTLinkTrackingDevice.h"
#include "mitkVirtualTrackingDevice.h"

#include <mitkIGTException.h>
#include <mitkIGTHardwareException.h>

#include <usGetModuleContext.h>
#include <usModule.h>
#include <usServiceProperties.h>
#include <usModuleContext.h>
#include <mitkTrackingDeviceTypeCollection.h>


mitk::TrackingDeviceSourceConfigurator::TrackingDeviceSourceConfigurator(mitk::NavigationToolStorage::Pointer NavigationTools, mitk::TrackingDevice::Pointer TrackingDevice)
{
//make a copy of the navigation tool storage because we will modify the storage
if (NavigationTools.IsNotNull())
  {
  m_NavigationTools = mitk::NavigationToolStorage::New();
  for (unsigned int i=0; i<NavigationTools->GetToolCount(); i++)
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
  m_ErrorMessage = "NavigationToolStorage is nullptr!";
  return false;
  }
else if (m_TrackingDevice.IsNull())
  {
  m_ErrorMessage = "TrackingDevice is nullptr!";
  return false;
  }
else
  {
  for (unsigned int i=0; i<m_NavigationTools->GetToolCount(); i++)
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
mitk::NavigationDataObjectVisualizationFilter::Pointer dummy; //this dummy is lost directly after creating the device
return this->CreateTrackingDeviceSource(dummy);
}

mitk::TrackingDeviceSource::Pointer mitk::TrackingDeviceSourceConfigurator::CreateTrackingDeviceSource(mitk::NavigationDataObjectVisualizationFilter::Pointer &visualizationFilter)
{
  if (!this->IsCreateTrackingDeviceSourcePossible()) {MITK_WARN << "Cannot create tracking decive: " << m_ErrorMessage; return nullptr;}

  mitk::TrackingDeviceSource::Pointer returnValue;

  us::ModuleContext* context = us::GetModuleContext();

  std::vector<us::ServiceReference<mitk::TrackingDeviceTypeCollection> > refs = context->GetServiceReferences<mitk::TrackingDeviceTypeCollection>();

  if (refs.empty())
  {
    MITK_ERROR << "No tracking device service found!";
  }

  mitk::TrackingDeviceTypeCollection* deviceTypeCollection = context->GetService<mitk::TrackingDeviceTypeCollection>(refs.front());

  //create tracking device source
  returnValue = deviceTypeCollection->GetTrackingDeviceTypeInformation(m_TrackingDevice->GetType())->
      CreateTrackingDeviceSource(m_TrackingDevice,m_NavigationTools, &m_ErrorMessage, &m_ToolCorrespondencesInToolStorage);

  //TODO: insert other tracking systems?
  if (returnValue.IsNull()) {MITK_WARN << "Cannot create tracking decive: " << m_ErrorMessage; return nullptr;}

  //create visualization filter
  visualizationFilter = CreateNavigationDataObjectVisualizationFilter(returnValue,m_NavigationTools);
  if (visualizationFilter.IsNull()) {MITK_WARN << "Cannot create tracking decive: " << m_ErrorMessage; return nullptr;}

  return returnValue;
}

std::string mitk::TrackingDeviceSourceConfigurator::GetErrorMessage()
{
  return this->m_ErrorMessage;
}

//############################ internal help methods ########################################

mitk::NavigationDataObjectVisualizationFilter::Pointer mitk::TrackingDeviceSourceConfigurator::CreateNavigationDataObjectVisualizationFilter(mitk::TrackingDeviceSource::Pointer trackingDeviceSource, mitk::NavigationToolStorage::Pointer navigationTools)
  {
  mitk::NavigationDataObjectVisualizationFilter::Pointer returnValue = mitk::NavigationDataObjectVisualizationFilter::New();
  for (unsigned int i=0; i<trackingDeviceSource->GetNumberOfIndexedOutputs(); i++)
    {
    // Note: If all tools have the same name only the first tool will always be returned and
    //       the others won't be updated during rendering.This could potentially lead to inconstencies
    mitk::NavigationTool::Pointer currentTool = navigationTools->GetToolByName(trackingDeviceSource->GetOutput(i)->GetName());
    if (currentTool.IsNull())
      {
      this->m_ErrorMessage = "Error: did not find corresponding tool in tracking device after initialization.";
      return nullptr;
      }
    returnValue->SetInput(i,trackingDeviceSource->GetOutput(i));
    returnValue->SetRepresentationObject(i,currentTool->GetDataNode()->GetData());
    }
  return returnValue;
  }

int mitk::TrackingDeviceSourceConfigurator::GetToolNumberInToolStorage(unsigned int outputID)
  {
  if (outputID < m_ToolCorrespondencesInToolStorage.size()) return m_ToolCorrespondencesInToolStorage.at(outputID);
  else return -1;
  }

std::string mitk::TrackingDeviceSourceConfigurator::GetToolIdentifierInToolStorage(unsigned int outputID)
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
  for (unsigned int i=0; i<m_ToolCorrespondencesInToolStorage.size(); i++)
    {returnValue.push_back(m_NavigationTools->GetTool(m_ToolCorrespondencesInToolStorage.at(i))->GetIdentifier());}
  return returnValue;
  }
