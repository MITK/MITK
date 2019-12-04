/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
