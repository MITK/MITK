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

#ifndef QMITKEXTAPPWORKBENCHADVISOR_H_
#define QMITKEXTAPPWORKBENCHADVISOR_H_

#ifdef __MINGW32__
// We need to inlclude winbase.h here in order to declare
// atomic intrinsics like InterlockedIncrement correctly.
// Otherwhise, they would be declared wrong within qatomic_windows.h .
#include <windows.h>
#endif

#include <QmitkWorkbenchAdvisor.h>

#include "mitkQtExtAppDll.h"

class MITK_QT_APP QmitkExtAppWorkbenchAdvisor : public QmitkWorkbenchAdvisor
{
public:

  void Initialize(cherry::IWorkbenchConfigurer::Pointer configurer);

  cherry::WorkbenchWindowAdvisor* CreateWorkbenchWindowAdvisor(
        cherry::IWorkbenchWindowConfigurer::Pointer configurer);

};

#endif /*QMITKEXTAPPWORKBENCHADVISOR_H_*/
