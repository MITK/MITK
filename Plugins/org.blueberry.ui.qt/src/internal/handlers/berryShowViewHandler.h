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

#ifndef BERRYSHOWVIEWHANDLER_H_
#define BERRYSHOWVIEWHANDLER_H_

#include <berryAbstractHandler.h>
#include <berryExecutionEvent.h>

#include "berryIWorkbenchWindow.h"

namespace berry
{

/**
 * \ingroup org_blueberry_ui_qt
 *
 * Shows the given view. If no view is specified in the parameters, then this
 * opens the view selection dialog.
 */
class ShowViewHandler : public AbstractHandler
{
  Q_OBJECT

public:
  berryObjectMacro(ShowViewHandler)

  /**
   * Creates a new ShowViewHandler that will open the view in its default location.
   */
  ShowViewHandler();

  Object::Pointer Execute(const ExecutionEvent::ConstPointer& event);

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
   * @param secondaryId
   *            An optional secondary id; may be <code>null</code>
   * @throws PartInitException
   *             If the part could not be initialized.
   */
  void OpenView(const QString& viewId, const QString& secondaryId, IWorkbenchWindow::Pointer activeWorkbenchWindow);
};

}

#endif /*BERRYSHOWVIEWHANDLER_H_*/
