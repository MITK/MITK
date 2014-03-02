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
    MITK_DEBUG << "QmitkRegisterClasses()";


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

