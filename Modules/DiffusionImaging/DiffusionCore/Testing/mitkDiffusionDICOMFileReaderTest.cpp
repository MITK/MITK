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

#include "mitkDiffusionDICOMFileReader.h"
#include "mitkDiffusionDICOMFileReaderTestHelper.h"
#include "mitkDICOMTagBasedSorter.h"
#include "mitkDICOMSortByTag.h"

#include <mitkIOUtil.h>

#include "mitkTestingMacros.h"

using mitk::DICOMTag;

int mitkDiffusionDICOMFileReaderTest(int argc, char* argv[])
{
  MITK_TEST_BEGIN("mitkDiffusionDICOMFileReaderTest");

  mitk::DiffusionDICOMFileReader::Pointer gdcmReader = mitk::DiffusionDICOMFileReader::New();
  MITK_TEST_CONDITION_REQUIRED(gdcmReader.IsNotNull(), "DICOMITKSeriesGDCMReader can be instantiated.");

  std::string output_filename = "/tmp/dicom_out.dwi";
  if( argc > 3)
  {
    mitk::DICOMFileReaderTestHelper::SetTestInputFilenames( argc-1,argv );
    output_filename = std::string( argv[argc-1] );
  }
  else
  {
    mitk::DICOMFileReaderTestHelper::SetTestInputFilenames( argc,argv );
  }

  // check the Set/GetInput function
  mitk::DICOMFileReaderTestHelper::TestInputFilenames( gdcmReader );
  MITK_INFO << "Test input filenanems";

  // check that output is a good reproduction of input (no duplicates, no new elements)
  mitk::DICOMFileReaderTestHelper::TestOutputsContainInputs( gdcmReader );
  MITK_INFO << "Test output";

  // repeat test with some more realistic sorting
  gdcmReader = mitk::DiffusionDICOMFileReader::New(); // this also tests destruction
  mitk::DICOMTagBasedSorter::Pointer tagSorter = mitk::DICOMTagBasedSorter::New();

  // Use tags as in Qmitk
  // all the things that split by tag in DicomSeriesReader
  tagSorter->AddDistinguishingTag( DICOMTag(0x0028, 0x0010) ); // Number of Rows
  tagSorter->AddDistinguishingTag( DICOMTag(0x0028, 0x0011) ); // Number of Columns
  tagSorter->AddDistinguishingTag( DICOMTag(0x0028, 0x0030) ); // Pixel Spacing
  tagSorter->AddDistinguishingTag( DICOMTag(0x0018, 0x1164) ); // Imager Pixel Spacing
  tagSorter->AddDistinguishingTag( DICOMTag(0x0020, 0x0037) ); // Image Orientation (Patient) // TODO add tolerance parameter (l. 1572 of original code)
  // TODO handle as real vectors! cluster with configurable errors!
  //tagSorter->AddDistinguishingTag( DICOMTag(0x0020, 0x000e) ); // Series Instance UID
  //tagSorter->AddDistinguishingTag( DICOMTag(0x0020, 0x0010) );
  tagSorter->AddDistinguishingTag( DICOMTag(0x0018, 0x0050) ); // Slice Thickness
  tagSorter->AddDistinguishingTag( DICOMTag(0x0028, 0x0008) ); // Number of Frames
  tagSorter->AddDistinguishingTag( DICOMTag(0x0020, 0x0052) ); // Frame of Reference UID

 // gdcmReader->AddSortingElement( tagSorter );
  //mitk::DICOMFileReaderTestHelper::TestOutputsContainInputs( gdcmReader );

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

  MITK_INFO << "Created sort";

  gdcmReader->AddSortingElement( tagSorter );
  mitk::DICOMFileReaderTestHelper::TestOutputsContainInputs( gdcmReader );

  MITK_INFO << "Created sort";

  //gdcmReader->PrintOutputs(std::cout, true);

  // really load images
  //mitk::DICOMFileReaderTestHelper::TestMitkImagesAreLoaded( gdcmReader );
  gdcmReader->LoadImages();

  mitk::Image::Pointer loaded_image = gdcmReader->GetOutput(0).GetMitkImage();

  mitk::DiffusionImage<short>::Pointer d_img = static_cast<mitk::DiffusionImage<short>*>( loaded_image.GetPointer() );

  try
  {
    mitk::IOUtil::Save(d_img, output_filename.c_str());
  }
  catch( const itk::ExceptionObject& e)
  {
    MITK_TEST_FAILED_MSG( << "Writer failed : " << e.what() );
  }

  MITK_TEST_END();
}
