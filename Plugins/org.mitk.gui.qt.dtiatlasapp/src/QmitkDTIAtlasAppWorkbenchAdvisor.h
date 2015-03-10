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


#ifndef QMITKDTIATLASAPPWORKBENCHADVISOR_H_
#define QMITKDTIATLASAPPWORKBENCHADVISOR_H_

#include <berryQtWorkbenchAdvisor.h>

class QmitkDTIAtlasAppWorkbenchAdvisor : public berry::QtWorkbenchAdvisor
{

public:

  static const QString WELCOME_PERSPECTIVE_ID; // = "org.mitk.qt.dtiatlasapp.defaultperspective"

  void Initialize(berry::IWorkbenchConfigurer::Pointer configurer);

  berry::WorkbenchWindowAdvisor* CreateWorkbenchWindowAdvisor(
        berry::IWorkbenchWindowConfigurer::Pointer configurer);

  QString GetInitialWindowPerspectiveId();

};

#endif /* QMITKDTIATLASAPPWORKBENCHADVISOR_H_ */
