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

#ifndef mitkUSDeviceWriterXML_H_Header_INCLUDED_
#define mitkUSDeviceWriterXML_H_Header_INCLUDED_

#include <MitkUSExports.h>

#include <mitkAbstractFileWriter.h>
#include <mitkUSDeviceReaderXML.h>

class TiXmlDocument;
class TiXmlElement;

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
    void CreateXmlInformationOfUltrasoundDeviceTag( TiXmlDocument &document,
                                                    TiXmlElement *ultrasoundDeviceTag,
                                                    mitk::USDeviceReaderXML::USDeviceConfigData &config);

    /**
    * \brief Creates the xml GENERALSETTINGS-Tag entry of the ultrasound video device configuration file.
    * \param parentTag The xml parent tag of the GENERALSETTINGS-Tag. This is the ULTRASOUNDDEVICE-Tag.
    * \param generalSettingsTag The GENERALSETTINGS-Tag which should be created.
    * \param config The struct containing all information of the ultrasound device.
    */
    void CreateXmlInformationOfGeneralSettingsTag( TiXmlElement *parentTag,
                                                   TiXmlElement *generalSettingsTag,
                                                   mitk::USDeviceReaderXML::USDeviceConfigData &config);

    /**
    * \brief Creates the xml PROBES-Tag entry of the ultrasound video device configuration file. All information
    * of all configured probes is extracted and then stored in the xml file.
    * \param parentTag The xml parent tag of the GENERALSETTINGS-Tag. This is the ULTRASOUNDDEVICE-Tag.
    * \param config The struct containing all information of the ultrasound device.
    */
    void CreateXmlInformationOfProbesTag( TiXmlElement *parentTag,
                                          mitk::USDeviceReaderXML::USDeviceConfigData &config);
  private:
    /**
    * \brief The filename of the ultrasound device configuration file which should be created.
    */
    std::string m_Filename;


  };
}

#endif // mitkUSDeviceWriterXML_H_Header_INCLUDED_
