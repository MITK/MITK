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
    m_EstimatedParameters(NULL)
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
