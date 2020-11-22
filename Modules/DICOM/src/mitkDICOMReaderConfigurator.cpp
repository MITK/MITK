/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkDICOMReaderConfigurator.h"

#include "mitkDICOMSortByTag.h"
#include "mitkSortByImagePositionPatient.h"

#include <tinyxml2.h>

mitk::DICOMReaderConfigurator
::DICOMReaderConfigurator()
{
}

mitk::DICOMReaderConfigurator
::~DICOMReaderConfigurator()
{
}

mitk::DICOMFileReader::Pointer
mitk::DICOMReaderConfigurator
::CreateFromConfigFile(const std::string& filename) const
{
  tinyxml2::XMLDocument doc;
  if (tinyxml2::XML_SUCCESS == doc.LoadFile(filename.c_str()))
  {
    return this->CreateFromXMLDocument( doc );
  }
  else
  {
    MITK_ERROR << "Unable to load file at '" << filename <<"'";
    return DICOMFileReader::Pointer();
  }
}

mitk::DICOMFileReader::Pointer
mitk::DICOMReaderConfigurator
::CreateFromUTF8ConfigString(const std::string& xmlContents) const
{
  tinyxml2::XMLDocument doc;
  doc.Parse(xmlContents.c_str());

  return this->CreateFromXMLDocument( doc );
}

mitk::DICOMFileReader::Pointer
mitk::DICOMReaderConfigurator
::CreateFromXMLDocument(tinyxml2::XMLDocument& doc) const
{
  tinyxml2::XMLHandle root(doc.RootElement());

  if (auto* rootElement = root.ToElement())
  {
    if (strcmp(rootElement->Value(), "DICOMFileReader")) // :-( no std::string methods
    {
      MITK_ERROR << "File should contain a <DICOMFileReader> tag at top-level! Found '"
                 << (rootElement->Value() ? std::string(rootElement->Value()) : std::string("!nothing!")) << "' instead";
      return nullptr;
    }

    const char* classnameC = rootElement->Attribute("class");
    if (!classnameC)
    {
      MITK_ERROR << "File should name a reader class in the class attribute: <DICOMFileReader class=\"...\">. Found nothing instead";
      return nullptr;
    }

    int version(1);
    if ( rootElement->QueryIntAttribute("version", &version) == tinyxml2::XML_SUCCESS)
    {
      if (version == 1)
      {
        MITK_WARN << "Warning the given configuration is for DICOMFileReaders of version 1. "
                  << "This old version may be interpreted differently. Reason: "
                  << "The default values for the following xml settings have been changed: "
                  << "FixTiltByShearing (false -> true); StrictSorting (true -> false); ExpectDistanceOne (true -> false).";
      }
      else if (version >2)
      {
        MITK_WARN << "This reader is only capable of creating DICOMFileReaders of version 1 and 2. "
          << "Will not continue, because given configuration is meant for version " << version << ".";
        return nullptr;
      }
    }
    else
    {
      MITK_ERROR << "File should name the version of the reader class in the version attribute: <DICOMFileReader class=\"" << classnameC << "\" version=\"...\">."
                 << " Found nothing instead, assuming version 1!";
      version = 1;
    }

    std::string classname(classnameC);

    double decimalPlacesForOrientation(mitk::DICOMITKSeriesGDCMReader::GetDefaultDecimalPlacesForOrientation());
    bool useDecimalPlacesForOrientation(false);
    useDecimalPlacesForOrientation =
      rootElement->QueryDoubleAttribute("decimalPlacesForOrientation", &decimalPlacesForOrientation) == tinyxml2::XML_SUCCESS; // attribute present and a double value

    if (classname == "ClassicDICOMSeriesReader")
    {
      mitk::ClassicDICOMSeriesReader::Pointer reader = mitk::ClassicDICOMSeriesReader::New();
      this->ConfigureCommonPropertiesOfDICOMITKSeriesGDCMReader(reader.GetPointer(), rootElement);
      this->ConfigureCommonPropertiesOfThreeDnTDICOMSeriesReader(reader.GetPointer(), rootElement);
      return reader.GetPointer();
    }
    if (classname == "ThreeDnTDICOMSeriesReader")
    {
      mitk::ThreeDnTDICOMSeriesReader::Pointer reader;
      if (useDecimalPlacesForOrientation)
        reader = mitk::ThreeDnTDICOMSeriesReader::New(decimalPlacesForOrientation);
      else
        reader = mitk::ThreeDnTDICOMSeriesReader::New();

      return ConfigureThreeDnTDICOMSeriesReader(reader, rootElement).GetPointer();
    }
    else
    if (classname == "DICOMITKSeriesGDCMReader")
    {
      bool simpleVolumeImport = QueryBooleanAttribute(rootElement, "simpleVolumeImport", mitk::DICOMITKSeriesGDCMReader::GetDefaultSimpleVolumeImport());

      mitk::DICOMITKSeriesGDCMReader::Pointer reader;
      if (useDecimalPlacesForOrientation)
        reader = mitk::DICOMITKSeriesGDCMReader::New( decimalPlacesForOrientation, simpleVolumeImport );
      else
        reader = mitk::DICOMITKSeriesGDCMReader::New( mitk::DICOMITKSeriesGDCMReader::GetDefaultDecimalPlacesForOrientation(), simpleVolumeImport );

      // simple volume import that ignores number of frames and inter slice distance

      return ConfigureDICOMITKSeriesGDCMReader(reader, rootElement).GetPointer();
    }
    else
    {
      MITK_ERROR << "DICOMFileReader tag names unknown class '" << classname << "'";
      return nullptr;
    }
  }
  else
  {
    MITK_ERROR << "Great confusion: no root element in XML document. Expecting a DICOMFileReader tag at top-level.";
    return nullptr;
  }
}

#define boolStringTrue(s) \
  (   s == "true" || s == "on" || s == "1" \
   || s == "TRUE" || s == "ON")

bool
mitk::DICOMReaderConfigurator
::QueryBooleanAttribute(const tinyxml2::XMLElement* element, const char* attributeName, bool defaultValue) const
{
  bool value(defaultValue);
  const auto* valueC = element->Attribute(attributeName);

  if (nullptr != valueC)
  {
    std::string valueS = valueC;
    value = boolStringTrue(valueS);
  }

  return value;
}

void
mitk::DICOMReaderConfigurator
::ConfigureCommonPropertiesOfThreeDnTDICOMSeriesReader(ThreeDnTDICOMSeriesReader::Pointer reader, const tinyxml2::XMLElement* element) const
{
  // add the "group3DnT" flag
  bool group3DnT = QueryBooleanAttribute(element, "group3DnT", ThreeDnTDICOMSeriesReader::GetDefaultGroup3DandT());

  reader->SetGroup3DandT( group3DnT );

  // add the "onlyCondenseSameSeries" flag
  bool onlyCondenseSameSeries = QueryBooleanAttribute(element, "onlyCondenseSameSeries", ThreeDnTDICOMSeriesReader::GetDefaultOnlyCondenseSameSeries());

  reader->SetOnlyCondenseSameSeries(onlyCondenseSameSeries);
}

mitk::ThreeDnTDICOMSeriesReader::Pointer
mitk::DICOMReaderConfigurator
::ConfigureThreeDnTDICOMSeriesReader(ThreeDnTDICOMSeriesReader::Pointer reader, const tinyxml2::XMLElement* element) const
{
  assert(element);

  // use all the base class configuration
  if (this->ConfigureDICOMITKSeriesGDCMReader( reader.GetPointer(), element ).IsNull())
  {
    return nullptr;
  }

  this->ConfigureCommonPropertiesOfThreeDnTDICOMSeriesReader(reader,element);
  return reader;
}

void
mitk::DICOMReaderConfigurator
::ConfigureCommonPropertiesOfDICOMITKSeriesGDCMReader(DICOMITKSeriesGDCMReader::Pointer reader, const tinyxml2::XMLElement* element) const
{
  assert(element);

  const char* configLabelC = element->Attribute("label");
  if (configLabelC)
  {
    std::string configLabel(configLabelC);
    reader->SetConfigurationLabel(configLabel);
  }

  const char* configDescriptionC = element->Attribute("description");
  if (configDescriptionC)
  {
    reader->SetConfigurationDescription(configDescriptionC);
  }

  // "fixTiltByShearing" flag
  bool fixTiltByShearing = QueryBooleanAttribute(element, "fixTiltByShearing", DICOMITKSeriesGDCMReader::GetDefaultFixTiltByShearing());

  reader->SetFixTiltByShearing( fixTiltByShearing );
}

mitk::DICOMITKSeriesGDCMReader::Pointer
mitk::DICOMReaderConfigurator
::ConfigureDICOMITKSeriesGDCMReader(DICOMITKSeriesGDCMReader::Pointer reader, const tinyxml2::XMLElement* element) const
{
  assert(element);

  this->ConfigureCommonPropertiesOfDICOMITKSeriesGDCMReader(reader, element);

  // "acceptTwoSlicesGroups" flag
  bool acceptTwoSlicesGroups = QueryBooleanAttribute(element, "acceptTwoSlicesGroups", true);

  reader->SetAcceptTwoSlicesGroups( acceptTwoSlicesGroups );

  // "toleratedOriginError" attribute (double)
  bool toleratedOriginErrorIsAbsolute = QueryBooleanAttribute(element, "toleratedOriginErrorIsAbsolute", false);

  double toleratedOriginError(0.3);
  if (element->QueryDoubleAttribute("toleratedOriginError", &toleratedOriginError) == tinyxml2::XML_SUCCESS) // attribute present and a double value
  {
    if (toleratedOriginErrorIsAbsolute)
    {
      reader->SetToleratedOriginOffset( toleratedOriginError );
    }
    else
    {
      reader->SetToleratedOriginOffsetToAdaptive( toleratedOriginError );
    }
  }

  // DICOMTagBasedSorters are the only thing we create at this point
  // TODO for-loop over all child elements of type DICOMTagBasedSorter, BUT actually a single sorter of this type is enough.
  auto* dElement = element->FirstChildElement("DICOMDatasetSorter");
  if (dElement)
  {
    const char* classnameC = dElement->Attribute("class");
    if (!classnameC)
    {
      MITK_ERROR << "File should name a DICOMDatasetSorter class in the class attribute of <DICOMDatasetSorter class=\"...\">. Found nothing instead";
      return nullptr;
    }

    std::string classname(classnameC);

    if (classname == "DICOMTagBasedSorter")
    {
      DICOMTagBasedSorter::Pointer tagSorter = CreateDICOMTagBasedSorter(dElement);
      if (tagSorter.IsNotNull())
      {
        reader->AddSortingElement( tagSorter );
      }
    }
    else
    {
      MITK_ERROR << "DICOMDatasetSorter tag names unknown class '" << classname << "'";
      return nullptr;
    }
  }

  return reader;
}

mitk::DICOMTagBasedSorter::Pointer
mitk::DICOMReaderConfigurator
::CreateDICOMTagBasedSorter(const tinyxml2::XMLElement* element) const
{
  mitk::DICOMTagBasedSorter::Pointer tagSorter = mitk::DICOMTagBasedSorter::New();

  // "strictSorting" parameter!
  bool strictSorting = QueryBooleanAttribute(element, "strictSorting", mitk::DICOMTagBasedSorter::GetDefaultStrictSorting());
  tagSorter->SetStrictSorting(strictSorting);

  // "strictSorting" parameter!
  bool expectDistanceOne = QueryBooleanAttribute(element, "expectDistanceOne", mitk::DICOMTagBasedSorter::GetDefaultExpectDistanceOne());
  tagSorter->SetExpectDistanceOne(expectDistanceOne);

  auto* dElement = element->FirstChildElement("Distinguishing");
  if (dElement)
  {
    for ( auto* tChild = dElement->FirstChildElement();
          tChild != nullptr;
          tChild = tChild->NextSiblingElement() )
    {
      try
      {
        mitk::DICOMTag tag = tagFromXMLElement(tChild);
        int i(5);
        if (tChild->QueryIntAttribute("cutDecimalPlaces", &i) == tinyxml2::XML_SUCCESS)
        {
          tagSorter->AddDistinguishingTag( tag, new mitk::DICOMTagBasedSorter::CutDecimalPlaces(i) );
        }
        else
        {
          tagSorter->AddDistinguishingTag( tag );
        }
      }
      catch(...)
      {
        return nullptr;
      }
    }
  }

  // "sorting tags"
  auto* sElement = element->FirstChildElement("Sorting");
  if (sElement)
  {
    DICOMSortCriterion::Pointer previousCriterion;
    DICOMSortCriterion::Pointer currentCriterion;

    for ( auto* tChildNode = sElement->LastChild();
        tChildNode != nullptr;
        tChildNode = tChildNode->PreviousSibling() )
    {
      auto* tChild = tChildNode->ToElement();
      if (!tChild) continue;

      if (!strcmp(tChild->Value(), "Tag"))
      {
        try
        {
          currentCriterion = this->CreateDICOMSortByTag(tChild, previousCriterion);
        }
        catch(...)
        {
          std::stringstream ss;
          ss << "Could not parse <Tag> element at input line " << tChild->GetLineNum() << "!";
          MITK_ERROR << ss.str();
          return nullptr;
        }
      }
      else
      if (!strcmp(tChild->Value(), "ImagePositionPatient"))
      {
        try
        {
          currentCriterion = this->CreateSortByImagePositionPatient(tChild, previousCriterion);
        }
        catch(...)
        {
          std::stringstream ss;
          ss << "Could not parse <ImagePositionPatient> element at input line " << tChild->GetLineNum() << "!";
          MITK_ERROR << ss.str();
          return nullptr;
        }
      }
      else
      {
        MITK_ERROR << "File contain unknown tag <" << tChild->Value()  << "> tag as child to <Sorting>! Cannot interpret...";
      }

      previousCriterion = currentCriterion;
    }

    tagSorter->SetSortCriterion( currentCriterion.GetPointer() );
  }

  return tagSorter;
}

std::string
mitk::DICOMReaderConfigurator
::requiredStringAttribute(const tinyxml2::XMLElement* xmlElement, const std::string& key) const
{
  assert(xmlElement);

  const char* gC = xmlElement->Attribute(key.c_str());
  if (gC)
  {
    std::string gS(gC);
    return gS;
  }
  else
  {
    std::stringstream ss;
    ss << "Expected an attribute '" << key << "' at "
          "input line " << xmlElement->GetLineNum() << "!";
    MITK_ERROR << ss.str();
    throw std::invalid_argument( ss.str() );
  }
}

unsigned int
mitk::DICOMReaderConfigurator
::hexStringToUInt(const std::string& s) const
{
  std::stringstream converter(s);
  unsigned int ui;
  converter >> std::hex >> ui;
  MITK_DEBUG << "Converted string '" << s << "' to unsigned int " << ui;
  return ui;
}

mitk::DICOMTag
mitk::DICOMReaderConfigurator
::tagFromXMLElement(const tinyxml2::XMLElement* xmlElement) const
{
  assert(xmlElement);

  if (strcmp(xmlElement->Value(), "Tag")) // :-( no std::string methods
  {
    std::stringstream ss;
    ss << "Expected a <Tag group=\"..\" element=\"..\"> tag at "
          "input line " << xmlElement->GetLineNum() << "!";
    MITK_ERROR << ss.str();
    throw std::invalid_argument( ss.str() );
  }

  std::string groupS = requiredStringAttribute(xmlElement, "group");
  std::string elementS = requiredStringAttribute(xmlElement, "element");

  try
  {
    // convert string to int (assuming string is in hex format with leading "0x" like "0x0020")
    unsigned int group = hexStringToUInt(groupS);
    unsigned int element = hexStringToUInt(elementS);
    return DICOMTag(group, element);
  }
  catch(...)
  {
    std::stringstream ss;
    ss << "Expected group and element values in <Tag group=\"..\" element=\"..\"> to be hexadecimal with leading 0x, e.g. '0x0020'"
          "(input line " << xmlElement->GetLineNum() << ")!";
    MITK_ERROR << ss.str();
    throw std::invalid_argument( ss.str() );
  }
}

mitk::DICOMSortCriterion::Pointer
mitk::DICOMReaderConfigurator
::CreateDICOMSortByTag(const tinyxml2::XMLElement* xmlElement, DICOMSortCriterion::Pointer secondaryCriterion) const
{
  mitk::DICOMTag tag = tagFromXMLElement(xmlElement);
  return DICOMSortByTag::New(tag, secondaryCriterion).GetPointer();
}

mitk::DICOMSortCriterion::Pointer
mitk::DICOMReaderConfigurator
::CreateSortByImagePositionPatient(const tinyxml2::XMLElement*, DICOMSortCriterion::Pointer secondaryCriterion) const
{
  return SortByImagePositionPatient::New(secondaryCriterion).GetPointer();
}



std::string
mitk::DICOMReaderConfigurator
::CreateConfigStringFromReader(DICOMFileReader::ConstPointer reader) const
{
  // check possible sub-classes from the most-specific one up to the most generic one
  const DICOMFileReader* cPointer = reader;
  tinyxml2::XMLDocument document;
  tinyxml2::XMLElement* root = nullptr;
  if (const auto* specificReader = dynamic_cast<const ClassicDICOMSeriesReader*>(cPointer))
  {
    root = this->CreateConfigStringFromReader(document, specificReader);
  }
  else
  if (const auto* specificReader = dynamic_cast<const ThreeDnTDICOMSeriesReader*>(cPointer))
  {
    root = this->CreateConfigStringFromReader(document, specificReader);
  }
  else
  if (const auto* specificReader = dynamic_cast<const DICOMITKSeriesGDCMReader*>(cPointer))
  {
    root = this->CreateConfigStringFromReader(document, specificReader);
  }
  else
  {
    MITK_WARN << "Unknown reader class passed to DICOMReaderConfigurator::CreateConfigStringFromReader(). Cannot serialize.";
    return ""; // no serialization, what a pity
  }

  if (nullptr != root)
  {
    document.InsertEndChild( root );

    tinyxml2::XMLPrinter printer;
    document.Print(&printer);

    std::string xmltext = printer.CStr();
    return xmltext;
  }
  else
  {
    MITK_WARN << "DICOMReaderConfigurator::CreateConfigStringFromReader() created empty serialization. Problem?";
    return "";
  }
}

tinyxml2::XMLElement*
mitk::DICOMReaderConfigurator
::CreateConfigStringFromReader(tinyxml2::XMLDocument &doc, const DICOMITKSeriesGDCMReader* reader) const
{
  auto* root = this->CreateDICOMFileReaderTag(doc, reader);
  assert(root);

  root->SetAttribute("fixTiltByShearing", reader->GetFixTiltByShearing());
  root->SetAttribute("acceptTwoSlicesGroups", reader->GetAcceptTwoSlicesGroups());
  root->SetAttribute("toleratedOriginError", reader->GetToleratedOriginError());
  root->SetAttribute("toleratedOriginErrorIsAbsolute", reader->IsToleratedOriginOffsetAbsolute());
  root->SetAttribute("decimalPlacesForOrientation", reader->GetDecimalPlacesForOrientation());

  // iterate DICOMDatasetSorter objects
  DICOMITKSeriesGDCMReader::ConstSorterList sorterList = reader->GetFreelyConfiguredSortingElements();
  for(auto sorterIter = sorterList.begin();
      sorterIter != sorterList.end();
      ++sorterIter)
  {
    const DICOMDatasetSorter* sorter = *sorterIter;
    if (const auto* specificSorter = dynamic_cast<const DICOMTagBasedSorter*>(sorter))
    {
      auto* sorterTag = this->CreateConfigStringFromDICOMDatasetSorter(doc, specificSorter);
      root->InsertEndChild(sorterTag);
    }
    else
    {
      MITK_WARN << "Unknown DICOMDatasetSorter class passed to DICOMReaderConfigurator::CreateConfigStringFromReader(). Cannot serialize.";
      return nullptr;
    }
  }

  return root;
}

tinyxml2::XMLElement*
mitk::DICOMReaderConfigurator
::CreateConfigStringFromDICOMDatasetSorter(tinyxml2::XMLDocument &doc, const DICOMTagBasedSorter* sorter) const
{
  assert(sorter);

  auto *sorterTag = doc.NewElement("DICOMDatasetSorter");
  sorterTag->SetAttribute("class", sorter->GetNameOfClass());
  sorterTag->SetAttribute("strictSorting", sorter->GetStrictSorting());
  sorterTag->SetAttribute("expectDistanceOne", sorter->GetExpectDistanceOne());

  auto *distinguishingTagsElement = doc.NewElement("Distinguishing");
  sorterTag->InsertEndChild(distinguishingTagsElement);
  mitk::DICOMTagList distinguishingTags = sorter->GetDistinguishingTags();
  for (auto tagIter = distinguishingTags.begin();
       tagIter != distinguishingTags.end();
       ++tagIter)
  {
    auto* tag = this->CreateConfigStringFromDICOMTag(doc, *tagIter);
    distinguishingTagsElement->InsertEndChild(tag);

    const DICOMTagBasedSorter::TagValueProcessor* processor = sorter->GetTagValueProcessorForDistinguishingTag(*tagIter);
    if (const auto* specificProcessor = dynamic_cast<const DICOMTagBasedSorter::CutDecimalPlaces*>(processor))
    {
      tag->SetAttribute("cutDecimalPlaces", specificProcessor->GetPrecision());
    }
  }

  auto *sortingElement = doc.NewElement("Sorting");
  sorterTag->InsertEndChild(sortingElement);
  mitk::DICOMSortCriterion::ConstPointer sortCriterion = sorter->GetSortCriterion();
  while (sortCriterion.IsNotNull())
  {
    std::string classname = sortCriterion->GetNameOfClass();
    if (classname == "SortByImagePositionPatient")
    {
      sortingElement->InsertEndChild( doc.NewElement("ImagePositionPatient") ); // no parameters
    }
    else
    if (classname == "DICOMSortByTag")
    {
      DICOMTagList pseudoTagList = sortCriterion->GetTagsOfInterest();
      if (pseudoTagList.size() == 1)
      {
        DICOMTag firstTag = pseudoTagList.front();

        auto* tagElement = this->CreateConfigStringFromDICOMTag(doc, firstTag);

        sortingElement->InsertEndChild( tagElement );
      }
      else
      {
        MITK_ERROR << "Encountered SortByTag class with MULTIPLE tag in CreateConfigStringFromDICOMDatasetSorter. Cannot serialize.";
        return nullptr;
      }
    }
    else
    {
      MITK_ERROR << "Encountered unknown class '" << classname << "' in CreateConfigStringFromDICOMDatasetSorter. Cannot serialize.";
      return nullptr;
    }

    sortCriterion = sortCriterion->GetSecondaryCriterion();
  }

  return sorterTag;
}

tinyxml2::XMLElement*
mitk::DICOMReaderConfigurator
::CreateConfigStringFromDICOMTag(tinyxml2::XMLDocument& doc, const DICOMTag& tag) const
{
  auto  tagElement = doc.NewElement("Tag"); // name group element
  tagElement->SetAttribute("name", tag.GetName().c_str());
  tagElement->SetAttribute("group", toHexString(tag.GetGroup()).c_str());
  tagElement->SetAttribute("element", toHexString(tag.GetElement()).c_str());
  return tagElement;
}

std::string
mitk::DICOMReaderConfigurator
::toHexString(unsigned int i) const
{
  std::stringstream ss;
  ss << "0x" << std::setfill ('0') << std::setw(4) << std::hex << i;
  return ss.str();
}


tinyxml2::XMLElement*
mitk::DICOMReaderConfigurator
::CreateConfigStringFromReader(tinyxml2::XMLDocument& doc, const ThreeDnTDICOMSeriesReader* reader) const
{
  auto* root = this->CreateConfigStringFromReader(doc, static_cast<const DICOMITKSeriesGDCMReader*>(reader));
  assert(root);

  root->SetAttribute("group3DnT", reader->GetGroup3DandT());

  return root;
}

tinyxml2::XMLElement*
mitk::DICOMReaderConfigurator
::CreateConfigStringFromReader(tinyxml2::XMLDocument& doc, const ClassicDICOMSeriesReader* reader) const
{
  return this->CreateDICOMFileReaderTag(doc, reader);
}

tinyxml2::XMLElement*
mitk::DICOMReaderConfigurator
::CreateDICOMFileReaderTag(tinyxml2::XMLDocument& doc, const DICOMFileReader* reader) const
{
  auto readerTag = doc.NewElement("DICOMFileReader");
  readerTag->SetAttribute("class", reader->GetNameOfClass());
  readerTag->SetAttribute("label", reader->GetConfigurationLabel().c_str());
  readerTag->SetAttribute("description", reader->GetConfigurationDescription().c_str());
  readerTag->SetAttribute("version", "1");

  return readerTag;
}
