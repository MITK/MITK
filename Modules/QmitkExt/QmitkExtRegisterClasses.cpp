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

