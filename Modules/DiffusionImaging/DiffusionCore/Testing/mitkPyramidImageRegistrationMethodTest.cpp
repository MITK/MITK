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

#include "Registration/mitkPyramidImageRegistrationMethod.h"

#include <itkTransformFileWriter.h>


int mitkPyramidImageRegistrationMethodTest( int argc, char* argv[] )
{
  if( argc < 4 )
  {
    MITK_ERROR << "Not enough input \n Usage: <TEST_NAME> fixed moving type [output_image [output_transform]]"
               << "\n \t fixed : the path to the fixed image \n"
               << " \t moving : path to the image to be registered"
               << " \t type : Affine or Rigid defining the type of the transformation"
               << " \t output_image : output file optional, (full) path, and optionally output_transform : also (full)path to file";
    return EXIT_FAILURE;
  }

  MITK_TEST_BEGIN("PyramidImageRegistrationMethodTest");

  mitk::Image::Pointer fixedImage = mitk::IOUtil::Load<mitk::Image>( argv[1] );
  mitk::Image::Pointer movingImage = mitk::IOUtil::Load<mitk::Image>( argv[2] );

  std::string type_flag( argv[3] );

  mitk::PyramidImageRegistrationMethod::Pointer registrationMethod = mitk::PyramidImageRegistrationMethod::New();
  registrationMethod->SetFixedImage( fixedImage );
  registrationMethod->SetMovingImage( movingImage );

  if( type_flag == "Rigid" )
  {
    registrationMethod->SetTransformToRigid();
  }
  else if( type_flag == "Affine" )
  {
    registrationMethod->SetTransformToAffine();
  }
  else
  {
    MITK_WARN << " No type specified, using 'Affine' .";
  }

  registrationMethod->Update();

  bool imageOutput = false;
  bool transformOutput = false;

  std::string image_out_filename, transform_out_filename;

  std::string first_output( argv[4] );
  // check for txt, otherwise suppose it is an image
  if( first_output.find(".txt") != std::string::npos )
  {
    transformOutput = true;
    transform_out_filename = first_output;
  }
  else
  {
    imageOutput = true;
    image_out_filename = first_output;
  }

  if( argc > 4 )
  {
    std::string second_output( argv[5] );
    if( second_output.find(".txt") != std::string::npos )
    {
      transformOutput = true;
      transform_out_filename = second_output;
    }
  }

  MITK_INFO << " Selected output: " << transform_out_filename  << " " << image_out_filename;

  try{

    unsigned int paramCount = registrationMethod->GetNumberOfParameters();
    double* params = new double[ paramCount ];
    registrationMethod->GetParameters( &params[0] );

    std::cout << "Parameters: ";
    for( unsigned int i=0; i< paramCount; i++)
    {
      std::cout << params[ i ] << " ";
    }
    std::cout << std::endl;

    if( imageOutput )
    {
      mitk::IOUtil::Save( registrationMethod->GetResampledMovingImage(), image_out_filename.c_str() );
    }


    if( transformOutput )
    {

      itk::TransformFileWriter::Pointer writer = itk::TransformFileWriter::New();

      // Get transform parameter for resampling / saving
      // Affine
      if( paramCount == 12 )
      {
        typedef itk::AffineTransform< double > TransformType;
        TransformType::Pointer transform = TransformType::New();

        TransformType::ParametersType affine_params( paramCount );
        registrationMethod->GetParameters( &affine_params[0] );

        transform->SetParameters( affine_params );
        writer->SetInput( transform );
      }
      // Rigid
      else
      {
        typedef itk::Euler3DTransform< double > RigidTransformType;
        RigidTransformType::Pointer rtransform = RigidTransformType::New();

        RigidTransformType::ParametersType rigid_params( paramCount );
        registrationMethod->GetParameters( &rigid_params[0] );

        rtransform->SetParameters( rigid_params );
        writer->SetInput( rtransform );
      }

      writer->SetFileName( transform_out_filename );
      writer->Update();
    }

  }
  catch( const std::exception &e)
  {
    MITK_ERROR << "Caught exception: " << e.what();
  }


  MITK_TEST_END();
}
