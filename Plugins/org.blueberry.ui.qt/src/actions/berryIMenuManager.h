/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYIMENUMANAGER_H
#define BERRYIMENUMANAGER_H

#include <berryIContributionManager.h>
#include <berryIContributionItem.h>

namespace berry {

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
struct IMenuManager : public virtual IContributionManager, public IContributionItem
{

  berryObjectMacro(berry::IMenuManager);

  /**
   * Adds a menu listener to this menu.
   * Has no effect if an identical listener is already registered.
   *
   * @param listener a menu listener
   */
  virtual void AddMenuListener(QObject* listener) = 0;

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
  virtual IMenuManager::Pointer FindMenuUsingPath(const QString& path) const = 0;

  /**
   * Finds the contribution item at the given path. A path
   * consists of contribution item ids separated by the separator
   * character. The path separator character is <code>'/'</code>.
   *
   * @param path the path string
   * @return the contribution item, or <code>null</code> if there is no
   *   such contribution item
   */
  virtual IContributionItem::Pointer FindUsingPath(const QString& path) const = 0;

  /**
   * Returns whether all items should be removed when the menu is about to
   * show, but before notifying menu listeners. The default is
   * <code>false</code>.
   *
   * @return <code>true</code> if all items should be removed when shown,
   *         <code>false</code> if not
   */
  virtual bool GetRemoveAllWhenShown() const = 0;

  /**
   * Returns whether this menu should be enabled or not.
   *
   * @return <code>true</code> if enabled, and
   *   <code>false</code> if disabled
   */
  bool IsEnabled() const override = 0;

  /**
   * Removes the given menu listener from this menu.
   * Has no effect if an identical listener is not registered.
   *
   * @param listener the menu listener
   */
  virtual void RemoveMenuListener(QObject* listener) = 0;

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

#endif // BERRYIMENUMANAGER_H
