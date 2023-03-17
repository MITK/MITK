/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDICOMReaderConfigurator_h
#define mitkDICOMReaderConfigurator_h

#include "mitkClassicDICOMSeriesReader.h"
#include "mitkDICOMTagBasedSorter.h"

namespace tinyxml2
{
  class XMLDocument;
  class XMLElement;
}

namespace mitk
{

/**
  \ingroup DICOMModule
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

  The root-tag \c \<DICOMFileReader\> names the class to be instantiated, currently this can be one of
   - DICOMITKSeriesGDCMReader
   - ThreeDnTDICOMSeriesReader

  Both classes bring simple configuration flags with them and a description of how images are sorted prior to loading.

  Flag for DICOMITKSeriesGDCMReader:
  <pre>fixTiltByShearing="true|false"</pre>
  Determines whether a potential gantry tilt should be "fixed" by shearing the output image.

  Flag for ThreeDnTDICOMSeriesReader:
  <pre>group3DnT="true|false"</pre>
  Determines whether images at the same spatial position should be interpreted as 3D+t images.

  The tags \c \<Distinguishing\> and \c \<Sorting\> describe the basic loading strategy of both
  reader mentioned above: first images are divided into incompatible groups (\c \<Distinguishing\>),
  and afterwards the images within each group are sorted by means of DICOMSortCriterion, which
  most commonly mentions a tag.

  Tag element and group are interpreted as the exadecimal numbers
  found all around the DICOM standard. The numbers can be prepended by a "0x" if this is preferred
  by the programmer (but they are taken as hexadecimal in all cases).

  \section DICOMReaderConfigurator_AboutTheFuture About the future evolution of this class

  This first version is hard coded for the current state of the implementation.

  If things should evolve in a way that needs us to split off readers for "old" versions,
  time should be taken to refactor this class.

  Basically, a serializer class should accompany each of the configurable classes. Such
  serializer classes should be registered and discovered via micro-services (to support extensions).
  A serializer should offer both methods to serialize a class and to desirialize it again.

  A "version" attribute at the top-level tag should be used to distinguish versions.

  Usually it should be enough to keep DE-serializers for all versions. Writers for the most
  recent version should be enough.
*/
class MITKDICOM_EXPORT DICOMReaderConfigurator : public itk::LightObject
{
  public:

    mitkClassMacroItkParent( DICOMReaderConfigurator, itk::LightObject );
    itkNewMacro( DICOMReaderConfigurator );

    DICOMFileReader::Pointer CreateFromConfigFile(const std::string& filename) const;
    DICOMFileReader::Pointer CreateFromUTF8ConfigString(const std::string& xmlContents) const;

    std::string CreateConfigStringFromReader(DICOMFileReader::ConstPointer reader) const;

  protected:

    DICOMReaderConfigurator();
    ~DICOMReaderConfigurator() override;

  private:

    DICOMFileReader::Pointer CreateFromXMLDocument(tinyxml2::XMLDocument& doc) const;
    DICOMTag tagFromXMLElement(const tinyxml2::XMLElement*) const;
    std::string requiredStringAttribute(const tinyxml2::XMLElement* xmlElement, const std::string& key) const;
    unsigned int hexStringToUInt(const std::string& s) const;

    ThreeDnTDICOMSeriesReader::Pointer ConfigureThreeDnTDICOMSeriesReader(ThreeDnTDICOMSeriesReader::Pointer reader, const tinyxml2::XMLElement*) const;
    DICOMITKSeriesGDCMReader::Pointer ConfigureDICOMITKSeriesGDCMReader(DICOMITKSeriesGDCMReader::Pointer reader, const tinyxml2::XMLElement*) const;
    void ConfigureCommonPropertiesOfDICOMITKSeriesGDCMReader(DICOMITKSeriesGDCMReader::Pointer reader, const tinyxml2::XMLElement* element) const;
    void ConfigureCommonPropertiesOfThreeDnTDICOMSeriesReader(ThreeDnTDICOMSeriesReader::Pointer reader, const tinyxml2::XMLElement* element) const;

    DICOMSortCriterion::Pointer CreateDICOMSortByTag(const tinyxml2::XMLElement* xmlElement, DICOMSortCriterion::Pointer secondaryCriterion) const;
    DICOMSortCriterion::Pointer CreateSortByImagePositionPatient(const tinyxml2::XMLElement* xmlElement, DICOMSortCriterion::Pointer secondaryCriterion) const;

    mitk::DICOMTagBasedSorter::Pointer CreateDICOMTagBasedSorter(const tinyxml2::XMLElement* element) const;

    tinyxml2::XMLElement* CreateConfigStringFromReader(tinyxml2::XMLDocument& doc, const DICOMITKSeriesGDCMReader* reader) const;
    tinyxml2::XMLElement* CreateConfigStringFromReader(tinyxml2::XMLDocument& doc, const ThreeDnTDICOMSeriesReader* reader) const;
    tinyxml2::XMLElement* CreateConfigStringFromReader(tinyxml2::XMLDocument& doc, const ClassicDICOMSeriesReader* reader) const;

    tinyxml2::XMLElement* CreateConfigStringFromDICOMDatasetSorter(tinyxml2::XMLDocument& doc, const DICOMTagBasedSorter* sorter) const;

    tinyxml2::XMLElement* CreateConfigStringFromDICOMTag(tinyxml2::XMLDocument& doc, const DICOMTag& tag) const;

    tinyxml2::XMLElement* CreateDICOMFileReaderTag(tinyxml2::XMLDocument& doc, const DICOMFileReader* reader) const;

    std::string toHexString(unsigned int i) const;

    /** Helper that queries an boolean xml attribute. If the attribute does not exist, the passed default value is used.*/
    bool QueryBooleanAttribute(const tinyxml2::XMLElement* element, const char* attributeName, bool defaultValue) const;
 };

} // namespace

#endif
