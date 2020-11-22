/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// MITK
#include "mitkUSDeviceReaderWriterConstants.h"
#include "mitkUSDeviceReaderXML.h"
#include <mitkIGTMimeTypes.h>
#include <mitkLocaleSwitch.h>

#include <mitkUSVideoDevice.h>

// Third Party
#include <itksys/SystemTools.hxx>
#include <fstream>
#include <tinyxml2.h>

namespace
{
  std::string ReadStringAttribute(const tinyxml2::XMLElement* elem, const char* name)
  {
    const char* attrib = elem->Attribute(name);

    return attrib != nullptr
      ? attrib
      : "";
  }
}

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




std::vector<itk::SmartPointer<mitk::BaseData>> mitk::USDeviceReaderXML::DoRead()
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

  tinyxml2::XMLDocument document;
  if (tinyxml2::XML_SUCCESS != document.LoadFile(m_Filename.c_str()))
  {
    MITK_ERROR << "Error when opening and reading file :" << m_Filename;
    return false;
  }

  tinyxml2::XMLHandle documentHandle(&document);
  auto* ultrasoundDeviceTag = documentHandle.FirstChildElement(USDeviceReaderWriterConstants::TAG_ULTRASOUNDDEVICE).ToElement();
  if (ultrasoundDeviceTag == nullptr)
  {
    MITK_ERROR << "Error parsing the file :" << m_Filename << std::endl << "Wrong xml format structure.";
    return false;
  }

  //Extract attribute information of the ULTRASOUNDDEVICE-Tag:
  this->ExtractAttributeInformationOfUltrasoundDeviceTag(ultrasoundDeviceTag);

  auto* generalSettingsTag = documentHandle.FirstChildElement(USDeviceReaderWriterConstants::TAG_ULTRASOUNDDEVICE).FirstChildElement(USDeviceReaderWriterConstants::TAG_GENERALSETTINGS).ToElement();
  if (generalSettingsTag == nullptr)
  {
    MITK_ERROR << "Error parsing the GENERALSETTINGS-Tag in the file :" << m_Filename;
    return false;
  }

  //Extract attribute information of the GENERALSETTINGS-Tag:
  this->ExtractAttributeInformationOfGeneralSettingsTag(generalSettingsTag);

  auto* probesTag = documentHandle.FirstChildElement(USDeviceReaderWriterConstants::TAG_ULTRASOUNDDEVICE).FirstChildElement(USDeviceReaderWriterConstants::TAG_PROBES).ToElement();
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
  for (auto* probeTag = probesTag->FirstChildElement(USDeviceReaderWriterConstants::TAG_PROBE);
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

void mitk::USDeviceReaderXML::ExtractAttributeInformationOfUltrasoundDeviceTag(const tinyxml2::XMLElement *ultrasoundTag)
{
  ultrasoundTag->QueryDoubleAttribute(USDeviceReaderWriterConstants::ATTR_FILEVERS, &m_DeviceConfig.fileversion);
  ultrasoundTag->QueryIntAttribute(USDeviceReaderWriterConstants::ATTR_IMAGESTREAMS, &m_DeviceConfig.numberOfImageStreams);
  ultrasoundTag->QueryIntAttribute(USDeviceReaderWriterConstants::ATTR_PORT, &m_DeviceConfig.port);
  ultrasoundTag->QueryBoolAttribute(USDeviceReaderWriterConstants::ATTR_SERVER, &m_DeviceConfig.server);

  m_DeviceConfig.deviceType = ReadStringAttribute(ultrasoundTag, USDeviceReaderWriterConstants::ATTR_TYPE);
  m_DeviceConfig.deviceName = ReadStringAttribute(ultrasoundTag, USDeviceReaderWriterConstants::ATTR_TYPE);
  m_DeviceConfig.manufacturer = ReadStringAttribute(ultrasoundTag, USDeviceReaderWriterConstants::ATTR_TYPE);
  m_DeviceConfig.model = ReadStringAttribute(ultrasoundTag, USDeviceReaderWriterConstants::ATTR_TYPE);
  m_DeviceConfig.comment = ReadStringAttribute(ultrasoundTag, USDeviceReaderWriterConstants::ATTR_TYPE);
  m_DeviceConfig.host = ReadStringAttribute(ultrasoundTag, USDeviceReaderWriterConstants::ATTR_HOST);
}

void mitk::USDeviceReaderXML::ExtractAttributeInformationOfGeneralSettingsTag(const tinyxml2::XMLElement *generalSettingsTag)
{
  generalSettingsTag->QueryBoolAttribute(USDeviceReaderWriterConstants::ATTR_GREYSCALE, &m_DeviceConfig.useGreyscale);
  generalSettingsTag->QueryBoolAttribute(USDeviceReaderWriterConstants::ATTR_RESOLUTIONOVERRIDE, &m_DeviceConfig.useResolutionOverride);
  generalSettingsTag->QueryIntAttribute(USDeviceReaderWriterConstants::ATTR_RESOLUTIONHEIGHT, &m_DeviceConfig.resolutionHeight);
  generalSettingsTag->QueryIntAttribute(USDeviceReaderWriterConstants::ATTR_RESOLUTIONWIDTH, &m_DeviceConfig.resolutionWidth);
  generalSettingsTag->QueryIntAttribute(USDeviceReaderWriterConstants::ATTR_SOURCEID, &m_DeviceConfig.sourceID);
  generalSettingsTag->QueryIntAttribute(USDeviceReaderWriterConstants::ATTR_OPENCVPORT, &m_DeviceConfig.opencvPort);

  m_DeviceConfig.filepathVideoSource = ReadStringAttribute(generalSettingsTag, USDeviceReaderWriterConstants::ATTR_FILEPATH);
}

void mitk::USDeviceReaderXML::ExtractProbe(const tinyxml2::XMLElement *probeTag)
{
  mitk::USProbe::Pointer ultrasoundProbe = mitk::USProbe::New();
  auto probeName = ReadStringAttribute(probeTag, USDeviceReaderWriterConstants::ATTR_NAME);
  ultrasoundProbe->SetName(probeName);

  auto* depthsTag = probeTag->FirstChildElement(USDeviceReaderWriterConstants::TAG_DEPTHS);
  if (depthsTag != nullptr)
  {
    for (auto* depthTag = depthsTag->FirstChildElement(USDeviceReaderWriterConstants::TAG_DEPTH);
      depthTag != nullptr; depthTag = depthTag->NextSiblingElement())
    {
      int depth = 0;
      mitk::Vector3D spacing;
      spacing[0] = 1;
      spacing[1] = 1;
      spacing[2] = 1;

      depthTag->QueryIntAttribute(USDeviceReaderWriterConstants::ATTR_DEPTH, &depth);

      auto* spacingTag = depthTag->FirstChildElement(USDeviceReaderWriterConstants::TAG_SPACING);
      if (spacingTag != nullptr)
      {
        spacingTag->QueryDoubleAttribute(USDeviceReaderWriterConstants::ATTR_X, &spacing[0]);
        spacingTag->QueryDoubleAttribute(USDeviceReaderWriterConstants::ATTR_Y, &spacing[1]);
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

  auto* croppingTag = probeTag->FirstChildElement(USDeviceReaderWriterConstants::TAG_CROPPING);
  if (croppingTag != nullptr)
  {
    croppingTag->QueryUnsignedAttribute(USDeviceReaderWriterConstants::ATTR_TOP, &croppingTop);
    croppingTag->QueryUnsignedAttribute(USDeviceReaderWriterConstants::ATTR_BOTTOM, &croppingBottom);
    croppingTag->QueryUnsignedAttribute(USDeviceReaderWriterConstants::ATTR_LEFT, &croppingLeft);
    croppingTag->QueryUnsignedAttribute(USDeviceReaderWriterConstants::ATTR_RIGHT, &croppingRight);
  }

  ultrasoundProbe->SetProbeCropping(croppingTop, croppingBottom, croppingLeft, croppingRight);
  m_DeviceConfig.probes.push_back(ultrasoundProbe);
}
