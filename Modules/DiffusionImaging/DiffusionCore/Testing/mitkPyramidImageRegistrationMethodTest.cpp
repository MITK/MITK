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

  mitk::Image::Pointer fixedImage = mitk::IOUtil::LoadImage( argv[1] );
  mitk::Image::Pointer movingImage = mitk::IOUtil::LoadImage( argv[2] );

  std::string type_flag( argv[3] );

  mitk::PyramidImageRegistrationMethod::Pointer registrationMethod = mitk::PyramidImageRegistrationMethod::New();
  registrationMethod->SetFixedImage( fixedImage );
  registrationMethod->SetMovingImage( movingImage );
  registrationMethod->Update();

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
        transform_out_filename = first_output;
    }
  }

  unsigned int paramCount = registrationMethod->GetNumberOfParameters();
  double* params = new double[ paramCount ];
  registrationMethod->GetParameters( &params[0] );

  typedef itk::MatrixOffsetTransformBase< double, 3, 3> BaseTransformType;
  BaseTransformType::Pointer base_transform = BaseTransformType::New();

  // Affine
  if( paramCount == 12 )
  {
      typedef itk::AffineTransform< double > TransformType;
      TransformType::Pointer transform = TransformType::New();

      TransformType::ParametersType affine_params( TransformType::ParametersDimension );
      registrationMethod->GetParameters( &affine_params[0] );

      transform->SetParameters( affine_params );

      base_transform = transform;

   /*   if( transformOutput )
      {
        itk::TransformFileWriter::Pointer writer = itk::TransformFileWriter::New();
        writer->SetInput( transform );
        writer->SetFileName( transform_out_filename );
        writer->Update();
      }*/
  }
  // Rigid
  else
  {
      typedef itk::Rigid3DTransform< double > RigidTransformType;
      RigidTransformType::Pointer transform = RigidTransformType::New();

      TransformType::ParametersType rigid_params( TransformType::ParametersDimension );
      registrationMethod->GetParameters( &rigid_params[0] );

      transform->SetParameters( rigid_params );

      base_transform = transform;


    /*  if( transformOutput )
      {
        itk::TransformFileWriter::Pointer writer = itk::TransformFileWriter::New();
        writer->SetInput( transform );
        writer->SetFilename( transform_out_filename );
        writer->Update();
      }*/
  }

  if( transformOutput )
  {
    itk::TransformFileWriter::Pointer writer = itk::TransformFileWriter::New();
    writer->SetInput( base_transform );
    writer->SetFileName( transform_out_filename );
    writer->Update();
  }

  std::cout << "Parameters: ";
  for( unsigned int i=0; i< paramCount; i++)
  {
    std::cout << params[ i ] << " ";
  }
  std::cout << std::endl;

  MITK_TEST_END();
}
