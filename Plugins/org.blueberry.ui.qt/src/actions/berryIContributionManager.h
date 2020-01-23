/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYICONTRIBUTIONMANAGER_H
#define BERRYICONTRIBUTIONMANAGER_H

#include <berryMacros.h>
#include <berryObject.h>

class QAction;

namespace berry {

struct IContributionItem;
struct IContributionManagerOverrides;

/**
 * A contribution manager organizes contributions to such UI components
 * as menus, toolbars and status lines.
 * <p>
 * A contribution manager keeps track of a list of contribution
 * items. Each contribution item may has an optional identifier, which can be used
 * to retrieve items from a manager, and for positioning items relative to
 * each other. The list of contribution items can be subdivided into named groups
 * using special contribution items that serve as group markers.
 * </p>
 * <p>
 * The <code>IContributionManager</code> interface provides general
 * protocol for adding, removing, and retrieving contribution items.
 * It also provides convenience methods that make it convenient
 * to contribute actions. This interface should be implemented
 * by all objects that wish to manage contributions.
 * </p>
 * <p>
 * There are several implementions of this interface in this package,
 * including ones for menus ({@link MenuManager <code>MenuManager</code>}),
 * tool bars ({@link ToolBarManager <code>ToolBarManager</code>}),
 * and status lines ({@link StatusLineManager <code>StatusLineManager</code>}).
 * </p>
 */
struct IContributionManager : public virtual Object
{

  berryObjectMacro(berry::IContributionManager);

  /**
   * Adds an action as a contribution item to this manager.
   * Equivalent to <code>Add(IContributionItem::Pointer(new QActionContributionItem(action, id)))</code>.
   *
   * @param action the action, this cannot be <code>null</code>
   * @param id the unique action id
   */
  virtual void Add(QAction* action, const QString& id) = 0;

  /**
   * Adds a contribution item to this manager.
   *
   * @param item the contribution item, this cannot be <code>null</code>
   */
  virtual void Add(const SmartPointer<IContributionItem>& item) = 0;

  /**
   * Adds a contribution item for the given action at the end of the group
   * with the given name.
   * Equivalent to
   * <code>AppendToGroup(groupName,IContributionItem::Pointer(new QActionContributionItem(action, id)))</code>.
   *
   * @param groupName the name of the group
   * @param action the action
   * @param id the unique action id
   * @exception ctkInvalidArgumentException if there is no group with
   *   the given name
   */
  virtual void AppendToGroup(const QString& groupName, QAction* action, const QString& id) = 0;

  /**
   * Adds a contribution item to this manager at the end of the group
   * with the given name.
   *
   * @param groupName the name of the group
   * @param item the contribution item
   * @exception ctkInvalidArgumentException if there is no group with
   *   the given name
   */
  virtual void AppendToGroup(const QString& groupName, const SmartPointer<IContributionItem>& item) = 0;

  /**
   * Finds the contribution item with the given id.
   *
   * @param id the contribution item id
   * @return the contribution item, or <code>null</code> if
   *   no item with the given id can be found
   */
  virtual SmartPointer<IContributionItem> Find(const QString& id) const = 0;

  /**
     * Returns all contribution items known to this manager.
     *
     * @return a list of contribution items
     */
  virtual QList<SmartPointer<IContributionItem> > GetItems() const = 0;

  /**
   * Returns the overrides for the items of this manager.
   *
   * @return the overrides for the items of this manager
   */
  virtual SmartPointer<IContributionManagerOverrides> GetOverrides() = 0;

  /**
   * Inserts a contribution item after the item with the given id.
   *
   * @param id the contribution item id
   * @param item the contribution item to insert
   * @exception IllegalArgumentException if there is no item with
   *   the given id
   */
  virtual void InsertAfter(const QString& id, const SmartPointer<IContributionItem>& item) = 0;

  /**
   * Inserts a contribution item before the item with the given id.
   *
   * @param id the contribution item id
   * @param item the contribution item to insert
   * @exception IllegalArgumentException if there is no item with
   *   the given id
   */
  virtual void InsertBefore(const QString& id, const SmartPointer<IContributionItem>& item) = 0;

  /**
   * Returns whether the list of contributions has recently changed and
   * has yet to be reflected in the corresponding widgets.
   *
   * @return <code>true</code> if this manager is dirty, and <code>false</code>
   *   if it is up-to-date
   */
  virtual bool IsDirty() const = 0;

  /**
   * Returns whether this manager has any contribution items.
   *
   * @return <code>true</code> if there are no items, and
   *   <code>false</code> otherwise
   */
  virtual bool IsEmpty() const = 0;

  /**
   * Marks this contribution manager as dirty.
   */
  virtual void MarkDirty() = 0;

  /**
   * Adds a contribution item to this manager at the beginning of the
   * group with the given name.
   *
   * @param groupName the name of the group
   * @param item the contribution item
   * @exception IllegalArgumentException if there is no group with
   *   the given name
   */
  virtual void PrependToGroup(const QString& groupName, const SmartPointer<IContributionItem>& item) = 0;

  /**
   * Removes and returns the contribution item with the given id from this manager.
   * Returns <code>null</code> if this manager has no contribution items
   * with the given id.
   *
   * @param id the contribution item id
   * @return the item that was found and removed, or <code>null</code> if none
   */
  virtual SmartPointer<IContributionItem> Remove(const QString& id) = 0;

  /**
   * Removes the given contribution item from the contribution items
   * known to this manager.
   *
   * @param item the contribution item
   * @return the <code>item</code> parameter if the item was removed,
   *   and <code>null</code> if it was not found
   */
  virtual SmartPointer<IContributionItem> Remove(const SmartPointer<IContributionItem>& item) = 0;

  /**
   * Removes all contribution items from this manager.
   */
  virtual void RemoveAll() = 0;

  /**
   * Updates this manager's underlying widget(s) with any changes which
   * have been made to it or its items.  Normally changes to a contribution
   * manager merely mark it as dirty, without updating the underlying widgets.
   * This brings the underlying widgets up to date with any changes.
   *
   * @param force <code>true</code> means update even if not dirty,
   *   and <code>false</code> for normal incremental updating
   */
  virtual void Update(bool force) = 0;
};

}

#endif // BERRYICONTRIBUTIONMANAGER_H
