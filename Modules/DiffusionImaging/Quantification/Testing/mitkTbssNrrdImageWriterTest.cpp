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
#include <mitkNrrdTbssImageWriter.h>
#include "mitkCoreObjectFactory.h"


/**Documentation
 *  test for the class "mitkNrrdTbssImageWriter".
 */
int mitkTbssNrrdImageWriterTest(int, char* argv[])
{
  MITK_TEST_BEGIN("TbssNrrdImageWriterTest");

  mitk::NrrdTbssImageReader::Pointer tbssNrrdReader = mitk::NrrdTbssImageReader::New();

  MITK_TEST_CONDITION_REQUIRED(tbssNrrdReader.GetPointer(), "Testing initialization of test object!");

  tbssNrrdReader->SetFileName(argv[1]);
  MITK_TEST_CONDITION_REQUIRED( tbssNrrdReader->CanReadFile(argv[1], "", ""), "Testing CanReadFile() method with valid input file name!");
  tbssNrrdReader->Update();

  mitk::TbssImage* tbssImg = tbssNrrdReader->GetOutput();

  MITK_TEST_CONDITION_REQUIRED(tbssImg != nullptr, "Testing that tbssImg is not null");

  mitk::NrrdTbssImageWriter::Pointer tbssNrrdWriter = mitk::NrrdTbssImageWriter::New();


  try{
    tbssNrrdWriter->SetFileName(argv[2]);
    tbssNrrdWriter->SetInput(tbssImg);
    tbssNrrdWriter->Update();
  }
  catch(...) {
    std::cout << "Writing failed" << std::endl;
    return EXIT_FAILURE;
  }

  mitk::NrrdTbssImageReader::Pointer tbssNrrdReader2 = mitk::NrrdTbssImageReader::New();
  tbssNrrdReader2->SetFileName(argv[1]);
  MITK_TEST_CONDITION_REQUIRED( tbssNrrdReader2->CanReadFile(argv[1], "", ""), "Testing CanReadFile() method with previously saved file!");

  tbssNrrdReader2->Update();
  mitk::TbssImage* tbssImg2 = tbssNrrdReader2->GetOutput();


  std::vector< std::pair<std::string, int> > groups = tbssImg->GetGroupInfo();
  std::vector< std::pair<std::string, int> > groups2 = tbssImg2->GetGroupInfo();

  MITK_TEST_CONDITION(groups == groups2, "Check that group information is identical");


  mitk::TbssImage::SizeType size = tbssImg->GetLargestPossibleRegion().GetSize();
  mitk::TbssImage::SizeType size2 = tbssImg2->GetLargestPossibleRegion().GetSize();

  MITK_TEST_CONDITION(size == size2, "Check that sizes are equal");


  MITK_TEST_END();
}
