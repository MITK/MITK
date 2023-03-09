/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef QmitkFlowApplicationWorkbenchAdvisor_h
#define QmitkFlowApplicationWorkbenchAdvisor_h

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

#endif
