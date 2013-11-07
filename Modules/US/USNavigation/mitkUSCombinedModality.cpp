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

//TempIncludes
#include <tinyxml.h>

const std::string mitk::USCombinedModality::DeviceClassIdentifier = "org.mitk.modules.us.USCombinedModality";

mitk::USCombinedModality::USCombinedModality(USDevice::Pointer usDevice, NavigationDataSource::Pointer trackingDevice, std::string manufacturer, std::string model)
  : mitk::USDevice(manufacturer, model), m_UltrasoundDevice(usDevice), m_TrackingDevice(trackingDevice),
  m_SmoothingFilter(mitk::NavigationDataSmoothingFilter::New()), m_DelayFilter(mitk::NavigationDataDelayFilter::New(0))
{
  // build tracking filter pipeline
  for (int i = 0; i < m_TrackingDevice->GetNumberOfOutputs(); i++)
  {
    m_SmoothingFilter->SetInput(i, m_TrackingDevice->GetOutput(i));
    mitk::NavigationData::Pointer nd1 = m_TrackingDevice->GetOutput(i);
    m_DelayFilter->SetInput(i, m_SmoothingFilter->GetOutput(i));
    mitk::NavigationData::Pointer nd2 = m_SmoothingFilter->GetOutput(i);
  }
  m_SmoothingFilter->SetNumerOfValues(10);
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
  std::string calibrationKey = this->GetIdentifierForCurrentCalibration();
  if (calibrationKey.empty())
  {
    MITK_WARN << "Could not get a key for the calibration.";
    return 0;
  }

  // find calibration for combination of probe identifier and depth
  std::map<std::string, mitk::AffineTransform3D::Pointer>::iterator calibrationIterator
    = m_Calibrations.find(calibrationKey);

  if (calibrationIterator == m_Calibrations.end())
  {
    MITK_WARN << "No calibration found for selected probe and depth.";
    return 0;
  }

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

  m_Metadata->SetDeviceIsCalibrated(true);

  if (m_ServiceRegistration != 0)
  {
    this->UpdateServiceProperty(mitk::USImageMetadata::PROP_DEV_ISCALIBRATED, true);
  }
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

  return m_UltrasoundDevice->Connect();
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
  if (m_UltrasoundDevice.IsNull())
  {
    MITK_ERROR("USCombinedModality")("USDevice") << "UltrasoundDevice must not be null.";
    mitkThrow() << "UltrasoundDevice must not be null.";
  }

  return m_UltrasoundDevice->Activate();
}

bool mitk::USCombinedModality::OnDeactivation()
{
  if (m_UltrasoundDevice.IsNull())
  {
    MITK_ERROR("USCombinedModality")("USDevice") << "UltrasoundDevice must not be null.";
    mitkThrow() << "UltrasoundDevice must not be null.";
  }

  m_UltrasoundDevice->Deactivate();

  return m_UltrasoundDevice->GetIsInitialized();
}

mitk::NavigationDataSource::Pointer mitk::USCombinedModality::GetNavigationDataSource()
{
  return m_DelayFilter.GetPointer();
}

void mitk::USCombinedModality::GenerateData()
{
  m_UltrasoundDevice->Update();
  mitk::Image::Pointer image = m_UltrasoundDevice->GetOutput();

  std::string calibrationKey = this->GetIdentifierForCurrentCalibration();
  if ( ! calibrationKey.empty() )
  {
    std::map<std::string, mitk::AffineTransform3D::Pointer>::iterator calibrationIterator
      = m_Calibrations.find(calibrationKey);
    if ( calibrationIterator != m_Calibrations.end())
    {
      // transform image according to callibration if one is set
      // for current configuration of probe and depth
      image->GetGeometry()->SetIndexToWorldTransform(calibrationIterator->second);
    }
  }

  // TODO: do processing here

  this->SetNthOutput(0, image);
}

std::string mitk::USCombinedModality::SerializeCalibration()
{
  std::stringstream result;
  result << "<calibrations>" << std::endl;
  // For each calibration in the set
  for (std::map<std::string, mitk::AffineTransform3D::Pointer>::iterator it = m_Calibrations.begin(); it != m_Calibrations.end(); it++)
  {
    mitk::AffineTransform3D::MatrixType matrix = it->second->GetMatrix();
    mitk::AffineTransform3D::OffsetType offset = it->second->GetOffset();
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
    elem.SetDoubleAttribute("O0", offset[0]);
    elem.SetDoubleAttribute("O1", offset[1]);
    elem.SetDoubleAttribute("O2", offset[2]);

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
    MITK_WARN << "Empty string passed to Deserialize() method of CombinedModality. Aborting...";
    return;
  }
  // Clear previous calibrations if necessary
  if (clearPreviousCalibrations) m_Calibrations.clear();

  // Parse Input
  TiXmlDocument doc;
  if(!doc.Parse(xmlString.c_str()))
  {
    MITK_WARN << "Unable to deserialize calibrations in CombinedModality. Error was: " << doc.ErrorDesc();
    return;
  }
  TiXmlElement* root = doc.FirstChildElement();
  if(root == NULL)
  {
    MITK_WARN << "Unable to deserialize calibrations in CombinedModality. String contained no root element.";
    return;
  }
  // Read Calibrations
  for(TiXmlElement* elem = root->FirstChildElement(); elem != NULL; elem = elem->NextSiblingElement())
  {
    mitk::AffineTransform3D::MatrixType matrix;
    mitk::AffineTransform3D::OffsetType offset;

    std::string calibName = elem->Value();

    // Deserialize Matrix
    elem->QueryFloatAttribute("M00", &matrix[0][0]);
    elem->QueryFloatAttribute("M01", &matrix[0][1]);
    elem->QueryFloatAttribute("M02", &matrix[0][2]);
    elem->QueryFloatAttribute("M10", &matrix[1][0]);
    elem->QueryFloatAttribute("M11", &matrix[1][1]);
    elem->QueryFloatAttribute("M12", &matrix[1][2]);
    elem->QueryFloatAttribute("M20", &matrix[2][0]);
    elem->QueryFloatAttribute("M21", &matrix[2][1]);
    elem->QueryFloatAttribute("M22", &matrix[2][2]);

    // Deserialize Offset
    elem->QueryFloatAttribute("O0", &offset[0]);
    elem->QueryFloatAttribute("O1", &offset[1]);
    elem->QueryFloatAttribute("O2", &offset[2]);

    mitk::AffineTransform3D::Pointer calibration = mitk::AffineTransform3D::New();
    calibration->SetMatrix(matrix);
    calibration->SetOffset(offset);
    m_Calibrations[calibName] = calibration;
  }
}

std::string mitk::USCombinedModality::GetIdentifierForCurrentCalibration()
{
  mitk::USControlInterfaceProbes::Pointer probesInterface = this->GetControlInterfaceProbes();
  if ( probesInterface && ! probesInterface->GetIsActive())
  {
    MITK_WARN << "Cannot get calibration as probes interface is not active.";
    return std::string();
  }

  // get probe identifier from control interface for probes
  std::string probeName;
  if ( ! probesInterface )
  {
    probeName = "default";
  }
  else
  {
    mitk::USProbe::Pointer curProbe = this->GetControlInterfaceProbes()->GetSelectedProbe();
    if (curProbe.IsNull())
    {
      MITK_WARN << "Cannot get calibration as current probe is null.";
      return std::string();
    }

    probeName = curProbe->GetName();
  }

  // get string for depth value from the micro service properties
  std::string depth;
  us::ServiceProperties::iterator depthIterator = m_ServiceProperties.find(US_PROPKEY_BMODE_DEPTH);
  if (depthIterator != m_ServiceProperties.end())
  {
    depth = depthIterator->second.ToString();
  }
  else
  {
    depth = "0";
  }

  return probeName + depth;
}