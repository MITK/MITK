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

#include "mitkAbstractUltrasoundTrackerDevice.h"
#include "mitkImageReadAccessor.h"
#include "mitkNavigationDataDelayFilter.h"
#include "mitkNavigationDataDisplacementFilter.h"
#include "mitkNavigationDataSmoothingFilter.h"
#include "mitkTrackingDeviceSource.h"

// US Control Interfaces
#include "mitkUSControlInterfaceBMode.h"
#include "mitkUSControlInterfaceDoppler.h"
#include "mitkUSControlInterfaceProbes.h"

// Microservices
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleContext.h>
#include <usServiceProperties.h>

#include <algorithm>

// TempIncludes
#include <tinyxml.h>

const std::string mitk::AbstractUltrasoundTrackerDevice::DeviceClassIdentifier =
  "org.mitk.modules.us.AbstractUltrasoundTrackerDevice";
const char *mitk::AbstractUltrasoundTrackerDevice::DefaultProbeIdentifier = "default";
const char *mitk::AbstractUltrasoundTrackerDevice::ProbeAndDepthSeperator = "_";

const std::string mitk::AbstractUltrasoundTrackerDevice::US_INTERFACE_NAME =
  "org.mitk.services.AbstractUltrasoundTrackerDevice";
const std::string mitk::AbstractUltrasoundTrackerDevice::US_PROPKEY_DEVICENAME = US_INTERFACE_NAME + ".devicename";
const std::string mitk::AbstractUltrasoundTrackerDevice::US_PROPKEY_CLASS = US_INTERFACE_NAME + ".class";
const std::string mitk::AbstractUltrasoundTrackerDevice::US_PROPKEY_ID = US_INTERFACE_NAME + ".id";
//____

mitk::AbstractUltrasoundTrackerDevice::AbstractUltrasoundTrackerDevice(USDevice::Pointer usDevice,
                                                                       NavigationDataSource::Pointer trackingDevice,
                                                                       bool trackedUltrasoundActive)
  : m_UltrasoundDevice(usDevice),
    m_TrackingDeviceDataSource(trackingDevice),
    m_SmoothingFilter(mitk::NavigationDataSmoothingFilter::New()),
    m_DelayFilter(mitk::NavigationDataDelayFilter::New(0)),
    m_DisplacementFilter(mitk::NavigationDataDisplacementFilter::New()),
    m_LastFilterOfIGTPipeline(nullptr),
    m_NumberOfSmoothingValues(0),
    m_DelayCount(0),
    m_IsTrackedUltrasoundActive(trackedUltrasoundActive)
{
  m_DisplacementFilter->SetTransform6DOF(true);

  this->RebuildFilterPipeline();

  // create a new output (for the image data)
  //___ mitk::Image::Pointer newOutput = mitk::Image::New();
  //___ this->SetNthOutput(0, newOutput);

  // Combined Modality should not spawn an own acquire thread, because
  // image acquiring is done by the included us device
  //___ m_UltrasoundDevice->SetSpawnAcquireThread(false);
}

mitk::AffineTransform3D::Pointer mitk::AbstractUltrasoundTrackerDevice::GetUSPlaneTransform()
{
  return mitk::AffineTransform3D::New();
}

void mitk::AbstractUltrasoundTrackerDevice::SetIsFreezed(bool freeze)
{
  if (m_UltrasoundDevice.IsNull() || m_TrackingDeviceDataSource.IsNull())
  {
    MITK_WARN << "Combined modality not correctly initialized, aborting!";
    return;
  }

  if (!m_UltrasoundDevice->GetIsActive())
  {
    MITK_WARN("mitkUSDevice")
      << "Cannot freeze or unfreeze if device is not active.";
    return;
  }

  this->OnFreeze(freeze);

  if (freeze)
  {
    m_IsFreezed = true;
  }
  else
  {
    m_IsFreezed = false;
  }
}

bool mitk::AbstractUltrasoundTrackerDevice::GetIsFreezed()
{
  return m_IsFreezed;
}

mitk::AbstractUltrasoundTrackerDevice::~AbstractUltrasoundTrackerDevice()
{
  if (m_ServiceRegistration != nullptr)
  {
    m_ServiceRegistration.Unregister();
  }
  m_ServiceRegistration = 0;
}

mitk::AffineTransform3D::Pointer mitk::AbstractUltrasoundTrackerDevice::GetCalibration()
{
  return this->GetCalibration(this->GetCurrentDepthValue(), this->GetIdentifierForCurrentProbe());
}

mitk::AffineTransform3D::Pointer mitk::AbstractUltrasoundTrackerDevice::GetCalibration(std::string depth)
{
  return this->GetCalibration(depth, this->GetIdentifierForCurrentProbe());
}

mitk::AffineTransform3D::Pointer mitk::AbstractUltrasoundTrackerDevice::GetCalibration(std::string depth,
                                                                                       std::string probe)
{
  // make sure that there is no '/' which would cause problems for TinyXML
  std::replace(probe.begin(), probe.end(), '/', '-');

  // create identifier for calibration from probe and depth
  std::string calibrationKey = probe + mitk::AbstractUltrasoundTrackerDevice::ProbeAndDepthSeperator + depth;

  // find calibration for combination of probe identifier and depth
  std::map<std::string, mitk::AffineTransform3D::Pointer>::iterator calibrationIterator =
    m_Calibrations.find(calibrationKey);

  if (calibrationIterator == m_Calibrations.end())
  {
    return 0;
  }

  return calibrationIterator->second;
}

void mitk::AbstractUltrasoundTrackerDevice::SetCalibration(mitk::AffineTransform3D::Pointer calibration)
{
  if (calibration.IsNull())
  {
    MITK_WARN << "Null pointer passed to SetCalibration of mitk::USDevice. Ignoring call.";
    return;
  }

  std::string calibrationKey = this->GetIdentifierForCurrentCalibration();
  if (calibrationKey.empty())
  {
    MITK_WARN << "Could not get a key for the calibration -> Calibration cannot be set.";
    return;
  }

  m_Calibrations[calibrationKey] = calibration;
}

bool mitk::AbstractUltrasoundTrackerDevice::RemoveCalibration()
{
  return this->RemoveCalibration(this->GetCurrentDepthValue(), this->GetIdentifierForCurrentProbe());
}

bool mitk::AbstractUltrasoundTrackerDevice::RemoveCalibration(std::string depth)
{
  return this->RemoveCalibration(depth, this->GetIdentifierForCurrentProbe());
}

bool mitk::AbstractUltrasoundTrackerDevice::RemoveCalibration(std::string depth, std::string probe)
{
  // make sure that there is no '/' which would cause problems for TinyXML
  std::replace(probe.begin(), probe.end(), '/', '-');

  // create identifier for calibration from probe and depth
  std::string calibrationKey = probe + mitk::AbstractUltrasoundTrackerDevice::ProbeAndDepthSeperator + depth;

  return m_Calibrations.erase(calibrationKey) > 0;
}

std::string mitk::AbstractUltrasoundTrackerDevice::GetDeviceClass()
{
  return DeviceClassIdentifier;
}

mitk::USImageSource::Pointer mitk::AbstractUltrasoundTrackerDevice::GetUSImageSource()
{
  if (m_UltrasoundDevice.IsNull())
  {
    MITK_ERROR("AbstractUltrasoundTrackerDevice")("USDevice") << "UltrasoundDevice must not be null.";
    mitkThrow() << "UltrasoundDevice must not be null.";
  }

  return m_UltrasoundDevice->GetUSImageSource();
}

mitk::NavigationDataSource::Pointer mitk::AbstractUltrasoundTrackerDevice::GetNavigationDataSource()
{
  if (m_LastFilterOfIGTPipeline.IsNull())
  {
    this->RebuildFilterPipeline();
  }
  m_LastFilterOfIGTPipeline->SetToolMetaDataCollection(this->m_TrackingDeviceDataSource->GetToolMetaDataCollection());
  return m_LastFilterOfIGTPipeline;
}

bool mitk::AbstractUltrasoundTrackerDevice::GetIsCalibratedForCurrentStatus()
{
  return m_Calibrations.find(this->GetIdentifierForCurrentCalibration()) != m_Calibrations.end();
}

bool mitk::AbstractUltrasoundTrackerDevice::GetContainsAtLeastOneCalibration()
{
  return !m_Calibrations.empty();
}

std::string mitk::AbstractUltrasoundTrackerDevice::SerializeCalibration()
{
  std::stringstream result;
  result << "<calibrations>" << std::endl;
  // For each calibration in the set
  for (std::map<std::string, mitk::AffineTransform3D::Pointer>::iterator it = m_Calibrations.begin();
       it != m_Calibrations.end();
       it++)
  {
    mitk::AffineTransform3D::MatrixType matrix = it->second->GetMatrix();
    mitk::AffineTransform3D::TranslationType translation = it->second->GetTranslation();
    TiXmlElement elem(it->first);
    // Serialize Matrix
    elem.SetDoubleAttribute("M00", matrix[0][0]);
    elem.SetDoubleAttribute("M01", matrix[0][1]);
    elem.SetDoubleAttribute("M02", matrix[0][2]);
    elem.SetDoubleAttribute("M10", matrix[1][0]);
    elem.SetDoubleAttribute("M11", matrix[1][1]);
    elem.SetDoubleAttribute("M12", matrix[1][2]);
    elem.SetDoubleAttribute("M20", matrix[2][0]);
    elem.SetDoubleAttribute("M21", matrix[2][1]);
    elem.SetDoubleAttribute("M22", matrix[2][2]);
    // Serialize Offset
    elem.SetDoubleAttribute("T0", translation[0]);
    elem.SetDoubleAttribute("T1", translation[1]);
    elem.SetDoubleAttribute("T2", translation[2]);

    result << elem << std::endl;
  }
  result << "</calibrations>" << std::endl;

  return result.str();
}

void mitk::AbstractUltrasoundTrackerDevice::DeserializeCalibration(const std::string &xmlString,
                                                                   bool clearPreviousCalibrations)
{
  // Sanitize Input
  if (xmlString == "")
  {
    MITK_ERROR << "Empty string passed to Deserialize() method of CombinedModality. Aborting...";
    mitkThrow() << "Empty string passed to Deserialize() method of CombinedModality. Aborting...";
    return;
  }
  // Clear previous calibrations if necessary
  if (clearPreviousCalibrations)
    m_Calibrations.clear();

  // Parse Input
  TiXmlDocument doc;
  if (!doc.Parse(xmlString.c_str()))
  {
    MITK_ERROR << "Unable to deserialize calibrations in CombinedModality. Error was: " << doc.ErrorDesc();
    mitkThrow() << "Unable to deserialize calibrations in CombinedModality. Error was: " << doc.ErrorDesc();
    return;
  }
  TiXmlElement *root = doc.FirstChildElement();
  if (root == nullptr)
  {
    MITK_ERROR << "Unable to deserialize calibrations in CombinedModality. String contained no root element.";
    mitkThrow() << "Unable to deserialize calibrations in CombinedModality. String contained no root element.";
    return;
  }
  // Read Calibrations
  for (TiXmlElement *elem = root->FirstChildElement(); elem != nullptr; elem = elem->NextSiblingElement())
  {
    mitk::AffineTransform3D::MatrixType matrix;
    mitk::AffineTransform3D::OffsetType translation;

    std::string calibName = elem->Value();

    // Deserialize Matrix
    elem->QueryDoubleAttribute("M00", &matrix[0][0]);
    elem->QueryDoubleAttribute("M01", &matrix[0][1]);
    elem->QueryDoubleAttribute("M02", &matrix[0][2]);
    elem->QueryDoubleAttribute("M10", &matrix[1][0]);
    elem->QueryDoubleAttribute("M11", &matrix[1][1]);
    elem->QueryDoubleAttribute("M12", &matrix[1][2]);
    elem->QueryDoubleAttribute("M20", &matrix[2][0]);
    elem->QueryDoubleAttribute("M21", &matrix[2][1]);
    elem->QueryDoubleAttribute("M22", &matrix[2][2]);

    // Deserialize Offset
    elem->QueryDoubleAttribute("T0", &translation[0]);
    elem->QueryDoubleAttribute("T1", &translation[1]);
    elem->QueryDoubleAttribute("T2", &translation[2]);

    mitk::AffineTransform3D::Pointer calibration = mitk::AffineTransform3D::New();
    calibration->SetMatrix(matrix);
    calibration->SetTranslation(translation);
    m_Calibrations[calibName] = calibration;
  }
}

void mitk::AbstractUltrasoundTrackerDevice::SetNumberOfSmoothingValues(unsigned int numberOfSmoothingValues)
{
  unsigned int oldNumber = m_NumberOfSmoothingValues;
  m_NumberOfSmoothingValues = numberOfSmoothingValues;

  // if filter should be activated or deactivated
  if ((oldNumber == 0 && numberOfSmoothingValues != 0) || (oldNumber != 0 && numberOfSmoothingValues == 0))
  {
    this->RebuildFilterPipeline();
  }
  m_SmoothingFilter->SetNumerOfValues(numberOfSmoothingValues);
}

void mitk::AbstractUltrasoundTrackerDevice::SetDelayCount(unsigned int delayCount)
{
  unsigned int oldCount = m_DelayCount;
  m_DelayCount = delayCount;

  // if filter should be activated or deactivated
  if ((oldCount == 0 && delayCount != 0) || (oldCount != 0 && delayCount == 0))
  {
    this->RebuildFilterPipeline();
  }
  m_DelayFilter->SetDelay(delayCount);
}

void mitk::AbstractUltrasoundTrackerDevice::GenerateData() {}

std::string mitk::AbstractUltrasoundTrackerDevice::GetIdentifierForCurrentCalibration()
{
  return this->GetIdentifierForCurrentProbe() + mitk::AbstractUltrasoundTrackerDevice::ProbeAndDepthSeperator +
         this->GetCurrentDepthValue();
}

std::string mitk::AbstractUltrasoundTrackerDevice::GetIdentifierForCurrentProbe()
{
  us::ServiceProperties usdeviceProperties = m_UltrasoundDevice->GetServiceProperties();

  us::ServiceProperties::const_iterator probeIt =
    usdeviceProperties.find(mitk::USDevice::GetPropertyKeys().US_PROPKEY_PROBES_SELECTED);

  // get probe identifier from control interface for probes
  std::string probeName = mitk::AbstractUltrasoundTrackerDevice::DefaultProbeIdentifier;
  if (probeIt != usdeviceProperties.end())
  {
    probeName = (probeIt->second).ToString();
  }

  // make sure that there is no '/' which would cause problems for TinyXML
  std::replace(probeName.begin(), probeName.end(), '/', '-');

  return probeName;
}

std::string mitk::AbstractUltrasoundTrackerDevice::GetCurrentDepthValue()
{
  us::ServiceProperties usdeviceProperties = m_UltrasoundDevice->GetServiceProperties();

  // get string for depth value from the micro service properties
  std::string depth;
  us::ServiceProperties::iterator depthIterator =
    usdeviceProperties.find(mitk::USDevice::GetPropertyKeys().US_PROPKEY_BMODE_DEPTH);

  if (depthIterator != usdeviceProperties.end())
  {
    depth = depthIterator->second.ToString();
  }
  else
  {
    depth = "0";
  }

  return depth;
}

void mitk::AbstractUltrasoundTrackerDevice::RebuildFilterPipeline()
{
  m_LastFilterOfIGTPipeline = m_TrackingDeviceDataSource;

  if (m_NumberOfSmoothingValues > 0)
  {
    m_SmoothingFilter->ConnectTo(m_LastFilterOfIGTPipeline.GetPointer());
    m_LastFilterOfIGTPipeline = m_SmoothingFilter;
  }

  if (m_DelayCount > 0)
  {
    m_DelayFilter->ConnectTo(m_LastFilterOfIGTPipeline.GetPointer());
    m_LastFilterOfIGTPipeline = m_DelayFilter;
  }

  if (m_IsTrackedUltrasoundActive)
  {
    m_DisplacementFilter->ConnectTo(m_LastFilterOfIGTPipeline.GetPointer());
    m_LastFilterOfIGTPipeline = m_DisplacementFilter;
  }
}

void mitk::AbstractUltrasoundTrackerDevice::UnregisterOnService()
{
  if (m_UltrasoundDevice->GetDeviceState() == USDevice::State_Activated)
  {
    m_UltrasoundDevice->Deactivate();
  }
  if (m_UltrasoundDevice->GetDeviceState() == USDevice::State_Connected)
  {
    m_UltrasoundDevice->Disconnect();
  }

  if (m_ServiceRegistration != nullptr)
    m_ServiceRegistration.Unregister();
  m_ServiceRegistration = 0;
}

void mitk::AbstractUltrasoundTrackerDevice::RegisterAsMicroservice()
{
  // Get Context
  us::ModuleContext *context = us::GetModuleContext();

  // Define ServiceProps
  // us::ServiceProperties props;
  mitk::UIDGenerator uidGen = mitk::UIDGenerator("org.mitk.services.AbstractUltrasoundTrackerDevice", 16);
  m_ServiceProperties[US_PROPKEY_ID] = uidGen.GetUID();
  m_ServiceProperties[US_PROPKEY_DEVICENAME] = m_UltrasoundDevice->GetName();
  m_ServiceProperties[US_PROPKEY_CLASS] = mitk::AbstractUltrasoundTrackerDevice::DeviceClassIdentifier;

  m_ServiceRegistration = context->RegisterService(this, m_ServiceProperties);
}

mitk::USAbstractControlInterface::Pointer mitk::AbstractUltrasoundTrackerDevice::GetControlInterfaceCustom()
{
  if (m_UltrasoundDevice.IsNull())
  {
    MITK_ERROR("USCombinedModality")("USDevice") << "UltrasoundDevice must not be null.";
    mitkThrow() << "UltrasoundDevice must not be null.";
  }

  return m_UltrasoundDevice->GetControlInterfaceCustom();
}

mitk::USControlInterfaceBMode::Pointer mitk::AbstractUltrasoundTrackerDevice::GetControlInterfaceBMode()
{
  if (m_UltrasoundDevice.IsNull())
  {
    MITK_ERROR("USCombinedModality")("USDevice") << "UltrasoundDevice must not be null.";
    mitkThrow() << "UltrasoundDevice must not be null.";
  }

  return m_UltrasoundDevice->GetControlInterfaceBMode();
}

mitk::USControlInterfaceProbes::Pointer mitk::AbstractUltrasoundTrackerDevice::GetControlInterfaceProbes()
{
  if (m_UltrasoundDevice.IsNull())
  {
    MITK_ERROR("USCombinedModality")("USDevice") << "UltrasoundDevice must not be null.";
    mitkThrow() << "UltrasoundDevice must not be null.";
  }

  return m_UltrasoundDevice->GetControlInterfaceProbes();
}

mitk::USControlInterfaceDoppler::Pointer mitk::AbstractUltrasoundTrackerDevice::GetControlInterfaceDoppler()
{
  if (m_UltrasoundDevice.IsNull())
  {
    MITK_ERROR("USCombinedModality")("USDevice") << "UltrasoundDevice must not be null.";
    mitkThrow() << "UltrasoundDevice must not be null.";
  }

  return m_UltrasoundDevice->GetControlInterfaceDoppler();
}
