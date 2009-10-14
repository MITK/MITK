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


#include "QmitkExtAppWorkbenchAdvisor.h"

#include <QApplication>

#include <QmitkExtRegisterClasses.h>

#include "QmitkExtAppWorkbenchWindowAdvisor.h"

const std::string QmitkWorkbenchAdvisor::DEFAULT_PERSP_ID = "org.mitk.perspectives.default";

void
QmitkExtAppWorkbenchAdvisor::Initialize(cherry::IWorkbenchConfigurer::Pointer configurer)
{
  QmitkWorkbenchAdvisor::Initialize(configurer);
  QmitkExtRegisterClasses();
}

cherry::WorkbenchWindowAdvisor*
QmitkExtAppWorkbenchAdvisor::CreateWorkbenchWindowAdvisor(
        cherry::IWorkbenchWindowConfigurer::Pointer configurer)
{
  return new QmitkExtAppWorkbenchWindowAdvisor(configurer);
}
