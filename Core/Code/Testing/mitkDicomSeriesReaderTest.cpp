/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2008-02-25 17:27:17 +0100 (Mo, 25 Feb 2008) $
Version:   $Revision: 7837 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkTestingMacros.h"

#include <iostream>

#include "mitkDicomSeriesReader.h"
#include "mitkProperties.h"


static std::set<std::string> GetTagInformationFromFile(mitk::DicomSeriesReader::StringContainer files)
{
  gdcm::Scanner scanner;
  std::set<std::string> tagInformations;

  const gdcm::Tag tagSliceLocation(0x0020, 0x1041); // slice location
  scanner.AddTag( tagSliceLocation );

  const gdcm::Tag tagInstanceNumber(0x0020, 0x0013); // (image) instance number
  scanner.AddTag( tagInstanceNumber );

  const gdcm::Tag tagSOPInstanceNumber(0x0008, 0x0018); // SOP instance number
  scanner.AddTag( tagSOPInstanceNumber );

  unsigned int slice(0);
  for(std::vector<std::string>::const_iterator fIter = files.begin();
      fIter != files.end();
      ++fIter, ++slice)
  {
    scanner.Scan(files); // make available image position for each file

    gdcm::Scanner::MappingType& tagValueMappings = const_cast<gdcm::Scanner::MappingType&>(scanner.GetMappings());
    tagInformations.insert(tagValueMappings[fIter->c_str()][tagSliceLocation]);
    tagInformations.insert(tagValueMappings[fIter->c_str()][tagInstanceNumber]);
    tagInformations.insert(tagValueMappings[fIter->c_str()][tagSOPInstanceNumber]);
  }

  return tagInformations;
}

int mitkDicomSeriesReaderTest(int argc, char* argv[])
{
  // always start with this!
  MITK_TEST_BEGIN("DicomSeriesReader")
  if(argc < 1)
  {
    MITK_ERROR << "No directory given!";
    return -1;
  }

  char* dir;
  dir = argv[1];

  //check if DICOMTags have been set as property for mitk::Image
  mitk::DicomSeriesReader::UidFileNamesMap seriesInFiles = mitk::DicomSeriesReader::GetSeries( dir );
  std::list<mitk::Image::Pointer> images;
  std::map<mitk::Image::Pointer, mitk::DicomSeriesReader::StringContainer> fileMap;
  std::set<std::string> tagInformations;
  // TODO sort series UIDs, implementation of map iterator might differ on different platforms (or verify this is a standard topic??)
  for (mitk::DicomSeriesReader::UidFileNamesMap::const_iterator seriesIter = seriesInFiles.begin();
       seriesIter != seriesInFiles.end();
       ++seriesIter)
  {
    mitk::DicomSeriesReader::StringContainer files = seriesIter->second;

    mitk::DataNode::Pointer node = mitk::DicomSeriesReader::LoadDicomSeries( files );
    MITK_TEST_CONDITION_REQUIRED(node.IsNotNull(),"Testing node")

    if (node.IsNotNull())
    {
      mitk::Image::Pointer image = dynamic_cast<mitk::Image*>( node->GetData() );

      images.push_back( image );
      fileMap.insert( std::pair<mitk::Image::Pointer, mitk::DicomSeriesReader::StringContainer>(image,files));
    }
  }
  for ( std::list<mitk::Image::Pointer>::const_iterator imageIter = images.begin();
        imageIter != images.end();
        ++imageIter )
  {
    const mitk::Image::Pointer image = *imageIter;
    tagInformations = GetTagInformationFromFile((*fileMap.find(image)).second);
    mitk::StringLookupTableProperty* sliceLocation = dynamic_cast<mitk::StringLookupTableProperty*>(image->GetProperty("dicom.image.0020.1041").GetPointer());
    MITK_TEST_CONDITION(sliceLocation != NULL, "Test if tag for slice location has been set to mitk image");
    if(sliceLocation != NULL)
    {
      for(int i = 0; i < sliceLocation->GetValue().GetLookupTable().size(); i++)
      {
        MITK_TEST_CONDITION(tagInformations.find(sliceLocation->GetValue().GetTableValue(i)) != tagInformations.end(), "Test if value for slice location is in list");
      }
    }
    mitk::StringLookupTableProperty* instanceNumber = dynamic_cast<mitk::StringLookupTableProperty*>(image->GetProperty("dicom.image.0020.0013").GetPointer());
    MITK_TEST_CONDITION(instanceNumber != NULL, "Test if tag for image instance number has been set to mitk image");
    if(instanceNumber != NULL)
    {
      for(int i = 0; i < instanceNumber->GetValue().GetLookupTable().size(); i++)
      {
        MITK_TEST_CONDITION(tagInformations.find(instanceNumber->GetValue().GetTableValue(i)) != tagInformations.end(), "Test if value for image instance number is in list");
      }
    }
    mitk::StringLookupTableProperty* SOPInstnaceNumber = dynamic_cast<mitk::StringLookupTableProperty*>(image->GetProperty("dicom.image.0008.0018").GetPointer());
    MITK_TEST_CONDITION(SOPInstnaceNumber != NULL, "Test if tag for SOP instance number has been set to mitk image");
    if(SOPInstnaceNumber != NULL)
    {
      for(int i = 0; i < SOPInstnaceNumber->GetValue().GetLookupTable().size(); i++)
      {
        MITK_TEST_CONDITION(tagInformations.find(SOPInstnaceNumber->GetValue().GetTableValue(i)) != tagInformations.end(), "Test if value for SOP instance number is in list");
      }
    }
  }

  MITK_TEST_END()
}
