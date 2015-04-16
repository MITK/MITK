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

#include "QmitkAboutHandler.h"

#include <QmitkAboutDialog.h>

berry::Object::Pointer QmitkAboutHandler::Execute(const berry::SmartPointer<const berry::ExecutionEvent>& /*event*/)
{
  QmitkAboutDialog* aboutDialog = new QmitkAboutDialog(QApplication::activeWindow(), NULL);
  aboutDialog->open();
  return berry::Object::Pointer();
}
