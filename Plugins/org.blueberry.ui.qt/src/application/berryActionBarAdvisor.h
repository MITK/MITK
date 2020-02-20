/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYACTIONBARADVISOR_H_
#define BERRYACTIONBARADVISOR_H_

#include <berryObject.h>

#include <org_blueberry_ui_qt_Export.h>

class QAction;

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
 * to berry::PlatformUI::CreateAndRunWorkbench()):
 * <ul>
 * <li>FillActionBars() - called after berry::WorkbenchWindowAdvisor::PreWindowOpen()
 * to configure a window's action bars</li>
 * </ul>
 * </p>
 *
 * @see berry::WorkbenchWindowAdvisor::CreateActionBarAdvisor()
 */
class BERRY_UI_QT ActionBarAdvisor : public Object
{

public:

  berryObjectMacro(berry::ActionBarAdvisor);

  enum FillType {
    /**
     * Bit flag for FillActionBars() indicating that the
     * operation is not filling the action bars of an actual workbench window,
     * but rather a proxy (used for perspective customization).
     */
    FILL_PROXY = 0x01,

    /**
     * Bit flag for FillActionBars() indicating that the
     * operation is supposed to fill (or describe) the workbench window's menu
     * bar.
     */
    FILL_MENU_BAR = 0x02,

    /**
     * Bit flag for FillActionBars() indicating that the
     * operation is supposed to fill (or describe) the workbench window's cool
     * bar.
     */
    FILL_TOOL_BAR = 0x04,

    /**
     * Bit flag for FillActionBars() indicating that the
     * operation is supposed to fill (or describe) the workbench window's status
     * line.
     */
    FILL_STATUS_LINE = 0x08
  };
  Q_DECLARE_FLAGS(FillFlags, FillType)

public:

  ~ActionBarAdvisor() override;

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
   * #FILL_PROXY, meaning this is a request to fill the action
   * bars of the corresponding workbench window; the
   * remaining flags indicate which combination of
   * the menu bar (#FILL_MENU_BAR),
   * the tool bar (#FILL_TOOL_BAR),
   * and the status line (#FILL_STATUS_LINE) are to be filled.
   * <p>
   * If <code>flags</code> does include #FILL_PROXY, then this
   * is a request to describe the actions bars of the given workbench window
   * (which will already have been filled);
   * again, the remaining flags indicate which combination of the menu bar,
   * the tool bar, and the status line are to be described.
   * The actions included in the proxy action bars can be the same instances
   * as in the actual window's action bars.
   * </p>
   * <p>
   * This method is called just after berry::WorkbenchWindowAdvisor::PreWindowOpen().
   * Clients must not call this method directly (although super calls are okay).
   * The default implementation calls MakeActions() if
   * #FILL_PROXY is specified, then calls FillMenuBar(),
   * FillToolBar(), and FillStatusLine()
   * if the corresponding flags are specified.
   * </p>
   * <p>
   * Subclasses may override, but it is recommended that they override the
   * methods mentioned above instead.
   * </p>
   *
   * @param flags bit mask composed from the constants
   * #FILL_MENU_BAR,
   * #FILL_TOOL_BAR,
   * #FILL_STATUS_LINE,
   * and #FILL_PROXY
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

  using Object::Register;

protected:

  /**
   * Returns the action bar configurer.
   *
   * @return the action bar configurer
   */
  virtual SmartPointer<IActionBarConfigurer> GetActionBarConfigurer() const;

  /**
   * Instantiates the actions used in the fill methods.
   * Use Register() to add it to the list of actions to
   * be disposed when the window is closed.
   *
   * @param window the window containing the action bars
   */
  virtual void MakeActions(IWorkbenchWindow* window);

  /**
   * Adds the given action to the list of actions to be disposed when the window is closed.
   *
   * @param action the action to register, this cannot be <code>null</code>
   * @param id the unique action id
   */
  virtual void Register(QAction* action, const QString& id);

  /**
   * Returns the action with the given id, or <code>null</code> if not found.
   *
   * @param id the action id
   * @return the action with the given id, or <code>null</code> if not found
   */
  virtual QAction* GetAction(const QString& id) const;

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

  QHash<QString, QAction*> actions;

};

}

Q_DECLARE_OPERATORS_FOR_FLAGS(berry::ActionBarAdvisor::FillFlags)

#endif /*BERRYACTIONBARADVISOR_H_*/
