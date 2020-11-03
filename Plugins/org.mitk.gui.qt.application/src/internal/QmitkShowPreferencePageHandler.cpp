/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkShowPreferencePageHandler.h"

#include <berryIWorkbenchCommandConstants.h>

#include <berryHandlerUtil.h>

#include <QmitkPreferencesDialog.h>

#include <QApplication>

berry::Object::Pointer QmitkShowPreferencePageHandler::Execute(const berry::ExecutionEvent::ConstPointer& event)
{
  const QString preferencePageId = event->GetParameter(berry::IWorkbenchCommandConstants::WINDOW_PREFERENCES_PARM_PAGEID);

  QmitkPreferencesDialog prefDialog(QApplication::activeWindow());
  prefDialog.SetSelectedPage(preferencePageId);

  prefDialog.exec();

  return berry::Object::Pointer();
}
