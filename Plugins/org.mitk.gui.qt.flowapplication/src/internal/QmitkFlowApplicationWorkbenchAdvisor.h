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

#ifndef QMITKFLOWAPPLICATIONWORKBENCHADVISOR_H_
#define QMITKFLOWAPPLICATIONWORKBENCHADVISOR_H_

#include <berryQtWorkbenchAdvisor.h>

class QmitkFlowApplicationWorkbenchAdvisor: public berry::QtWorkbenchAdvisor
{
public:

  static const QString DEFAULT_PERSPECTIVE_ID; // = "org.mitk.extapp.defaultperspective"

  void Initialize(berry::IWorkbenchConfigurer::Pointer configurer) override;

  berry::WorkbenchWindowAdvisor* CreateWorkbenchWindowAdvisor(
      berry::IWorkbenchWindowConfigurer::Pointer configurer) override;

  QString GetInitialWindowPerspectiveId() override;

};

#endif /*QMITKFLOWAPPLICATIONWORKBENCHADVISOR_H_*/
