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

#include "mitkPyramidImageRegistrationMethod.h"

int mitkPyramidImageRegistrationMethodTest( int argc, char* argv[] )
{
  if( argc < 3 )
  {
    MITK_ERROR << "Not enough input \n Usage: <TEST_NAME> fixed moving";
    return EXIT_FAILURE;
  }

  MITK_TEST_BEGIN("PyramidImageRegistrationMethodTest");

  mitk::Image::Pointer fixedImage = mitk::IOUtil::LoadImage( argv[1] );
  mitk::Image::Pointer movingImage = mitk::IOUtil::LoadImage( argv[2] );

  mitk::PyramidImageRegistrationMethod::Pointer registrationMethod = mitk::PyramidImageRegistrationMethod::New();
  registrationMethod->SetFixedImage( fixedImage );
  registrationMethod->SetMovingImage( movingImage );
  registrationMethod->Update();

  unsigned int paramCount = registrationMethod->GetNumberOfParameters();
  double* params = new double[ paramCount ];
  registrationMethod->GetParameters( &params[0] );

  std::cout << "Parameters: ";
  for( unsigned int i=0; i< paramCount; i++)
  {
    std::cout << params[ i ] << " ";
  }
  std::cout << std::endl;

  MITK_TEST_END();
}
