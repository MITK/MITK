/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
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

#include "QmitkExtRegisterClasses.h"

#include "QmitkRenderingManagerFactory.h"

#include "QmitkCallbackFromGUIThread.h"
#include "QmitkNodeDescriptorManager.h"

#include "QmitkDrawPaintbrushToolGUI.h"

#include "mitkNodePredicateDataType.h"
#include "mitkNodePredicateProperty.h"
#include "mitkNodePredicateAnd.h"
#include "mitkProperties.h"

#include <QtCore>

#include <iostream>

void QmitkExtRegisterClasses()
{
  static bool alreadyDone = false;
  if (!alreadyDone)
  {
    MITK_DEBUG << "QmitkExtRegisterClasses()";
    
    static QmitkCallbackFromGUIThread globalQmitkCallbackFromGUIThread;  
    

    alreadyDone = true;
  }
}

