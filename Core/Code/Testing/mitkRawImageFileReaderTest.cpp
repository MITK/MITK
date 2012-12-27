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


#include "mitkRawImageFileReader.h"

int mitkRawImageFileReaderTest(int /*argc*/, char* /*argv*/[])
{
  // instantiation
  mitk::RawImageFileReader::Pointer reader = mitk::RawImageFileReader::New();

  // freeing
  reader = NULL;

  std::cout<<"[TEST DONE]"<<std::endl;
  return EXIT_SUCCESS;
}
