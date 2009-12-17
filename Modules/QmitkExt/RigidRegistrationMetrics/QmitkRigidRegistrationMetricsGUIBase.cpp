/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision: -1 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkRigidRegistrationMetricsGUIBase.h"

QmitkRigidRegistrationMetricsGUIBase::QmitkRigidRegistrationMetricsGUIBase(QWidget* parent, Qt::WindowFlags f) : QWidget( parent, f ), 
  m_MovingImage(NULL)
{
}

QmitkRigidRegistrationMetricsGUIBase::~QmitkRigidRegistrationMetricsGUIBase()
{
}

void QmitkRigidRegistrationMetricsGUIBase::SetMovingImage(mitk::Image::Pointer movingImage)
{
  m_MovingImage = movingImage;
}

