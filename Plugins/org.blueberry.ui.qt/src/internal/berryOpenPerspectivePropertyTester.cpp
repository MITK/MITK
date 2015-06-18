/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

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

