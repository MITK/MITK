/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitkPropertyManager.cpp,v $
Language:  C++
Date:      $Date$
Version:   $Revision: 1.12 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include "QmitkRegisterClasses.h"

#include "QmitkApplicationCursor.h"
#include "QmitkCallbackFromGUIThread.h"
#include "QmitkRenderingManagerFactory.h"
#include "QmitkBinaryThresholdToolGUIFactory.h"
#include "QmitkCalculateGrayValueStatisticsToolGUIFactory.h"
#include "QmitkDrawPaintbrushToolGUIFactory.h"
#include "QmitkErasePaintbrushToolGUIFactory.h"

#include <iostream>

void QmitkRegisterClasses()
{
  static bool alreadyDone = false;
  if (!alreadyDone)
  {
    std::cout << "QmitkRegisterClasses()" << std::endl;
  
  itk::ObjectFactoryBase::RegisterFactory( QmitkBinaryThresholdToolGUIFactory::New() );
  itk::ObjectFactoryBase::RegisterFactory( QmitkCalculateGrayValueStatisticsToolGUIFactory::New() );
  itk::ObjectFactoryBase::RegisterFactory( QmitkDrawPaintbrushToolGUIFactory::New() );
  itk::ObjectFactoryBase::RegisterFactory( QmitkErasePaintbrushToolGUIFactory::New() );

  //We have to put this in a file containing a class that is directly used
  //somewhere. Otherwise, e.g. when put in VtkRenderWindowInteractor.cpp, 
  //it is removed by the linker. 

  // Create and register RenderingManagerFactory for this platform.
  static QmitkRenderingManagerFactory qmitkRenderingManagerFactory;
  static QmitkApplicationCursor globalQmitkApplicationCursor; // create one instance
  static QmitkCallbackFromGUIThread globalQmitkCallbackFromGUIThread;  

  alreadyDone = true;
  }
}

