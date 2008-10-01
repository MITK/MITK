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


#ifndef CHERRYWINDOWMANAGER_H_
#define CHERRYWINDOWMANAGER_H_

#include <vector>

#include <cherrySmartPointer.h>
#include <list>

namespace cherry
{

class Window;

/**
 * A manager for a group of windows. Window managers are an optional JFace
 * feature used in applications which create many different windows (dialogs,
 * wizards, etc.) in addition to a main window. A window manager can be used to
 * remember all the windows that an application has created (independent of
 * whether they are presently open or closed). There can be several window
 * managers, and they can be arranged into a tree. This kind of organization
 * makes it simple to close whole subgroupings of windows.
 * <p>
 * Creating a window manager is as simple as creating an instance of
 * <code>WindowManager</code>. Associating a window with a window manager is
 * done with <code>WindowManager.add(Window)</code>. A window is automatically
 * removed from its window manager as a side effect of closing the window.
 * </p>
 *
 * @see Window
 */
class WindowManager {

private:

    /**
     * List of windows managed by this window manager
     * (element type: <code>Window</code>).
     */
    std::vector<SmartPointer<Window> > windows;

    /**
     * List of window managers who have this window manager
     * as their parent (element type: <code>WindowManager</code>).
     */
    std::list<WindowManager*> subManagers;

    /**
     * Adds the given window manager to the list of
     * window managers that have this one as a parent.
     * </p>
     * @param wm the child window manager
     */
    void AddWindowManager(WindowManager* wm);


public:

    /**
     * Creates an empty window manager without a parent window
     * manager (that is, a root window manager).
     */
    WindowManager();

    /**
     * Creates an empty window manager with the given
     * window manager as parent.
     *
     * @param parent the parent window manager
     */
    WindowManager(WindowManager* parent);

    /**
     * Adds the given window to the set of windows managed by
     * this window manager. Does nothing is this window is
     * already managed by this window manager.
     *
     * @param window the window
     */
    void Add(SmartPointer<Window> window);

    /**
     * Attempts to close all windows managed by this window manager,
     * as well as windows managed by any descendent window managers.
     *
     * @return <code>true</code> if all windows were sucessfully closed,
     * and <code>false</code> if any window refused to close
     */
    bool Close();

    /**
     * Returns this window manager's number of windows
     *
     * @return the number of windows
     * @since 3.0
     */
    int GetWindowCount();

    /**
     * Returns this window manager's set of windows.
     *
     * @return a possibly empty list of window
     */
    std::vector<SmartPointer<Window> > GetWindows();

    /**
     * Removes the given window from the set of windows managed by
     * this window manager. Does nothing is this window is
     * not managed by this window manager.
     *
     * @param window the window
     */
    void Remove(SmartPointer<Window> window);
};

}

#endif /* CHERRYWINDOWMANAGER_H_ */
