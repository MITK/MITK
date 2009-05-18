/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitkPropertyManager.cpp,v $
Language:  C++
Date:      $Date: 2009-05-12 20:04:59 +0200 (Tue, 12 May 2009) $
Version:   $Revision: 17180 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include "QmitkRegisterClasses.h"

#include "QmitkRenderingManagerFactory.h"

#include "QmitkBinaryThresholdToolGUIFactory.h"
#include "QmitkCalculateGrayValueStatisticsToolGUIFactory.h"
#include "QmitkDrawPaintbrushToolGUIFactory.h"
#include "QmitkErasePaintbrushToolGUIFactory.h"

#include <iostream>

void QmitkExtRegisterClasses()
{
  static bool alreadyDone = false;
  if (!alreadyDone)
  {
    std::cout << "QmitkExtRegisterClasses()" << std::endl;
  
  itk::ObjectFactoryBase::RegisterFactory( QmitkBinaryThresholdToolGUIFactory::New() );
  itk::ObjectFactoryBase::RegisterFactory( QmitkCalculateGrayValueStatisticsToolGUIFactory::New() );
  itk::ObjectFactoryBase::RegisterFactory( QmitkDrawPaintbrushToolGUIFactory::New() );
  itk::ObjectFactoryBase::RegisterFactory( QmitkErasePaintbrushToolGUIFactory::New() );

  alreadyDone = true;
  }
}

