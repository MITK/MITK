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

/**
  \ingroup DICOMReaderModule
  \brief Too-simple factory to create DICOMFileReader%s.

  This class is able to instantiate and configure (where possible) DICOMFileReader%s from XML descriptions.
  \note This is a bad factory example, because the factory is not extensible and needs to know all the specific readers. A flexible implementation should be provided in a future version.

  In its current version, the XML input is meant to be structured like
\verbatim
<?xml version="1.0" standalone=no>
<DICOMFileReader
  label="Example Reader"
  description="Sort images by example tags"
  class="DICOMITKSeriesGDCMReader"
  fixTiltByShearing="true">
<Distinguishing>
 <Tag name="SeriesInstanceUID" group="0020" element="000e"/>
 <Tag name="SeriesNumber" group="0020" element="0011"/>
</Distinguishing>
<Sorting>
 <ImagePositionPatient/>
 <Tag name="SOPInstanceUID" group="0x0008" element="0x0018"/>
</Sorting>
</DICOMFileReader>
\endverbatim

  The root-tag \i <DICOMFileReader> names the class to be instantiated, currently this can be one of
   - DICOMITKSeriesGDCMReader
   - ThreeDnTDICOMSeriesReader

  Both classes bring simple configuration flags with them and a description of how images are sorted prior to loading.

  Flag for DICOMITKSeriesGDCMReader:
  <pre>fixTiltByShearing="true|false"</pre>
  Determines whether a potential gantry tilt should be "fixed" by shearing the output image.

  Flag for ThreeDnTDICOMSeriesReader:
  <pre>group3DnT="true|false"</pre>
  Determines whether images at the same spatial position should be interpreted as 3D+t images.

  The tags <Distinguishing> and <Sorting> describe the basic loading strategy of both
  reader mentioned above: first images are divided into incompatible groups (<Distinguishing>),
  and afterwards the images within each group are sorted by means of DICOMSortCriterion, which
  most commonly mentions a tag.

  Tag element and group are interpreted as the hexadecimal numbers
  found all around the DICOM standard. The numbers can be prepended by a "0x" if this is preferred
  by the programmer (but they are taken as hexadecimal in all cases).
*/
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
    unsigned int hexStringToUInt(const std::string& s);

    ThreeDnTDICOMSeriesReader::Pointer ConfigureThreeDnTDICOMSeriesReader(ThreeDnTDICOMSeriesReader::Pointer reader, TiXmlElement*);
    DICOMITKSeriesGDCMReader::Pointer ConfigureDICOMITKSeriesGDCMReader(DICOMITKSeriesGDCMReader::Pointer reader, TiXmlElement*);

    DICOMSortCriterion::Pointer CreateDICOMSortByTag(TiXmlElement* xmlElement, DICOMSortCriterion::Pointer secondaryCriterion);
    DICOMSortCriterion::Pointer CreateSortByImagePositionPatient(TiXmlElement* xmlElement, DICOMSortCriterion::Pointer secondaryCriterion);
 };

} // namespace

#endif // mitkDICOMReaderConfigurator_h
