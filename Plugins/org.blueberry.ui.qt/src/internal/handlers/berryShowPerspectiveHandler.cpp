/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryShowPerspectiveHandler.h"

#include <berryIPreferences.h>
#include <berryIWorkbenchCommandConstants.h>
#include <berryIWorkbenchWindow.h>

#include <berryHandlerUtil.h>
#include <berryObjects.h>

#include "internal/berryPreferenceConstants.h"
#include "internal/berryQtShowPerspectiveDialog.h"
#include "internal/berryWorkbench.h"
#include "berryWorkbenchPlugin.h"

#include <QMessageBox>

namespace berry
{

Object::Pointer ShowPerspectiveHandler::Execute(const ExecutionEvent::ConstPointer& event)
{
  IWorkbenchWindow::Pointer window = HandlerUtil::GetActiveWorkbenchWindowChecked(event);

  // Get the perspective identifier, if any.
  const ExecutionEvent::ParameterMap& parameters = event->GetParameters();
  auto idParam = parameters.find(IWorkbenchCommandConstants::PERSPECTIVES_SHOW_PERSPECTIVE_PARM_ID);
  auto newWindowParam = parameters.find(IWorkbenchCommandConstants::PERSPECTIVES_SHOW_PERSPECTIVE_PARM_NEWWINDOW);

  if (idParam == parameters.end() || idParam.value().isEmpty())
  {
    OpenOther(window.GetPointer());
  }
  else
  {
    if (newWindowParam == parameters.end() || newWindowParam.value().compare("false", Qt::CaseInsensitive) == 0)
    {
      OpenPerspective(idParam.value(), window.GetPointer());
    }
    else
    {
      OpenNewWindowPerspective(idParam.value(), window.GetPointer());
    }
  }
  return Object::Pointer();
}

void ShowPerspectiveHandler::OpenNewWindowPerspective(const QString& perspectiveId, const IWorkbenchWindow* activeWorkbenchWindow)
{
  IWorkbench* workbench = activeWorkbenchWindow->GetWorkbench();
  try
  {
    IAdaptable* input = dynamic_cast<Workbench*>(workbench)->GetDefaultPageInput();
    workbench->OpenWorkbenchWindow(perspectiveId, input);
  }
  catch (const WorkbenchException& e)
  {
    QMessageBox::critical(activeWorkbenchWindow->GetShell()->GetControl(), "Problems Changing Perspective", e.message());
//    ErrorDialog.openError(activeWorkbenchWindow.getShell(),
//                          WorkbenchMessages.ChangeToPerspectiveMenu_errorTitle, e
//                          .getMessage(), e.getStatus());
  }
}

void ShowPerspectiveHandler::OpenOther(IWorkbenchWindow* activeWorkbenchWindow)
{
  QtShowPerspectiveDialog dialog(activeWorkbenchWindow->GetWorkbench()->GetPerspectiveRegistry(),
                                 activeWorkbenchWindow->GetShell()->GetControl());

  int returnCode = dialog.exec();
  if (returnCode == QDialog::Rejected)
  {
    return;
  }

  QString perspectiveId = dialog.GetSelection();
  if (!perspectiveId.isEmpty())
  {
    int openPerspMode = WorkbenchPlugin::GetDefault()->GetPreferences()->GetInt(PreferenceConstants::OPEN_PERSP_MODE,
                                                                                PreferenceConstants::OPM_ACTIVE_PAGE);
    IWorkbenchPage::Pointer page = activeWorkbenchWindow->GetActivePage();
    IPerspectiveDescriptor::Pointer persp = page == nullptr ? IPerspectiveDescriptor::Pointer() : page->GetPerspective();

    // only open it in a new window if the preference is set and the
    // current workbench page doesn't have an active perspective
    if (PreferenceConstants::OPM_NEW_WINDOW == openPerspMode && persp.IsNotNull())
    {
      OpenNewWindowPerspective(perspectiveId, activeWorkbenchWindow);
    }
    else
    {
      OpenPerspective(perspectiveId, activeWorkbenchWindow);
    }
  }
}

void ShowPerspectiveHandler::OpenPerspective(const QString& perspectiveId, IWorkbenchWindow* activeWorkbenchWindow)
{
  IWorkbench* workbench = activeWorkbenchWindow->GetWorkbench();

  IWorkbenchPage::Pointer activePage = activeWorkbenchWindow->GetActivePage();
  IPerspectiveDescriptor::Pointer desc = workbench->GetPerspectiveRegistry()->FindPerspectiveWithId(perspectiveId);
  if (desc.IsNull())
  {
    throw ExecutionException(QString("Perspective ") + perspectiveId + " cannot be found.");
  }

  try
  {
    if (activePage.IsNotNull())
    {
      activePage->SetPerspective(desc);
    }
    else
    {
      IAdaptable* input = dynamic_cast<Workbench*>(workbench)->GetDefaultPageInput();
      activeWorkbenchWindow->OpenPage(perspectiveId, input);
    }
  }
  catch (const WorkbenchException& e)
  {
    throw ExecutionException("Perspective could not be opened.", e);
  }
}

}
