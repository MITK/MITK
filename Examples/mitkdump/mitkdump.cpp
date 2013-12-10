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

#include "mitkDICOMITKSeriesGDCMReader.h"
#include "mitkDICOMTagBasedSorter.h"
#include "mitkDICOMSortByTag.h"

int main(int argc, char* argv[])
{
  mitk::StringList inputFiles; // TODO
  for (int a = 1; a < argc; ++a)
  {
    inputFiles.push_back( std::string(argv[a]) );
  }

  // ----------------- Configure reader -------------------

  mitk::DICOMITKSeriesGDCMReader::Pointer gdcmReader =
    mitk::DICOMITKSeriesGDCMReader::New();

  mitk::DICOMTagBasedSorter::Pointer tagSorter =
    mitk::DICOMTagBasedSorter::New();

  // all the things that split by tag in DicomSeriesReader
  tagSorter->AddDistinguishingTag( std::make_pair(0x0028, 0x0010) ); // Number of Rows
  tagSorter->AddDistinguishingTag( std::make_pair(0x0028, 0x0011) ); // Number of Columns
  tagSorter->AddDistinguishingTag( std::make_pair(0x0028, 0x0030) ); // Pixel Spacing
  tagSorter->AddDistinguishingTag( std::make_pair(0x0018, 0x1164) ); // Imager Pixel Spacing
  tagSorter->AddDistinguishingTag( std::make_pair(0x0020, 0x0037) ); // Image Orientation (Patient) // TODO add tolerance parameter (l. 1572 of original code)
  tagSorter->AddDistinguishingTag( std::make_pair(0x0020, 0x000e) ); // Series Instance UID
  tagSorter->AddDistinguishingTag( std::make_pair(0x0018, 0x0050) ); // Slice Thickness
  tagSorter->AddDistinguishingTag( std::make_pair(0x0028, 0x0008) ); // Number of Frames

  // a sorter...
  // TODO ugly syntax, improve..
  mitk::DICOMSortCriterion::ConstPointer sorting =
    mitk::DICOMSortByTag::New( std::make_pair(0x0020, 0x0013), // instance number
      mitk::DICOMSortByTag::New( std::make_pair(0x0020, 0x0012), // aqcuisition number
        mitk::DICOMSortByTag::New( std::make_pair(0x0008, 0x0032), // aqcuisition time
          mitk::DICOMSortByTag::New( std::make_pair(0x0018, 0x1060), // trigger time
            mitk::DICOMSortByTag::New( std::make_pair(0x0008, 0x0018) // SOP instance UID (last resort, not really meaningful but decides clearly)
            ).GetPointer()
          ).GetPointer()
        ).GetPointer()
      ).GetPointer()
    ).GetPointer();
  tagSorter->SetSortCriterion( sorting );

  gdcmReader->AddSortingElement( tagSorter );

  // ----------------- Load -------------------

  gdcmReader->SetInputFiles( inputFiles );

  MITK_INFO << "Analyzing " << inputFiles.size() << " file ...";
  gdcmReader->AnalyzeInputFiles();
  MITK_INFO << "Loading " << inputFiles.size() << " file ...";
  gdcmReader->LoadImages();

  unsigned int numberOfOutputs = gdcmReader->GetNumberOfOutputs();
  for (unsigned int o = 0; o < numberOfOutputs; ++o)
  {
    const mitk::DICOMImageBlockDescriptor block = gdcmReader->GetOutput(o);

    const mitk::DICOMImageFrameList& outputFiles = block.GetImageFrameList();
    mitk::Image::Pointer mitkImage = block.GetMitkImage();

    MITK_INFO << "-------------------------------------------";
    MITK_INFO << "Output " << o << " at " << (void*) mitkImage.GetPointer();
    MITK_INFO << "  Number of files: " << outputFiles.size();
    MITK_INFO << "  Dimensions: " << mitkImage->GetDimension(0) << " " << mitkImage->GetDimension(1) << " " << mitkImage->GetDimension(2);
  }
}
