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

#include "SelectionServiceQt.h"

// berry includes
#include <berryPlatformUI.h>
#include <berryQtWorkbenchAdvisor.h>

#include <QPoint>

class SelectionServiceQtWorkbenchAdvisor : public berry::QtWorkbenchAdvisor
{

public:

  static const QString DEFAULT_PERSPECTIVE_ID;

  berry::WorkbenchWindowAdvisor* CreateWorkbenchWindowAdvisor(
      berry::IWorkbenchWindowConfigurer::Pointer configurer)
  {
    // Set an individual initial size
    configurer->SetInitialSize(QPoint(600,400));
    // Set the window title
    configurer->SetTitle("Qt Selection Service");

    return new berry::WorkbenchWindowAdvisor(configurer);
  }

  QString GetInitialWindowPerspectiveId()
  {
    return DEFAULT_PERSPECTIVE_ID;
  }

};

const QString SelectionServiceQtWorkbenchAdvisor::DEFAULT_PERSPECTIVE_ID = "org.mitk.example.extendedperspective";

SelectionServiceQt::SelectionServiceQt()
{
}

SelectionServiceQt::~SelectionServiceQt()
{
}

QVariant SelectionServiceQt::Start(berry::IApplicationContext* /*context*/)
{
  berry::Display* display = berry::PlatformUI::CreateDisplay();
  wbAdvisor.reset(new SelectionServiceQtWorkbenchAdvisor);

  int code = berry::PlatformUI::CreateAndRunWorkbench(display, wbAdvisor.data());

  // exit the application with an appropriate return code
  return code == berry::PlatformUI::RETURN_RESTART
              ? EXIT_RESTART : EXIT_OK;
}

void SelectionServiceQt::Stop()
{

}
