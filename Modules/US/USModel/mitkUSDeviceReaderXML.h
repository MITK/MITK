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


#ifndef mitkUSDeviceReaderXML_H_HEADER_INCLUDED_
#define mitkUSDeviceReaderXML_H_HEADER_INCLUDED_

#include <MitkUSExports.h>

#include <mitkAbstractFileReader.h>
#include <mitkUSProbe.h>

class TiXmlElement;
class TiXmlNode;

namespace mitk {

  class MITKUS_EXPORT USDeviceReaderXML : public AbstractFileReader
  {
  public:
    USDeviceReaderXML();
    ~USDeviceReaderXML() override;

    using AbstractFileReader::Read;
    std::vector<itk::SmartPointer<BaseData>> Read() override;
    bool ReadUltrasoundDeviceConfiguration();

    void SetFilename(std::string filename);

    typedef struct USDeviceConfigData_
    {
      double fileversion;
      std::string deviceType;
      std::string deviceName;
      std::string manufacturer;
      std::string model;
      std::string comment;
      std::string host;
      int port;
      bool server;
      int numberOfImageStreams;

      bool useGreyscale;
      bool useResolutionOverride;
      int resolutionWidth;
      int resolutionHeight;
      int sourceID;
      std::string filepathVideoSource;
      int opencvPort;

      std::vector <mitk::USProbe::Pointer> probes;

      USDeviceConfigData_()
        : fileversion(0), deviceType("Unknown"), deviceName("Unknown"),
          manufacturer("Unknown"), comment(""), host("localhost"),
          port(18944), server(false), numberOfImageStreams(1),
          useGreyscale(true), useResolutionOverride(true),
          resolutionWidth(640), resolutionHeight(480), sourceID(0),
          filepathVideoSource(""), opencvPort(0)
        { };

    }USDeviceConfigData;

    USDeviceConfigData &GetUSDeviceConfigData();

  protected:
    USDeviceReaderXML(const USDeviceReaderXML& other);
    mitk::USDeviceReaderXML* Clone() const override;

    /**
    * \brief Extracts all stored attribute information of the ULTRASOUNDDEVICE-Tag.
    */
    void ExtractAttributeInformationOfUltrasoundDeviceTag(TiXmlElement *element);

    /**
    * \brief Extracts all stored attribute information of the GENERALSETTINGS-Tag.
    */
    void ExtractAttributeInformationOfGeneralSettingsTag(TiXmlElement *element);

    /**
    * \brief Extracts all stored information of a single ultrasound probe.
    */
    void ExtractProbe(TiXmlElement *element);

  private:
    std::string m_Filename;
    USDeviceConfigData m_DeviceConfig;
  };

} // namespace mitk

#endif // mitkUSDeviceReaderXML_H_HEADER_INCLUDED_
