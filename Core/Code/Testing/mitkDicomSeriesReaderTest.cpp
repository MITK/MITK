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

#include "mitkTestingMacros.h"

#include <iostream>

#include "mitkDicomSeriesReader.h"
#include "mitkProperties.h"
#include "mitkImage.h"

static std::map<std::string, std::map<gdcm::Tag, std::string> > GetTagInformationFromFile(mitk::DicomSeriesReader::StringContainer files)
{
  gdcm::Scanner scanner;
  std::map<std::string, std::map<gdcm::Tag, std::string> > tagInformations;

  const gdcm::Tag tagSliceLocation(0x0020, 0x1041); // slice location
  scanner.AddTag( tagSliceLocation );

  const gdcm::Tag tagInstanceNumber(0x0020, 0x0013); // (image) instance number
  scanner.AddTag( tagInstanceNumber );

  const gdcm::Tag tagSOPInstanceNumber(0x0008, 0x0018); // SOP instance number
  scanner.AddTag( tagSOPInstanceNumber );

  //unsigned int slice(0);
  scanner.Scan(files);

//  return const_cast<gdcm::Scanner::MappingType&>(scanner.GetMappings());
  gdcm::Scanner::MappingType& tagValueMappings = const_cast<gdcm::Scanner::MappingType&>(scanner.GetMappings());

  for(std::vector<std::string>::const_iterator fIter = files.begin();
      fIter != files.end();
      ++fIter)
  {
    std::map<gdcm::Tag, std::string> tags;
    tags.insert(std::pair<gdcm::Tag, std::string> (tagSliceLocation, tagValueMappings[fIter->c_str()][tagSliceLocation]));
    tags.insert(std::pair<gdcm::Tag, std::string> (tagInstanceNumber, tagValueMappings[fIter->c_str()][tagInstanceNumber]));
    tags.insert(std::pair<gdcm::Tag, std::string> (tagSOPInstanceNumber, tagValueMappings[fIter->c_str()][tagSOPInstanceNumber]));

    tagInformations.insert(std::pair<std::string, std::map<gdcm::Tag, std::string> > (fIter->c_str(), tags));
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
  mitk::DicomSeriesReader::FileNamesGrouping seriesInFiles = mitk::DicomSeriesReader::GetSeries( dir, true );
  std::list<mitk::Image::Pointer> images;
  std::map<mitk::Image::Pointer, mitk::DicomSeriesReader::StringContainer> fileMap;

  // TODO sort series UIDs, implementation of map iterator might differ on different platforms (or verify this is a standard topic??)
  for (mitk::DicomSeriesReader::FileNamesGrouping::const_iterator seriesIter = seriesInFiles.begin();
       seriesIter != seriesInFiles.end();
       ++seriesIter)
  {
    mitk::DicomSeriesReader::StringContainer files = seriesIter->second.GetFilenames();

    mitk::DataNode::Pointer node = mitk::DicomSeriesReader::LoadDicomSeries( files );
    MITK_TEST_CONDITION_REQUIRED(node.IsNotNull(),"Testing node")

    if (node.IsNotNull())
    {
      mitk::Image::Pointer image = dynamic_cast<mitk::Image*>( node->GetData() );

      images.push_back( image );
      fileMap.insert( std::pair<mitk::Image::Pointer, mitk::DicomSeriesReader::StringContainer>(image,files));
    }
  }

  //Test if DICOM tags have been added correctly to the mitk::image properties

  const gdcm::Tag tagSliceLocation(0x0020, 0x1041); // slice location
  const gdcm::Tag tagInstanceNumber(0x0020, 0x0013); // (image) instance number
  const gdcm::Tag tagSOPInstanceNumber(0x0008, 0x0018); // SOP instance number

  for ( std::list<mitk::Image::Pointer>::const_iterator imageIter = images.begin();
        imageIter != images.end();
        ++imageIter )
  {
    const mitk::Image::Pointer image = *imageIter;

    //Get tag information for all dicom files of this image
    std::map<std::string, std::map<gdcm::Tag, std::string> > tagInformations = GetTagInformationFromFile((*fileMap.find(image)).second);

    mitk::StringLookupTableProperty* sliceLocation = dynamic_cast<mitk::StringLookupTableProperty*>(image->GetProperty("dicom.image.0020.1041").GetPointer());
    mitk::StringLookupTableProperty* instanceNumber = dynamic_cast<mitk::StringLookupTableProperty*>(image->GetProperty("dicom.image.0020.0013").GetPointer());
    mitk::StringLookupTableProperty* SOPInstnaceNumber = dynamic_cast<mitk::StringLookupTableProperty*>(image->GetProperty("dicom.image.0008.0018").GetPointer());

    mitk::StringLookupTableProperty* files = dynamic_cast<mitk::StringLookupTableProperty*>(image->GetProperty("files").GetPointer());

    MITK_TEST_CONDITION(sliceLocation != NULL, "Test if tag for slice location has been set to mitk image");
    if(sliceLocation != NULL)
    {
      for(int i = 0; i < (int)sliceLocation->GetValue().GetLookupTable().size(); i++)
      {
        if(i < (int)files->GetValue().GetLookupTable().size())
        {
          MITK_INFO << "Table value: " << sliceLocation->GetValue().GetTableValue(i) << " and File value: " << tagInformations[files->GetValue().GetTableValue(i).c_str()][tagSliceLocation] << std::endl;
          MITK_INFO << "Filename: " << files->GetValue().GetTableValue(i).c_str() << std::endl;
          MITK_TEST_CONDITION(sliceLocation->GetValue().GetTableValue(i) == tagInformations[files->GetValue().GetTableValue(i).c_str()][tagSliceLocation], "Test if value for slice location is correct");
        }
      }
    }

    MITK_TEST_CONDITION(instanceNumber != NULL, "Test if tag for image instance number has been set to mitk image");
    if(instanceNumber != NULL)
    {
      for(int i = 0; i < (int)instanceNumber->GetValue().GetLookupTable().size(); i++)
      {
        if(i < (int)files->GetValue().GetLookupTable().size())
        {
          MITK_INFO << "Table value: " << instanceNumber->GetValue().GetTableValue(i) << " and File value: " << tagInformations[files->GetValue().GetTableValue(i).c_str()][tagInstanceNumber] << std::endl;
          MITK_INFO << "Filename: " << files->GetValue().GetTableValue(i).c_str() << std::endl;
          MITK_TEST_CONDITION(instanceNumber->GetValue().GetTableValue(i) == tagInformations[files->GetValue().GetTableValue(i).c_str()][tagInstanceNumber], "Test if value for instance number is correct");
        }
      }
    }

    MITK_TEST_CONDITION(SOPInstnaceNumber != NULL, "Test if tag for SOP instance number has been set to mitk image");
    if(SOPInstnaceNumber != NULL)
    {
      for(int i = 0; i < (int)SOPInstnaceNumber->GetValue().GetLookupTable().size(); i++)
      {
        if(i < (int)files->GetValue().GetLookupTable().size())
        {
          MITK_INFO << "Table value: " << instanceNumber->GetValue().GetTableValue(i) << " and File value: " << tagInformations[files->GetValue().GetTableValue(i).c_str()][tagSOPInstanceNumber] << std::endl;
          MITK_INFO << "Filename: " << files->GetValue().GetTableValue(i).c_str() << std::endl;
          MITK_TEST_CONDITION(SOPInstnaceNumber->GetValue().GetTableValue(i) == tagInformations[files->GetValue().GetTableValue(i).c_str()][tagSOPInstanceNumber], "Test if value for SOP instance number is correct");
        }
      }
    }
  }

  MITK_TEST_END()
}
