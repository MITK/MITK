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

mitk::PyramidImageRegistrationMethod::PyramidImageRegistrationMethod()
  : m_FixedImage(NULL),
    m_MovingImage(NULL),
    m_CrossModalityRegistration(true),
    m_UseAffineTransform(true)
{

}

mitk::PyramidImageRegistrationMethod::~PyramidImageRegistrationMethod()
{

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



}
