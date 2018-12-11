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
#include "mitkUSDeviceWriterXML.h"
#include <mitkIGTMimeTypes.h>
#include <mitkLocaleSwitch.h>
#include <mitkUSDevice.h>

// Third Party
#include <tinyxml.h>
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
  TiXmlDocument document;
  TiXmlDeclaration* xmlDeclaration = new TiXmlDeclaration("1.0", "", "");
  document.LinkEndChild(xmlDeclaration);


  //Create the xml information of the ULTRASOUNDDEVICE-Tag:
  TiXmlElement *ultrasoundDeviceTag = new TiXmlElement(TAG_ULTRASOUNDDEVICE);
  this->CreateXmlInformationOfUltrasoundDeviceTag(document, ultrasoundDeviceTag, config);


  //Create the xml information of the GENERALSETTINGS-Tag:
  TiXmlElement *generalSettingsTag = new TiXmlElement(TAG_GENERALSETTINGS);
  this->CreateXmlInformationOfGeneralSettingsTag(ultrasoundDeviceTag, generalSettingsTag, config);

  //Create the xml information of the PROBES-Tag:
  this->CreateXmlInformationOfProbesTag(ultrasoundDeviceTag, config);

  return document.SaveFile(m_Filename);
}

void mitk::USDeviceWriterXML::CreateXmlInformationOfUltrasoundDeviceTag(
  TiXmlDocument &document, TiXmlElement * ultrasoundDeviceTag,
  mitk::USDeviceReaderXML::USDeviceConfigData &config)
{
  ultrasoundDeviceTag->SetAttribute(ATTR_FILEVERS, config.fileversion);
  ultrasoundDeviceTag->SetAttribute(ATTR_TYPE, config.deviceType);
  ultrasoundDeviceTag->SetAttribute(ATTR_NAME, config.deviceName);
  ultrasoundDeviceTag->SetAttribute(ATTR_MANUFACTURER, config.manufacturer);
  ultrasoundDeviceTag->SetAttribute(ATTR_MODEL, config.model);
  ultrasoundDeviceTag->SetAttribute(ATTR_COMMENT, config.comment);
  ultrasoundDeviceTag->SetAttribute(ATTR_IMAGESTREAMS, config.numberOfImageStreams);

  if (config.deviceType.compare("oigtl") == 0)
  {
    ultrasoundDeviceTag->SetAttribute(ATTR_HOST, config.host);
    ultrasoundDeviceTag->SetAttribute(ATTR_PORT, config.port);
    std::string value = config.server ? "true" : "false";
    ultrasoundDeviceTag->SetAttribute(ATTR_SERVER, value);
  }

  document.LinkEndChild(ultrasoundDeviceTag);
}

void mitk::USDeviceWriterXML::CreateXmlInformationOfGeneralSettingsTag(TiXmlElement *parentTag, TiXmlElement *generalSettingsTag, mitk::USDeviceReaderXML::USDeviceConfigData & config)
{
  std::string value = config.useGreyscale ? "true" : "false";
  generalSettingsTag->SetAttribute(ATTR_GREYSCALE, value);
  value = config.useResolutionOverride ? "true" : "false";
  generalSettingsTag->SetAttribute(ATTR_RESOLUTIONOVERRIDE, value);
  generalSettingsTag->SetAttribute(ATTR_RESOLUTIONWIDTH, config.resolutionWidth);
  generalSettingsTag->SetAttribute(ATTR_RESOLUTIONHEIGHT, config.resolutionHeight);

  generalSettingsTag->SetAttribute(ATTR_SOURCEID, config.sourceID);
  generalSettingsTag->SetAttribute(ATTR_FILEPATH, config.filepathVideoSource);
  generalSettingsTag->SetAttribute(ATTR_OPENCVPORT, config.opencvPort);

  parentTag->LinkEndChild(generalSettingsTag);
}

void mitk::USDeviceWriterXML::CreateXmlInformationOfProbesTag(TiXmlElement * parentTag, mitk::USDeviceReaderXML::USDeviceConfigData & config)
{
  if (config.probes.size() != 0)
  {
    TiXmlElement *probesTag = new TiXmlElement(TAG_PROBES);
    parentTag->LinkEndChild(probesTag);

    for (size_t index = 0; index < config.probes.size(); ++index)
    {
      TiXmlElement *probeTag = new TiXmlElement(TAG_PROBE);
      probesTag->LinkEndChild(probeTag);

      mitk::USProbe::Pointer probe = config.probes.at(index);
      probeTag->SetAttribute(ATTR_NAME, probe->GetName());
      std::map<int, mitk::Vector3D> depthsAndSpacing = probe->GetDepthsAndSpacing();
      if (depthsAndSpacing.size() != 0)
      {
        TiXmlElement *depthsTag = new TiXmlElement(TAG_DEPTHS);
        probeTag->LinkEndChild(depthsTag);
        for (std::map<int, mitk::Vector3D>::iterator it = depthsAndSpacing.begin(); it != depthsAndSpacing.end(); it++)
        {
          TiXmlElement *depthTag = new TiXmlElement(TAG_DEPTH);
          depthTag->SetAttribute(ATTR_DEPTH, it->first);
          depthsTag->LinkEndChild(depthTag);

          TiXmlElement *spacingTag = new TiXmlElement(TAG_SPACING);
          spacingTag->SetDoubleAttribute(ATTR_X, it->second[0], 6);
          spacingTag->SetDoubleAttribute(ATTR_Y, it->second[1], 6);
          depthTag->LinkEndChild(spacingTag);
        }

        TiXmlElement *croppingTag = new TiXmlElement(TAG_CROPPING);
        probeTag->LinkEndChild(croppingTag);
        croppingTag->SetAttribute(ATTR_TOP, probe->GetProbeCropping().top);
        croppingTag->SetAttribute(ATTR_BOTTOM, probe->GetProbeCropping().bottom);
        croppingTag->SetAttribute(ATTR_LEFT, probe->GetProbeCropping().left);
        croppingTag->SetAttribute(ATTR_RIGHT, probe->GetProbeCropping().right);
      }
    }
  }
}
