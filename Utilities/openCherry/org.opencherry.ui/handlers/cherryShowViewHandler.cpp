/*=========================================================================
 
Program:   openCherry Platform
Language:  C++
Date:      $Date$
Version:   $Revision$
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include "cherryShowViewHandler.h"
#include "cherryHandlerUtil.h"

#include "../cherryUIException.h"
#include "../cherryWorkbench.h"
#include "../cherryIWorkbenchPage.h"
#include "../cherryIViewDescriptor.h"
#include "../cherryPlatformUI.h"

#include "dialogs/cherryIShowViewDialog.h"

#include <org.opencherry.osgi/cherryObjectString.h>
#include <org.opencherry.core.commands/common/cherryCommandExceptions.h>

#include <vector>

namespace cherry
{

const std::string ShowViewHandler::PARAMETER_NAME_VIEW_ID = "org.opencherry.ui.showView.viewId"; //$NON-NLS-1$

ShowViewHandler::ShowViewHandler()
{
}

Object::Pointer ShowViewHandler::Execute(
    const ExecutionEvent::Pointer event)
{
  IWorkbenchWindow::Pointer window = HandlerUtil::GetActiveWorkbenchWindowChecked(event);
  // Get the view identifier, if any.
  const ExecutionEvent::ParameterMap& parameters = event->GetParameters();
  ExecutionEvent::ParameterMap::const_iterator result = parameters.find(PARAMETER_NAME_VIEW_ID);
  std::string value;
  if (result != parameters.end()) value = result->second;

  if (value == "")
  {
    this->OpenOther(window);
  }
  else
  {
    try
    {
      //std::string viewId(value.Cast<ObjectString>()->c_str());
      this->OpenView(value, window);
    }
    catch (PartInitException e)
    {
      throw ExecutionException("Part could not be initialized", e); //$NON-NLS-1$
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

  IShowViewDialog::Pointer dialog = PlatformUI::GetWorkbench().Cast<Workbench>()->CreateStandardDialog(Workbench::DIALOG_ID_SHOW_VIEW).Cast<IShowViewDialog>();
  if (dialog.IsNull()) return;
  
  int returnCode = dialog->Exec();

  if (returnCode == IDialog::CANCEL)
  {
    return;
  }

  const std::vector<IViewDescriptor::Pointer> descriptors =
      dialog.Cast<IShowViewDialog>()->GetSelection();
  for (unsigned int i = 0; i < descriptors.size(); ++i)
  {
    try
    {
      this->OpenView(descriptors[i]->GetId(), window);
    }
    catch (PartInitException e)
    {
//      StatusUtil.handleStatus(e.getStatus(),
//          WorkbenchMessages.ShowView_errorTitle
//          + ": " + e.getMessage(), //$NON-NLS-1$
//          StatusManager.SHOW);
    }
  }
}

void ShowViewHandler::OpenView(const std::string& viewId, IWorkbenchWindow::Pointer activeWorkbenchWindow)
{
  const IWorkbenchPage::Pointer activePage = activeWorkbenchWindow->GetActivePage();
  if (activePage.IsNull())
  {
    return;
  }

  activePage->ShowView(viewId);
}

}
