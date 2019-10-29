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

// MITK
#include "mitkUSDeviceReaderWriterConstants.h"
#include "mitkUSDeviceReaderXML.h"
#include <mitkIGTMimeTypes.h>
#include <mitkLocaleSwitch.h>

#include <mitkUSVideoDevice.h>

// Third Party
#include <itksys/SystemTools.hxx>
#include <fstream>
#include <tinyxml.h>

mitk::USDeviceReaderXML::USDeviceReaderXML() : AbstractFileReader(
  mitk::IGTMimeTypes::USDEVICEINFORMATIONXML_MIMETYPE(),
  "MITK USDevice Reader (XML)"), m_Filename("")
{
  RegisterService();
}

mitk::USDeviceReaderXML::~USDeviceReaderXML()
{
}

mitk::USDeviceReaderXML::USDeviceConfigData &mitk::USDeviceReaderXML::GetUSDeviceConfigData()
{
  return m_DeviceConfig;
}

mitk::USDeviceReaderXML::USDeviceReaderXML(const mitk::USDeviceReaderXML& other) : AbstractFileReader(other)
{
}

mitk::USDeviceReaderXML* mitk::USDeviceReaderXML::Clone() const
{
  return new USDeviceReaderXML(*this);
}




std::vector<itk::SmartPointer<mitk::BaseData>> mitk::USDeviceReaderXML::Read()
{
  MITK_WARN << "This method is not implemented. \
  Please use the method ReadUltrasoundDeviceConfiguration() instead.";
  std::vector<mitk::BaseData::Pointer> result;
  return result;
}

bool mitk::USDeviceReaderXML::ReadUltrasoundDeviceConfiguration()
{
  MITK_INFO << "Try to start reading xml device configuration...";
  if (m_Filename == "")
  {
    MITK_WARN << "Cannot read file - empty filename!";
    return false;
  }

  TiXmlDocument document(m_Filename);
  if (!document.LoadFile())
  {
    MITK_ERROR << "Error when opening and reading file :" << m_Filename;
    return false;
  }

  TiXmlHandle documentHandle(&document);
  TiXmlElement* ultrasoundDeviceTag = documentHandle.FirstChildElement(TAG_ULTRASOUNDDEVICE).ToElement();
  if (ultrasoundDeviceTag == nullptr)
  {
    MITK_ERROR << "Error parsing the file :" << m_Filename << std::endl << "Wrong xml format structure.";
    return false;
  }

  //Extract attribute information of the ULTRASOUNDDEVICE-Tag:
  this->ExtractAttributeInformationOfUltrasoundDeviceTag(ultrasoundDeviceTag);

  TiXmlElement* generalSettingsTag = documentHandle.FirstChildElement(TAG_ULTRASOUNDDEVICE).FirstChildElement(TAG_GENERALSETTINGS).ToElement();
  if (generalSettingsTag == nullptr)
  {
    MITK_ERROR << "Error parsing the GENERALSETTINGS-Tag in the file :" << m_Filename;
    return false;
  }

  //Extract attribute information of the GENERALSETTINGS-Tag:
  this->ExtractAttributeInformationOfGeneralSettingsTag(generalSettingsTag);

  TiXmlElement* probesTag = documentHandle.FirstChildElement(TAG_ULTRASOUNDDEVICE).FirstChildElement(TAG_PROBES).ToElement();
  if (probesTag == nullptr)
  {
    MITK_ERROR << "Error: PROBES-Tag was not found in the file :" << m_Filename << "Therefore, creating default probe.";
    //Create default ultrasound probe:
    mitk::USProbe::Pointer ultrasoundProbeDefault = mitk::USProbe::New();
    ultrasoundProbeDefault->SetName("default");
    ultrasoundProbeDefault->SetDepth(0);
    m_DeviceConfig.probes.push_back(ultrasoundProbeDefault);
    return true;
  }

  //Extract all saved and configured probes of the USDevice:
  for (TiXmlElement* probeTag = probesTag->FirstChildElement(TAG_PROBE);
    probeTag != nullptr; probeTag = probeTag->NextSiblingElement())
  {
    this->ExtractProbe(probeTag);
  }
  return true;
}

void mitk::USDeviceReaderXML::SetFilename(std::string filename)
{
  m_Filename = filename;
}

void mitk::USDeviceReaderXML::ExtractAttributeInformationOfUltrasoundDeviceTag(TiXmlElement *ultrasoundTag)
{
  ultrasoundTag->QueryDoubleAttribute(ATTR_FILEVERS, &m_DeviceConfig.fileversion);
  ultrasoundTag->QueryStringAttribute(ATTR_TYPE, &m_DeviceConfig.deviceType);
  ultrasoundTag->QueryStringAttribute(ATTR_NAME, &m_DeviceConfig.deviceName);
  ultrasoundTag->QueryStringAttribute(ATTR_MANUFACTURER, &m_DeviceConfig.manufacturer);
  ultrasoundTag->QueryStringAttribute(ATTR_MODEL, &m_DeviceConfig.model);
  ultrasoundTag->QueryStringAttribute(ATTR_COMMENT, &m_DeviceConfig.comment);
  ultrasoundTag->QueryIntAttribute(ATTR_IMAGESTREAMS, &m_DeviceConfig.numberOfImageStreams);
  ultrasoundTag->QueryStringAttribute(ATTR_HOST, &m_DeviceConfig.host);
  ultrasoundTag->QueryIntAttribute(ATTR_PORT, &m_DeviceConfig.port);
  ultrasoundTag->QueryBoolAttribute(ATTR_SERVER, &m_DeviceConfig.server);
}

void mitk::USDeviceReaderXML::ExtractAttributeInformationOfGeneralSettingsTag(TiXmlElement *generalSettingsTag)
{
  generalSettingsTag->QueryBoolAttribute(ATTR_GREYSCALE, &m_DeviceConfig.useGreyscale);
  generalSettingsTag->QueryBoolAttribute(ATTR_RESOLUTIONOVERRIDE, &m_DeviceConfig.useResolutionOverride);
  generalSettingsTag->QueryIntAttribute(ATTR_RESOLUTIONHEIGHT, &m_DeviceConfig.resolutionHeight);
  generalSettingsTag->QueryIntAttribute(ATTR_RESOLUTIONWIDTH, &m_DeviceConfig.resolutionWidth);
  generalSettingsTag->QueryIntAttribute(ATTR_SOURCEID, &m_DeviceConfig.sourceID);
  generalSettingsTag->QueryStringAttribute(ATTR_FILEPATH, &m_DeviceConfig.filepathVideoSource);
  generalSettingsTag->QueryIntAttribute(ATTR_OPENCVPORT, &m_DeviceConfig.opencvPort);
}

void mitk::USDeviceReaderXML::ExtractProbe(TiXmlElement *probeTag)
{
  mitk::USProbe::Pointer ultrasoundProbe = mitk::USProbe::New();
  std::string probeName;
  probeTag->QueryStringAttribute(ATTR_NAME, &probeName);
  ultrasoundProbe->SetName(probeName);

  TiXmlElement* depthsTag = probeTag->FirstChildElement(TAG_DEPTHS);
  if (depthsTag != nullptr)
  {
    for (TiXmlElement* depthTag = depthsTag->FirstChildElement(TAG_DEPTH);
      depthTag != nullptr; depthTag = depthTag->NextSiblingElement())
    {
      int depth = 0;
      mitk::Vector3D spacing;
      spacing[0] = 1;
      spacing[1] = 1;
      spacing[2] = 1;

      depthTag->QueryIntAttribute(ATTR_DEPTH, &depth);

      TiXmlElement* spacingTag = depthTag->FirstChildElement(TAG_SPACING);
      if (spacingTag != nullptr)
      {
        spacingTag->QueryDoubleAttribute(ATTR_X, &spacing[0]);
        spacingTag->QueryDoubleAttribute(ATTR_Y, &spacing[1]);
      }

      ultrasoundProbe->SetDepthAndSpacing(depth, spacing);
    }
  }
  else
  {
    MITK_ERROR << "Error: DEPTHS-Tag was not found in the file :" << m_Filename
      << "Therefore, creating default depth [0] and spacing [1,1,1] for the probe.";
    ultrasoundProbe->SetDepth(0);
  }

  unsigned int croppingTop = 0;
  unsigned int croppingBottom = 0;
  unsigned int croppingLeft = 0;
  unsigned int croppingRight = 0;

  TiXmlElement* croppingTag = probeTag->FirstChildElement(TAG_CROPPING);
  if (croppingTag != nullptr)
  {
    croppingTag->QueryUnsignedAttribute(ATTR_TOP, &croppingTop);
    croppingTag->QueryUnsignedAttribute(ATTR_BOTTOM, &croppingBottom);
    croppingTag->QueryUnsignedAttribute(ATTR_LEFT, &croppingLeft);
    croppingTag->QueryUnsignedAttribute(ATTR_RIGHT, &croppingRight);
  }

  ultrasoundProbe->SetProbeCropping(croppingTop, croppingBottom, croppingLeft, croppingRight);
  m_DeviceConfig.probes.push_back(ultrasoundProbe);
}
