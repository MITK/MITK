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

#include "mitkClassicDICOMSeriesReader.h"

#include "mitkDICOMTagBasedSorter.h"

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
::CreateFromConfigFile(const std::string& filename)
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
::CreateFromUTF8ConfigString(const std::string& xmlContents)
{
  TiXmlDocument doc;
  doc.Parse(xmlContents.c_str(), 0, TIXML_ENCODING_UTF8);

  return this->CreateFromTiXmlDocument( doc );
}

mitk::DICOMFileReader::Pointer
mitk::DICOMReaderConfigurator
::CreateFromTiXmlDocument(TiXmlDocument& doc)
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

    std::string classname(classnameC);

    double decimalPlacesForOrientation(5);
    bool useDecimalPlacesForOrientation(false);
    useDecimalPlacesForOrientation =
      rootElement->QueryDoubleAttribute("decimalPlacesForOrientation", &decimalPlacesForOrientation) == TIXML_SUCCESS; // attribute present and a double value

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

mitk::ThreeDnTDICOMSeriesReader::Pointer
mitk::DICOMReaderConfigurator
::ConfigureThreeDnTDICOMSeriesReader(ThreeDnTDICOMSeriesReader::Pointer reader, TiXmlElement* element)
{
  assert(element);

  // use all the base class configuration
  if (this->ConfigureDICOMITKSeriesGDCMReader( reader.GetPointer(), element ).IsNull())
  {
    return NULL;
  }

  // add the "group3DnT" flag
  bool group3DnT(true);
  const char* group3DnTC = element->Attribute("group3DnT");
  if (group3DnTC)
  {
    std::string group3DnTS(group3DnTC);
    group3DnT = boolStringTrue(group3DnTS);
  }

  reader->SetGroup3DandT( group3DnT );

  return reader;
}

mitk::DICOMITKSeriesGDCMReader::Pointer
mitk::DICOMReaderConfigurator
::ConfigureDICOMITKSeriesGDCMReader(DICOMITKSeriesGDCMReader::Pointer reader, TiXmlElement* element)
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

  // "toleratedOriginError" attribute (double)
  double toleratedOriginError(-0.3);
  if (element->QueryDoubleAttribute("toleratedOriginError", &toleratedOriginError) == TIXML_SUCCESS) // attribute present and a double value
  {
    if (toleratedOriginError >= 0.0)
    {
      reader->SetToleratedOriginOffset( toleratedOriginError );
    }
    else
    {
      reader->SetToleratedOriginOffsetToAdaptive( -1.0 * toleratedOriginError );
    }
  }



  // "distinguishing tags"
  mitk::DICOMTagBasedSorter::Pointer tagSorter = mitk::DICOMTagBasedSorter::New();
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
        tagSorter->AddDistinguishingTag( tag );

      }
      catch(...)
      {
        return NULL;
      }
    }
  }

  // "sorting tags"
  TiXmlElement* sElement = element->FirstChildElement("Sorting");
  if (dElement)
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

  reader->AddSortingElement( tagSorter );

  return reader;
}

std::string
mitk::DICOMReaderConfigurator
::requiredStringAttribute(TiXmlElement* xmlElement, const std::string& key)
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
::hexStringToUInt(const std::string& s)
{
  std::stringstream converter(s);
  unsigned int ui;
  converter >> std::hex >> ui;
  MITK_DEBUG << "Converted string '" << s << "' to unsigned int " << ui;
  return ui;
}

mitk::DICOMTag
mitk::DICOMReaderConfigurator
::tagFromXMLElement(TiXmlElement* xmlElement)
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
::CreateDICOMSortByTag(TiXmlElement* xmlElement, DICOMSortCriterion::Pointer secondaryCriterion)
{
  mitk::DICOMTag tag = tagFromXMLElement(xmlElement);
  return DICOMSortByTag::New(tag, secondaryCriterion).GetPointer();
}

mitk::DICOMSortCriterion::Pointer
mitk::DICOMReaderConfigurator
::CreateSortByImagePositionPatient(TiXmlElement*, DICOMSortCriterion::Pointer secondaryCriterion)
{
  return SortByImagePositionPatient::New(secondaryCriterion).GetPointer();
}
