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
    if (classname == "ThreeDnTDICOMSeriesReader")
    {
      mitk::ThreeDnTDICOMSeriesReader::Pointer reader = mitk::ThreeDnTDICOMSeriesReader::New();
      return ConfigureThreeDnTDICOMSeriesReader(reader, rootElement).GetPointer();
    }
    else
    if (classname == "DICOMITKSeriesGDCMReader")
    {
      mitk::DICOMITKSeriesGDCMReader::Pointer reader = mitk::DICOMITKSeriesGDCMReader::New();
      return ConfigureDICOMITKSeriesGDCMReader(reader, rootElement).GetPointer();
    }
    else
    {
      MITK_ERROR << "DICOMFileReader tag names unknown class '" << classname << "'";
      return NULL;
    }
  }

  /*
  mitk::DICOMTagBasedSorter::Pointer tagSorter = mitk::DICOMTagBasedSorter::New();

  TiXmlHandle hroot (0);
    hroot = TiXmlHandle (doc.FirstChildElement());

    MITK_INFO<< "Loading dicom reader configuration... "<<hroot.Element()->Value();

    if (hroot.FirstChild("DistinguishingTags").Element() == NULL)
      return tagSorter;

    TiXmlElement* distTag = hroot.FirstChild("DistinguishingTags").FirstChild().Element();


    for( ;distTag; distTag=distTag->NextSiblingElement())
    {
      tagSorter->AddDistinguishingTag(this->getTagFromXMLElement(distTag));
    }

    // Sorting tags are loaded in reverse order
    if (hroot.FirstChild("SortingTags").Element() == NULL)
      return tagSorter;

    TiXmlNode* sortTag = hroot.FirstChild("SortingTags").ToNode()->LastChild();

    mitk::DICOMSortCriterion::Pointer sortingCrit;
    for( ;sortTag; sortTag=sortTag->PreviousSibling())
    {
      if (sortingCrit.IsNull())
        sortingCrit  = mitk::DICOMSortByTag::New(this->getTagFromXMLElement(sortTag->ToElement())).GetPointer();
      else
        sortingCrit = mitk::DICOMSortByTag::New(this->getTagFromXMLElement(sortTag->ToElement()), sortingCrit).GetPointer();
    }
    tagSorter->SetSortCriterion(sortingCrit.GetPointer());

    return tagSorter;
  */
  return mitk::ClassicDICOMSeriesReader::New().GetPointer();
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
  else
  {
    return NULL;
  }
}

mitk::DICOMITKSeriesGDCMReader::Pointer
mitk::DICOMReaderConfigurator
::ConfigureDICOMITKSeriesGDCMReader(DICOMITKSeriesGDCMReader::Pointer reader, TiXmlElement* element)
{
  assert(element);

  // "fixTiltByShearing" flag
  bool fixTiltByShearing(false);
  const char* fixTiltByShearingC = element->Attribute("fixTiltByShearing");
  if (fixTiltByShearingC)
  {
    std::string fixTiltByShearingS(fixTiltByShearingC);
    fixTiltByShearing = boolStringTrue(fixTiltByShearingS);
  }

  reader->SetFixTiltByShearing( fixTiltByShearing );


  // "distinguishing tags"
  mitk::DICOMTagBasedSorter::Pointer tagSorter = mitk::DICOMTagBasedSorter::New();
  TiXmlElement* dElement = element->FirstChildElement("DistinguishingTags");
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

  // "sorting"

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

  // convert string to int (assuming string is in hex format with leading "0x" like "0x0020")
  unsigned int group(0);
  unsigned int element(0);

  return DICOMTag(group, element);
}
