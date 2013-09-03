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


#include "mitkImage.h"
#include "mitkImageTimeSelector.h"
#include "mitkIOUtil.h"
#include "mitkTestingMacros.h"

#include <itkExceptionObject.h>

#include <itksys/SystemTools.hxx>


int mitkImageTimeSelectorTest(int argc, char* argv[])
{
  MITK_TEST_BEGIN(ImageTimeSelector)

  if(argc==0)
  {
    MITK_TEST_FAILED_MSG( << "no file specified [FAILED]" )
  }

  mitk::Image::Pointer image = NULL;
  try
  {
    mitk::DataNode::Pointer node = mitk::IOUtil::LoadDataNode(argv[1]);
    image = dynamic_cast<mitk::Image*>(node->GetData());
    MITK_TEST_CONDITION_REQUIRED(!image.IsNull(), "file not an image")
  }
  catch (const itk::ExceptionObject& ex )
  {
    MITK_TEST_FAILED_MSG( << "Exception: " << ex.what() << "[FAILED]" )
  }

  //Take a time step
  mitk::ImageTimeSelector::Pointer timeSelector = mitk::ImageTimeSelector::New();
  timeSelector->SetInput(image);
  timeSelector->SetTimeNr( 0 );
  timeSelector->UpdateLargestPossibleRegion();
  mitk::Image::Pointer result = timeSelector->GetOutput();


  MITK_TEST_CONDITION(result->IsInitialized(), "Testin IsInitialized()")
  MITK_TEST_CONDITION(result->IsVolumeSet(0), "Test IsVolumeSet(0)")

  MITK_TEST_END()
}
