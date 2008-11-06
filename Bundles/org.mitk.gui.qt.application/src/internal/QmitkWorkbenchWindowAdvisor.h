/*=========================================================================

Program:   openCherry Platform
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

#include <application/cherryWorkbenchWindowAdvisor.h>

class QmitkWorkbenchWindowAdvisor : public cherry::WorkbenchWindowAdvisor
{
public:

  QmitkWorkbenchWindowAdvisor(cherry::IWorkbenchWindowConfigurer::Pointer configurer);

  cherry::ActionBarAdvisor::Pointer CreateActionBarAdvisor(
      cherry::IActionBarConfigurer::Pointer configurer);

  void PostWindowCreate();
};

#endif /*QMITKWORKBENCHWINDOWADVISOR_H_*/
