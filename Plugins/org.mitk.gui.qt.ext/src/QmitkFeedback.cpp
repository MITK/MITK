/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkFeedback.h"

#include "internal/QmitkFeedbackDialog.h"

#include <berryIWorkbench.h>
#include <berryIWorkbenchWindow.h>
#include <berryPlatformUI.h>
#include <berryShell.h>

#include <mitkEnvironmentInfo.h>

#include <QWidget>

namespace
{
  QWidget* GetActiveWorkbenchWindowWidget()
  {
    if (!berry::PlatformUI::IsWorkbenchRunning())
      return nullptr;

    const auto window = berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow();

    if (window.IsNull())
      return nullptr;

    return static_cast<QWidget*>(window->GetShell()->GetControl());
  }
}

void QmitkFeedback::OpenDialog(QWidget* parent)
{
  if (parent == nullptr)
    parent = GetActiveWorkbenchWindowWidget();

  QmitkFeedbackDialog dialog(mitk::CollectEnvironmentInfo(), parent);
  dialog.exec();
}
