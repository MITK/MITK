/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitkPropertyManager.cpp,v $
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

#include "QmitkRegisterClasses.h"

#include "QmitkApplicationCursor.h"
#include "QmitkRenderingManagerFactory.h"
#include "mitkGlobalInteraction.h"

#include <iostream>

void QmitkRegisterClasses()
{
  static bool alreadyDone = false;
  if (!alreadyDone)
  {
    std::cout << "QmitkRegisterClasses()" << std::endl;


    //We have to put this in a file containing a class that is directly used
    //somewhere. Otherwise, e.g. when put in VtkRenderWindowInteractor.cpp, 
    //it is removed by the linker.

    // Create and initialize GlobalInteraction
    if(! (mitk::GlobalInteraction::GetInstance()->IsInitialized()))
      mitk::GlobalInteraction::GetInstance()->Initialize("global");

    // Create and register RenderingManagerFactory for this platform.
    static QmitkRenderingManagerFactory qmitkRenderingManagerFactory;
    static QmitkApplicationCursor globalQmitkApplicationCursor; // create one instance

    alreadyDone = true;
  }
}

