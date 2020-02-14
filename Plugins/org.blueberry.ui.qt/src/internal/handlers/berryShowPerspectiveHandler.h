/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYSHOWPERSPECTIVEHANDLER_H
#define BERRYSHOWPERSPECTIVEHANDLER_H

#include <berryAbstractHandler.h>
#include <berryExecutionEvent.h>

namespace berry {

struct IWorkbenchWindow;

/**
 * Shows the given perspective. If no perspective is specified in the
 * parameters, then this opens the perspective selection dialog.
 */
class ShowPerspectiveHandler : public AbstractHandler
{
  Q_OBJECT

public:

  berryObjectMacro(ShowPerspectiveHandler);

  Object::Pointer Execute(const ExecutionEvent::ConstPointer& event) override;

private:

  /**
   * Opens the specified perspective in a new window.
   *
   * @param perspectiveId
   *            The perspective to open; must not be <code>null</code>
   * @throws ExecutionException
   *             If the perspective could not be opened.
   */
  void OpenNewWindowPerspective(const QString& perspectiveId,
                                const IWorkbenchWindow* activeWorkbenchWindow);

  /**
   * Opens a view selection dialog, allowing the user to chose a view.
   *
   * @throws ExecutionException
   *             If the perspective could not be opened.
   */
  void OpenOther(IWorkbenchWindow* activeWorkbenchWindow);

  /**
   * Opens the perspective with the given identifier.
   *
   * @param perspectiveId
   *            The perspective to open; must not be <code>null</code>
   * @throws ExecutionException
   *             If the perspective could not be opened.
   */
  void OpenPerspective(const QString& perspectiveId,
                       IWorkbenchWindow* activeWorkbenchWindow);
};

}

#endif // BERRYSHOWPERSPECTIVEHANDLER_H
