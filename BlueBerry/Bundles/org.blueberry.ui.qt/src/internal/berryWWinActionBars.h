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


#ifndef BERRYWWINACTIONBARS_H
#define BERRYWWINACTIONBARS_H

#include <berryIActionBars.h>

namespace berry {

class WorkbenchWindow;

class WWinActionBars : public IActionBars
{

private:

  WorkbenchWindow* window;

public:

  /**
   * PerspActionBars constructor comment.
   */
  WWinActionBars(WorkbenchWindow* window);

  /**
   * Clears the global action handler list.
   */
  void ClearGlobalActionHandlers();

  /**
   * Get the handler for a window action.
   *
   * @param actionID an action ID declared in the registry
   * @return an action handler which implements the action ID, or
   *    <code>null</code> if none is registered.
   */
  Action* GetGlobalActionHandler(const QString& actionID) const;

  /**
   * Returns the menu manager.  If items are added or
   * removed from the manager be sure to call <code>updateActionBars</code>.
   *
   * @return the menu manager
   */
  IMenuManager* GetMenuManager();

  IServiceLocator* GetServiceLocator();

  /**
   * Returns the status line manager.  If items are added or
   * removed from the manager be sure to call <code>updateActionBars</code>.
   *
   * @return the status line manager
   */
  IStatusLineManager* GetStatusLineManager();

  /**
   * Returns the tool bar manager.
   *
   */
  IToolBarManager* GetToolBarManager();

  /**
   * Add a handler for a window action.
   *
   * The standard action ID's for the workbench are defined in
   * <code>IWorkbenchActions</code>.
   *
   * @see IWorkbenchActionConstants
   *
   * @param actionID an action ID declared in the registry
   * @param handler an action which implements the action ID.
   *    <code>null</code> may be passed to deregister a handler.
   */
  void SetGlobalActionHandler(const QString& actionID, Action* handler);

  /**
   * Commits all UI changes.  This should be called
   * after additions or subtractions have been made to a
   * menu, status line, or toolbar.
   */
  void UpdateActionBars();
};

}

#endif // BERRYWWINACTIONBARS_H
