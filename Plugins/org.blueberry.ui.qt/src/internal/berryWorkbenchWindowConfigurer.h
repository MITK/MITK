/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYWORKBENCHWINDOWCONFIGURER_H_
#define BERRYWORKBENCHWINDOWCONFIGURER_H_

#include "application/berryIActionBarConfigurer.h"
#include "application/berryIWorkbenchConfigurer.h"
#include "application/berryIWorkbenchWindowConfigurer.h"

#include <berryWeakPointer.h>

#include <QSet>
#include <QPoint>

class QToolBar;
class QMenuBar;

namespace berry
{

class WorkbenchWindow;

/**
 * Internal class providing special access for configuring workbench windows.
 * <p>
 * Note that these objects are only available to the main application
 * (the plug-in that creates and owns the workbench).
 * </p>
 * <p>
 * This class is not intended to be instantiated or subclassed by clients.
 * </p>
 */
class WorkbenchWindowConfigurer : public IWorkbenchWindowConfigurer
{

  friend class WorkbenchWindow;

public:

  berryObjectMacro(WorkbenchWindowConfigurer);

  /**
   * Creates a new workbench window configurer.
   * <p>
   * This method is declared package-private:. Clients obtain instances
   * via {@link WorkbenchAdvisor#getWindowConfigurer
   * WorkbenchAdvisor.getWindowConfigurer}
   * </p>
   *
   * @param window the workbench window that this object configures
   * @see WorkbenchAdvisor#getWindowConfigurer
   */
  WorkbenchWindowConfigurer(const SmartPointer<WorkbenchWindow>& window);

  /*
   * @see org.blueberry.ui.application.IWorkbenchWindowConfigurer#getWindow
   */
  SmartPointer<IWorkbenchWindow> GetWindow() override;

  /*
   * @see org.blueberry.ui.application.IWorkbenchWindowConfigurer#getWorkbenchConfigurer()
   */
  SmartPointer<IWorkbenchConfigurer> GetWorkbenchConfigurer() override;

  /*
   * @see org.blueberry.ui.application.IWorkbenchWindowConfigurer#getTitle
   */
  QString GetTitle() override;

  /*
   * @see org.blueberry.ui.application.IWorkbenchWindowConfigurer#setTitle
   */
  void SetTitle(const QString& title) override;

  /*
   * @see org.blueberry.ui.application.IWorkbenchWindowConfigurer#getShowMenuBar
   */
  bool GetShowMenuBar() const override;

  /*
   * @see org.blueberry.ui.application.IWorkbenchWindowConfigurer#setShowMenuBar
   */
  void SetShowMenuBar(bool show) override;

  /*
   * @see org.blueberry.ui.application.IWorkbenchWindowConfigurer#getShowToolBar
   */
  bool GetShowToolBar() const override;

  /*
   * @see org.blueberry.ui.application.IWorkbenchWindowConfigurer
   */
  void SetShowToolBar(bool show) override;

  /*
   * @see org.blueberry.ui.application.IWorkbenchWindowConfigurer
   */
  bool GetShowPerspectiveBar() const override;

  /*
   * @see org.blueberry.ui.application.IWorkbenchWindowConfigurer
   */
  void SetShowPerspectiveBar(bool show) override;

  /*
   * @see org.blueberry.ui.application.IWorkbenchWindowConfigurer#getShowStatusLine
   */
  bool GetShowStatusLine() const override;

  /*
   * @see org.blueberry.ui.application.IWorkbenchWindowConfigurer#setShowStatusLine
   */
  void SetShowStatusLine(bool show) override;

  /*
   * @see org.blueberry.ui.application.IWorkbenchWindowConfigurer
   */
  bool GetShowProgressIndicator() const override;

  /*
   * @see org.blueberry.ui.application.IWorkbenchWindowConfigurer
   */
  void SetShowProgressIndicator(bool show) override;

  /*
   * @see org.blueberry.ui.application.IWorkbenchWindowConfigurer#getData
   */
//  Object getData(String key) {
//        if (key == null) {
//            throw new IllegalArgumentException();
//        }
//        return extraData.get(key);
//    }

  /*
   * @see org.blueberry.ui.application.IWorkbenchWindowConfigurer#setData
   */
// void setData(String key, Object data) {
//        if (key == null) {
//            throw new IllegalArgumentException();
//        }
//        if (data != null) {
//            extraData.put(key, data);
//        } else {
//            extraData.remove(key);
//        }
//    }

  /*
   * @see org.blueberry.ui.application.IWorkbenchWindowConfigurer#addEditorAreaTransfer
   */
  void AddEditorAreaTransfer(const QStringList& transferTypes) override;

  /*
   * @see org.blueberry.ui.application.IWorkbenchWindowConfigurer
   */
  void ConfigureEditorAreaDropListener(IDropTargetListener* listener) override;

  /**
   * Returns the array of <code>Transfer</code> added by the application
   */
  QStringList GetTransfers() const;

  /**
   * Returns the drop listener provided by the application.
   */
  IDropTargetListener* GetDropTargetListener() const;

  /*
   * @see org.blueberry.ui.application.IWorkbenchWindowConfigurer
   */
  SmartPointer<IActionBarConfigurer> GetActionBarConfigurer() override;

  /*
   * @see org.blueberry.ui.application.IWorkbenchWindowConfigurer
   */
  Qt::WindowFlags GetWindowFlags() const override;

  /*
   * @see org.blueberry.ui.application.IWorkbenchWindowConfigurer
   */
  void SetWindowFlags(Qt::WindowFlags shellStyle) override;

  /*
   * @see org.blueberry.ui.application.IWorkbenchWindowConfigurer
   */
  QPoint GetInitialSize() const override;

  /*
   * @see org.blueberry.ui.application.IWorkbenchWindowConfigurer
   */
  void SetInitialSize(QPoint size) override;

  /**
   * Creates the default window contents.
   *
   * @param shell the shell
   */
  void CreateDefaultContents(SmartPointer<Shell> shell);

  /*
   * @see org.blueberry.ui.application.IWorkbenchWindowConfigurer
   */
  QMenuBar* CreateMenuBar() override;

  /*
   * @see org.blueberry.ui.application.IWorkbenchWindowConfigurer
   */
  QWidget* CreateToolBar(QWidget* parent) override;

  /*
   * @see org.blueberry.ui.application.IWorkbenchWindowConfigurer
   */
//    public: Control createStatusLineControl(Composite parent) {
//        return window.getStatusLineManager().createControl(parent);
//    }

  /*
   * @see org.blueberry.ui.application.IWorkbenchWindowConfigurer
   */
  QWidget* CreatePageComposite(QWidget* parent) override;

  /*
   * @see org.blueberry.ui.application.IWorkbenchWindowConfigurer#saveState(org.blueberry.ui.IMemento)
   */
  bool SaveState(SmartPointer<IMemento> memento) override;

protected:

  /**
   * Returns the title as set by <code>setTitle</code>, without consulting the shell.
   *
   * @return the window title as set, or <code>null</code> if not set
   */
  QString BasicGetTitle();

private:

  /**
   * Action bar configurer that changes this workbench window.
   * This implementation keeps track of tool bar items
   */
  class WindowActionBarConfigurer : public IActionBarConfigurer
  {

  private:

    IActionBarConfigurer::Pointer proxy;
    WeakPointer<WorkbenchWindow> window;

  public:

    berryObjectMacro(WindowActionBarConfigurer);

    WindowActionBarConfigurer(WeakPointer<WorkbenchWindow> wnd);

    /**
     * Sets the proxy to use, or <code>null</code> for none.
     *
     * @param proxy the proxy
     */
    void SetProxy(IActionBarConfigurer::Pointer proxy);

    /*
     * @see org.blueberry.ui.application.IActionBarConfigurer#getWindowConfigurer()
     */
    IWorkbenchWindowConfigurer::Pointer GetWindowConfigurer() override;

      /*
       * @see org.blueberry.ui.application.IActionBarConfigurer
       */
  //        public: IStatusLineManager getStatusLineManager() {
  //            if (proxy != null) {
  //                return proxy.getStatusLineManager();
  //            }
  //            return window.getStatusLineManager();
  //        }

    /*
     * @see org.blueberry.ui.application.IActionBarConfigurer
     */
    IMenuManager* GetMenuManager() override;

    /*
     * @see org.blueberry.ui.internal.AbstractActionBarConfigurer
     */
    IToolBarManager* GetToolBarManager() override;

    /*
     * @see org.blueberry.ui.application.IActionBarConfigurer
     */
  //        public: void registerGlobalAction(IAction action) {
  //            if (proxy != null) {
  //                proxy.registerGlobalAction(action);
  //            }
  //            window.registerGlobalAction(action);
  //        }



  /*
   * @see org.blueberry.ui.application.IActionBarConfigurer#createToolBarManager()
   */
  //    public: IToolBarManager createToolBarManager() {
  //      if (proxy != null) {
  //        return proxy.createToolBarManager();
  //      }
  //      return getActionBarPresentationFactory().createToolBarManager();
  //    }

  };

  /**
   * The workbench window associated with this configurer.
   */
  WeakPointer<WorkbenchWindow> window;

  /**
   * The shell style bits to use when the window's shell is being created.
   */
  Qt::WindowFlags shellStyle;

  /**
   * The window title to set when the window's shell has been created.
   */
  QString windowTitle;

  /*
   * Whether the workbench window should show the fast view bars.
   */
  //bool showFastViewBars = false;

  /**
   * Whether the workbench window should show the perspective bar
   */
  bool showPerspectiveBar;

  /**
   * Whether the workbench window should show the status line.
   */
  bool showStatusLine;

  /**
   * Whether the workbench window should show the main tool bar.
   */
  bool showToolBar;

  /**
   * Whether the workbench window should show the main menu bar.
   */
  bool showMenuBar;

  /**
   * Whether the workbench window should have a progress indicator.
   */
  bool showProgressIndicator;

  /*
   * Table to hold arbitrary key-data settings (key type: <code>String</code>,
   * value type: <code>Object</code>).
   * @see #setData
   */
  //Map extraData = new HashMap(1);

  /**
   * Holds the list drag and drop <code>Transfer</code> for the
   * editor area
   */
  QSet<QString> transferTypes;

  /**
   * The <code>DropTargetListener</code> implementation for handling a
   * drop into the editor area.
   */
  IDropTargetListener* dropTargetListener;

  /**
   * The initial size to use for the shell.
   */
  QPoint initialSize;

  /**
   * Object for configuring this workbench window's action bars.
   * Lazily initialized to an instance unique to this window.
   */
  SmartPointer<WindowActionBarConfigurer> actionBarConfigurer;

};

}

#endif /*BERRYWORKBENCHWINDOWCONFIGURER_H_*/
