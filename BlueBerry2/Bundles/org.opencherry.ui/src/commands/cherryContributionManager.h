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

#ifndef CHERRYCONTRIBUTIONMANAGER_H_
#define CHERRYCONTRIBUTIONMANAGER_H_

#include <deque>

#include "cherryIContributionManager.h"
#include "cherryIContributionManagerOverrides.h"

namespace cherry
{

/**
 * Abstract base class for all contribution managers, and standard
 * implementation of <code>IContributionManager</code>. This class provides
 * functionality common across the specific managers defined by this framework.
 * <p>
 * This class maintains a list of contribution items and a dirty flag, both as
 * internal state. In addition to providing implementations of most
 * <code>IContributionManager</code> methods, this class automatically
 * coalesces adjacent separators, hides beginning and ending separators, and
 * deals with dynamically changing sets of contributions. When the set of
 * contributions does change dynamically, the changes are propagated to the
 * control via the <code>update</code> method, which subclasses must
 * implement.
 * </p>
 * <p>
 * Note: A <code>ContributionItem</code> cannot be shared between different
 * <code>ContributionManager</code>s.
 * </p>
 */
class CHERRY_UI ContributionManager: public virtual IContributionManager
{

  // Internal debug flag.
  // protected static final boolean DEBUG = false;

private:

  typedef std::deque<SmartPointer<IContributionItem> > IContributionContainer;
  /**
   * The list of contribution items.
   */
  IContributionContainer contributions;

  /**
   * Indicates whether the widgets are in sync with the contributions.
   */
  bool isDirty;

  /**
   * Number of dynamic contribution items.
   */
  int dynamicItems;

  /**
   * The overrides for items of this manager
   */
  SmartPointer<IContributionManagerOverrides> overrides;

  struct ContributionManagerOverrides: public IContributionManagerOverrides
  {
    ObjectBool::Pointer GetEnabled(SmartPointer<IContributionItem> item);

    ObjectInt::Pointer GetAccelerator(SmartPointer<IContributionItem> item);

    ObjectString::Pointer GetAcceleratorText(SmartPointer<IContributionItem> item);

    ObjectString::Pointer GetText(SmartPointer<IContributionItem> item);
  };

protected:

  /**
   * Creates a new contribution manager.
   */
  ContributionManager();

  /**
   * This method allows subclasses of <code>ContributionManager</code> to
   * prevent certain items in the contributions list.
   * <code>ContributionManager</code> will either block or allow an addition
   * based on the result of this method call. This can be used to prevent
   * duplication, for example.
   *
   * @param itemToAdd
   *            The contribution item to be added; may be <code>null</code>.
   * @return <code>true</code> if the addition should be allowed;
   *         <code>false</code> otherwise. The default implementation allows
   *         all items.
   * @since 3.0
   */
  bool AllowItem(SmartPointer<IContributionItem> itemToAdd);

  /**
   * Internal debug method for printing statistics about this manager to
   * <code>System.out</code>.
   */
  void DumpStatistics();

  /**
   * Returns whether this contribution manager contains dynamic items. A
   * dynamic contribution item contributes items conditionally, dependent on
   * some internal state.
   *
   * @return <code>true</code> if this manager contains dynamic items, and
   *         <code>false</code> otherwise
   */
  bool HasDynamicItems();

  IContributionContainer::iterator FindIter(const std::string& id);

  /**
   * Returns the index of the object in the internal structure. This is
   * different from <code>indexOf(String id)</code> since some contribution
   * items may not have an id.
   *
   * @param item
   *            The contribution item
   * @return the index, or -1 if the item is not found
   * @since 3.0
   */
  int IndexOf(SmartPointer<IContributionItem> item);

  /**
   * The given item was added to the list of contributions. Marks the manager
   * as dirty and updates the number of dynamic items, and the memento.
   *
   * @param item
   *            the item to be added
   *
   */
  void ItemAdded(SmartPointer<IContributionItem> item);

  /**
   * The given item was removed from the list of contributions. Marks the
   * manager as dirty and updates the number of dynamic items.
   *
   * @param item
   *            remove given parent from list of contributions
   */
  void ItemRemoved(SmartPointer<IContributionItem> item);

  /**
   * Sets whether this manager is dirty. When dirty, the list of contributions
   * is not accurately reflected in the corresponding widgets.
   *
   * @param dirty
   *            <code>true</code> if this manager is dirty, and
   *            <code>false</code> if it is up-to-date
   */
  void SetDirty(bool dirty);

  /**
   * An internal method for setting the order of the contribution items.
   *
   * @param items
   *            the contribution items in the specified order
   * @since 3.0
   */
  void InternalSetItems(
      const std::vector<SmartPointer<IContributionItem> >& items);

public:

  /*
   * (non-Javadoc) Method declared on IContributionManager.
   */
  void Add(SmartPointer<IContributionItem> item);

  /*
   * (non-Javadoc) Method declared on IContributionManager.
   */
  void AppendToGroup(const std::string& groupName, SmartPointer<
      IContributionItem> item);

  /*
   * (non-Javadoc) Method declared on IContributionManager.
   */
  SmartPointer<IContributionItem> Find(const std::string& id);

  /*
   * (non-Javadoc) Method declared on IContributionManager.
   */
  std::vector<SmartPointer<IContributionItem> > GetItems();

  /**
   * Return the number of contributions in this manager.
   *
   * @return the number of contributions in this manager
   * @since 3.3
   */
  int GetSize();

  /**
   * The <code>ContributionManager</code> implementation of this method
   * declared on <code>IContributionManager</code> returns the current
   * overrides. If there is no overrides it lazily creates one which overrides
   * no item state.
   *
   * @since 2.0
   */
  SmartPointer<IContributionManagerOverrides> GetOverrides();

  /**
   * Returns the index of the item with the given id.
   *
   * @param id
   *            The id of the item whose index is requested.
   *
   * @return <code>int</code> the index or -1 if the item is not found
   */
  int IndexOf(const std::string& id);

  /**
   * Insert the item at the given index.
   *
   * @param index
   *            The index to be used for insertion
   * @param item
   *            The item to be inserted
   */
  void Insert(int index, SmartPointer<IContributionItem> item);

  /*
   * (non-Javadoc) Method declared on IContributionManager.
   */
  void InsertAfter(const std::string& ID, SmartPointer<IContributionItem> item);

  /*
   * (non-Javadoc) Method declared on IContributionManager.
   */
  void
      InsertBefore(const std::string& ID, SmartPointer<IContributionItem> item);

  /*
   * (non-Javadoc) Method declared on IContributionManager.
   */
  bool IsDirty();

  /*
   * (non-Javadoc) Method declared on IContributionManager.
   */
  bool IsEmpty();

  /*
   * (non-Javadoc) Method declared on IContributionManager.
   */
  void MarkDirty();

  /*
   * (non-Javadoc) Method declared on IContributionManager.
   */
  void PrependToGroup(const std::string& groupName, SmartPointer<
      IContributionItem> item);

  /*
   * (non-Javadoc) Method declared on IContributionManager.
   */
  SmartPointer<IContributionItem> Remove(const std::string& ID);

  /*
   * (non-Javadoc) Method declared on IContributionManager.
   */
  SmartPointer<IContributionItem> Remove(SmartPointer<IContributionItem> item);

  /*
   * (non-Javadoc) Method declared on IContributionManager.
   */
  void RemoveAll();

  /**
   * Replaces the item of the given identifier with another contribution item.
   * This can be used, for example, to replace large contribution items with
   * placeholders to avoid memory leaks. If the identifier cannot be found in
   * the current list of items, then this does nothing. If multiple
   * occurrences are found, then the replacement items is put in the first
   * position and the other positions are removed.
   *
   * @param identifier
   *            The identifier to look for in the list of contributions;
   *            should not be <code>null</code>.
   * @param replacementItem
   *            The contribution item to replace the old item; must not be
   *            <code>null</code>. Use
   *            {@link org.eclipse.jface.action.ContributionManager#remove(java.lang.String) remove}
   *            if that is what you want to do.
   * @return <code>true</code> if the given identifier can be; <code>
   * @since 3.0
   */
  bool ReplaceItem(const std::string& identifier, SmartPointer<
      IContributionItem> replacementItem);

  /**
   * Sets the overrides for this contribution manager
   *
   * @param newOverrides
   *            the overrides for the items of this manager
   * @since 2.0
   */
  void SetOverrides(SmartPointer<IContributionManagerOverrides> newOverrides);

private:

  /**
   * Adds a contribution item to the start or end of the group with the given
   * name.
   *
   * @param groupName
   *            the name of the group
   * @param item
   *            the contribution item
   * @param append
   *            <code>true</code> to add to the end of the group, and
   *            <code>false</code> to add the beginning of the group
   * @exception IllegalArgumentException
   *                if there is no group with the given name
   */
  void AddToGroup(const std::string& groupName,
      SmartPointer<IContributionItem> item, bool append);

};

}

#endif /* CHERRYCONTRIBUTIONMANAGER_H_ */
