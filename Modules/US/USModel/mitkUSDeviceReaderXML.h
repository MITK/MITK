/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef mitkUSDeviceReaderXML_H_HEADER_INCLUDED_
#define mitkUSDeviceReaderXML_H_HEADER_INCLUDED_

#include <MitkUSExports.h>

#include <mitkAbstractFileReader.h>
#include <mitkUSProbe.h>

namespace tinyxml2 {
  class XMLElement;
}

namespace mitk {

  class MITKUS_EXPORT USDeviceReaderXML : public AbstractFileReader
  {
  public:
    USDeviceReaderXML();
    ~USDeviceReaderXML() override;

    using AbstractFileReader::Read;

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
    std::vector<itk::SmartPointer<BaseData>> DoRead() override;

    USDeviceReaderXML(const USDeviceReaderXML& other);
    mitk::USDeviceReaderXML* Clone() const override;

    /**
    * \brief Extracts all stored attribute information of the ULTRASOUNDDEVICE-Tag.
    */
    void ExtractAttributeInformationOfUltrasoundDeviceTag(const tinyxml2::XMLElement *element);

    /**
    * \brief Extracts all stored attribute information of the GENERALSETTINGS-Tag.
    */
    void ExtractAttributeInformationOfGeneralSettingsTag(const tinyxml2::XMLElement *element);

    /**
    * \brief Extracts all stored information of a single ultrasound probe.
    */
    void ExtractProbe(const tinyxml2::XMLElement *element);

  private:
    std::string m_Filename;
    USDeviceConfigData m_DeviceConfig;
  };

} // namespace mitk

#endif // mitkUSDeviceReaderXML_H_HEADER_INCLUDED_
