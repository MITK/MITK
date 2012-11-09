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

  MITK_TEST_CONDITION_REQUIRED(tbssNrrdReader.GetPointer(), "Testing initialization of test object!");

  RegisterDiffusionImagingObjectFactory();


  tbssNrrdReader->SetFileName(argv[1]);
  MITK_TEST_CONDITION_REQUIRED( tbssNrrdReader->CanReadFile(argv[1], "", ""), "Testing CanReadFile() method with valid input file name!");
  tbssNrrdReader->Update();

  mitk::TbssImage* tbssImg = tbssNrrdReader->GetOutput(0);

  MITK_TEST_CONDITION_REQUIRED(tbssImg != NULL, "Testing that tbssImg is not null");

  mitk::TbssImage::SizeType size = tbssImg->GetLargestPossibleRegion().GetSize();

  MITK_TEST_CONDITION_REQUIRED(size[0]==2 && size[1]==2 && size[2]==2, "Test size of tbss image");

  // Test groups
  std::vector< std::pair<std::string, int> > groups = tbssImg->GetGroupInfo();

  std::pair<std::string, int> group1 = groups.at(0);
  std::pair<std::string, int> group2 = groups.at(1);



  MITK_TEST_CONDITION_REQUIRED(group1.first.compare("group1") && group1.second==1, "Test group 1 info");

  MITK_TEST_CONDITION_REQUIRED(group2.first.compare("group2") && group2.second==1, "Test group 2 info");



  MITK_TEST_END();
}
