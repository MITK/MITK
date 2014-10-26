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

#include "mitkUSCombinedModality.h"
#include "mitkUSDevice.h"
#include "mitkNavigationDataSource.h"
#include "mitkImageReadAccessor.h"
#include <mitkNavigationDataSmoothingFilter.h>
#include <mitkNavigationDataDelayFilter.h>
#include "mitkTrackingDeviceSource.h"

// US Control Interfaces
#include "mitkUSControlInterfaceProbes.h"
#include "mitkUSControlInterfaceBMode.h"
#include "mitkUSControlInterfaceDoppler.h"

#include <algorithm>

//TempIncludes
#include <tinyxml.h>

const std::string mitk::USCombinedModality::DeviceClassIdentifier = "org.mitk.modules.us.USCombinedModality";
const char* mitk::USCombinedModality::DefaultProbeIdentifier = "default";
const char* mitk::USCombinedModality::ProbeAndDepthSeperator = "_";

mitk::USCombinedModality::USCombinedModality(USDevice::Pointer usDevice, NavigationDataSource::Pointer trackingDevice, std::string manufacturer, std::string model)
  : mitk::USDevice(manufacturer, model), m_UltrasoundDevice(usDevice), m_TrackingDevice(trackingDevice),
  m_SmoothingFilter(mitk::NavigationDataSmoothingFilter::New()), m_DelayFilter(mitk::NavigationDataDelayFilter::New(0)),
  m_NumberOfSmoothingValues(0), m_DelayCount(0)
{
  this->RebuildFilterPipeline();

  //create a new output (for the image data)
  mitk::Image::Pointer newOutput = mitk::Image::New();
  this->SetNthOutput(0,newOutput);

  // Combined Modality should not spawn an own acquire thread, because
  // image acquiring is done by the included us device
  this->SetSpawnAcquireThread(false);
}

mitk::USCombinedModality::~USCombinedModality()
{
}

std::string mitk::USCombinedModality::GetDeviceClass()
{
  return DeviceClassIdentifier;
}

mitk::USImageSource::Pointer mitk::USCombinedModality::GetUSImageSource()
{
  if (m_UltrasoundDevice.IsNull())
  {
    MITK_ERROR("USCombinedModality")("USDevice") << "UltrasoundDevice must not be null.";
    mitkThrow() << "UltrasoundDevice must not be null.";
  }

  return m_UltrasoundDevice->GetUSImageSource();
}

mitk::USAbstractControlInterface::Pointer mitk::USCombinedModality::GetControlInterfaceCustom()
{
    if (m_UltrasoundDevice.IsNull())
  {
    MITK_ERROR("USCombinedModality")("USDevice") << "UltrasoundDevice must not be null.";
    mitkThrow() << "UltrasoundDevice must not be null.";
  }

  return m_UltrasoundDevice->GetControlInterfaceCustom();
}

mitk::USControlInterfaceBMode::Pointer mitk::USCombinedModality::GetControlInterfaceBMode()
{
  if (m_UltrasoundDevice.IsNull())
  {
    MITK_ERROR("USCombinedModality")("USDevice") << "UltrasoundDevice must not be null.";
    mitkThrow() << "UltrasoundDevice must not be null.";
  }

  return m_UltrasoundDevice->GetControlInterfaceBMode();
}

mitk::USControlInterfaceProbes::Pointer mitk::USCombinedModality::GetControlInterfaceProbes()
{
  if (m_UltrasoundDevice.IsNull())
  {
    MITK_ERROR("USCombinedModality")("USDevice") << "UltrasoundDevice must not be null.";
    mitkThrow() << "UltrasoundDevice must not be null.";
  }

  return m_UltrasoundDevice->GetControlInterfaceProbes();
}

mitk::USControlInterfaceDoppler::Pointer mitk::USCombinedModality::GetControlInterfaceDoppler()
{
  if (m_UltrasoundDevice.IsNull())
  {
    MITK_ERROR("USCombinedModality")("USDevice") << "UltrasoundDevice must not be null.";
    mitkThrow() << "UltrasoundDevice must not be null.";
  }

  return m_UltrasoundDevice->GetControlInterfaceDoppler();
}

void mitk::USCombinedModality::UnregisterOnService()
{
  if (m_DeviceState == State_Activated) { this->Deactivate(); }
  if (m_DeviceState == State_Connected) { this->Disconnect(); }

  mitk::USDevice::UnregisterOnService();
}

mitk::AffineTransform3D::Pointer mitk::USCombinedModality::GetCalibration()
{
  return this->GetCalibration(this->GetCurrentDepthValue(), this->GetIdentifierForCurrentProbe());
}

mitk::AffineTransform3D::Pointer mitk::USCombinedModality::GetCalibration(std::string depth)
{
  return this->GetCalibration(depth, this->GetIdentifierForCurrentProbe());
}

mitk::AffineTransform3D::Pointer mitk::USCombinedModality::GetCalibration(std::string depth, std::string probe)
{
  // make sure that there is no '/' which would cause problems for TinyXML
  std::replace(probe.begin(), probe.end(), '/', '-');

  // create identifier for calibration from probe and depth
  std::string calibrationKey = probe + mitk::USCombinedModality::ProbeAndDepthSeperator + depth;

  // find calibration for combination of probe identifier and depth
  std::map<std::string, mitk::AffineTransform3D::Pointer>::iterator calibrationIterator
    = m_Calibrations.find(calibrationKey);

  if (calibrationIterator == m_Calibrations.end()) { return 0; }

  return calibrationIterator->second;
}

void mitk::USCombinedModality::SetCalibration (mitk::AffineTransform3D::Pointer calibration)
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

bool mitk::USCombinedModality::RemoveCalibration()
{
  return this->RemoveCalibration(this->GetCurrentDepthValue(), this->GetIdentifierForCurrentProbe());
}

bool mitk::USCombinedModality::RemoveCalibration(std::string depth)
{
  return this->RemoveCalibration(depth, this->GetIdentifierForCurrentProbe());
}

bool mitk::USCombinedModality::RemoveCalibration(std::string depth, std::string probe)
{
  // make sure that there is no '/' which would cause problems for TinyXML
  std::replace(probe.begin(), probe.end(), '/', '-');

  // create identifier for calibration from probe and depth
  std::string calibrationKey = probe + mitk::USCombinedModality::ProbeAndDepthSeperator + depth;

  return m_Calibrations.erase(calibrationKey) > 0;
}

void mitk::USCombinedModality::SetNumberOfSmoothingValues(unsigned int numberOfSmoothingValues)
{
  unsigned int oldNumber = m_NumberOfSmoothingValues;
  m_NumberOfSmoothingValues = numberOfSmoothingValues;

  // if filter should be activated or deactivated
  if ( ( oldNumber == 0 && numberOfSmoothingValues != 0 ) ||
    ( oldNumber != 0 && numberOfSmoothingValues == 0 ) )
  {
    this->RebuildFilterPipeline();
  }
  m_SmoothingFilter->SetNumerOfValues(numberOfSmoothingValues);
}

void mitk::USCombinedModality::SetDelayCount(unsigned int delayCount)
{
  unsigned int oldCount = m_DelayCount;
  m_DelayCount = delayCount;

  // if filter should be activated or deactivated
  if ( ( oldCount == 0 && delayCount != 0 ) ||
    ( oldCount != 0 && delayCount == 0 ) )
  {
    this->RebuildFilterPipeline();
  }
  m_DelayFilter->SetDelay(delayCount);
}

bool mitk::USCombinedModality::OnInitialization()
{
  if (m_UltrasoundDevice.IsNull())
  {
    MITK_ERROR("USCombinedModality")("USDevice") << "UltrasoundDevice must not be null.";
    mitkThrow() << "UltrasoundDevice must not be null.";
  }

  if ( m_UltrasoundDevice->GetDeviceState() < mitk::USDevice::State_Initialized )
  {
    return m_UltrasoundDevice->Initialize();
  }
  else
  {
    return true;
  }
}

bool mitk::USCombinedModality::OnConnection()
{
  if (m_UltrasoundDevice.IsNull())
  {
    MITK_ERROR("USCombinedModality")("USDevice") << "UltrasoundDevice must not be null.";
    mitkThrow() << "UltrasoundDevice must not be null.";
  }

  // connect ultrasound device only if it is not already connected
  if ( m_UltrasoundDevice->GetDeviceState() >= mitk::USDevice::State_Connected )
  {
    return true;
  }
  else
  {
    return m_UltrasoundDevice->Connect();
  }
}

bool mitk::USCombinedModality::OnDisconnection()
{
  if (m_UltrasoundDevice.IsNull())
  {
    MITK_ERROR("USCombinedModality")("USDevice") << "UltrasoundDevice must not be null.";
    mitkThrow() << "UltrasoundDevice must not be null.";
  }

  return m_UltrasoundDevice->Disconnect();
}

bool mitk::USCombinedModality::OnActivation()
{
  if ( m_UltrasoundDevice.IsNull() )
  {
    MITK_ERROR("USCombinedModality")("USDevice") << "UltrasoundDevice must not be null.";
    mitkThrow() << "UltrasoundDevice must not be null.";
  }

  mitk::TrackingDeviceSource::Pointer trackingDeviceSource = dynamic_cast<mitk::TrackingDeviceSource*>(m_TrackingDevice.GetPointer());
  if ( trackingDeviceSource.IsNull() )
  {
    MITK_WARN("USCombinedModality")("USDevice") << "Cannot start tracking as TrackingDeviceSource is null.";
  }
  trackingDeviceSource->StartTracking();

  // activate ultrasound device only if it is not already activated
  if ( m_UltrasoundDevice->GetDeviceState() >= mitk::USDevice::State_Activated )
  {
    return true;
  }
  else
  {
    return m_UltrasoundDevice->Activate();
  }
}

bool mitk::USCombinedModality::OnDeactivation()
{
  if ( m_UltrasoundDevice.IsNull() )
  {
    MITK_ERROR("USCombinedModality")("USDevice") << "UltrasoundDevice must not be null.";
    mitkThrow() << "UltrasoundDevice must not be null.";
  }

  mitk::TrackingDeviceSource::Pointer trackingDeviceSource = dynamic_cast<mitk::TrackingDeviceSource*>(m_TrackingDevice.GetPointer());
  if ( trackingDeviceSource.IsNull() )
  {
    MITK_WARN("USCombinedModality")("USDevice") << "Cannot stop tracking as TrackingDeviceSource is null.";
  }
  trackingDeviceSource->StopTracking();

  m_UltrasoundDevice->Deactivate();

  return m_UltrasoundDevice->GetIsConnected();
}

void mitk::USCombinedModality::OnFreeze(bool freeze)
{
  if (m_UltrasoundDevice.IsNull())
  {
    MITK_ERROR("USCombinedModality")("USDevice") << "UltrasoundDevice must not be null.";
    mitkThrow() << "UltrasoundDevice must not be null.";
  }
  m_UltrasoundDevice->SetIsFreezed(freeze);

  mitk::TrackingDeviceSource::Pointer trackingDeviceSource = dynamic_cast<mitk::TrackingDeviceSource*>(m_TrackingDevice.GetPointer());
  if ( trackingDeviceSource.IsNull() )
  {
    MITK_WARN("USCombinedModality")("USDevice") << "Cannot freeze tracking.";
  }
  else
  {
    if ( freeze ) { trackingDeviceSource->StopTracking(); }
    else { trackingDeviceSource->StartTracking(); }
  }
}

mitk::NavigationDataSource::Pointer mitk::USCombinedModality::GetNavigationDataSource()
{
  return m_LastFilter.GetPointer();
}

bool mitk::USCombinedModality::GetIsCalibratedForCurrentStatus()
{
  return m_Calibrations.find(this->GetIdentifierForCurrentCalibration()) != m_Calibrations.end();
}

bool mitk::USCombinedModality::GetContainsAtLeastOneCalibration()
{
  return ! m_Calibrations.empty();
}

void mitk::USCombinedModality::GenerateData()
{
  if (m_UltrasoundDevice->GetIsFreezed()) {return;} //if the image is freezed: do nothing

  //get next image from ultrasound image source
  mitk::Image::Pointer image = m_UltrasoundDevice->GetUSImageSource()->GetNextImage();

  if ( image.IsNull() || ! image->IsInitialized() ) //check the image
    {
    MITK_WARN << "Invalid image in USCombinedModality, aborting!";
    return;
    }

  //get output and initialize it if it wasn't initialized before
  mitk::Image::Pointer output = this->GetOutput();
  if ( ! output->IsInitialized() ) { output->Initialize(image); }

  //now update image data
  mitk::ImageReadAccessor inputReadAccessor(image, image->GetSliceData(0,0,0));
  output->SetSlice(inputReadAccessor.GetData()); //copy image data
  output->GetGeometry()->SetSpacing(image->GetGeometry()->GetSpacing()); //copy spacing because this might also change

  //and update calibration (= transformation of the image)
  std::string calibrationKey = this->GetIdentifierForCurrentCalibration();
  if ( ! calibrationKey.empty() )
  {
    std::map<std::string, mitk::AffineTransform3D::Pointer>::iterator calibrationIterator
      = m_Calibrations.find(calibrationKey);
    if ( calibrationIterator != m_Calibrations.end())
    {
      // transform image according to callibration if one is set
      // for current configuration of probe and depth
      this->GetOutput()->GetGeometry()->SetIndexToWorldTransform(calibrationIterator->second);
    }
  }
}

std::string mitk::USCombinedModality::SerializeCalibration()
{
  std::stringstream result;
  result << "<calibrations>" << std::endl;
  // For each calibration in the set
  for (std::map<std::string, mitk::AffineTransform3D::Pointer>::iterator it = m_Calibrations.begin(); it != m_Calibrations.end(); it++)
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

void mitk::USCombinedModality::DeserializeCalibration(const std::string& xmlString, bool clearPreviousCalibrations)
{
  // Sanitize Input
  if (xmlString == "")
  {
    MITK_ERROR << "Empty string passed to Deserialize() method of CombinedModality. Aborting...";
    mitkThrow() << "Empty string passed to Deserialize() method of CombinedModality. Aborting...";
    return;
  }
  // Clear previous calibrations if necessary
  if (clearPreviousCalibrations) m_Calibrations.clear();

  // Parse Input
  TiXmlDocument doc;
  if(!doc.Parse(xmlString.c_str()))
  {
    MITK_ERROR << "Unable to deserialize calibrations in CombinedModality. Error was: " << doc.ErrorDesc();
    mitkThrow() << "Unable to deserialize calibrations in CombinedModality. Error was: " << doc.ErrorDesc();
    return;
  }
  TiXmlElement* root = doc.FirstChildElement();
  if(root == NULL)
  {
    MITK_ERROR << "Unable to deserialize calibrations in CombinedModality. String contained no root element.";
    mitkThrow() << "Unable to deserialize calibrations in CombinedModality. String contained no root element.";
    return;
  }
  // Read Calibrations
  for(TiXmlElement* elem = root->FirstChildElement(); elem != NULL; elem = elem->NextSiblingElement())
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

std::string mitk::USCombinedModality::GetIdentifierForCurrentCalibration()
{
  return this->GetIdentifierForCurrentProbe()
    + mitk::USCombinedModality::ProbeAndDepthSeperator
    + this->GetCurrentDepthValue();
}

std::string mitk::USCombinedModality::GetIdentifierForCurrentProbe()
{
  us::ServiceProperties usdeviceProperties = m_UltrasoundDevice->GetServiceProperties();

  us::ServiceProperties::const_iterator probeIt = usdeviceProperties.find(
        mitk::USCombinedModality::GetPropertyKeys().US_PROPKEY_PROBES_SELECTED);

  // get probe identifier from control interface for probes
  std::string probeName = mitk::USCombinedModality::DefaultProbeIdentifier;
  if (probeIt != usdeviceProperties.end())
  {
    probeName = (probeIt->second).ToString();
  }

  // make sure that there is no '/' which would cause problems for TinyXML
  std::replace(probeName.begin(), probeName.end(), '/', '-');

  return probeName;
}

std::string mitk::USCombinedModality::GetCurrentDepthValue()
{
  us::ServiceProperties usdeviceProperties = m_UltrasoundDevice->GetServiceProperties();

  // get string for depth value from the micro service properties
  std::string depth;
  us::ServiceProperties::iterator depthIterator = usdeviceProperties.find(
    mitk::USCombinedModality::GetPropertyKeys().US_PROPKEY_BMODE_DEPTH);

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

void mitk::USCombinedModality::RebuildFilterPipeline()
{
  m_LastFilter = m_TrackingDevice;

  if ( m_NumberOfSmoothingValues > 0 )
  {
    for (unsigned int i = 0; i < m_TrackingDevice->GetNumberOfOutputs(); i++)
    {
      m_SmoothingFilter->SetInput(i, m_LastFilter->GetOutput(i));
    }
    m_LastFilter = m_SmoothingFilter;
  }

  if ( m_DelayCount > 0 )
  {
    for (unsigned int i = 0; i < m_TrackingDevice->GetNumberOfOutputs(); i++)
    {
      m_DelayFilter->SetInput(i, m_LastFilter->GetOutput(i));
    }
    m_LastFilter = m_DelayFilter;
  }
}
