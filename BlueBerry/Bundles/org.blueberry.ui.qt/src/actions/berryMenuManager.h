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

#ifndef BERRYMENUMANAGER_H_
#define BERRYMENUMANAGER_H_

#include "berryIMenuManager.h"
#include "berryContributionManager.h"

#include <org_blueberry_ui_qt_Export.h>

#include <QIcon>

class QMenu;
class QMenuProxy;
class QAction;

namespace berry
{

class MMMenuListener;

/**
 * A menu manager is a contribution manager which realizes itself and its items
 * in a menu control; either as a menu bar, a sub-menu, or a context menu.
 * <p>
 * This class may be instantiated; it may also be subclassed.
 * </p>
 */
class BERRY_UI_QT MenuManager: public ContributionManager, public IMenuManager
{

public:

  berryObjectMacro(MenuManager)

private:

  friend class MMMenuListener;

  /**
   * The menu id.
   */
  QString id;

  /**
   * List of registered menu listeners (element type: <code>IMenuListener</code>).
   */
  //IMenuListener::Events menuEvents;
  //GuiTk::IMenuListener::Pointer menuListener;

  /**
   * The menu control; <code>null</code> before
   * creation and after disposal.
   */
  QMenuProxy* menu;
  QAction* menuItem;

  QScopedPointer<MMMenuListener> menuListener;

  /**
   * The menu item widget; <code>null</code> before
   * creation and after disposal. This field is used
   * when this menu manager is a sub-menu.
   */
  //SmartPointer<IMenuItem> menuItem;

  /**
   * The text for a sub-menu.
   */
  QString menuText;

  /**
   * The image for a sub-menu.
   */
  QIcon image;

  /**
   * A resource manager to remember all of the images that have been used by this menu.
   */
  //LocalResourceManager imageManager;

  /**
   * The overrides for items of this manager
   */
  SmartPointer<IContributionManagerOverrides> overrides;

  /**
   * The parent contribution manager.
   */
  IContributionManager* parent;

  /**
   * Indicates whether <code>removeAll</code> should be
   * called just before the menu is displayed.
   */
  bool removeAllWhenShown;

  /**
   * allows a submenu to display a shortcut key. This is often used with the
   * QuickMenu command or action which can pop up a menu using the shortcut.
   */
  QString definitionId;

protected:

  /**
   * Indicates this item is visible in its manager; <code>true</code>
   * by default.
   */
  bool visible;

public:

  /**
   * Creates a menu manager with the given text and id.
   * Typically no text is given when creating a context menu.
   * Supply a text and id for creating a sub-menu, where it needs to be referred to by the id.
   *
   * @param text the text for the menu, or <code>""</code> if none
   * @param id the menu id, or <code>""</code> if it is to have no id
   */
  MenuManager(const QString& text = QString(), const QString& id = QString());

  ~MenuManager();

  /**
   * Creates a menu manager with the given text, image, and id.
   * Typically used for creating a sub-menu, where it needs to be referred to by id.
   *
   * @param text the text for the menu, or <code>""</code> if none
   * @param image the image for the menu, or <code>ImageDescriptor::Pointer(0)</code> if none
   * @param id the menu id, or <code>""</code> if it is to have no id
   */
  MenuManager(const QString& text, const QIcon& image,
              const QString& id);

  /*
   * @see IMenuManager#AddMenuListener(SmartPointer<IMenuListener>)
   */
  //void AddMenuListener(SmartPointer<IMenuListener> listener);

  bool IsDirty() const;

  /**
   * Creates and returns a Qt menu control for this menu,
   * and installs all registered contributions.
   * Does not create a new control if one already exists.
   * <p>
   * Note that the menu is not expected to be dynamic.
   * </p>
   *
   * @param parent the parent control
   * @return the menu control
   */
  QMenu* CreateContextMenu(QWidget* parent);

  /**
   * Creates and returns a Qt menu bar control for this menu,
   * for use in the given <code>QWidget</code>, and installs all registered
   * contributions. Does not create a new control if one already exists.
   *
   * @param parent the parent decorations
   * @return the menu control
   * @since 2.1
   */
  QMenuBar* CreateMenuBar(QWidget* parent);

  /*
   * @see IContributionItem#Fill(QStatusBar*)
   */
  void Fill(QStatusBar* parent);

  /*
   * @see IContributionItem#Fill(QToolBar*, int)
   */
  QAction* Fill(QToolBar* parent, QAction *index);

  /*
   * @see IContributionItem#Fill(QMenu*, int)
   */
  QAction* Fill(QMenu* parent, QAction *before);

  /*
   * @see IContributionItem#Fill(QMenuBar*, int)
   */
  QAction* Fill(QMenuBar* parent, QAction *before);

  /*
   * @see IMenuManager#FindMenuUsingPath(const QString&)
   */
  IMenuManager::Pointer FindMenuUsingPath(const QString& path) const;

  /*
   * @see IMenuManager#FindUsingPath(const QString&)
   */
  IContributionItem::Pointer FindUsingPath(const QString& path) const;

  /**
   * Returns the menu id. The menu id is used when creating a contribution
   * item for adding this menu as a sub menu of another.
   *
   * @return the menu id
   */
  QString GetId() const;

  /**
   * Returns the SWT menu control for this menu manager.
   *
   * @return the menu control
   */
  QMenu* GetMenu() const;

  /**
   * Returns the text shown in the menu, potentially with a shortcut
   * appended.
   *
   * @return the menu text
   */
  QString GetMenuText() const;

  /**
   * Returns the image for this menu as an image descriptor.
   *
   * @return the image, or <code>null</code> if this menu has no image
   */
  QIcon GetImage() const;

  /*
   * @see IContributionManager#GetOverrides()
   */
  SmartPointer<IContributionManagerOverrides> GetOverrides();

  /**
   * Returns the parent contribution manager of this manger.
   *
   * @return the parent contribution manager
   */
  IContributionManager* GetParent() const;

  /*
   * @see IMenuManager#GetRemoveAllWhenShown()
   */
  bool GetRemoveAllWhenShown() const;

  /*
   * @see IContributionItem#IsDynamic()
   */
  bool IsDynamic() const;

  /**
   * Returns whether this menu should be enabled or not.
   * Used to enable the menu item containing this menu when it is realized as a sub-menu.
   * <p>
   * The default implementation of this framework method
   * returns <code>true</code>. Subclasses may reimplement.
   * </p>
   *
   * @return <code>true</code> if enabled, and
   *   <code>false</code> if disabled
   */
  bool IsEnabled() const;

  /*
   * @see IContributionItem#IsGroupMarker()
   */
  bool IsGroupMarker() const;

  /*
   * @see IContributionItem#IsSeparator()
   */
  bool IsSeparator() const;

  /*
   * @see IContributionItem#IsVisible()
   */
  bool IsVisible() const;

  /**
   * The <code>MenuManager</code> implementation of this <code>ContributionManager</code> method
   * also propagates the dirty flag up the parent chain.
   */
  void MarkDirty();

  /*
   * @see IMenuManager#removeMenuListener(IMenuListener)
   */
  //void RemoveMenuListener(SmartPointer<IMenuListener> listener);

  /*
   * @IContributionItem#SaveWidgetState()
   */
  void SaveWidgetState();

  /**
   * Sets the overrides for this contribution manager
   *
   * @param newOverrides the overrides for the items of this manager
   */
  void SetOverrides(SmartPointer<IContributionManagerOverrides> newOverrides);

  /*
   * @see IContributionItem#SetParent(IContributionManager)
   */
  void SetParent(IContributionManager* manager);

  /*
   * @see IMenuManager#SetRemoveAllWhenShown(boolean)
   */
  void SetRemoveAllWhenShown(bool removeAll);

  /*
   * @see IContributionItem#SetVisible(bool)
   */
  void SetVisible(bool visible);

  /**
   * Sets the action definition id of this action. This simply allows the menu
   * item text to include a short cut if available.  It can be used to
   * notify a user of a key combination that will open a quick menu.
   *
   * @param definitionId
   *            the command definition id
   */
  void SetActionDefinitionId(const QString& definitionId);

  /*
   * @see IContributionItem#Update()
   */
  void Update();

  void Update(const QString& property);

  /**
   * The <code>MenuManager</code> implementation of this <code>IContributionManager</code>
   * updates this menu, but not any of its submenus.
   *
   * @see #updateAll
   */
  void Update(bool force);

  /*
   * @see IMenuManager#UpdateAll(bool)
   */
  void UpdateAll(bool force);

private:

  /**
   * Notifies any menu listeners that a menu is about to show.
   * Only listeners registered at the time this method is called are notified.
   *
   * @param manager the menu manager
   *
   * @see IMenuListener#menuAboutToShow
   */
  //void FireAboutToShow(IMenuManager::Pointer manager);

  /**
   * Notifies any menu listeners that a menu is about to hide.
   * Only listeners registered at the time this method is called are notified.
   *
   * @param manager the menu manager
   *
   */
  //void FireAboutToHide(IMenuManager::Pointer manager);

  /**
   * Notifies all listeners that this menu is about to appear.
   */
  //void HandleAboutToShow();

  /**
   * Notifies all listeners that this menu is about to disappear.
   */
  //void HandleAboutToHide();

  /**
   * Initializes the menu control.
   */
  void InitializeMenu();

  /**
   * Dispose any images allocated for this menu
   */
 // void DisposeOldImages();

  /**
   * Updates the menu item for this sub menu.
   * The menu item is disabled if this sub menu is empty.
   * Does nothing if this menu is not a submenu.
   */
  void UpdateMenuItem();

  QAction* FillMenu(QWidget* parent, QAction* before);

  void DumpActionInfo(QMenuProxy* menu);
  void DumpActionInfo(QWidget* widget, int level);

protected:

  /**
   * Get all the items from the implementation's widget.
   *
   * @return the menu items
   */
  //QList<SmartPointer<IMenuItem> > GetMenuItems();

  /**
   * Get an item from the implementation's widget.
   *
   * @param index
   *            of the item
   * @return the menu item
   */
  //SmartPointer<IMenuItem> GetMenuItem(unsigned int index);

  /**
   * Get the menu item count for the implementation's widget.
   *
   * @return the number of items
   */
  //unsigned int GetMenuItemCount();

  /**
   * Call an <code>IContributionItem</code>'s fill method with the
   * implementation's widget. The default is to use the <code>Menu</code>
   * widget.<br>
   * <code>fill(Menu menu, int index)</code>
   *
   * @param ci
   *            An <code>IContributionItem</code> whose <code>fill()</code>
   *            method should be called.
   * @param index
   *            The position the <code>fill()</code> method should start
   *            inserting at.
   */
  void DoItemFill(IContributionItem::Pointer ci, QAction *before);

  /**
   * Incrementally builds the menu from the contribution items.
   * This method leaves out double separators and separators in the first
   * or last position.
   *
   * @param force <code>true</code> means update even if not dirty,
   *   and <code>false</code> for normal incremental updating
   * @param recursive <code>true</code> means recursively update
   *   all submenus, and <code>false</code> means just this menu
   */
  void Update(bool force, bool recursive);

};

}

#endif /* BERRYMENUMANAGER_H_ */
