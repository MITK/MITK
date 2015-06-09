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


#ifndef QMITKDIFFUSIONIMAGINGAPPWORKBENCHADVISOR_H_
#define QMITKDIFFUSIONIMAGINGAPPWORKBENCHADVISOR_H_

#include <berryQtWorkbenchAdvisor.h>

class QmitkDiffusionImagingAppWorkbenchAdvisor : public berry::QtWorkbenchAdvisor
{

public:

  static const QString WELCOME_PERSPECTIVE_ID; // = "org.mitk.qt.diffusionimagingapp.defaultperspective"

  void Initialize(berry::IWorkbenchConfigurer::Pointer configurer) override;

  berry::WorkbenchWindowAdvisor* CreateWorkbenchWindowAdvisor(
        berry::IWorkbenchWindowConfigurer::Pointer configurer) override;

  QString GetInitialWindowPerspectiveId() override;

};

#endif /* QMITKDIFFUSIONIMAGINGAPPWORKBENCHADVISOR_H_ */
