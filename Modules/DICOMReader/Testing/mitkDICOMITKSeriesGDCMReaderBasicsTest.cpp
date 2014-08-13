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
#include "mitkDICOMFileReaderTestHelper.h"
#include "mitkDICOMFilenameSorter.h"
#include "mitkDICOMTagBasedSorter.h"
#include "mitkDICOMSortByTag.h"

#include "mitkTestingMacros.h"

using mitk::DICOMTag;

int mitkDICOMITKSeriesGDCMReaderBasicsTest(int argc, char* argv[])
{
  MITK_TEST_BEGIN("mitkDICOMITKSeriesGDCMReaderBasicsTest");

  mitk::DICOMITKSeriesGDCMReader::Pointer gdcmReader = mitk::DICOMITKSeriesGDCMReader::New();
  MITK_TEST_CONDITION_REQUIRED(gdcmReader.IsNotNull(), "DICOMITKSeriesGDCMReader can be instantiated.");

  mitk::DICOMFileReaderTestHelper::SetTestInputFilenames( argc,argv );

  // check the Set/GetInput function
  mitk::DICOMFileReaderTestHelper::TestInputFilenames( gdcmReader );

  // check that output is a good reproduction of input (no duplicates, no new elements)
  mitk::DICOMFileReaderTestHelper::TestOutputsContainInputs( gdcmReader );


  // repeat test with filename based sorter in-between
  mitk::DICOMFilenameSorter::Pointer filenameSorter = mitk::DICOMFilenameSorter::New();
  gdcmReader->AddSortingElement( filenameSorter );
  mitk::DICOMFileReaderTestHelper::TestOutputsContainInputs( gdcmReader );

  // repeat test with some more realistic sorting
  gdcmReader = mitk::DICOMITKSeriesGDCMReader::New(); // this also tests destruction
  mitk::DICOMTagBasedSorter::Pointer tagSorter = mitk::DICOMTagBasedSorter::New();

  // all the things that split by tag in DicomSeriesReader
  tagSorter->AddDistinguishingTag( DICOMTag(0x0028, 0x0010) ); // Number of Rows
  tagSorter->AddDistinguishingTag( DICOMTag(0x0028, 0x0011) ); // Number of Columns
  tagSorter->AddDistinguishingTag( DICOMTag(0x0028, 0x0030) ); // Pixel Spacing
  tagSorter->AddDistinguishingTag( DICOMTag(0x0018, 0x1164) ); // Imager Pixel Spacing
  tagSorter->AddDistinguishingTag( DICOMTag(0x0020, 0x0037), new mitk::DICOMTagBasedSorter::CutDecimalPlaces(5) ); // Image Orientation (Patient)
  tagSorter->AddDistinguishingTag( DICOMTag(0x0020, 0x000e) ); // Series Instance UID
  tagSorter->AddDistinguishingTag( DICOMTag(0x0018, 0x0050) ); // Slice Thickness
  tagSorter->AddDistinguishingTag( DICOMTag(0x0028, 0x0008) ); // Number of Frames
  tagSorter->AddDistinguishingTag( DICOMTag(0x0020, 0x0052) ); // Frame of Reference UID

  // a sorter...
  mitk::DICOMSortCriterion::ConstPointer sorting =
    mitk::DICOMSortByTag::New( DICOMTag(0x0020, 0x0013), // instance number
      mitk::DICOMSortByTag::New( DICOMTag(0x0020, 0x0012), // aqcuisition number
        mitk::DICOMSortByTag::New( DICOMTag(0x0008, 0x0032), // aqcuisition time
          mitk::DICOMSortByTag::New( DICOMTag(0x0018, 0x1060), // trigger time
            mitk::DICOMSortByTag::New( DICOMTag(0x0008, 0x0018) // SOP instance UID (last resort, not really meaningful but decides clearly)
            ).GetPointer()
          ).GetPointer()
        ).GetPointer()
      ).GetPointer()
    ).GetPointer();
  tagSorter->SetSortCriterion( sorting );

  gdcmReader->AddSortingElement( tagSorter );
  mitk::DICOMFileReaderTestHelper::TestOutputsContainInputs( gdcmReader );

  gdcmReader->PrintOutputs(std::cout, true);

  // really load images
  mitk::DICOMFileReaderTestHelper::TestMitkImagesAreLoaded( gdcmReader );

  MITK_TEST_END();
}
