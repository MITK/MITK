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
