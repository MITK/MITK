/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkUSDeviceWriterXML_h
#define mitkUSDeviceWriterXML_h

#include <MitkUSExports.h>

#include <mitkAbstractFileWriter.h>
#include <mitkUSDeviceReaderXML.h>

namespace tinyxml2
{
  class XMLDocument;
  class XMLElement;
}

namespace mitk {
  class MITKUS_EXPORT USDeviceWriterXML : public AbstractFileWriter
  {
  public:

    USDeviceWriterXML();
    ~USDeviceWriterXML() override;

    using AbstractFileWriter::Write;
    void Write() override;

    /**
    * \brief Sets the filename of the ultrasound device configuration file which should be created.
    */
    void SetFilename(std::string filename);

    /**
    * \brief Writes the configuration settings of an ultrasound device to a xml-file.
    * \param config The struct containing all information of the ultrasound device.
    */
    bool WriteUltrasoundDeviceConfiguration(mitk::USDeviceReaderXML::USDeviceConfigData &config);

  protected:
    USDeviceWriterXML(const USDeviceWriterXML& other);
    mitk::USDeviceWriterXML* Clone() const override;

    /**
    * \brief Creates the xml ULTRASOUNDDEVICE-Tag entry of the ultrasound video device configuration file.
    * \param document A reference to the xml document.
    * \param ultrasoundDeviceTag The ULTRASOUNDDEVICETAG which should be created.
    * \param config The struct containing all information of the ultrasound device.
    */
    void CreateXmlInformationOfUltrasoundDeviceTag( tinyxml2::XMLDocument &document,
                                                    tinyxml2::XMLElement *ultrasoundDeviceTag,
                                                    mitk::USDeviceReaderXML::USDeviceConfigData &config);

    /**
    * \brief Creates the xml GENERALSETTINGS-Tag entry of the ultrasound video device configuration file.
    * \param parentTag The xml parent tag of the GENERALSETTINGS-Tag. This is the ULTRASOUNDDEVICE-Tag.
    * \param generalSettingsTag The GENERALSETTINGS-Tag which should be created.
    * \param config The struct containing all information of the ultrasound device.
    */
    void CreateXmlInformationOfGeneralSettingsTag( tinyxml2::XMLElement *parentTag,
                                                   tinyxml2::XMLElement *generalSettingsTag,
                                                   mitk::USDeviceReaderXML::USDeviceConfigData &config);

    /**
    * \brief Creates the xml PROBES-Tag entry of the ultrasound video device configuration file. All information
    * of all configured probes is extracted and then stored in the xml file.
    * \param parentTag The xml parent tag of the GENERALSETTINGS-Tag. This is the ULTRASOUNDDEVICE-Tag.
    * \param config The struct containing all information of the ultrasound device.
    */
    void CreateXmlInformationOfProbesTag( tinyxml2::XMLElement*parentTag,
                                          mitk::USDeviceReaderXML::USDeviceConfigData &config);
  private:
    /**
    * \brief The filename of the ultrasound device configuration file which should be created.
    */
    std::string m_Filename;


  };
}

#endif
