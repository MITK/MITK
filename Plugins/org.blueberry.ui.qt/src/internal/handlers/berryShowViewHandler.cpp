/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryShowViewHandler.h"

#include "berryIWorkbenchCommandConstants.h"
#include "berryHandlerUtil.h"

#include "berryUIException.h"
#include "berryIWorkbenchPage.h"
#include "berryIViewDescriptor.h"
#include "berryPlatformUI.h"

#include "berryWorkbenchPlugin.h"
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

  return Object::Pointer(nullptr);
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
