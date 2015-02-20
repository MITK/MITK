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


#ifndef BERRYIACTIONBARS_H
#define BERRYIACTIONBARS_H

#include <berryObject.h>

namespace berry {

struct IServiceLocator;
struct IMenuManager;
struct IStatusLineManager;
struct IToolBarManager;

class Action;

/**
 * Used by a part to access its menu, toolbar, and status line managers.
 * <p>
 * Within the workbench each part, editor or view, has a private set of action
 * bars.  This set, which contains a menu, toolbar, and status line, appears
 * in the local toolbar for a view and in the window for an editor.  The view
 * may provide an implementation for pre-existing actions or add new actions to
 * the action bars.
 * </p><p>
 * In a workbench window there are a number of actions which are applicable to
 * all parts.  Some common examples are <code>CUT</code>, <code>COPY</code> and
 * <code>PASTE</code>. These actions, known as "global actions", are contributed to
 * the workbench window by the window itself and shared by all parts.  The
 * presentation is owned by the window.  The implementation is delegated to the
 * active part.
 * </p><p>
 * To participate in the global action design an <code>IWorkbenchPart</code> should
 * register a handler for each global action which is implemented by the part.  This
 * can be done by calling <code>setGlobalActionHandler</code>.  For convenience, the
 * standard global actions are defined in
 * <code>IWorkbenchActionConstants</code>.
 * </p><p>
 * Additional work is required for the <code>Delete</code> global action.  In
 * this case the accelerator is defined in the menu item text but is not hooked
 * on the window.  This is to support text editors where the <code>Delete</code>
 * key is functional even when the <code>Delete</code> action is disabled (no text
 * is selected).  An implementation for this accelerator must be defined locally,
 * in each part, by listening for <code>Delete</code> key events.
 * </p><p>
 * A part may also contribute new actions to the action bars as required.  To do
 * this, call <code>getMenuManager</code>, <code>getToolBarManager</code>, or
 * <code>getStatusLineManager</code> as appropriate to get the action target.
 * Add the action(s) to the target and call <code>update</code> to commit
 * any changes to the underlying widgets.
 * </p><p>
 * This interface is not intended to be implemented by clients.
 * </p>
 * @noimplement This interface is not intended to be implemented by clients.
 */
struct IActionBars : public Object
{
  berryObjectMacro(berry::IActionBars)

  /**
   * Clears the global action handler list.
   * <p>
   * Note: Clients who manipulate the global action list are
   * responsible for calling <code>updateActionBars</code> so that the changes
   * can be propagated throughout the workbench.
   * </p>
   */
  virtual void ClearGlobalActionHandlers() = 0;

  /**
   * Returns the global action handler for the action with the given id.
   *
   * @param actionId an action id declared in the registry
   * @return an action handler which implements the action id, or
   *   <code>null</code> if none is registered
   * @see IWorkbenchActionConstants
   * @see #setGlobalActionHandler(String, IAction)
   */
  virtual Action* GetGlobalActionHandler(const QString& actionId) const = 0;

  /**
   * Returns the menu manager.
   * <p>
   * Note: Clients who add or remove items from the returned menu manager are
   * responsible for calling <code>updateActionBars</code> so that the changes
   * can be propagated throughout the workbench.
   * </p>
   *
   * @return the menu manager
   */
  virtual IMenuManager* GetMenuManager() = 0;

  /**
   * Returns the service locator for these action bars. The locator is found
   * by looking locally, and then ascending the action bar hierarchy.
   *
   * @return The service locator; never <code>null</code>.
   */
  virtual IServiceLocator* GetServiceLocator() = 0;

  /**
   * Returns the status line manager.
   * <p>
   * Note: Clients who add or remove items from the returned status line
   * manager are responsible for calling <code>updateActionBars</code> so
   * that the changes can be propagated throughout the workbench.
   * </p>
   *
   * @return the status line manager
   */
  virtual IStatusLineManager* GetStatusLineManager() = 0;

  /**
   * Returns the tool bar manager.
   * <p>
   * Note: Clients who add or remove items from the returned tool bar manager are
   * responsible for calling <code>updateActionBars</code> so that the changes
   * can be propagated throughout the workbench.
   * </p>
   *
   * @return the tool bar manager
   */
  virtual IToolBarManager* GetToolBarManager() = 0;

  /**
   * Sets the global action handler for the action with the given id.
   * <p>
   * Note: Clients who manipulate the global action list are
   * responsible for calling <code>updateActionBars</code> so that the changes
   * can be propagated throughout the workbench.
   * </p>
   *
   * @param actionId an action id declared in the registry
   * @param handler an action which implements the action id, or
   *  <code>null</code> to clear any existing handler
   * @see IWorkbenchActionConstants
   */
  virtual void SetGlobalActionHandler(const QString& actionId, Action* handler) = 0;

  /**
   * Updates the action bars.
   * <p>
   * Clients who add or remove items from the menu, tool bar, or status line
   * managers, or that update global action handlers, should call this method
   * to propagated the changes throughout the workbench.
   * </p>
   *
   * @see #setGlobalActionHandler(String, IAction)
   * @see #clearGlobalActionHandlers()
   */
  virtual void UpdateActionBars() = 0;
};

}

#endif // BERRYIACTIONBARS_H
