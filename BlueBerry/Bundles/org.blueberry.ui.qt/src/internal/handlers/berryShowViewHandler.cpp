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

#include "berryShowViewHandler.h"

#include "berryIWorkbenchCommandConstants.h"
#include "berryHandlerUtil.h"

#include "berryUIException.h"
#include "berryIWorkbenchPage.h"
#include "berryIViewDescriptor.h"
#include "berryPlatformUI.h"

#include "internal/berryWorkbenchPlugin.h"
#include "internal/berryQtShowViewDialog.h"

#include <berryCommandExceptions.h>

#include <vector>

namespace berry
{

ShowViewHandler::ShowViewHandler()
{
}

Object::Pointer ShowViewHandler::Execute(const ExecutionEvent::ConstPointer& event)
{
  IWorkbenchWindow::Pointer window = HandlerUtil::GetActiveWorkbenchWindowChecked(event);
  // Get the view identifier, if any.
  const ExecutionEvent::ParameterMap& parameters = event->GetParameters();
  ExecutionEvent::ParameterMap::const_iterator result = parameters.find(IWorkbenchCommandConstants::VIEWS_SHOW_VIEW_PARM_ID);
  QString viewId = result != parameters.end() ? result.value() : QString::null;
  result = parameters.find(IWorkbenchCommandConstants::VIEWS_SHOW_VIEW_SECONDARY_ID);
  QString secondary = result != parameters.end() ? result.value() : QString::null;

  if (viewId.isEmpty())
  {
    this->OpenOther(window);
  }
  else
  {
    try
    {
      this->OpenView(viewId, secondary, window);
    }
    catch (const PartInitException& e)
    {
      throw ExecutionException("Part could not be initialized", e);
    }
  }

  return Object::Pointer(0);
}

void ShowViewHandler::OpenOther(IWorkbenchWindow::Pointer window)
{
  const IWorkbenchPage::Pointer page = window->GetActivePage();
  if (page.IsNull())
  {
    return;
  }

  QtShowViewDialog dialog(window.GetPointer(), WorkbenchPlugin::GetDefault()->GetViewRegistry());

  int returnCode = dialog.exec();

  if (returnCode == QDialog::Rejected)
  {
    return;
  }

  const QList<QString> descriptors = dialog.GetSelection();
  for (QString id : descriptors)
  {
    try
    {
      this->OpenView(id, QString(), window);
    }
    catch (const PartInitException& e)
    {
      BERRY_WARN << e.what();
//      StatusUtil.handleStatus(e.getStatus(),
//          WorkbenchMessages.ShowView_errorTitle
//          + ": " + e.getMessage(), //$NON-NLS-1$
//          StatusManager.SHOW);
    }
  }
}

void ShowViewHandler::OpenView(const QString& viewId, const QString& secondaryId, IWorkbenchWindow::Pointer activeWorkbenchWindow)
{
  const IWorkbenchPage::Pointer activePage = activeWorkbenchWindow->GetActivePage();
  if (activePage.IsNull())
  {
    return;
  }

  activePage->ShowView(viewId, secondaryId, IWorkbenchPage::VIEW_ACTIVATE);
}

}
