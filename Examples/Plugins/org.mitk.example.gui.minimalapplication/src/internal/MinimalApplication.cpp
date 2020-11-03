/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "MinimalApplication.h"

// Berry
#include <berryPlatformUI.h>
#include <berryQtWorkbenchAdvisor.h>

#include <QPoint>

class MinimalWorkbenchAdvisor : public berry::WorkbenchAdvisor
{
public:
  static const QString DEFAULT_PERSPECTIVE_ID;

  berry::WorkbenchWindowAdvisor *CreateWorkbenchWindowAdvisor(
    berry::IWorkbenchWindowConfigurer::Pointer configurer) override
  {
    // Set an individual initial size
    configurer->SetInitialSize(QPoint(600, 400));
    // Set an individual title
    configurer->SetTitle("Minimal Application");
    // Enable or disable the perspective bar
    configurer->SetShowPerspectiveBar(false);

    return new berry::WorkbenchWindowAdvisor(configurer);
  }

  QString GetInitialWindowPerspectiveId() override { return DEFAULT_PERSPECTIVE_ID; }
};

const QString MinimalWorkbenchAdvisor::DEFAULT_PERSPECTIVE_ID = "org.mitk.example.minimalperspective";

MinimalApplication::MinimalApplication()
{
}

MinimalApplication::~MinimalApplication()
{
}

QVariant MinimalApplication::Start(berry::IApplicationContext * /*context*/)
{
  QScopedPointer<berry::Display> display(berry::PlatformUI::CreateDisplay());

  QScopedPointer<MinimalWorkbenchAdvisor> wbAdvisor(new MinimalWorkbenchAdvisor());
  int code = berry::PlatformUI::CreateAndRunWorkbench(display.data(), wbAdvisor.data());

  // exit the application with an appropriate return code
  return code == berry::PlatformUI::RETURN_RESTART ? EXIT_RESTART : EXIT_OK;
}

void MinimalApplication::Stop()
{
}
