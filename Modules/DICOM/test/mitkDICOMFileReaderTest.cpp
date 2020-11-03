/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkDICOMNullFileReader.h"
#include "mitkDICOMFileReaderTestHelper.h"

#include "mitkTestingMacros.h"

int mitkDICOMFileReaderTest(int argc, char* argv[])
{
  MITK_TEST_BEGIN("mitkDICOMFileReaderTest");

  mitk::DICOMNullFileReader::Pointer simpleReader = mitk::DICOMNullFileReader::New();
  MITK_TEST_CONDITION_REQUIRED(simpleReader.IsNotNull(), "DICOMNullFileReader can be instantiated.");

  mitk::DICOMFileReaderTestHelper::SetTestInputFilenames( argc,argv );

  mitk::DICOMFileReaderTestHelper::TestInputFilenames( simpleReader );
  mitk::DICOMFileReaderTestHelper::TestOutputsContainInputs( simpleReader );

  MITK_TEST_END();
}
