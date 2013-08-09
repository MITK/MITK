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


int mitkDicomRTReaderTest(int a/* argc */, char* b/*argv*/[])
{
  // always start with this!
  MITK_TEST_BEGIN("DicomRTReader")

  mitk::DicomRTReader::Pointer _DicomRTReader = mitk::DicomRTReader::New();

  int helloWorld = _DicomRTReader->ReadContourData(b[2]);

  MITK_TEST_CONDITION_REQUIRED( 2 == 2, "Test" );

  // always end with this!
  MITK_TEST_END()
}
