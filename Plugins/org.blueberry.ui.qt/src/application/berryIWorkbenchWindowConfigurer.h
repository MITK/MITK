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

#ifndef BERRYIWORKBENCHWINDOWCONFIGURER_H_
#define BERRYIWORKBENCHWINDOWCONFIGURER_H_

#include <berryObject.h>

#include <org_blueberry_ui_qt_Export.h>

class QMenuBar;

namespace berry
{

struct IActionBarConfigurer;
struct IDropTargetListener;
struct IMemento;
struct IWorkbenchConfigurer;
struct IWorkbenchWindow;

/**
 * Interface providing special access for configuring workbench windows.
 * <p>
 * %Window configurer objects are in 1-1 correspondence with the workbench
 * windows they configure. Clients may use <code>Get/SetData</code> to
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
 * @see IWorkbenchConfigurer#GetWindowConfigurer()
 * @see WorkbenchAdvisor#PreWindowOpen()
 * @note This interface is not intended to be implemented by clients.
 */
struct BERRY_UI_QT IWorkbenchWindowConfigurer : public Object
{

  berryObjectMacro(berry::IWorkbenchWindowConfigurer)

  ~IWorkbenchWindowConfigurer();

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
  virtual SmartPointer<IActionBarConfigurer> GetActionBarConfigurer() = 0;

  /**
   * Returns the title of the underlying workbench window.
   *
   * @return the window title
   */
  virtual QString GetTitle() = 0;

  /**
   * Sets the title of the underlying workbench window.
   *
   * @param title the window title
   */
  virtual void SetTitle(const QString& title) = 0;

  /**
   * Returns whether the underlying workbench window has a menu bar.
   * <p>
   * The initial value is <code>true</code>.
   * </p>
   *
   * @return <code>true</code> for a menu bar, and <code>false</code>
   * for no menu bar
   */
  virtual bool GetShowMenuBar() const = 0;

  /**
   * Sets whether the underlying workbench window has a menu bar.
   *
   * @param show <code>true</code> for a menu bar, and <code>false</code>
   * for no menu bar
   */
  virtual void SetShowMenuBar(bool show) = 0;

  /**
   * Returns whether the underlying workbench window has a tool bar.
   * <p>
   * The initial value is <code>true</code>.
   * </p>
   *
   * @return <code>true</code> for a tool bar, and <code>false</code>
   * for no tool bar
   */
  virtual bool GetShowToolBar() const = 0;

  /**
   * Sets whether the underlying workbench window has a tool bar.
   *
   * @param show <code>true</code> for a tool bar, and <code>false</code>
   * for no tool bar
   */
  virtual void SetShowToolBar(bool show) = 0;

  /**
   * Returns whether the underlying workbench window has a status line.
   * <p>
   * The initial value is <code>true</code>.
   * </p>
   *
   * @return <code>true</code> for a status line, and <code>false</code>
   * for no status line
   */
  virtual bool GetShowStatusLine() const = 0;

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
  virtual bool GetShowPerspectiveBar() const = 0;

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
  virtual bool GetShowProgressIndicator() const = 0;

  /**
   * Sets whether the underlying workbench window has a progress indicator.
   *
   * @param show <code>true</code> for a progress indicator, and <code>false</code>
   * for no progress indicator
   */
  virtual void SetShowProgressIndicator(bool show) = 0;

  /**
   * Returns the style bits to use for the window's main widget when it is created.
   * The default is <code>0</code>.
   *
   * @return the style bits
   */
  virtual Qt::WindowFlags GetWindowFlags() const = 0;

  /**
   * Sets the style bits to use for the window's main widget when it is created.
   * This method has no effect after the widget is created.
   * That is, it must be called within the <code>WorkbenchAdvisor#PreWindowOpen()</code>
   * callback.
   * <p>
   * For more details on the applicable style bits, see the
   * documentation for Qt::WindowFlags.
   * </p>
   *
   * @param windowFlags the style bits
   */
  virtual void SetWindowFlags(Qt::WindowFlags windowFlags) = 0;

  /**
   * Returns the size to use for the window's shell when it is created.
   *
   * @return the initial size to use for the shell
   */
  virtual QPoint GetInitialSize() const = 0;

  /**
   * Sets the size to use for the window's shell when it is created.
   * This method has no effect after the shell is created.
   * That is, it must be called within the <code>WorkbenchAdvisor#PreWindowOpen()</code>
   * callback.
   *
   * @param initialSize the initial size to use for the shell
   */
  virtual void SetInitialSize(QPoint initialSize) = 0;

  /*
   * Returns the data associated with this workbench window at the given key.
   *
   * @param key the key
   * @return the data, or <code>null</code> if there is no data at the given
   * key
   */
  //virtual Object getData(String key);

  /*
   * Sets the data associated with this workbench window at the given key.
   *
   * @param key the key
   * @param data the data, or <code>null</code> to delete existing data
   */
  //virtual void setData(String key, Object data);

  /**
   * Adds the given drag and drop Mime types to the ones
   * supported for drag and drop on the editor area of this workbench window.
   * <p>
   * The workbench advisor would ordinarily call this method from the
   * <code>PreWindowOpen</code> callback.
   * A newly-created workbench window supports no drag and drop transfer
   * types.
   * </p>
   * <p>
   * Note that drag and drop to the editor area requires adding one or more
   * transfer types (using <code>AddEditorAreaTransfer</code>) and
   * configuring a drop target listener
   * (with <code>ConfigureEditorAreaDropListener</code>)
   * capable of handling any of those transfer types.
   * </p>
   *
   * @param transfer a drag and drop transfer object
   * @see #configureEditorAreaDropListener
   * @see org.blueberry.ui.part.EditorInputTransfer
   */
  virtual void AddEditorAreaTransfer(const QStringList& transferTypes) = 0;

  /**
   * Configures the drop target listener for the editor area of this workbench window.
   * <p>
   * The workbench advisor ordinarily calls this method from the
   * <code>PreWindowOpen</code> callback.
   * A newly-created workbench window has no configured drop target listener for its
   * editor area.
   * </p>
   * <p>
   * Note that drag and drop to the editor area requires adding one or more
   * transfer types (using <code>AddEditorAreaTransfer</code>) and
   * configuring a drop target listener
   * (with <code>ConfigureEditorAreaDropListener</code>)
   * capable of handling any of those transfer types.
   * </p>
   *
   * @param dropTargetListener the drop target listener that will handle
   * requests to drop an object on to the editor area of this window
   *
   * @see #AddEditorAreaTransfer
   */
  virtual void ConfigureEditorAreaDropListener(IDropTargetListener* dropTargetListener) = 0;

  /**
   * Creates the menu bar for the window's shell.
   * <p>
   * This should only be called if the advisor is defining custom window contents
   * in <code>CreateWindowContents</code>, and may only be called once.
   * The caller must set it in the shell using <code>Shell.setMenuBar(Menu)</code>
   * but must not make add, remove or change items in the result.
   * The menu bar is populated by the window's menu manager.
   * The application can add to the menu manager in the advisor's
   * <code>FillActionBars</code> method instead.
   * </p>
   *
   * @return the menu bar, suitable for setting in the shell
   */
  virtual QMenuBar* CreateMenuBar() = 0;

  /**
   * Creates the tool bar control.
   * <p>
   * This should only be called if the advisor is defining custom window contents
   * in <code>CreateWindowContents</code>, and may only be called once.
   * The caller must lay out the tool bar appropriately within the parent,
   * but must not add, remove or change items in the result (hence the
   * return type of <code>QWidget</code>).
   * The tool bar is populated by the window's tool bar manager.
   * The application can add to the tool bar manager in the advisor's
   * <code>FillActionBars</code> method instead.
   * </p>
   *
   * @param parent the parent widget
   * @return the tool bar control, suitable for laying out in the parent
   */
  virtual QWidget* CreateToolBar(QWidget* parent) = 0;

  /*
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
   * in <code>WorkbenchWindowAdvisor#CreateWindowContents()</code>, and may only be called once.
   * The caller must lay out the page composite appropriately within the parent,
   * but must not add, remove or change items in the result.
   * The page composite is populated by the workbench.
   * </p>
   *
   * @param parent the parent composite
   * @return the page composite, suitable for laying out in the parent
   */
  virtual QWidget* CreatePageComposite(QWidget* parent) = 0;

  /**
   * Saves the current state of the window using the specified memento.
   *
   * @param memento the memento in which to save the window's state
   * @return a status object indicating whether the save was successful
   * @see IWorkbenchConfigurer#RestoreWorkbenchWindow(IMemento::Pointer)
   */
  virtual bool SaveState(SmartPointer<IMemento> memento) = 0;
};

}

#endif /*BERRYIWORKBENCHWINDOWCONFIGURER_H_*/
