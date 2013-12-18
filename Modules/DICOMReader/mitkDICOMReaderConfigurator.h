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

#ifndef mitkDICOMReaderConfigurator_h
#define mitkDICOMReaderConfigurator_h

#include "mitkThreeDnTDICOMSeriesReader.h"

// to put into private implementation
#include "tinyxml.h"

namespace mitk
{

class DICOMReader_EXPORT DICOMReaderConfigurator : public itk::LightObject
{
  public:

    mitkClassMacro( DICOMReaderConfigurator, itk::LightObject )
    itkNewMacro( DICOMReaderConfigurator )

    DICOMFileReader::Pointer CreateFromConfigFile(const std::string& filename);
    DICOMFileReader::Pointer CreateFromUTF8ConfigString(const std::string& xmlContents);

  protected:

    DICOMReaderConfigurator();
    virtual ~DICOMReaderConfigurator();

  private:

    DICOMFileReader::Pointer CreateFromTiXmlDocument(TiXmlDocument& doc);
    DICOMTag tagFromXMLElement(TiXmlElement*);
    std::string requiredStringAttribute(TiXmlElement* xmlElement, const std::string& key);

    ThreeDnTDICOMSeriesReader::Pointer ConfigureThreeDnTDICOMSeriesReader(ThreeDnTDICOMSeriesReader::Pointer reader, TiXmlElement*);
    DICOMITKSeriesGDCMReader::Pointer ConfigureDICOMITKSeriesGDCMReader(DICOMITKSeriesGDCMReader::Pointer reader, TiXmlElement*);
 };

} // namespace

#endif // mitkDICOMReaderConfigurator_h
