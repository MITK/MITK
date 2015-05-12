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
#include <berryIDropTargetListener.h>


class QmitkWorkbenchWindowAdvisor : public berry::WorkbenchWindowAdvisor
{
public:

    QmitkWorkbenchWindowAdvisor(berry::SmartPointer<berry::IWorkbenchWindowConfigurer> configurer);

    berry::SmartPointer<berry::ActionBarAdvisor> CreateActionBarAdvisor(
        berry::SmartPointer<berry::IActionBarConfigurer> configurer) override;

    void PostWindowCreate() override;

    void PreWindowOpen() override;

private:

    QScopedPointer<berry::IDropTargetListener> dropTargetListener;
};

#endif /*QMITKWORKBENCHWINDOWADVISOR_H_*/
