/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "QmitkRenderingManagerFactory.h"
#include "QmitkRenderingManager.h"

/** bug #1333: CoreObjectFactory-like system for Qt dependent classes **/
#include "QmitkBinaryThresholdToolGUI.h"
#include "QmitkCalculateGrayValueStatisticsToolGUI.h"

QmitkRenderingManagerFactory
::QmitkRenderingManagerFactory()
{
  mitk::RenderingManager::SetFactory( this );

  /** bug #1333: CoreObjectFactory-like system for Qt dependent classes **/
  itk::ObjectFactoryBase::RegisterFactory( QmitkBinaryThresholdToolGUIFactory::New() );
  itk::ObjectFactoryBase::RegisterFactory( QmitkCalculateGrayValueStatisticsToolGUIFactory::New() );
}

QmitkRenderingManagerFactory
::~QmitkRenderingManagerFactory()
{
}

mitk::RenderingManager::Pointer
QmitkRenderingManagerFactory
::CreateRenderingManager() const
{
  QmitkRenderingManager::Pointer specificSmartPtr = QmitkRenderingManager::New();
  mitk::RenderingManager::Pointer smartPtr = specificSmartPtr.GetPointer();
  return smartPtr;
}

