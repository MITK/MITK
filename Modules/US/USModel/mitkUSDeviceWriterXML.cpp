/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// MITK
#include "mitkUSDeviceReaderWriterConstants.h"
#include "mitkUSDeviceWriterXML.h"
#include <mitkIGTMimeTypes.h>
#include <mitkLocaleSwitch.h>
#include <mitkUSDevice.h>

// Third Party
#include <tinyxml2.h>
#include <itksys/SystemTools.hxx>
#include <fstream>
#include <iostream>

mitk::USDeviceWriterXML::USDeviceWriterXML() : AbstractFileWriter(USDevice::GetStaticNameOfClass(),
  mitk::IGTMimeTypes::USDEVICEINFORMATIONXML_MIMETYPE(),
  "MITK USDevice Writer (XML)"), m_Filename("")
{
  RegisterService();
}

mitk::USDeviceWriterXML::USDeviceWriterXML(const mitk::USDeviceWriterXML& other) : AbstractFileWriter(other)
{
}

mitk::USDeviceWriterXML::~USDeviceWriterXML()
{
}

mitk::USDeviceWriterXML* mitk::USDeviceWriterXML::Clone() const
{
  return new USDeviceWriterXML(*this);
}

void mitk::USDeviceWriterXML::Write()
{
  if (m_Filename == "")
  {
    MITK_WARN << "Cannot write to file - empty filename!";
    return;
  }
}

void mitk::USDeviceWriterXML::SetFilename(std::string filename)
{
  m_Filename = filename;
}

bool mitk::USDeviceWriterXML::WriteUltrasoundDeviceConfiguration(mitk::USDeviceReaderXML::USDeviceConfigData & config)
{
  tinyxml2::XMLDocument document;
  document.InsertEndChild(document.NewDeclaration());

  //Create the xml information of the ULTRASOUNDDEVICE-Tag:
  auto *ultrasoundDeviceTag = document.NewElement(TAG_ULTRASOUNDDEVICE);
  this->CreateXmlInformationOfUltrasoundDeviceTag(document, ultrasoundDeviceTag, config);


  //Create the xml information of the GENERALSETTINGS-Tag:
  auto *generalSettingsTag = document.NewElement(TAG_GENERALSETTINGS);
  this->CreateXmlInformationOfGeneralSettingsTag(ultrasoundDeviceTag, generalSettingsTag, config);

  //Create the xml information of the PROBES-Tag:
  this->CreateXmlInformationOfProbesTag(ultrasoundDeviceTag, config);

  return document.SaveFile(m_Filename.c_str()) == tinyxml2::XML_SUCCESS;
}

void mitk::USDeviceWriterXML::CreateXmlInformationOfUltrasoundDeviceTag(
  tinyxml2::XMLDocument &document, tinyxml2::XMLElement* ultrasoundDeviceTag,
  mitk::USDeviceReaderXML::USDeviceConfigData &config)
{
  ultrasoundDeviceTag->SetAttribute(ATTR_FILEVERS, config.fileversion);
  ultrasoundDeviceTag->SetAttribute(ATTR_TYPE, config.deviceType.c_str());
  ultrasoundDeviceTag->SetAttribute(ATTR_NAME, config.deviceName.c_str());
  ultrasoundDeviceTag->SetAttribute(ATTR_MANUFACTURER, config.manufacturer.c_str());
  ultrasoundDeviceTag->SetAttribute(ATTR_MODEL, config.model.c_str());
  ultrasoundDeviceTag->SetAttribute(ATTR_COMMENT, config.comment.c_str());
  ultrasoundDeviceTag->SetAttribute(ATTR_IMAGESTREAMS, config.numberOfImageStreams);

  if (config.deviceType.compare("oigtl") == 0)
  {
    ultrasoundDeviceTag->SetAttribute(ATTR_HOST, config.host.c_str());
    ultrasoundDeviceTag->SetAttribute(ATTR_PORT, config.port);
    std::string value = config.server ? "true" : "false";
    ultrasoundDeviceTag->SetAttribute(ATTR_SERVER, value.c_str());
  }

  document.InsertEndChild(ultrasoundDeviceTag);
}

void mitk::USDeviceWriterXML::CreateXmlInformationOfGeneralSettingsTag(tinyxml2::XMLElement *parentTag, tinyxml2::XMLElement *generalSettingsTag, mitk::USDeviceReaderXML::USDeviceConfigData & config)
{
  std::string value = config.useGreyscale ? "true" : "false";
  generalSettingsTag->SetAttribute(ATTR_GREYSCALE, value.c_str());
  value = config.useResolutionOverride ? "true" : "false";
  generalSettingsTag->SetAttribute(ATTR_RESOLUTIONOVERRIDE, value.c_str());
  generalSettingsTag->SetAttribute(ATTR_RESOLUTIONWIDTH, config.resolutionWidth);
  generalSettingsTag->SetAttribute(ATTR_RESOLUTIONHEIGHT, config.resolutionHeight);

  generalSettingsTag->SetAttribute(ATTR_SOURCEID, config.sourceID);
  generalSettingsTag->SetAttribute(ATTR_FILEPATH, config.filepathVideoSource.c_str());
  generalSettingsTag->SetAttribute(ATTR_OPENCVPORT, config.opencvPort);

  parentTag->InsertEndChild(generalSettingsTag);
}

void mitk::USDeviceWriterXML::CreateXmlInformationOfProbesTag(tinyxml2::XMLElement *parentTag, mitk::USDeviceReaderXML::USDeviceConfigData & config)
{
  if (config.probes.size() != 0)
  {
    auto* doc = parentTag->GetDocument();
    auto *probesTag = doc->NewElement(TAG_PROBES);
    parentTag->InsertEndChild(probesTag);

    for (size_t index = 0; index < config.probes.size(); ++index)
    {
      auto *probeTag = doc->NewElement(TAG_PROBE);
      probesTag->InsertEndChild(probeTag);

      mitk::USProbe::Pointer probe = config.probes.at(index);
      probeTag->SetAttribute(ATTR_NAME, probe->GetName().c_str());
      std::map<int, mitk::Vector3D> depthsAndSpacing = probe->GetDepthsAndSpacing();
      if (depthsAndSpacing.size() != 0)
      {
        auto *depthsTag = doc->NewElement(TAG_DEPTHS);
        probeTag->InsertEndChild(depthsTag);
        for (std::map<int, mitk::Vector3D>::iterator it = depthsAndSpacing.begin(); it != depthsAndSpacing.end(); it++)
        {
          auto *depthTag = doc->NewElement(TAG_DEPTH);
          depthTag->SetAttribute(ATTR_DEPTH, it->first);
          depthsTag->InsertEndChild(depthTag);

          auto *spacingTag = doc->NewElement(TAG_SPACING);
          spacingTag->SetAttribute(ATTR_X, it->second[0]);
          spacingTag->SetAttribute(ATTR_Y, it->second[1]);
          depthTag->InsertEndChild(spacingTag);
        }

        auto *croppingTag = doc->NewElement(TAG_CROPPING);
        probeTag->InsertEndChild(croppingTag);
        croppingTag->SetAttribute(ATTR_TOP, probe->GetProbeCropping().top);
        croppingTag->SetAttribute(ATTR_BOTTOM, probe->GetProbeCropping().bottom);
        croppingTag->SetAttribute(ATTR_LEFT, probe->GetProbeCropping().left);
        croppingTag->SetAttribute(ATTR_RIGHT, probe->GetProbeCropping().right);
      }
    }
  }
}
