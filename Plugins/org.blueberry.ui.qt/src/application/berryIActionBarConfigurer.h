/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYIACTIONBARCONFIGURER_H_
#define BERRYIACTIONBARCONFIGURER_H_

#include <berryObject.h>

#include <org_blueberry_ui_qt_Export.h>


namespace berry
{

struct IWorkbenchWindowConfigurer;
struct IMenuManager;
struct IToolBarManager;

/**
 * Interface providing special access for configuring the action bars
 * of a workbench window.
 * <p>
 * Note that these objects are only available to the main application
 * (the plug-in that creates and owns the workbench).
 * </p>
 * <p>
 * This interface is not intended to be implemented by clients.
 * </p>
 *
 * @see org.blueberry.ui.application.WorkbenchAdvisor#fillActionBars
 * @noimplement This interface is not intended to be implemented by clients.
 */
struct BERRY_UI_QT IActionBarConfigurer : public Object
{

  berryObjectMacro(berry::IActionBarConfigurer);

  ~IActionBarConfigurer() override;

  /**
   * Returns the workbench window configurer for the window
   * containing this configurer's action bars.
   *
   * @return the workbench window configurer
   */
  virtual SmartPointer<IWorkbenchWindowConfigurer> GetWindowConfigurer() = 0;

  /**
   * Creates a menu manager for the main menu bar of a workbench window. The
   * action bar advisor should use this factory method rather than creating a
   * <code>MenuManager</code> directly.
   *
   * @return the menu manager
   */
  virtual IMenuManager* GetMenuManager() = 0;

  /**
   * Creates a tool bar manager for the workbench window's tool bar. The
   * action bar advisor should use this factory method rather than creating a
   * <code>ToolBarManager</code> directly.
   *
   * @return the tool bar manager
   */
  virtual IToolBarManager* GetToolBarManager() = 0;

  /*
   * Returns the status line manager of a workbench window.
   *
   * @return the status line manager
   */
  //virtual IStatusLineManager GetStatusLineManager() = 0;

  /*
   * Register the action as a global action with a workbench
   * window.
   * <p>
   * For a workbench retarget action
   * ({@link org.blueberry.ui.actions.RetargetAction RetargetAction})
   * to work, it must be registered.
   * You should also register actions that will participate
   * in custom key bindings.
   * </p>
   *
   * @param action the global action
   * @see org.blueberry.ui.actions.RetargetAction
   */
  //virtual void RegisterGlobalAction(IAction action) = 0;

};

}

#endif /*BERRYIACTIONBARCONFIGURER_H_*/
