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
int mitkTbssNrrdImageReaderTest(int argc , char* argv[])
{
  MITK_TEST_BEGIN("TbssNrrdImageReaderTest");




  mitk::NrrdTbssImageReader::Pointer tbssNrrdReader = mitk::NrrdTbssImageReader::New();
  // testing correct initialization 
  //MITK_TEST_CONDITION_REQUIRED(tbssNrrdReader.GetPointer(), "Testing initialization of test object!");



  RegisterDiffusionImagingObjectFactory();


  std::cout << argc << std::endl;
  std::cout << argv[1] << std::endl;

  tbssNrrdReader->SetFileName(argv[1]);
  MITK_TEST_CONDITION_REQUIRED( tbssNrrdReader->CanReadFile(argv[1], "", ""), "Testing CanReadFile() method with valid input file name!");
  tbssNrrdReader->Update();



 // MITK_TEST_CONDITION_REQUIRED(tbssImg != NULL, "Testing that tbssImg is not null");



  MITK_TEST_END();  
}
