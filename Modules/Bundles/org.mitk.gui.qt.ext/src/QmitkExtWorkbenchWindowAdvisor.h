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

#ifndef QMITKEXTWORKBENCHWINDOWADVISOR_H_
#define QMITKEXTWORKBENCHWINDOWADVISOR_H_

#include <cherryWorkbenchWindowAdvisor.h>

#include "mitkQtCommonExtDll.h"

class MITK_QT_COMMON_EXT_EXPORT QmitkExtWorkbenchWindowAdvisor : public cherry::WorkbenchWindowAdvisor
{
public:

    QmitkExtWorkbenchWindowAdvisor(cherry::IWorkbenchWindowConfigurer::Pointer configurer);

    cherry::ActionBarAdvisor::Pointer CreateActionBarAdvisor(
        cherry::IActionBarConfigurer::Pointer configurer);

    void PostWindowCreate();

    void ShowViewToolbar(bool show);

private:

    bool showViewToolbar;
};

#endif /*QMITKEXTWORKBENCHWINDOWADVISOR_H_*/
