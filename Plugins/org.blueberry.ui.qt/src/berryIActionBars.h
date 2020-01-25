/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


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
 * A part may also contribute new actions to the action bars as required.  To do
 * this, call <code>GetMenuManager</code>, <code>GetToolBarManager</code>, or
 * <code>GetStatusLineManager</code> as appropriate to get the action target.
 * Add the action(s) to the target and call <code>update</code> to commit
 * any changes to the underlying widgets.
 * </p><p>
 * This interface is not intended to be implemented by clients.
 * </p>
 * @noimplement This interface is not intended to be implemented by clients.
 */
struct IActionBars : public Object
{
  berryObjectMacro(berry::IActionBars);

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
   * Updates the action bars.
   * <p>
   * Clients who add or remove items from the menu, tool bar, or status line
   * managers, or that update global action handlers, should call this method
   * to propagated the changes throughout the workbench.
   * </p>
   */
  virtual void UpdateActionBars() = 0;
};

}

#endif // BERRYIACTIONBARS_H
