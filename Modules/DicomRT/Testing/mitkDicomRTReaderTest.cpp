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
#include "mitkDicomRTReader.h"
#include <iostream>


int mitkDicomRTReaderTest(int  argc , char* argv[])
{
  // always start with this!
  MITK_TEST_BEGIN("DicomRTReader")

  mitk::DicomRTReader::Pointer _DicomRTReader = mitk::DicomRTReader::New();

  int helloWorld = _DicomRTReader->ReadContourDataSequence0_1(argv[1]);

  MITK_TEST_CONDITION_REQUIRED( helloWorld == 1, "Test" );

  // always end with this!
  MITK_TEST_END()
}
