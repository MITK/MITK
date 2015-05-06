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

#include "berryIntroHandler.h"

#include <berryPlatformUI.h>
#include <berryHandlerUtil.h>

#include "internal/berryWorkbench.h"
#include "internal/intro/berryIntroDescriptor.h"

#include <QApplication>
#include <QMessageBox>

namespace berry {

IntroHandler::IntroHandler()
{
  workbench = dynamic_cast<Workbench*>(PlatformUI::GetWorkbench());
  introDescriptor = workbench->GetIntroDescriptor();
}

IntroHandler::~IntroHandler()
{
}

SmartPointer<Object> IntroHandler::Execute(const SmartPointer<const ExecutionEvent> &event)
{
  if (introDescriptor.IsNull())
  {
    QMessageBox::warning(QApplication::activeWindow(),
                         "No Welcome Content Found",
                         "There is no welcome content suitable for display in this application.");
  }
  else
  {
    IWorkbenchWindow::Pointer window = HandlerUtil::GetActiveWorkbenchWindow(event);
    workbench->GetIntroManager()->ShowIntro(window, false);
  }
  return Object::Pointer();
}

bool IntroHandler::IsEnabled() const
{

  bool enabled = false;
  IWorkbenchWindow::Pointer window = PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow();
  if (window.IsNotNull())
  {
    enabled = !window->GetPages().isEmpty();
  }
  return enabled;
}

}
