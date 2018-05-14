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

#include "mitkDWIHeadMotionCorrectionFilter.h"

/**
 * @brief Custom test to provide CMD-line access to the mitk::DWIHeadMotionCorrectionFilter
 *
 * @param argv : Input and Output image full path
 */
int mitkConvertDWITypeTest( int argc, char* argv[] )
{
  MITK_TEST_BEGIN("mitkConvertDWITypeTest");

  MITK_TEST_CONDITION_REQUIRED( argc > 2, "Specify input and output.");

  mitk::Image::Pointer inputImage = mitk::IOUtil::Load<mitk::Image>( argv[1] );

  try
  {
    mitk::IOUtil::Save(inputImage, argv[2]);
  }
  catch( const itk::ExceptionObject& e)
  {
    MITK_ERROR << "Caught exception: " << e.what();
    mitkThrow() << "Failed with exception from subprocess!";
  }

  MITK_TEST_END();
}
