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

#include "QmitkRigidRegistrationTransformsGUIBase.h"

QmitkRigidRegistrationTransformsGUIBase::QmitkRigidRegistrationTransformsGUIBase(QWidget* parent, Qt::WindowFlags f) : QWidget( parent, f ),
  m_FixedImage(NULL), m_MovingImage(NULL)
{
}

QmitkRigidRegistrationTransformsGUIBase::~QmitkRigidRegistrationTransformsGUIBase()
{
}

void QmitkRigidRegistrationTransformsGUIBase::SetFixedImage(mitk::Image::Pointer fixedImage)
{
  m_FixedImage = fixedImage;
}

void QmitkRigidRegistrationTransformsGUIBase::SetMovingImage(mitk::Image::Pointer movingImage)
{
  m_MovingImage = movingImage;
}
