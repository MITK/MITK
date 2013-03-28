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

/**
 *  TODO
 */
int mitkFileReaderManagerTest(int argc , char* argv[])
{
  // always start with this!
  MITK_TEST_BEGIN("FileReaderManager")
  //MITK_TEST_CONDITION_REQUIRED(argc == 2,"Testing invocation")

  // let's create an object of our class
  mitk::PointSetReader::Pointer myPointSetReader = mitk::PointSetReader::New();
  MITK_TEST_CONDITION_REQUIRED(myPointSetReader.IsNotNull(),"Testing instantiation")

  // always end with this!
  MITK_TEST_END()
}
