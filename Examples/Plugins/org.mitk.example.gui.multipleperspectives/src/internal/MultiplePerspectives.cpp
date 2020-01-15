/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "MultiplePerspectives.h"

// berry includes
#include <berryPlatformUI.h>
#include <berryQtWorkbenchAdvisor.h>

#include <QPoint>

class MultiplePerspectivesWorkbenchAdvisor : public berry::QtWorkbenchAdvisor
{
public:
  static const QString DEFAULT_PERSPECTIVE_ID;

  berry::WorkbenchWindowAdvisor *CreateWorkbenchWindowAdvisor(
    berry::IWorkbenchWindowConfigurer::Pointer configurer) override
  {
    //! [initial window size]
    // Set an individual initial size
    configurer->SetInitialSize(QPoint(600, 400));
    //! [initial window size]

    // Set an individual title
    configurer->SetTitle("Multiple Perspectives");

    //! [Visibility of perspective bar]
    // Enable or disable the perspective bar
    configurer->SetShowPerspectiveBar(true);
    //! [Visibility of perspective bar]

    return new berry::WorkbenchWindowAdvisor(configurer);
  }

  QString GetInitialWindowPerspectiveId() override { return DEFAULT_PERSPECTIVE_ID; }
};

const QString MultiplePerspectivesWorkbenchAdvisor::DEFAULT_PERSPECTIVE_ID = "org.mitk.example.minimalperspective";

MultiplePerspectives::MultiplePerspectives()
{
}

MultiplePerspectives::~MultiplePerspectives()
{
}

QVariant MultiplePerspectives::Start(berry::IApplicationContext * /*context*/)
{
  berry::Display *display = berry::PlatformUI::CreateDisplay();
  wbAdvisor.reset(new MultiplePerspectivesWorkbenchAdvisor);

  int code = berry::PlatformUI::CreateAndRunWorkbench(display, wbAdvisor.data());

  // exit the application with an appropriate return code
  return code == berry::PlatformUI::RETURN_RESTART ? EXIT_RESTART : EXIT_OK;
}

void MultiplePerspectives::Stop()
{
}
