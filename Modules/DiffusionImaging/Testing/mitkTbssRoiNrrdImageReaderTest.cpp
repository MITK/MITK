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

#include <mitkNrrdTbssRoiImageReader.h>
#include "mitkDiffusionImagingObjectFactory.h"
#include "mitkCoreObjectFactory.h"


/**Documentation
 *  test for the class "mitkNrrdTbssImageReader".
 */
int mitkTbssRoiNrrdImageReaderTest(int argc , char* argv[])
{
  MITK_TEST_BEGIN("NrrdTbssRoiImageReaderTest");


  mitk::NrrdTbssRoiImageReader::Pointer roiReader = mitk::NrrdTbssRoiImageReader::New();

  MITK_TEST_CONDITION_REQUIRED(roiReader.GetPointer(), "Testing initialization of test object!");

  RegisterDiffusionImagingObjectFactory();


  roiReader->SetFileName(argv[1]);
  MITK_TEST_CONDITION_REQUIRED( roiReader->CanReadFile(argv[1], "", ""), "Testing CanReadFile() method with valid input file name!");
  roiReader->Update();

  mitk::TbssRoiImage* roiImg = roiReader->GetOutput(0);

  MITK_TEST_CONDITION_REQUIRED(roiImg != NULL, "Testing that roiImg is not null");

  mitk::TbssRoiImage::SizeType size = roiImg->GetLargestPossibleRegion().GetSize();

  MITK_TEST_CONDITION_REQUIRED(size[0]==182 && size[1]==218 && size[2]==182, "Test size of ROI image");


  // Test indices of the roi
  std::vector< itk::Index<3> > indices = roiImg->GetRoi();

  MITK_TEST_CONDITION(indices.size() == 24, "Test the number of indices belonging to the Roi");

  itk::Index<3> ix = indices.at(0);

  MITK_TEST_CONDITION(ix[0]==90 && ix[1]==132 && ix[2]==74, "Test whether first index is correct");

  ix = indices.at(23);

  MITK_TEST_CONDITION(ix[0]==90 && ix[1]==117 && ix[2]==88, "Test whether last index is correct");




  MITK_TEST_END();
}
