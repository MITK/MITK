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

#ifndef CHERRYSHOWVIEWHANDLER_H_
#define CHERRYSHOWVIEWHANDLER_H_

#include <cherryAbstractHandler.h>
#include <cherryExecutionEvent.h>

#include "../cherryIWorkbenchWindow.h"
#include "../cherryUiDll.h"

namespace cherry
{

/**
 * \ingroup org_opencherry_ui
 * 
 * Shows the given view. If no view is specified in the parameters, then this
 * opens the view selection dialog.
 * 
 * @since 3.1
 */
class CHERRY_UI ShowViewHandler : public AbstractHandler
{

public:
  cherryClassMacro(ShowViewHandler)

private:

  /**
   * The name of the parameter providing the view identifier.
   */
  static const std::string PARAMETER_NAME_VIEW_ID;

public:

  /**
   * Creates a new ShowViewHandler that will open the view in its default location.
   */
  ShowViewHandler();

  Object::Pointer Execute(
      const ExecutionEvent::Pointer event);

private:

  /**
   * Opens a view selection dialog, allowing the user to chose a view.
   */
  void OpenOther(IWorkbenchWindow::Pointer window);

  /**
   * Opens the view with the given identifier.
   * 
   * @param viewId
   *            The view to open; must not be <code>null</code>
   * @throws PartInitException
   *             If the part could not be initialized.
   */
  void OpenView(const std::string& viewId, IWorkbenchWindow::Pointer activeWorkbenchWindow);
};

}

#endif /*CHERRYSHOWVIEWHANDLER_H_*/
