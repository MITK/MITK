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

#include "mitkClassicDICOMSeriesReader.h"
#include "mitkDICOMTagBasedSorter.h"

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

  Tag element and group are interpreted as the exadecimal numbers
  found all around the DICOM standard. The numbers can be prepended by a "0x" if this is preferred
  by the programmer (but they are taken as hexadecimal in all cases).

  \section DICOMReaderConfigurator_AboutTheFuture About the future evolution of this class

  This first version is hard coded for the current state of the implementation.

  If things should evolve in a way that needs us to splitt off readers for "old" versions,
  time should be taken to refactor this class.

  Basically, a serializer class should accompany each of the configurable classes. Such
  serializer classes should be registered and discovered via micro-services (to support extensions).
  A serializer should offer both methods to serialize a class and to desirialize it again.

  A "version" attribute at the top-level tag should be used to distinguish versions.

  Usually it should be enough to keep DE-serializers for all versions. Writers for the most
  recent version should be enough.
*/
class MitkDICOMReader_EXPORT DICOMReaderConfigurator : public itk::LightObject
{
  public:

    mitkClassMacro( DICOMReaderConfigurator, itk::LightObject )
    itkNewMacro( DICOMReaderConfigurator )

    DICOMFileReader::Pointer CreateFromConfigFile(const std::string& filename) const;
    DICOMFileReader::Pointer CreateFromUTF8ConfigString(const std::string& xmlContents) const;

    std::string CreateConfigStringFromReader(DICOMFileReader::ConstPointer reader) const;

  protected:

    DICOMReaderConfigurator();
    virtual ~DICOMReaderConfigurator();

  private:

    DICOMFileReader::Pointer CreateFromTiXmlDocument(TiXmlDocument& doc) const;
    DICOMTag tagFromXMLElement(TiXmlElement*) const;
    std::string requiredStringAttribute(TiXmlElement* xmlElement, const std::string& key) const;
    unsigned int hexStringToUInt(const std::string& s) const;

    ThreeDnTDICOMSeriesReader::Pointer ConfigureThreeDnTDICOMSeriesReader(ThreeDnTDICOMSeriesReader::Pointer reader, TiXmlElement*) const;
    DICOMITKSeriesGDCMReader::Pointer ConfigureDICOMITKSeriesGDCMReader(DICOMITKSeriesGDCMReader::Pointer reader, TiXmlElement*) const;
    void ConfigureCommonPropertiesOfDICOMITKSeriesGDCMReader(DICOMITKSeriesGDCMReader::Pointer reader, TiXmlElement* element) const;
    void ConfigureCommonPropertiesOfThreeDnTDICOMSeriesReader(ThreeDnTDICOMSeriesReader::Pointer reader, TiXmlElement* element) const;

    DICOMSortCriterion::Pointer CreateDICOMSortByTag(TiXmlElement* xmlElement, DICOMSortCriterion::Pointer secondaryCriterion) const;
    DICOMSortCriterion::Pointer CreateSortByImagePositionPatient(TiXmlElement* xmlElement, DICOMSortCriterion::Pointer secondaryCriterion) const;

    mitk::DICOMTagBasedSorter::Pointer CreateDICOMTagBasedSorter(TiXmlElement* element) const;

    TiXmlElement* CreateConfigStringFromReader(const DICOMITKSeriesGDCMReader* reader) const;
    TiXmlElement* CreateConfigStringFromReader(const ThreeDnTDICOMSeriesReader* reader) const;
    TiXmlElement* CreateConfigStringFromReader(const ClassicDICOMSeriesReader* reader) const;

    TiXmlElement* CreateConfigStringFromDICOMDatasetSorter(const DICOMTagBasedSorter* sorter) const;

    TiXmlElement* CreateConfigStringFromDICOMTag(const DICOMTag& tag) const;

    TiXmlElement* CreateDICOMFileReaderTag(const DICOMFileReader* reader) const;
    const char* toString(bool) const;
    std::string toHexString(unsigned int i) const;
 };

} // namespace

#endif // mitkDICOMReaderConfigurator_h
