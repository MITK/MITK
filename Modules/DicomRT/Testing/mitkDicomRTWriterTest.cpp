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
#include "mitkDicomRTWriter.h"
#include <iostream>

/**
 *  blaaa
 */
int mitkDicomRTWriterTest(int /* argc */, char* /*argv*/[])
{
  // always start with this!
  MITK_TEST_BEGIN("DicomRTWriter")

  mitk::DicomRTWriter::Pointer _DicomRTWriter = mitk::DicomRTWriter::New();

  std::string helloWorld = _DicomRTWriter->SayHelloWorld();

  MITK_TEST_CONDITION_REQUIRED( helloWorld == "Hello World", "SayHelloWorld worked correctly" );

  // always end with this!
  MITK_TEST_END()
}

