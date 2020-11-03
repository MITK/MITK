/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkAboutHandler.h"

#include <QmitkAboutDialog.h>

berry::Object::Pointer QmitkAboutHandler::Execute(const berry::SmartPointer<const berry::ExecutionEvent>& /*event*/)
{
  auto   aboutDialog = new QmitkAboutDialog(QApplication::activeWindow(), nullptr);
  aboutDialog->open();
  return berry::Object::Pointer();
}
