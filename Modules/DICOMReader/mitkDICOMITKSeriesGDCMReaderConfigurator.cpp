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

#include "mitkDICOMITKSeriesGDCMReaderConfigurator.h"

#include "mitkDICOMSortByTag.h"
#include "mitkDICOMSortCriterion.h"

// TinyXml
#include "tinyxml.h"

mitk::DICOMITKSeriesGDCMReaderConfigurator::DICOMITKSeriesGDCMReaderConfigurator()
{
}


mitk::DICOMTagBasedSorter::Pointer mitk::DICOMITKSeriesGDCMReaderConfigurator::LoadDicomReaderConfigFile(const std::string &filename)
{
  mitk::DICOMTagBasedSorter::Pointer tagSorter = mitk::DICOMTagBasedSorter::New();


  TiXmlDocument doc (filename);
  TiXmlHandle hroot (0);
  if (doc.LoadFile() && doc.FirstChildElement())
  {
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
  }
  return tagSorter;
}

//void mitk::DICOMITKSeriesGDCMReaderConfigurator::SaveConfigurationToFile(const std::string &filename)
//{

//}

//void mitk::DICOMITKSeriesGDCMReaderConfigurator::SaveConfigurationToFile(const std::string &filename, DICOMFileReader::Pointer)
//{
//}

//void mitk::DICOMITKSeriesGDCMReaderConfigurator::SaveDicomReaderConfiguration(const strd::string &filename)
//{
//}

mitk::DICOMITKSeriesGDCMReader::Pointer mitk::DICOMITKSeriesGDCMReaderConfigurator::CreateDicomReaderInstance()
{
  return mitk::DICOMITKSeriesGDCMReader::New();
}

mitk::DICOMITKSeriesGDCMReader::Pointer mitk::DICOMITKSeriesGDCMReaderConfigurator::CreateDicomReaderInstance(const std::string &filename)
{
  mitk::DICOMITKSeriesGDCMReader::Pointer seriesReader = mitk::DICOMITKSeriesGDCMReader::New();
  mitk::DICOMTagBasedSorter::Pointer sorter = this->LoadDicomReaderConfigFile(filename);
  seriesReader->AddSortingElement(sorter);

  return seriesReader;
}

mitk::DICOMTag mitk::DICOMITKSeriesGDCMReaderConfigurator::getTagFromXMLElement(TiXmlElement* xmlElement)
{
  int group;
  int element;
  xmlElement->QueryIntAttribute("group", &group);
  xmlElement->QueryIntAttribute("element", &element);
  return DICOMTag(group, element);
}
