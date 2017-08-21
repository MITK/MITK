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
#include "mitkIOUtil.h"

#include <mitkDWIHeadMotionCorrectionFilter.h>

typedef short                                       DiffusionPixelType;
typedef mitk::Image  DiffusionImageType;

/**
 * @brief Custom test to provide CMD-line access to the mitk::DWIHeadMotionCorrectionFilter
 *
 * @param argv : Input and Output image full path
 */
int mitkDWHeadMotionCorrectionTest( int argc, char* argv[] )
{
  MITK_TEST_BEGIN("mitkDWHeadMotionCorrectionTest");

  MITK_TEST_CONDITION_REQUIRED( argc > 2, "Specify input and output.");

//  itk::MultiThreader::SetGlobalMaximumNumberOfThreads(1);

  mitk::Image::Pointer inputImage = dynamic_cast<mitk::Image*>(mitk::IOUtil::Load( argv[1] )[0].GetPointer());
  DiffusionImageType* dwimage =
      static_cast<DiffusionImageType*>( inputImage.GetPointer() );

  mitk::DWIHeadMotionCorrectionFilter::Pointer corrfilter =
      mitk::DWIHeadMotionCorrectionFilter::New();

  corrfilter->SetInput( dwimage );
  corrfilter->SetAverageUnweighted(false);
  corrfilter->Update();

  try
  {
    mitk::IOUtil::Save(corrfilter->GetOutput(), argv[2]);
  }
  catch( const itk::ExceptionObject& e)
  {
    MITK_ERROR << "Catched exception: " << e.what();
    mitkThrow() << "Failed with exception from subprocess!";
  }

  MITK_TEST_END();
}
