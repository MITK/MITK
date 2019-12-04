/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryOpenPerspectivePropertyTester.h"

#include "berryWorkbenchWindow.h"


#include <QDebug>

namespace berry {

QString OpenPerspectivePropertyTester::PROPERTY_IS_PERSPECTIVE_OPEN = "isPerspectiveOpen";

bool OpenPerspectivePropertyTester::Test(Object::ConstPointer receiver, const QString& property,
                                         const QList<Object::Pointer>& args, Object::Pointer /*expectedValue*/)
{
  if (args.empty() && dynamic_cast<const WorkbenchWindow*>(receiver.GetPointer()))
  {
    WorkbenchWindow::ConstPointer window = receiver.Cast<const WorkbenchWindow>();
    if (PROPERTY_IS_PERSPECTIVE_OPEN == property)
    {
      IWorkbenchPage::Pointer page = window->GetActivePage();
      if (page.IsNotNull())
      {
        return page->GetPerspective().IsNotNull();
      }
    }
  }
  return false;
}

}

