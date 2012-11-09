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

#ifndef QMITKWORKBENCHWINDOWADVISOR_H_
#define QMITKWORKBENCHWINDOWADVISOR_H_

#include <berryWorkbenchWindowAdvisor.h>


class QmitkWorkbenchWindowAdvisor : public berry::WorkbenchWindowAdvisor
{
public:

    QmitkWorkbenchWindowAdvisor(berry::IWorkbenchWindowConfigurer::Pointer configurer);

    berry::ActionBarAdvisor::Pointer CreateActionBarAdvisor(
        berry::IActionBarConfigurer::Pointer configurer);

    void PostWindowCreate();

    void PreWindowOpen();

private:

    berry::IDropTargetListener::Pointer dropTargetListener;
};

#endif /*QMITKWORKBENCHWINDOWADVISOR_H_*/
