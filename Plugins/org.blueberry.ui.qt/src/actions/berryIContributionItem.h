/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYICONTRIBUTIONITEM_H
#define BERRYICONTRIBUTIONITEM_H

#include <berryMacros.h>
#include <berryObject.h>

class QStatusBar;
class QMenu;
class QMenuBar;
class QToolBar;
class QAction;

namespace berry {

struct IContributionManager;

/**
 * A contribution item represents a contribution to a shared UI resource such as a
 * menu or tool bar. More generally, contribution items are managed by a contribution
 * manager.
 * For instance, in a tool bar a contribution item is a tool bar button or a separator.
 * In a menu bar a contribution item is a menu, and in a menu a contribution item
 * is a menu item or separator.
 * <p>
 * A contribution item can realize itself in different Qt widgets, using the different
 * <code>fill</code> methods.  The same type of contribution item can be used with a
 * <code>MenuBarManager</code>, <code>ToolBarManager</code>,
 * </code>or a <code>StatusLineManager</code>.
 * </p>
 * <p>
 * This interface is internal to the framework; it should not be implemented outside
 * the framework.
 * </p>
 *
 * @see IContributionManager
 * @noimplement This interface is not intended to be implemented by clients.
 */
struct IContributionItem : public virtual Object
{

  berryObjectMacro(berry::IContributionItem);


  /**
   * Fills the given status bar control with controls representing this
   * contribution item.  Used by <code>StatusLineManager</code>.
   *
   * @param parent the parent control
   */
  virtual void Fill(QStatusBar* parent) = 0;

  /**
   * Fills the given menu bar with controls representing this contribution item.
   * Used by <code>MenuBarManager</code>.
   *
   * @param parent the parent menu
   * @param index the index where the controls are inserted,
   *   or <code>-1</code> to insert at the end
   */
  virtual void Fill(QMenuBar* parent, QAction* before) = 0;

  /**
   * Fills the given menu with controls representing this contribution item.
   * Used by <code>MenuManager</code>.
   *
   * @param parent the parent menu
   * @param index the index where the controls are inserted,
   *   or <code>-1</code> to insert at the end
   */
  virtual void Fill(QMenu* parent, QAction* before) = 0;

  /**
   * Fills the given tool bar with controls representing this contribution item.
   * Used by <code>ToolBarManager</code>.
   *
   * @param parent the parent tool bar
   * @param index the index where the controls are inserted,
   *   or <code>-1</code> to insert at the end
   */
  virtual void Fill(QToolBar* parent, QAction* before) = 0;

  /**
   * Returns the identifier of this contribution item.
   * The id is used for retrieving an item from its manager.
   *
   * @return the contribution item identifier, or <code>null</code>
   *   if none
   */
  virtual QString GetId() const = 0;

  /**
   * Returns whether this contribution item is enabled.
   *
   * @return <code>true</code> if this item is enabled
   */
  virtual bool IsEnabled() const = 0;

  /**
   * Returns whether this contribution item is dirty. A dirty item will be
   * recreated when the action bar is updated.
   *
   * @return <code>true</code> if this item is dirty
   */
  virtual bool IsDirty() const = 0;

  /**
   * Returns whether this contribution item is dynamic. A dynamic contribution
   * item contributes items conditionally, dependent on some internal state.
   *
   * @return <code>true</code> if this item is dynamic, and
   *  <code>false</code> for normal items
   */
  virtual bool IsDynamic() const = 0;

  /**
   * Returns whether this contribution item is a group marker.
   * This information is used when adding items to a group.
   *
   * @return <code>true</code> if this item is a group marker, and
   *  <code>false</code> for normal items
   *
   * @see GroupMarker
   * @see IContributionManager#appendToGroup(String, IContributionItem)
   * @see IContributionManager#prependToGroup(String, IContributionItem)
   */
  virtual bool IsGroupMarker() const = 0;

  /**
   * Returns whether this contribution item is a separator.
   * This information is used to enable hiding of unnecessary separators.
   *
   * @return <code>true</code> if this item is a separator, and
   *  <code>false</code> for normal items
   * @see Separator
   */
  virtual bool IsSeparator() const = 0;

  /**
   * Returns whether this contribution item is visibile within its manager.
   *
   * @return <code>true</code> if this item is visible, and
   *  <code>false</code> otherwise
   */
  virtual bool IsVisible() const = 0;

  /**
   * Saves any state information of the control(s) owned by this contribution item.
   * The contribution manager calls this method before disposing of the controls.
   */
  virtual void SaveWidgetState() = 0;

  /**
   * Sets the parent manager of this item
   *
   * @param parent the parent contribution manager
   */
  virtual void SetParent(IContributionManager* parent) = 0;

  /**
   * Sets whether this contribution item is visibile within its manager.
   *
   * @param visible <code>true</code> if this item should be visible, and
   *  <code>false</code> otherwise
   */
  virtual void SetVisible(bool visible) = 0;

  /**
   * Updates any controls cached by this contribution item with any
   * changes which have been made to this contribution item since the last update.
   * Called by contribution manager update methods.
   */
  virtual void Update() = 0;

  /**
   * Updates any controls cached by this contribution item with changes
   * for the the given property.
   *
   * @param id the id of the changed property
   */
  virtual void Update(const QString& id) = 0;
};

}

#endif // BERRYICONTRIBUTIONITEM_H
