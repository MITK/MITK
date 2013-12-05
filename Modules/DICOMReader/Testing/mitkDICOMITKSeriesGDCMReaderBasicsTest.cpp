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

#include "mitkTestingMacros.h"

int mitkDICOMITKSeriesGDCMReaderBasicsTest(int argc, char* argv[])
{
  MITK_TEST_BEGIN("mitkDICOMITKSeriesGDCMReaderBasicsTest");

  mitk::DICOMITKSeriesGDCMReader::Pointer gdcmReader = mitk::DICOMITKSeriesGDCMReader::New();
  MITK_TEST_CONDITION_REQUIRED(gdcmReader.IsNotNull(), "DICOMITKSeriesGDCMReaderBasics can be instantiated.");

  mitk::DICOMFileReaderTestHelper::SetTestInputFilenames( argc,argv );

  mitk::DICOMFileReaderTestHelper::TestInputFilenames( gdcmReader );
  mitk::DICOMFileReaderTestHelper::TestOutputsContainInputs( gdcmReader );

  MITK_TEST_END();
}
