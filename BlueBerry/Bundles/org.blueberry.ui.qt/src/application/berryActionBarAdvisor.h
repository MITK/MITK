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

#ifndef BERRYACTIONBARADVISOR_H_
#define BERRYACTIONBARADVISOR_H_

#include <berryObject.h>

#include <org_blueberry_ui_qt_Export.h>

namespace berry
{

struct IMenuManager;
struct IToolBarManager;
struct IStatusLineManager;
struct IActionBarConfigurer;
struct IMemento;
struct IWorkbenchWindow;

/**
 * Public base class for configuring the action bars of a workbench window.
 * <p>
 * An application should declare a subclass of <code>ActionBarAdvisor</code>
 * and override methods to configure a window's action bars to suit the needs of the
 * particular application.
 * </p>
 * <p>
 * The following advisor methods are called at strategic points in the
 * workbench's lifecycle (all occur within the dynamic scope of the call
 * to {@link PlatformUI#createAndRunWorkbench PlatformUI.createAndRunWorkbench}):
 * <ul>
 * <li><code>fillActionBars</code> - called after <code>WorkbenchWindowAdvisor.preWindowOpen</code>
 * to configure a window's action bars</li>
 * </ul>
 * </p>
 *
 * @see WorkbenchWindowAdvisor#createActionBarAdvisor(IActionBarConfigurer)
 */
class BERRY_UI_QT ActionBarAdvisor : public Object
{

public:

  berryObjectMacro(berry::ActionBarAdvisor)

  enum FillType {
    /**
     * Bit flag for {@link #fillActionBars fillActionBars} indicating that the
     * operation is not filling the action bars of an actual workbench window,
     * but rather a proxy (used for perspective customization).
     */
    FILL_PROXY = 0x01,

    /**
     * Bit flag for {@link #fillActionBars fillActionBars} indicating that the
     * operation is supposed to fill (or describe) the workbench window's menu
     * bar.
     */
    FILL_MENU_BAR = 0x02,

    /**
     * Bit flag for {@link #fillActionBars fillActionBars} indicating that the
     * operation is supposed to fill (or describe) the workbench window's cool
     * bar.
     */
    FILL_TOOL_BAR = 0x04,

    /**
     * Bit flag for {@link #fillActionBars fillActionBars} indicating that the
     * operation is supposed to fill (or describe) the workbench window's status
     * line.
     */
    FILL_STATUS_LINE = 0x08
  };
  Q_DECLARE_FLAGS(FillFlags, FillType)

public:

  /**
   * Creates a new action bar advisor to configure a workbench
   * window's action bars via the given action bar configurer.
   *
   * @param configurer the action bar configurer
   */
  ActionBarAdvisor(const SmartPointer<IActionBarConfigurer>& configurer);

  /**
   * Configures the action bars using the given action bar configurer.
   * Under normal circumstances, <code>flags</code> does not include
   * <code>FILL_PROXY</code>, meaning this is a request to fill the action
   * bars of the corresponding workbench window; the
   * remaining flags indicate which combination of
   * the menu bar (<code>FILL_MENU_BAR</code>),
   * the tool bar (<code>FILL_COOL_BAR</code>),
   * and the status line (<code>FILL_STATUS_LINE</code>) are to be filled.
   * <p>
   * If <code>flags</code> does include <code>FILL_PROXY</code>, then this
   * is a request to describe the actions bars of the given workbench window
   * (which will already have been filled);
   * again, the remaining flags indicate which combination of the menu bar,
   * the tool bar, and the status line are to be described.
   * The actions included in the proxy action bars can be the same instances
   * as in the actual window's action bars. Calling <code>ActionFactory</code>
   * to create new action instances is not recommended, because these
   * actions internally register listeners with the window and there is no
   * opportunity to dispose of these actions.
   * </p>
   * <p>
   * This method is called just after {@link WorkbenchWindowAdvisor#preWindowOpen()}.
   * Clients must not call this method directly (although super calls are okay).
   * The default implementation calls <code>makeActions</code> if
   * <code>FILL_PROXY</code> is specified, then calls <code>fillMenuBar</code>,
   * <code>fillCoolBar</code>, and <code>fillStatusLine</code>
   * if the corresponding flags are specified.
   * </p>
   * <p>
   * Subclasses may override, but it is recommended that they override the
   * methods mentioned above instead.
   * </p>
   *
   * @param flags bit mask composed from the constants
   * {@link #FILL_MENU_BAR FILL_MENU_BAR},
   * {@link #FILL_COOL_BAR FILL_COOL_BAR},
   * {@link #FILL_STATUS_LINE FILL_STATUS_LINE},
   * and {@link #FILL_PROXY FILL_PROXY}
   */
  virtual void FillActionBars(FillFlags flags);

  /**
   * Saves arbitrary application-specific state information
   * for this action bar advisor.
   * <p>
   * The default implementation simply returns an OK status.
   * Subclasses may extend or override.
   * </p>
   *
   * @param memento the memento in which to save the advisor's state
   * @return a status object indicating whether the save was successful
   */
  virtual bool SaveState(SmartPointer<IMemento> memento);

  /**
   * Restores arbitrary application-specific state information
   * for this action bar advisor.
   * <p>
   * The default implementation simply returns an OK status.
   * Subclasses may extend or override.
   * </p>
   *
   * @param memento the memento from which to restore the advisor's state
   * @return a status object indicating whether the restore was successful
   */
  public: virtual bool RestoreState(SmartPointer<IMemento> memento);

protected:

  /**
   * Returns the action bar configurer.
   *
   * @return the action bar configurer
   */
  virtual SmartPointer<IActionBarConfigurer> GetActionBarConfigurer() const;

  /**
   * Instantiates the actions used in the fill methods.
   * Use {@link #register(IAction)} to register the action with the key binding service
   * and add it to the list of actions to be disposed when the window is closed.
   *
   * @param window the window containing the action bars
   */
  virtual void MakeActions(IWorkbenchWindow* window);

  /*
   * Registers the given action with the key binding service
   * (by calling {@link IActionBarConfigurer#registerGlobalAction(IAction)}),
   * and adds it to the list of actions to be disposed when the window is closed.
   * <p>
   * In order to participate in key bindings, the action must have an action
   * definition id (aka command id), and a corresponding command extension.
   * See the <code>org.blueberry.ui.commands</code> extension point documentation
   * for more details.
   * </p>
   *
   * @param action the action to register, this cannot be <code>null</code>
   *
   * @see IAction#setActionDefinitionId(String)
   * @see #disposeAction(IAction)
   */
//    protected: virtual void Register(IAction action) {
//      Assert.isNotNull(action, "Action must not be null"); //$NON-NLS-1$
//        String id = action.getId();
//        Assert.isNotNull(id, "Action must not have null id"); //$NON-NLS-1$
//        getActionBarConfigurer().registerGlobalAction(action);
//        actions.put(id, action);
//    }

  /*
   * Returns the action with the given id, or <code>null</code> if not found.
   *
   * @param id the action id
   * @return the action with the given id, or <code>null</code> if not found
   * @see IAction#getId()
   */
//    protected: virtual IAction GetAction(const QString& id) {
//        return (IAction) actions.get(id);
//    }

  /**
   * Fills the menu bar with the main menus for the window.
   * <p>
   * The default implementation does nothing.
   * Subclasses may override.
   * </p>
   *
   * @param menuBar the menu manager for the menu bar
   */
  virtual void FillMenuBar(IMenuManager* menuBar);

  /**
   * Fills the tool bar with the main toolbars for the window.
   * <p>
   * The default implementation does nothing.
   * Subclasses may override.
   * </p>
   *
   * @param toolBar the  bar manager
   */
  virtual void FillToolBar(IToolBarManager* toolBar);

  /**
   * Fills the status line with the main status line contributions
   * for the window.
   * <p>
   * The default implementation does nothing.
   * Subclasses may override.
   * </p>
   *
   * @param statusLine the status line manager
   */
  virtual void FillStatusLine(IStatusLineManager* statusLine);

private:

  SmartPointer<IActionBarConfigurer> actionBarConfigurer;

  //private: Map actions = new HashMap();

};

}

Q_DECLARE_OPERATORS_FOR_FLAGS(berry::ActionBarAdvisor::FillFlags)

#endif /*BERRYACTIONBARADVISOR_H_*/
