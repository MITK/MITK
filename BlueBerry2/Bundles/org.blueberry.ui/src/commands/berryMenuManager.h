/*=========================================================================

 Program:   BlueBerry Platform
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

#ifndef BERRYMENUMANAGER_H_
#define BERRYMENUMANAGER_H_

#include "berryIMenuManager.h"
#include "berryIMenuListener.h"
#include "berryContributionManager.h"

#include "../guitk/berryGuiTkIMenuListener.h"

#include "../berryUiDll.h"

namespace berry
{

using namespace osgi::framework;

struct IMenu;
struct IMenuItem;
struct ImageDescriptor;

/**
 * A menu manager is a contribution manager which realizes itself and its items
 * in a menu control; either as a menu bar, a sub-menu, or a context menu.
 * <p>
 * This class may be instantiated; it may also be subclassed.
 * </p>
 */
class BERRY_UI MenuManager: public ContributionManager, public IMenuManager
{

public:

  osgiObjectMacro(MenuManager);

  friend class GuiTkMenuListener;

private:

  /**
   * The menu id.
   */
  std::string id;

  /**
   * List of registered menu listeners (element type: <code>IMenuListener</code>).
   */
  IMenuListener::Events menuEvents;
  GuiTk::IMenuListener::Pointer menuListener;

  /**
   * The menu control; <code>null</code> before
   * creation and after disposal.
   */
  SmartPointer<IMenu> menu;

  /**
   * The menu item widget; <code>null</code> before
   * creation and after disposal. This field is used
   * when this menu manager is a sub-menu.
   */
  SmartPointer<IMenuItem> menuItem;

  /**
   * The text for a sub-menu.
   */
  std::string menuText;

  /**
   * The image for a sub-menu.
   */
  SmartPointer<ImageDescriptor> image;

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
  IContributionManager::WeakPtr parent;

  /**
   * Indicates whether <code>removeAll</code> should be
   * called just before the menu is displayed.
   */
  bool removeAllWhenShown;

  /**
   * allows a submenu to display a shortcut key. This is often used with the
   * QuickMenu command or action which can pop up a menu using the shortcut.
   */
  std::string definitionId;

protected:

  /**
   * Indicates this item is visible in its manager; <code>true</code>
   * by default.
   * @since 3.3
   */
  bool visible;

//  /**
//   * The contribution manager
//   */
//  ContributionManager contributionManager;

public:

  /**
   * Creates a menu manager with the given text and id.
   * Typically no text is given when creating a context menu.
   * Supply a text and id for creating a sub-menu, where it needs to be referred to by the id.
   *
   * @param text the text for the menu, or <code>""</code> if none
   * @param id the menu id, or <code>""</code> if it is to have no id
   */
  MenuManager(const std::string text = "", const std::string& id = "");

  ~MenuManager();

  /**
   * Creates a menu manager with the given text, image, and id.
   * Typically used for creating a sub-menu, where it needs to be referred to by id.
   *
   * @param text the text for the menu, or <code>""</code> if none
   * @param image the image for the menu, or <code>ImageDescriptor::Pointer(0)</code> if none
   * @param id the menu id, or <code>""</code> if it is to have no id
   */
  MenuManager(const std::string& text, SmartPointer<ImageDescriptor> image,
      const std::string& id);

  /* (non-Javadoc)
   * @see IMenuManager#AddMenuListener(SmartPointer<IMenuListener>)
   */
  void AddMenuListener(SmartPointer<IMenuListener> listener);

  /**
   * Creates and returns an SWT context menu control for this menu,
   * and installs all registered contributions.
   * Does not create a new control if one already exists.
   * <p>
   * Note that the menu is not expected to be dynamic.
   * </p>
   *
   * @param parent the parent control
   * @return the menu control
   */
  SmartPointer<IMenu> CreateContextMenu(void* parent);

  /**
   * Creates and returns an SWT menu bar control for this menu,
   * for use in the given <code>Decorations</code>, and installs all registered
   * contributions. Does not create a new control if one already exists.
   *
   * @param parent the parent decorations
   * @return the menu control
   * @since 2.1
   */
  SmartPointer<IMenu> CreateMenuBar(void* parent);

  /**
   * Disposes of this menu manager and frees all allocated SWT resources.
   * Notifies all contribution items of the dispose. Note that this method does
   * not clean up references between this menu manager and its associated
   * contribution items. Use <code>removeAll</code> for that purpose.
   */
  void Dispose();

  /* (non-Javadoc)
   * @see org.eclipse.jface.action.IContributionItem#fill(org.eclipse.swt.widgets.Composite)
   */
  void Fill(void* parent);

  /* (non-Javadoc)
   * @see org.eclipse.jface.action.IContributionItem#fill(org.eclipse.swt.widgets.CoolBar, int)
   */
  void Fill(SmartPointer<IToolBar> parent, int index);

  /* (non-Javadoc)
   * @see org.eclipse.jface.action.IContributionItem#fill(org.eclipse.swt.widgets.Menu, int)
   */
  void Fill(SmartPointer<IMenu> parent, int index);

  /* (non-Javadoc)
   * @see org.eclipse.jface.action.IMenuManager#findMenuUsingPath(java.lang.String)
   */
  IMenuManager::Pointer FindMenuUsingPath(const std::string& path);

  /* (non-Javadoc)
   * @see org.eclipse.jface.action.IMenuManager#findUsingPath(java.lang.String)
   */
  IContributionItem::Pointer FindUsingPath(const std::string& path);

  /**
   * Returns the menu id. The menu id is used when creating a contribution
   * item for adding this menu as a sub menu of another.
   *
   * @return the menu id
   */
  std::string GetId();

  /**
   * Returns the SWT menu control for this menu manager.
   *
   * @return the menu control
   */
  SmartPointer<IMenu> GetMenu();

  /**
   * Returns the text shown in the menu, potentially with a shortcut
   * appended.
   *
   * @return the menu text
   */
  std::string GetMenuText();

  /**
   * Returns the image for this menu as an image descriptor.
   *
   * @return the image, or <code>null</code> if this menu has no image
   * @since 3.4
   */
  SmartPointer<ImageDescriptor> GetImageDescriptor();

  /* (non-Javadoc)
   * @see org.eclipse.jface.action.IContributionManager#getOverrides()
   */
  SmartPointer<IContributionManagerOverrides> GetOverrides();

  /**
   * Returns the parent contribution manager of this manger.
   *
   * @return the parent contribution manager
   * @since 2.0
   */
  IContributionManager::Pointer GetParent();

  /* (non-Javadoc)
   * @see org.eclipse.jface.action.IMenuManager#getRemoveAllWhenShown()
   */
  bool GetRemoveAllWhenShown();

  /* (non-Javadoc)
   * @see org.eclipse.jface.action.IContributionItem#isDynamic()
   */
  bool IsDynamic();

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
  bool IsEnabled();

  /* (non-Javadoc)
   * @see org.eclipse.jface.action.IContributionItem#isGroupMarker()
   */
  bool IsGroupMarker();

  /* (non-Javadoc)
   * @see org.eclipse.jface.action.IContributionItem#isSeparator()
   */
  bool IsSeparator();

  /* (non-Javadoc)
   * @see org.eclipse.jface.action.IContributionItem#isVisible()
   */
  bool IsVisible();

  /**
   * The <code>MenuManager</code> implementation of this <code>ContributionManager</code> method
   * also propagates the dirty flag up the parent chain.
   *
   * @since 3.1
   */
  void MarkDirty();

  /* (non-Javadoc)
   * @see org.eclipse.jface.action.IMenuManager#removeMenuListener(org.eclipse.jface.action.IMenuListener)
   */
  void RemoveMenuListener(SmartPointer<IMenuListener> listener);

  /* (non-Javadoc)
   * @see org.eclipse.jface.action.IContributionItem#saveWidgetState()
   */
  void SaveWidgetState();

  /**
   * Sets the overrides for this contribution manager
   *
   * @param newOverrides the overrides for the items of this manager
   * @since 2.0
   */
  void SetOverrides(SmartPointer<IContributionManagerOverrides> newOverrides);

  /* (non-Javadoc)
   * @see org.eclipse.jface.action.IContributionItem#setParent(org.eclipse.jface.action.IContributionManager)
   */
  void SetParent(IContributionManager::Pointer manager);

  /* (non-Javadoc)
   * @see org.eclipse.jface.action.IMenuManager#setRemoveAllWhenShown(boolean)
   */
  void SetRemoveAllWhenShown(bool removeAll);

  /* (non-Javadoc)
   * @see org.eclipse.jface.action.IContributionItem#setVisible(boolean)
   */
  void SetVisible(bool visible);

  /**
   * Sets the action definition id of this action. This simply allows the menu
   * item text to include a short cut if available.  It can be used to
   * notify a user of a key combination that will open a quick menu.
   *
   * @param definitionId
   *            the command definition id
   * @since 3.4
   */
  void SetActionDefinitionId(const std::string& definitionId);

  /* (non-Javadoc)
   * @see org.eclipse.jface.action.IContributionItem#update()
   */
  void Update();

  /**
   * The <code>MenuManager</code> implementation of this <code>IContributionManager</code>
   * updates this menu, but not any of its submenus.
   *
   * @see #updateAll
   */
  void Update(bool force);

  /* (non-Javadoc)
   * @see org.eclipse.jface.action.IContributionItem#update(java.lang.String)
   */
  void Update(const std::string& property);

  /* (non-Javadoc)
   * @see org.eclipse.jface.action.IMenuManager#updateAll(boolean)
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
  void FireAboutToShow(IMenuManager::Pointer manager);

  /**
   * Notifies any menu listeners that a menu is about to hide.
   * Only listeners registered at the time this method is called are notified.
   *
   * @param manager the menu manager
   *
   */
  void FireAboutToHide(IMenuManager::Pointer manager);

  /**
   * Notifies all listeners that this menu is about to appear.
   */
  void HandleAboutToShow();

  /**
   * Notifies all listeners that this menu is about to disappear.
   */
  void HandleAboutToHide();

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

protected:

  /**
   * Returns whether the menu control is created
   * and not disposed.
   *
   * @return <code>true</code> if the control is created
   *  and not disposed, <code>false</code> otherwise
   * @since 3.4 protected, was added in 3.1 as private method
   */
  bool MenuExist();

  /**
   * Get all the items from the implementation's widget.
   *
   * @return the menu items
   * @since 3.4
   */
  std::vector<SmartPointer<IMenuItem> > GetMenuItems();

  /**
   * Get an item from the implementation's widget.
   *
   * @param index
   *            of the item
   * @return the menu item
   * @since 3.4
   */
  SmartPointer<IMenuItem> GetMenuItem(unsigned int index);

  /**
   * Get the menu item count for the implementation's widget.
   *
   * @return the number of items
   * @since 3.4
   */
  unsigned int GetMenuItemCount();

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
   * @since 3.4
   */
  void DoItemFill(IContributionItem::Pointer ci, int index);

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
