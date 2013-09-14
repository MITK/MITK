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

#include "mitkPyramidImageRegistrationMethod.h"

#include "mitkException.h"
#include "mitkImageAccessByItk.h"

mitk::PyramidImageRegistrationMethod::PyramidImageRegistrationMethod()
  : m_FixedImage(NULL),
    m_MovingImage(NULL),
    m_CrossModalityRegistration(true),
    m_UseAffineTransform(true),
    m_UseWindowedSincInterpolator(false),
    m_EstimatedParameters(NULL),
    m_Verbose(false)
{

}

mitk::PyramidImageRegistrationMethod::~PyramidImageRegistrationMethod()
{
  if( m_EstimatedParameters != NULL)
  {
    delete [] m_EstimatedParameters;
  }
}

void mitk::PyramidImageRegistrationMethod::SetFixedImage(mitk::Image::Pointer fixed)
{
  if( fixed.IsNotNull() )
  {
    m_FixedImage = fixed;
  }
}

void mitk::PyramidImageRegistrationMethod::SetMovingImage(mitk::Image::Pointer moving)
{
  if( moving.IsNotNull() )
  {
    m_MovingImage = moving;
  }
}

void mitk::PyramidImageRegistrationMethod::Update()
{
  if( m_MovingImage.IsNull() )
  {
    mitkThrow() << " Moving image is null";
  }

  if( m_FixedImage.IsNull() )
  {
    mitkThrow() << " Moving image is null";
  }

  unsigned int allowedDimension = 3;

  if( m_FixedImage->GetDimension() != allowedDimension ||
      m_MovingImage->GetDimension() != allowedDimension )
  {
    mitkThrow() << " Only 3D Images supported.";
  }

  //
  // One possibility: use the FixedDimesnionByItk, but this instantiates ALL possible
  // pixel type combinations!
  // AccessTwoImagesFixedDimensionByItk( m_FixedImage, m_MovingImage, RegisterTwoImages, 3);
  // in helper: TypeSubset : short, float
  AccessTwoImagesFixedDimensionTypeSubsetByItk( m_FixedImage, m_MovingImage, RegisterTwoImages, 3);

}

mitk::PyramidImageRegistrationMethod::TransformMatrixType mitk::PyramidImageRegistrationMethod
::GetLastRotationMatrix()
{
  TransformMatrixType output;
  if( m_EstimatedParameters == NULL )
  {
    output.set_identity();
    return output;
  }

  typedef itk::MatrixOffsetTransformBase< double, 3, 3> BaseTransformType;
  BaseTransformType::Pointer base_transform = BaseTransformType::New();

  if( this->m_UseAffineTransform )
  {
    typedef itk::AffineTransform< double > TransformType;
    TransformType::Pointer transform = TransformType::New();

    TransformType::ParametersType affine_params( TransformType::ParametersDimension );
    this->GetParameters( &affine_params[0] );

    transform->SetParameters( affine_params );

    base_transform = transform;
  }
  else
  {
    typedef itk::Euler3DTransform< double > RigidTransformType;
    RigidTransformType::Pointer rtransform = RigidTransformType::New();

    RigidTransformType::ParametersType rigid_params( RigidTransformType::ParametersDimension  );
    this->GetParameters( &rigid_params[0] );

    rtransform->SetParameters( rigid_params );

    base_transform = rtransform;
  }

  return base_transform->GetMatrix().GetVnlMatrix();

}

mitk::Image::Pointer mitk::PyramidImageRegistrationMethod
::GetResampledMovingImage()
{

  mitk::Image::Pointer output = mitk::Image::New();
  //output->Initialize( this->m_FixedImage );

  AccessFixedDimensionByItk_1( this->m_MovingImage, ResampleMitkImage, 3, output );

  return output;

}
