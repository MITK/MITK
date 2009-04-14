/*=========================================================================

Program:   openCherry Platform
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

#ifndef CHERRYWORKBENCHWINDOWCONFIGURER_H_
#define CHERRYWORKBENCHWINDOWCONFIGURER_H_

#include "../application/cherryIActionBarConfigurer.h"
#include "../application/cherryIWorkbenchConfigurer.h"
#include "../application/cherryIWorkbenchWindowConfigurer.h"

#include <cherryWeakPointer.h>

namespace cherry
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
 *
 * @since 3.0
 */
class WorkbenchWindowConfigurer : public IWorkbenchWindowConfigurer {

public:
  cherryObjectMacro(WorkbenchWindowConfigurer);

    /**
     * The workbench window associated with this configurer.
     */
    private: WeakPointer<WorkbenchWindow> window;

    /**
     * The shell style bits to use when the window's shell is being created.
     */
    private: int shellStyle;

    /**
     * The window title to set when the window's shell has been created.
     */
    private: std::string windowTitle;

    /**
     * Whether the workbench window should show the fast view bars.
     */
    //private: boolean showFastViewBars = false;

    /**
     * Whether the workbench window should show the perspective bar
     */
    private: bool showPerspectiveBar;

    /**
     * Whether the workbench window should show the status line.
     */
    private: bool showStatusLine;

    /**
     * Whether the workbench window should show the main tool bar.
     */
    private: bool showToolBar;

    /**
     * Whether the workbench window should show the main menu bar.
     */
    private: bool showMenuBar;

    /**
     * Whether the workbench window should have a progress indicator.
     */
    private: bool showProgressIndicator;

    /**
     * Table to hold arbitrary key-data settings (key type: <code>String</code>,
     * value type: <code>Object</code>).
     * @see #setData
     */
    //private: Map extraData = new HashMap(1);

    /**
     * Holds the list drag and drop <code>Transfer</code> for the
     * editor area
     */
    //private: ArrayList transferTypes = new ArrayList(3);

    /**
     * The <code>DropTargetListener</code> implementation for handling a
     * drop into the editor area.
     */
    //private: DropTargetListener dropTargetListener = null;


    /**
     * The initial size to use for the shell.
     */
    private: Point initialSize;


    /**
     * Action bar configurer that changes this workbench window.
     * This implementation keeps track of of cool bar items
     */
    public:
    class WindowActionBarConfigurer : public IActionBarConfigurer {

        private: IActionBarConfigurer::Pointer proxy;
        private: WeakPointer<WorkbenchWindow> window;

        public: cherryObjectMacro(WindowActionBarConfigurer);

        public: WindowActionBarConfigurer(WeakPointer<WorkbenchWindow> wnd);

        /**
         * Sets the proxy to use, or <code>null</code> for none.
         *
         * @param proxy the proxy
         */
        public: void SetProxy(IActionBarConfigurer::Pointer proxy);

        /* (non-Javadoc)
         * @see org.opencherry.ui.application.IActionBarConfigurer#getWindowConfigurer()
         */
        public: IWorkbenchWindowConfigurer::Pointer GetWindowConfigurer();

        /**
         * Returns whether the given id is for a cool item.
         *
         * @param the item id
         * @return <code>true</code> if it is a cool item,
         * and <code>false</code> otherwise
         */
//        /* package */boolean containsCoolItem(String id) {
//            ICoolBarManager cbManager = getCoolBarManager();
//            if (cbManager == null) {
//        return false;
//      }
//            IContributionItem cbItem = cbManager.find(id);
//            if (cbItem == null) {
//        return false;
//      }
//            //@ issue: maybe we should check if cbItem is visible?
//            return true;
//        }

        /* (non-Javadoc)
         * @see org.opencherry.ui.application.IActionBarConfigurer
         */
//        public: IStatusLineManager getStatusLineManager() {
//            if (proxy != null) {
//                return proxy.getStatusLineManager();
//            }
//            return window.getStatusLineManager();
//        }

        /* (non-Javadoc)
         * @see org.opencherry.ui.application.IActionBarConfigurer
         */
        public: void* GetMenuManager();

        /* (non-Javadoc)
         * @see org.opencherry.ui.internal.AbstractActionBarConfigurer
         */
//        public: ICoolBarManager getCoolBarManager() {
//            if (proxy != null) {
//                return proxy.getCoolBarManager();
//            }
//            return window.getCoolBarManager2();
//        }

        /* (non-Javadoc)
         * @see org.opencherry.ui.application.IActionBarConfigurer
         */
//        public: void registerGlobalAction(IAction action) {
//            if (proxy != null) {
//                proxy.registerGlobalAction(action);
//            }
//            window.registerGlobalAction(action);
//        }



    /* (non-Javadoc)
     * @see org.opencherry.ui.application.IActionBarConfigurer#createToolBarManager()
     */
//    public: IToolBarManager createToolBarManager() {
//      if (proxy != null) {
//        return proxy.createToolBarManager();
//      }
//      return getActionBarPresentationFactory().createToolBarManager();
//    }

    /* (non-Javadoc)
     * @see org.opencherry.ui.application.IActionBarConfigurer#createToolBarContributionItem(org.opencherry.jface.action.IToolBarManager, java.lang.String)
     */
//    public: IToolBarContributionItem createToolBarContributionItem(IToolBarManager toolBarManager, String id) {
//      if (proxy != null) {
//        return proxy.createToolBarContributionItem(toolBarManager, id);
//      }
//      return getActionBarPresentationFactory().createToolBarContributionItem(toolBarManager, id);
//    }
    };

    /**
      * Object for configuring this workbench window's action bars.
      * Lazily initialized to an instance unique to this window.
      */
     private: WindowActionBarConfigurer::Pointer actionBarConfigurer;

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
     public: WorkbenchWindowConfigurer(SmartPointer<WorkbenchWindow> window);

    /* (non-javadoc)
     * @see org.opencherry.ui.application.IWorkbenchWindowConfigurer#getWindow
     */
    public: IWorkbenchWindow::Pointer GetWindow();

    /* (non-Javadoc)
     * @see org.opencherry.ui.application.IWorkbenchWindowConfigurer#getWorkbenchConfigurer()
     */
    public: IWorkbenchConfigurer::Pointer GetWorkbenchConfigurer();

    /**
     * Returns the title as set by <code>setTitle</code>, without consulting the shell.
     *
     * @return the window title as set, or <code>null</code> if not set
     */
    /* package */std::string BasicGetTitle();

    /* (non-javadoc)
     * @see org.opencherry.ui.application.IWorkbenchWindowConfigurer#getTitle
     */
    public: std::string GetTitle();

    /* (non-javadoc)
     * @see org.opencherry.ui.application.IWorkbenchWindowConfigurer#setTitle
     */
    public: void SetTitle(const std::string& title);

    /* (non-javadoc)
     * @see org.opencherry.ui.application.IWorkbenchWindowConfigurer#getShowMenuBar
     */
    public: bool GetShowMenuBar();

    /* (non-javadoc)
     * @see org.opencherry.ui.application.IWorkbenchWindowConfigurer#setShowMenuBar
     */
    public: void SetShowMenuBar(bool show);

    /* (non-javadoc)
     * @see org.opencherry.ui.application.IWorkbenchWindowConfigurer#getShowToolBar
     */
    public: bool GetShowCoolBar();

    /* (non-javadoc)
     * @see org.opencherry.ui.application.IWorkbenchWindowConfigurer
     */
    public: void SetShowCoolBar(bool show);

    /* (non-javadoc)
     * @see org.opencherry.ui.application.IWorkbenchWindowConfigurer
     */
    public: bool GetShowPerspectiveBar();

    /* (non-javadoc)
     * @see org.opencherry.ui.application.IWorkbenchWindowConfigurer
     */
    public: void SetShowPerspectiveBar(bool show);

    /* (non-javadoc)
     * @see org.opencherry.ui.application.IWorkbenchWindowConfigurer#getShowStatusLine
     */
    public: bool GetShowStatusLine();

    /* (non-javadoc)
     * @see org.opencherry.ui.application.IWorkbenchWindowConfigurer#setShowStatusLine
     */
    public: void SetShowStatusLine(bool show);

    /* (non-Javadoc)
     * @see org.opencherry.ui.application.IWorkbenchWindowConfigurer
     */
    public: bool GetShowProgressIndicator();

    /* (non-Javadoc)
     * @see org.opencherry.ui.application.IWorkbenchWindowConfigurer
     */
    public: void SetShowProgressIndicator(bool show);

    /* (non-javadoc)
     * @see org.opencherry.ui.application.IWorkbenchWindowConfigurer#getData
     */
//    public: Object getData(String key) {
//        if (key == null) {
//            throw new IllegalArgumentException();
//        }
//        return extraData.get(key);
//    }

    /* (non-javadoc)
     * @see org.opencherry.ui.application.IWorkbenchWindowConfigurer#setData
     */
//    public: void setData(String key, Object data) {
//        if (key == null) {
//            throw new IllegalArgumentException();
//        }
//        if (data != null) {
//            extraData.put(key, data);
//        } else {
//            extraData.remove(key);
//        }
//    }

    /* (non-Javadoc)
     * @see org.opencherry.ui.application.IWorkbenchWindowConfigurer#addEditorAreaTransfer
     */
//    public: void addEditorAreaTransfer(Transfer tranfer) {
//        if (tranfer != null && !transferTypes.contains(tranfer)) {
//            transferTypes.add(tranfer);
//            Transfer[] transfers = new Transfer[transferTypes.size()];
//            transferTypes.toArray(transfers);
//            IWorkbenchPage[] pages = window.getPages();
//            for (int i = 0; i < pages.length; i++) {
//                WorkbenchPage page = (WorkbenchPage) pages[i];
//                DropTarget dropTarget = ((EditorSashContainer) page
//                        .getEditorPresentation().getLayoutPart())
//                        .getDropTarget();
//                if (dropTarget != null) {
//                    dropTarget.setTransfer(transfers);
//                }
//            }
//        }
//    }

    /* (non-Javadoc)
     * @see org.opencherry.ui.application.IWorkbenchWindowConfigurer
     */
//    public: void configureEditorAreaDropListener(
//            DropTargetListener dropTargetListener) {
//        if (dropTargetListener != null) {
//            this.dropTargetListener = dropTargetListener;
//            IWorkbenchPage[] pages = window.getPages();
//            for (int i = 0; i < pages.length; i++) {
//                WorkbenchPage page = (WorkbenchPage) pages[i];
//                DropTarget dropTarget = ((EditorSashContainer) page
//                        .getEditorPresentation().getLayoutPart())
//                        .getDropTarget();
//                if (dropTarget != null) {
//                    dropTarget.addDropListener(this.dropTargetListener);
//                }
//            }
//        }
//    }

    /**
     * Returns the array of <code>Transfer</code> added by the application
     */
//    /* package */Transfer[] getTransfers() {
//        Transfer[] transfers = new Transfer[transferTypes.size()];
//        transferTypes.toArray(transfers);
//        return transfers;
//    }

    /**
     * Returns the drop listener provided by the application.
     */
//    /* package */DropTargetListener getDropTargetListener() {
//        return dropTargetListener;
//    }

    /* (non-javadoc)
     * @see org.opencherry.ui.application.IWorkbenchWindowConfigurer
     */
    public: IActionBarConfigurer::Pointer GetActionBarConfigurer();

    /**
     * Returns whether the given id is for a cool item.
     *
     * @param the item id
     * @return <code>true</code> if it is a cool item,
     * and <code>false</code> otherwise
     */
//    /* package */boolean containsCoolItem(String id) {
//        // trigger lazy initialization
//        getActionBarConfigurer();
//        return actionBarConfigurer.containsCoolItem(id);
//    }

    /* (non-Javadoc)
     * @see org.opencherry.ui.application.IWorkbenchWindowConfigurer
     */
    public: int GetShellStyle();

    /* (non-Javadoc)
     * @see org.opencherry.ui.application.IWorkbenchWindowConfigurer
     */
    public: void SetShellStyle(int shellStyle);

    /* (non-Javadoc)
     * @see org.opencherry.ui.application.IWorkbenchWindowConfigurer
     */
    public: Point GetInitialSize();

    /* (non-Javadoc)
     * @see org.opencherry.ui.application.IWorkbenchWindowConfigurer
     */
    public: void SetInitialSize(Point size);

    /**
     * Creates the default window contents.
     *
     * @param shell the shell
     */
    public: void CreateDefaultContents(Shell::Pointer shell);

    /* (non-Javadoc)
     * @see org.opencherry.ui.application.IWorkbenchWindowConfigurer
     */
//    public: Menu createMenuBar() {
//        return window.getMenuManager().createMenuBar(window.getShell());
//    }

    /* (non-Javadoc)
     * @see org.opencherry.ui.application.IWorkbenchWindowConfigurer
     */
//    public: Control createCoolBarControl(Composite parent) {
//      ICoolBarManager coolBarManager = window.getCoolBarManager2();
//      if (coolBarManager != null) {
//          if (coolBarManager instanceof ICoolBarManager2) {
//        return ((ICoolBarManager2) coolBarManager).createControl2(parent);
//      }
//          if (coolBarManager instanceof CoolBarManager) {
//        return ((CoolBarManager) coolBarManager).createControl(parent);
//      }
//        }
//        return null;
//    }

    /* (non-Javadoc)
     * @see org.opencherry.ui.application.IWorkbenchWindowConfigurer
     */
//    public: Control createStatusLineControl(Composite parent) {
//        return window.getStatusLineManager().createControl(parent);
//    }

    /* (non-Javadoc)
     * @see org.opencherry.ui.application.IWorkbenchWindowConfigurer
     */
    public: void* CreatePageComposite(void* parent);

  /* (non-Javadoc)
   * @see org.opencherry.ui.application.IWorkbenchWindowConfigurer#saveState(org.opencherry.ui.IMemento)
   */
  public: bool SaveState(IMemento::Pointer memento) ;

};

}

#endif /*CHERRYWORKBENCHWINDOWCONFIGURER_H_*/
