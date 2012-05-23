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

#include <mitkTestingMacros.h>
#include <mitkNrrdTbssImageReader.h>
#include "mitkDiffusionImagingObjectFactory.h"
#include "mitkCoreObjectFactory.h"


/**Documentation
 *  test for the class "mitkNrrdTbssImageReader".
 */
int mitkTbssNrrdImageReaderTest(int /*argc*/ , char* argv[])
{
  MITK_TEST_BEGIN("TbssNrrdImageReaderTest");


  std::cout << "1" << std::endl;

  mitk::NrrdTbssImageReader::Pointer tbssNrrdReader = mitk::NrrdTbssImageReader::New();
  // testing correct initialization 
  MITK_TEST_CONDITION_REQUIRED(tbssNrrdReader.GetPointer(), "Testing initialization of test object!");


  try{
    RegisterDiffusionImagingObjectFactory();

  }

  MITK_TEST_END();  
}
