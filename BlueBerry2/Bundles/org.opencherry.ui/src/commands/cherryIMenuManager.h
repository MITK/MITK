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


#ifndef CHERRYIMENUMANAGER_H_
#define CHERRYIMENUMANAGER_H_

#include "cherryIContributionItem.h"
#include "cherryIContributionManager.h"

#include "../cherryUiDll.h"

namespace cherry {

struct CHERRY_UI IMenuListener;

/**
 * The <code>IMenuManager</code> interface provides protocol for managing
 * contributions to a menu bar and its sub menus.
 * An <code>IMenuManager</code> is also an <code>IContributionItem</code>,
 * allowing sub-menus to be nested in parent menus.
 * <p>
 * This interface is internal to the framework; it should not be implemented outside
 * the framework.
 * </p>
 * <p>
 * This package provides a concrete menu manager implementation,
 * {@link MenuManager <code>MenuManager</code>}.
 * </p>
 * @noimplement This interface is not intended to be implemented by clients.
 */
struct CHERRY_UI IMenuManager : public virtual IContributionManager, public IContributionItem {

  osgiInterfaceMacro(cherry::IMenuManager);

    /**
     * Adds a menu listener to this menu.
     * Has no effect if an identical listener is already registered.
     *
     * @param listener a menu listener
     */
    virtual void AddMenuListener(SmartPointer<IMenuListener> listener) = 0;

    /**
     * Finds the manager for the menu at the given path. A path
     * consists of contribution item ids separated by the separator
     * character.  The path separator character is <code>'/'</code>.
     * <p>
     * Convenience for <code>findUsingPath(path)</code> which
     * extracts an <code>IMenuManager</code> if possible.
     * </p>
     *
     * @param path the path string
     * @return the menu contribution item, or <code>null</code>
     *   if there is no such contribution item or if the item does
     *   not have an associated menu manager
     */
    virtual SmartPointer<IMenuManager> FindMenuUsingPath(const std::string& path) = 0;

    /**
     * Finds the contribution item at the given path. A path
     * consists of contribution item ids separated by the separator
     * character. The path separator character is <code>'/'</code>.
     *
     * @param path the path string
     * @return the contribution item, or <code>null</code> if there is no
     *   such contribution item
     */
    virtual SmartPointer<IContributionItem> FindUsingPath(const std::string& path) = 0;

    /**
   * Returns whether all items should be removed when the menu is about to
   * show, but before notifying menu listeners. The default is
   * <code>false</code>.
   *
   * @return <code>true</code> if all items should be removed when shown,
   *         <code>false</code> if not
   */
    virtual bool GetRemoveAllWhenShown() = 0;

    /**
     * Returns whether this menu should be enabled or not.
     *
     * @return <code>true</code> if enabled, and
     *   <code>false</code> if disabled
     */
    virtual bool IsEnabled() = 0;

    /**
     * Removes the given menu listener from this menu.
     * Has no effect if an identical listener is not registered.
     *
     * @param listener the menu listener
     */
    virtual void RemoveMenuListener(SmartPointer<IMenuListener> listener) = 0;

    /**
   * Sets whether all items should be removed when the menu is about to show,
   * but before notifying menu listeners.
   *
   * @param removeAll
   *            <code>true</code> if all items should be removed when shown,
   *            <code>false</code> if not
   */
    virtual void SetRemoveAllWhenShown(bool removeAll) = 0;

    /**
     * Incrementally builds the menu from the contribution items, and
     * does so recursively for all submenus.
     *
     * @param force <code>true</code> means update even if not dirty,
     *   and <code>false</code> for normal incremental updating
     */
    virtual void UpdateAll(bool force) = 0;
};

}

#endif /* CHERRYIMENUMANAGER_H_ */
