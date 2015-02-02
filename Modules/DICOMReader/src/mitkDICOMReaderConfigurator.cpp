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

#include "mitkDICOMReaderConfigurator.h"

#include "mitkDICOMSortByTag.h"
#include "mitkSortByImagePositionPatient.h"

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
  TiXmlDocument doc (filename);
  if (doc.LoadFile())
  {
    return this->CreateFromTiXmlDocument( doc );
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
  TiXmlDocument doc;
  doc.Parse(xmlContents.c_str(), 0, TIXML_ENCODING_UTF8);

  return this->CreateFromTiXmlDocument( doc );
}

mitk::DICOMFileReader::Pointer
mitk::DICOMReaderConfigurator
::CreateFromTiXmlDocument(TiXmlDocument& doc) const
{
  TiXmlHandle root(doc.RootElement());

  if (TiXmlElement* rootElement = root.ToElement())
  {
    if (strcmp(rootElement->Value(), "DICOMFileReader")) // :-( no std::string methods
    {
      MITK_ERROR << "File should contain a <DICOMFileReader> tag at top-level! Found '"
                 << (rootElement->Value() ? std::string(rootElement->Value()) : std::string("!nothing!")) << "' instead";
      return NULL;
    }

    const char* classnameC = rootElement->Attribute("class");
    if (!classnameC)
    {
      MITK_ERROR << "File should name a reader class in the class attribute: <DICOMFileReader class=\"...\">. Found nothing instead";
      return NULL;
    }

    int version(1);
    if ( rootElement->QueryIntAttribute("version", &version) == TIXML_SUCCESS)
    {
      if (version != 1)
      {
        MITK_WARN << "This reader is only capable of creating DICOMFileReaders of version 1. "
                  << "Will not continue, because given configuration is meant for version " << version << ".";
        return NULL;
      }
    }
    else
    {
      MITK_ERROR << "File should name the version of the reader class in the version attribute: <DICOMFileReader class=\"" << classnameC << "\" version=\"...\">."
                 << " Found nothing instead, assuming version 1!";
      version = 1;
    }

    std::string classname(classnameC);

    double decimalPlacesForOrientation(5);
    bool useDecimalPlacesForOrientation(false);
    useDecimalPlacesForOrientation =
      rootElement->QueryDoubleAttribute("decimalPlacesForOrientation", &decimalPlacesForOrientation) == TIXML_SUCCESS; // attribute present and a double value

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
      mitk::DICOMITKSeriesGDCMReader::Pointer reader;
      if (useDecimalPlacesForOrientation)
        reader = mitk::DICOMITKSeriesGDCMReader::New(decimalPlacesForOrientation);
      else
        reader = mitk::DICOMITKSeriesGDCMReader::New();

      return ConfigureDICOMITKSeriesGDCMReader(reader, rootElement).GetPointer();
    }
    else
    {
      MITK_ERROR << "DICOMFileReader tag names unknown class '" << classname << "'";
      return NULL;
    }
  }
  else
  {
    MITK_ERROR << "Great confusion: no root element in XML document. Expecting a DICOMFileReader tag at top-level.";
    return NULL;
  }
}

#define boolStringTrue(s) \
  (   s == "true" || s == "on" || s == "1" \
   || s == "TRUE" || s == "ON")

void
mitk::DICOMReaderConfigurator
::ConfigureCommonPropertiesOfThreeDnTDICOMSeriesReader(ThreeDnTDICOMSeriesReader::Pointer reader, TiXmlElement* element) const
{
  // add the "group3DnT" flag
  bool group3DnT(true);
  const char* group3DnTC = element->Attribute("group3DnT");
  if (group3DnTC)
  {
    std::string group3DnTS(group3DnTC);
    group3DnT = boolStringTrue(group3DnTS);
  }

  reader->SetGroup3DandT( group3DnT );
}

mitk::ThreeDnTDICOMSeriesReader::Pointer
mitk::DICOMReaderConfigurator
::ConfigureThreeDnTDICOMSeriesReader(ThreeDnTDICOMSeriesReader::Pointer reader, TiXmlElement* element) const
{
  assert(element);

  // use all the base class configuration
  if (this->ConfigureDICOMITKSeriesGDCMReader( reader.GetPointer(), element ).IsNull())
  {
    return NULL;
  }

  this->ConfigureCommonPropertiesOfThreeDnTDICOMSeriesReader(reader,element);
  return reader;
}

void
mitk::DICOMReaderConfigurator
::ConfigureCommonPropertiesOfDICOMITKSeriesGDCMReader(DICOMITKSeriesGDCMReader::Pointer reader, TiXmlElement* element) const
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
    std::string configDescription(configDescriptionC);
    reader->SetConfigurationDescription(configDescriptionC);
  }

  // "fixTiltByShearing" flag
  bool fixTiltByShearing(false);
  const char* fixTiltByShearingC = element->Attribute("fixTiltByShearing");
  if (fixTiltByShearingC)
  {
    std::string fixTiltByShearingS(fixTiltByShearingC);
    fixTiltByShearing = boolStringTrue(fixTiltByShearingS);
  }

  reader->SetFixTiltByShearing( fixTiltByShearing );

}

mitk::DICOMITKSeriesGDCMReader::Pointer
mitk::DICOMReaderConfigurator
::ConfigureDICOMITKSeriesGDCMReader(DICOMITKSeriesGDCMReader::Pointer reader, TiXmlElement* element) const
{
  assert(element);

  this->ConfigureCommonPropertiesOfDICOMITKSeriesGDCMReader(reader, element);

  // "acceptTwoSlicesGroups" flag
  bool acceptTwoSlicesGroups(true);
  const char* acceptTwoSlicesGroupsC = element->Attribute("acceptTwoSlicesGroups");
  if (acceptTwoSlicesGroupsC)
  {
    std::string acceptTwoSlicesGroupsS(acceptTwoSlicesGroupsC);
    acceptTwoSlicesGroups = boolStringTrue(acceptTwoSlicesGroupsS);
  }

  reader->SetAcceptTwoSlicesGroups( acceptTwoSlicesGroups );

  // "toleratedOriginError" attribute (double)
  bool toleratedOriginErrorIsAbsolute(false);
  const char* toleratedOriginErrorIsAbsoluteC = element->Attribute("toleratedOriginErrorIsAbsolute");
  if (toleratedOriginErrorIsAbsoluteC)
  {
    std::string toleratedOriginErrorIsAbsoluteS(toleratedOriginErrorIsAbsoluteC);
    toleratedOriginErrorIsAbsolute = boolStringTrue(toleratedOriginErrorIsAbsoluteS);
  }

  double toleratedOriginError(0.3);
  if (element->QueryDoubleAttribute("toleratedOriginError", &toleratedOriginError) == TIXML_SUCCESS) // attribute present and a double value
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
  TiXmlElement* dElement = element->FirstChildElement("DICOMDatasetSorter");
  if (dElement)
  {
    const char* classnameC = dElement->Attribute("class");
    if (!classnameC)
    {
      MITK_ERROR << "File should name a DICOMDatasetSorter class in the class attribute of <DICOMDatasetSorter class=\"...\">. Found nothing instead";
      return NULL;
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
      return NULL;
    }
  }

  return reader;
}

mitk::DICOMTagBasedSorter::Pointer
mitk::DICOMReaderConfigurator
::CreateDICOMTagBasedSorter(TiXmlElement* element) const
{
  mitk::DICOMTagBasedSorter::Pointer tagSorter = mitk::DICOMTagBasedSorter::New();

  // "strictSorting" parameter!
  bool strictSorting(true);
  const char* strictSortingC = element->Attribute("strictSorting");
  if (strictSortingC)
  {
    std::string strictSortingS(strictSortingC);
    strictSorting = boolStringTrue(strictSortingS);
  }
  tagSorter->SetStrictSorting(strictSorting);

  // "strictSorting" parameter!
  bool expectDistanceOne(true);
  const char* expectDistanceOneC = element->Attribute("expectDistanceOne");
  if (expectDistanceOneC)
  {
    std::string expectDistanceOneS(expectDistanceOneC);
    expectDistanceOne = boolStringTrue(expectDistanceOneS);
  }
  tagSorter->SetExpectDistanceOne(expectDistanceOne);

  TiXmlElement* dElement = element->FirstChildElement("Distinguishing");
  if (dElement)
  {
    for ( TiXmlElement* tChild = dElement->FirstChildElement();
          tChild != NULL;
          tChild = tChild->NextSiblingElement() )
    {
      try
      {
        mitk::DICOMTag tag = tagFromXMLElement(tChild);
        int i(5);
        if (tChild->QueryIntAttribute("cutDecimalPlaces", &i) == TIXML_SUCCESS)
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
        return NULL;
      }
    }
  }

  // "sorting tags"
  TiXmlElement* sElement = element->FirstChildElement("Sorting");
  if (sElement)
  {
    DICOMSortCriterion::Pointer previousCriterion;
    DICOMSortCriterion::Pointer currentCriterion;

    for ( TiXmlNode* tChildNode = sElement->LastChild();
        tChildNode != NULL;
        tChildNode = tChildNode->PreviousSibling() )
    {
      TiXmlElement* tChild = tChildNode->ToElement();
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
          ss << "Could not parse <Tag> element at (input line " << tChild->Row() << ", col. " << tChild->Column() << ")!";
          MITK_ERROR << ss.str();
          return NULL;
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
          ss << "Could not parse <ImagePositionPatient> element at (input line " << tChild->Row() << ", col. " << tChild->Column() << ")!";
          MITK_ERROR << ss.str();
          return NULL;
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
::requiredStringAttribute(TiXmlElement* xmlElement, const std::string& key) const
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
    ss << "Expected an attribute '" << key << "' at this position "
          "(input line " << xmlElement->Row() << ", col. " << xmlElement->Column() << ")!";
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
::tagFromXMLElement(TiXmlElement* xmlElement) const
{
  assert(xmlElement);

  if (strcmp(xmlElement->Value(), "Tag")) // :-( no std::string methods
  {
    std::stringstream ss;
    ss << "Expected a <Tag group=\"..\" element=\"..\"> tag at this position "
          "(input line " << xmlElement->Row() << ", col. " << xmlElement->Column() << ")!";
    MITK_ERROR << ss.str();
    throw std::invalid_argument( ss.str() );
  }

  std::string name = requiredStringAttribute(xmlElement, "name");
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
          "(input line " << xmlElement->Row() << ", col. " << xmlElement->Column() << ")!";
    MITK_ERROR << ss.str();
    throw std::invalid_argument( ss.str() );
  }
}

mitk::DICOMSortCriterion::Pointer
mitk::DICOMReaderConfigurator
::CreateDICOMSortByTag(TiXmlElement* xmlElement, DICOMSortCriterion::Pointer secondaryCriterion) const
{
  mitk::DICOMTag tag = tagFromXMLElement(xmlElement);
  return DICOMSortByTag::New(tag, secondaryCriterion).GetPointer();
}

mitk::DICOMSortCriterion::Pointer
mitk::DICOMReaderConfigurator
::CreateSortByImagePositionPatient(TiXmlElement*, DICOMSortCriterion::Pointer secondaryCriterion) const
{
  return SortByImagePositionPatient::New(secondaryCriterion).GetPointer();
}



std::string
mitk::DICOMReaderConfigurator
::CreateConfigStringFromReader(DICOMFileReader::ConstPointer reader) const
{
  // check possible sub-classes from the most-specific one up to the most generic one
  const DICOMFileReader* cPointer = reader;
  TiXmlElement* root;
  if (const ClassicDICOMSeriesReader* specificReader = dynamic_cast<const ClassicDICOMSeriesReader*>(cPointer))
  {
    root = this->CreateConfigStringFromReader(specificReader);
  }
  else
  if (const ThreeDnTDICOMSeriesReader* specificReader = dynamic_cast<const ThreeDnTDICOMSeriesReader*>(cPointer))
  {
    root = this->CreateConfigStringFromReader(specificReader);
  }
  else
  if (const DICOMITKSeriesGDCMReader* specificReader = dynamic_cast<const DICOMITKSeriesGDCMReader*>(cPointer))
  {
    root = this->CreateConfigStringFromReader(specificReader);
  }
  else
  {
    MITK_WARN << "Unknown reader class passed to DICOMReaderConfigurator::CreateConfigStringFromReader(). Cannot serialize.";
    return ""; // no serialization, what a pity
  }

  if (root)
  {
    TiXmlDocument document;
    document.LinkEndChild( root );

    TiXmlPrinter printer;
    printer.SetIndent( "  " );

    document.Accept( &printer );
    std::string xmltext = printer.CStr();
    return xmltext;
  }
  else
  {
    MITK_WARN << "DICOMReaderConfigurator::CreateConfigStringFromReader() created empty serialization. Problem?";
    return "";
  }
}

TiXmlElement*
mitk::DICOMReaderConfigurator
::CreateConfigStringFromReader(const DICOMITKSeriesGDCMReader* reader) const
{
  TiXmlElement* root = this->CreateDICOMFileReaderTag(reader);
  assert(root);

  root->SetAttribute("fixTiltByShearing", toString(reader->GetFixTiltByShearing()));
  root->SetAttribute("acceptTwoSlicesGroups", toString(reader->GetAcceptTwoSlicesGroups()));
  root->SetDoubleAttribute("toleratedOriginError", reader->GetToleratedOriginError());
  root->SetAttribute("toleratedOriginErrorIsAbsolute", toString(reader->IsToleratedOriginOffsetAbsolute()));
  root->SetDoubleAttribute("decimalPlacesForOrientation", reader->GetDecimalPlacesForOrientation());

  // iterate DICOMDatasetSorter objects
  DICOMITKSeriesGDCMReader::ConstSorterList sorterList = reader->GetFreelyConfiguredSortingElements();
  for(DICOMITKSeriesGDCMReader::ConstSorterList::const_iterator sorterIter = sorterList.begin();
      sorterIter != sorterList.end();
      ++sorterIter)
  {
    const DICOMDatasetSorter* sorter = *sorterIter;
    if (const DICOMTagBasedSorter* specificSorter = dynamic_cast<const DICOMTagBasedSorter*>(sorter))
    {
      TiXmlElement* sorterTag = this->CreateConfigStringFromDICOMDatasetSorter(specificSorter);
      root->LinkEndChild(sorterTag);
    }
    else
    {
      MITK_WARN << "Unknown DICOMDatasetSorter class passed to DICOMReaderConfigurator::CreateConfigStringFromReader(). Cannot serialize.";
      return NULL;
    }
  }

  return root;
}

TiXmlElement*
mitk::DICOMReaderConfigurator
::CreateConfigStringFromDICOMDatasetSorter(const DICOMTagBasedSorter* sorter) const
{
  assert(sorter);

  TiXmlElement* sorterTag = new TiXmlElement("DICOMDatasetSorter");
  sorterTag->SetAttribute("class", sorter->GetNameOfClass());
  sorterTag->SetAttribute("strictSorting", toString(sorter->GetStrictSorting()));
  sorterTag->SetAttribute("expectDistanceOne", toString(sorter->GetExpectDistanceOne()));

  TiXmlElement* distinguishingTagsElement = new TiXmlElement("Distinguishing");
  sorterTag->LinkEndChild(distinguishingTagsElement);
  mitk::DICOMTagList distinguishingTags = sorter->GetDistinguishingTags();
  for (DICOMTagList::iterator tagIter = distinguishingTags.begin();
       tagIter != distinguishingTags.end();
       ++tagIter)
  {
    TiXmlElement* tag = this->CreateConfigStringFromDICOMTag(*tagIter);
    distinguishingTagsElement->LinkEndChild(tag);

    const DICOMTagBasedSorter::TagValueProcessor* processor = sorter->GetTagValueProcessorForDistinguishingTag(*tagIter);
    if (const DICOMTagBasedSorter::CutDecimalPlaces* specificProcessor = dynamic_cast<const DICOMTagBasedSorter::CutDecimalPlaces*>(processor))
    {
      tag->SetDoubleAttribute("cutDecimalPlaces", specificProcessor->GetPrecision());
    }
  }

  TiXmlElement* sortingElement = new TiXmlElement("Sorting");
  sorterTag->LinkEndChild(sortingElement);
  mitk::DICOMSortCriterion::ConstPointer sortCriterion = sorter->GetSortCriterion();
  while (sortCriterion.IsNotNull())
  {
    std::string classname = sortCriterion->GetNameOfClass();
    if (classname == "SortByImagePositionPatient")
    {
      sortingElement->LinkEndChild( new TiXmlElement("ImagePositionPatient") ); // no parameters
    }
    else
    if (classname == "DICOMSortByTag")
    {
      DICOMTagList pseudoTagList = sortCriterion->GetTagsOfInterest();
      if (pseudoTagList.size() == 1)
      {
        DICOMTag firstTag = pseudoTagList.front();

        TiXmlElement* tagElement = this->CreateConfigStringFromDICOMTag(firstTag);

        sortingElement->LinkEndChild( tagElement );
      }
      else
      {
        MITK_ERROR << "Encountered SortByTag class with MULTIPLE tag in CreateConfigStringFromDICOMDatasetSorter. Cannot serialize.";
        return NULL;
      }
    }
    else
    {
      MITK_ERROR << "Encountered unknown class '" << classname << "' in CreateConfigStringFromDICOMDatasetSorter. Cannot serialize.";
      return NULL;
    }

    sortCriterion = sortCriterion->GetSecondaryCriterion();
  }

  return sorterTag;
}

TiXmlElement*
mitk::DICOMReaderConfigurator
::CreateConfigStringFromDICOMTag(const DICOMTag& tag) const
{
  TiXmlElement* tagElement = new TiXmlElement("Tag"); // name group element
  tagElement->SetAttribute("name", tag.GetName().c_str());
  tagElement->SetAttribute("group", toHexString(tag.GetGroup()));
  tagElement->SetAttribute("element", toHexString(tag.GetElement()));
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


TiXmlElement*
mitk::DICOMReaderConfigurator
::CreateConfigStringFromReader(const ThreeDnTDICOMSeriesReader* reader) const
{
  TiXmlElement* root = this->CreateConfigStringFromReader(static_cast<const DICOMITKSeriesGDCMReader*>(reader));
  assert(root);

  root->SetAttribute("group3DnT", toString(reader->GetGroup3DandT()));

  return root;
}

const char*
mitk::DICOMReaderConfigurator
::toString(bool b) const
{
  return b ? "true" : "false";
}

TiXmlElement*
mitk::DICOMReaderConfigurator
::CreateConfigStringFromReader(const ClassicDICOMSeriesReader* reader) const
{
  return this->CreateDICOMFileReaderTag(reader);
}

TiXmlElement*
mitk::DICOMReaderConfigurator
::CreateDICOMFileReaderTag(const DICOMFileReader* reader) const
{
  TiXmlElement* readerTag = new TiXmlElement("DICOMFileReader");
  readerTag->SetAttribute("class", reader->GetNameOfClass());
  readerTag->SetAttribute("label", reader->GetConfigurationLabel().c_str());
  readerTag->SetAttribute("description", reader->GetConfigurationDescription().c_str());
  readerTag->SetAttribute("version", "1");

  return readerTag;
}
