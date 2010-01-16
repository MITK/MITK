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

#ifndef QMITKWORKBENCHWINDOWADVISOR_H_
#define QMITKWORKBENCHWINDOWADVISOR_H_

#include <berryWorkbenchWindowAdvisor.h>

#include "mitkQtAppDll.h"

class MITK_QT_APP QmitkWorkbenchWindowAdvisor : public berry::WorkbenchWindowAdvisor
{
public:

    QmitkWorkbenchWindowAdvisor(berry::IWorkbenchWindowConfigurer::Pointer configurer);

    berry::ActionBarAdvisor::Pointer CreateActionBarAdvisor(
        berry::IActionBarConfigurer::Pointer configurer);

    void PostWindowCreate();
};

#endif /*QMITKWORKBENCHWINDOWADVISOR_H_*/
