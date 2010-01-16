/*=========================================================================

Program:   BlueBerry Platform
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

#ifndef BERRYIWORKBENCHWINDOWCONFIGURER_H_
#define BERRYIWORKBENCHWINDOWCONFIGURER_H_

#include <berryMacros.h>

#include "../berryUiDll.h"

#include "../berryShell.h"
#include "../berryIMemento.h"
#include "../berryPoint.h"

namespace berry
{

struct IWorkbenchConfigurer;
struct IWorkbenchWindow;

/**
 * Interface providing special access for configuring workbench windows.
 * <p>
 * Window configurer objects are in 1-1 correspondence with the workbench
 * windows they configure. Clients may use <code>get/setData</code> to
 * associate arbitrary state with the window configurer object.
 * </p>
 * <p>
 * Note that these objects are only available to the main application
 * (the plug-in that creates and owns the workbench).
 * </p>
 * <p>
 * This interface is not intended to be implemented by clients.
 * </p>
 *
 * @see IWorkbenchConfigurer#getWindowConfigurer
 * @see WorkbenchAdvisor#preWindowOpen
 * @since 3.0
 * @noimplement This interface is not intended to be implemented by clients.
 */
struct BERRY_UI IWorkbenchWindowConfigurer : public Object {

  berryInterfaceMacro(IWorkbenchWindowConfigurer, berry);

  /**
     * Returns the underlying workbench window.
     *
     * @return the workbench window
     */
    virtual SmartPointer<IWorkbenchWindow> GetWindow() = 0;

    /**
     * Returns the workbench configurer.
     *
     * @return the workbench configurer
     */
    virtual SmartPointer<IWorkbenchConfigurer> GetWorkbenchConfigurer() = 0;

    /**
     * Returns the action bar configurer for this workbench
     * window.
     *
     * @return the action bar configurer
     */
    //virtual IActionBarConfigurer GetActionBarConfigurer() = 0;

    /**
     * Returns the title of the underlying workbench window.
     *
     * @return the window title
     */
    virtual std::string GetTitle() = 0;

    /**
     * Sets the title of the underlying workbench window.
     *
     * @param title the window title
     */
    virtual void SetTitle(const std::string& title) = 0;

    /**
     * Returns whether the underlying workbench window has a menu bar.
     * <p>
     * The initial value is <code>true</code>.
     * </p>
     *
     * @return <code>true</code> for a menu bar, and <code>false</code>
     * for no menu bar
     */
    virtual bool GetShowMenuBar() = 0;

    /**
     * Sets whether the underlying workbench window has a menu bar.
     *
     * @param show <code>true</code> for a menu bar, and <code>false</code>
     * for no menu bar
     */
    virtual void SetShowMenuBar(bool show) = 0;

    /**
     * Returns whether the underlying workbench window has a cool bar.
     * <p>
     * The initial value is <code>true</code>.
     * </p>
     *
     * @return <code>true</code> for a cool bar, and <code>false</code>
     * for no cool bar
     */
    virtual bool GetShowCoolBar() = 0;

    /**
     * Sets whether the underlying workbench window has a cool bar.
     *
     * @param show <code>true</code> for a cool bar, and <code>false</code>
     * for no cool bar
     */
    virtual void SetShowCoolBar(bool show) = 0;

    /**
     * Returns whether the underlying workbench window has a status line.
     * <p>
     * The initial value is <code>true</code>.
     * </p>
     *
     * @return <code>true</code> for a status line, and <code>false</code>
     * for no status line
     */
    virtual bool GetShowStatusLine() = 0;

    /**
     * Sets whether the underlying workbench window has a status line.
     *
     * @param show <code>true</code> for a status line, and <code>false</code>
     * for no status line
     */
    virtual void SetShowStatusLine(bool show) = 0;

    /**
     * Returns whether the underlying workbench window has a perspective bar (the
     * perspective bar provides buttons to quickly switch between perspectives).
     * <p>
     * The initial value is <code>false</code>.
     * </p>
     *
     * @return <code>true</code> for a perspective bar, and <code>false</code>
     * for no perspective bar
     */
    virtual bool GetShowPerspectiveBar() = 0;

    /**
     * Sets whether the underlying workbench window has a perspective bar (the
     * perspective bar provides buttons to quickly switch between perspectives).
     *
     * @param show <code>true</code> for a perspective bar, and
     * <code>false</code> for no perspective bar
     */
    virtual void SetShowPerspectiveBar(bool show) = 0;

    /**
     * Returns whether the underlying workbench window has a progress indicator.
     * <p>
     * The initial value is <code>false</code>.
     * </p>
     *
     * @return <code>true</code> for a progress indicator, and <code>false</code>
     * for no progress indicator
     */
    virtual bool GetShowProgressIndicator() = 0;

    /**
     * Sets whether the underlying workbench window has a progress indicator.
     *
     * @param show <code>true</code> for a progress indicator, and <code>false</code>
     * for no progress indicator
     */
    virtual void SetShowProgressIndicator(bool show) = 0;

    /**
     * Returns the style bits to use for the window's shell when it is created.
     * The default is <code>SWT.SHELL_TRIM</code>.
     *
     * @return the shell style bits
     */
    virtual int GetShellStyle() = 0;

    /**
     * Sets the style bits to use for the window's shell when it is created.
     * This method has no effect after the shell is created.
     * That is, it must be called within the <code>preWindowOpen</code>
     * callback on <code>WorkbenchAdvisor</code>.
     * <p>
     * For more details on the applicable shell style bits, see the
     * documentation for {@link org.blueberry.swt.widgets.Shell}.
     * </p>
     *
     * @param shellStyle the shell style bits
     */
    virtual void SetShellStyle(int shellStyle) = 0;

    /**
     * Returns the size to use for the window's shell when it is created.
     *
     * @return the initial size to use for the shell
     */
    virtual Point GetInitialSize() = 0;

    /**
     * Sets the size to use for the window's shell when it is created.
     * This method has no effect after the shell is created.
     * That is, it must be called within the <code>preWindowOpen</code>
     * callback on <code>WorkbenchAdvisor</code>.
     *
     * @param initialSize the initial size to use for the shell
     */
    virtual void SetInitialSize(Point initialSize) = 0;

    /**
     * Returns the data associated with this workbench window at the given key.
     *
     * @param key the key
     * @return the data, or <code>null</code> if there is no data at the given
     * key
     */
    //virtual Object getData(String key);

    /**
     * Sets the data associated with this workbench window at the given key.
     *
     * @param key the key
     * @param data the data, or <code>null</code> to delete existing data
     */
    //virtual void setData(String key, Object data);

    /**
     * Adds the given drag and drop <code>Transfer</code> type to the ones
     * supported for drag and drop on the editor area of this workbench window.
     * <p>
     * The workbench advisor would ordinarily call this method from the
     * <code>preWindowOpen</code> callback.
     * A newly-created workbench window supports no drag and drop transfer
     * types. Adding <code>EditorInputTransfer.getInstance()</code>
     * enables <code>IEditorInput</code>s to be transferred.
     * </p>
     * <p>
     * Note that drag and drop to the editor area requires adding one or more
     * transfer types (using <code>addEditorAreaTransfer</code>) and
     * configuring a drop target listener
     * (with <code>configureEditorAreaDropListener</code>)
     * capable of handling any of those transfer types.
     * </p>
     *
     * @param transfer a drag and drop transfer object
     * @see #configureEditorAreaDropListener
     * @see org.blueberry.ui.part.EditorInputTransfer
     */
    //virtual void addEditorAreaTransfer(Transfer transfer);

    /**
     * Configures the drop target listener for the editor area of this workbench window.
     * <p>
     * The workbench advisor ordinarily calls this method from the
     * <code>preWindowOpen</code> callback.
     * A newly-created workbench window has no configured drop target listener for its
     * editor area.
     * </p>
     * <p>
     * Note that drag and drop to the editor area requires adding one or more
     * transfer types (using <code>addEditorAreaTransfer</code>) and
     * configuring a drop target listener
     * (with <code>configureEditorAreaDropListener</code>)
     * capable of handling any of those transfer types.
     * </p>
     *
     * @param dropTargetListener the drop target listener that will handle
     * requests to drop an object on to the editor area of this window
     *
     * @see #addEditorAreaTransfer
     */
    //virtual void configureEditorAreaDropListener(
    //        DropTargetListener dropTargetListener);


    /**
     * Creates the menu bar for the window's shell.
     * <p>
     * This should only be called if the advisor is defining custom window contents
     * in <code>createWindowContents</code>, and may only be called once.
     * The caller must set it in the shell using <code>Shell.setMenuBar(Menu)</code>
     * but must not make add, remove or change items in the result.
     * The menu bar is populated by the window's menu manager.
     * The application can add to the menu manager in the advisor's
     * <code>fillActionBars</code> method instead.
     * </p>
     *
     * @return the menu bar, suitable for setting in the shell
     */
    //virtual Menu createMenuBar();

    /**
     * Creates the cool bar control.
     * <p>
     * This should only be called if the advisor is defining custom window contents
     * in <code>createWindowContents</code>, and may only be called once.
     * The caller must lay out the cool bar appropriately within the parent,
     * but must not add, remove or change items in the result (hence the
     * return type of <code>Control</code>).
     * The cool bar is populated by the window's cool bar manager.
     * The application can add to the cool bar manager in the advisor's
     * <code>fillActionBars</code> method instead.
     * </p>
     *
     * @param parent the parent composite
     * @return the cool bar control, suitable for laying out in the parent
     */
    //virtual Control createCoolBarControl(Composite parent);

    /**
     * Creates the status line control.
     * <p>
     * This should only be called if the advisor is defining custom window contents
     * in <code>createWindowContents</code>, and may only be called once.
     * The caller must lay out the status line appropriately within the parent,
     * but must not add, remove or change items in the result (hence the
     * return type of <code>Control</code>).
     * The status line is populated by the window's status line manager.
     * The application can add to the status line manager in the advisor's
     * <code>fillActionBars</code> method instead.
     * </p>
     *
     * @param parent the parent composite
     * @return the status line control, suitable for laying out in the parent
     */
    //virtual Control createStatusLineControl(Composite parent);

    /**
     * Creates the page composite, in which the window's pages, and their
     * views and editors, appear.
     * <p>
     * This should only be called if the advisor is defining custom window contents
     * in <code>createWindowContents</code>, and may only be called once.
     * The caller must lay out the page composite appropriately within the parent,
     * but must not add, remove or change items in the result (hence the
     * return type of <code>Control</code>).
     * The page composite is populated by the workbench.
     * </p>
     *
     * @param parent the parent composite
     * @return the page composite, suitable for laying out in the parent
     */
    virtual void* CreatePageComposite(void* parent) = 0;

  /**
   * Saves the current state of the window using the specified memento.
   *
   * @param memento the memento in which to save the window's state
   * @return a status object indicating whether the save was successful
     * @see IWorkbenchConfigurer#restoreWorkbenchWindow(IMemento)
   * @since 3.1
   */
  virtual bool SaveState(IMemento::Pointer memento) = 0;
};

}

#endif /*BERRYIWORKBENCHWINDOWCONFIGURER_H_*/
