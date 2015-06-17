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
