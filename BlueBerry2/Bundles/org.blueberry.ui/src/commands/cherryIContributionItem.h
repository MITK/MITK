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


#ifndef CHERRYICONTRIBUTIONITEM_H_
#define CHERRYICONTRIBUTIONITEM_H_

#include <osgi/framework/Object.h>
#include <osgi/framework/Macros.h>

#include "../cherryUiDll.h"

namespace cherry {

using namespace osgi::framework;

struct IContributionManager;
struct IMenu;
struct IToolBar;

/**
 * A contribution item represents a contribution to a shared UI resource such as a
 * menu or tool bar. More generally, contribution items are managed by a contribution
 * manager.
 * For instance, in a tool bar a contribution item is a tool bar button or a separator.
 * In a menu bar a contribution item is a menu, and in a menu a contribution item
 * is a menu item or separator.
 * <p>
 * A contribution item can realize itself in different SWT widgets, using the different
 * <code>fill</code> methods.  The same type of contribution item can be used with a
 * <code>MenuBarManager</code>, <code>ToolBarManager</code>, <code>CoolBarManager</code>,
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
struct CHERRY_UI IContributionItem : public virtual Object {

  osgiInterfaceMacro(cherry::IContributionItem)

    /**
     * Disposes of this contribution item. Called by the parent
     * contribution manager when the manager is being disposed.
     * Clients should not call this method directly, unless they
     * have removed this contribution item from the containing
     * IContributionManager before the contribution lifecycle
     * has ended.
     *
     * @since 2.1
     */
    virtual void Dispose() = 0;

    /**
     * Fills the given composite control with controls representing this
     * contribution item.  Used by <code>StatusLineManager</code>.
     *
     * @param parent the parent control
     */
    virtual void Fill(void* parent) = 0;

    /**
     * Fills the given menu with controls representing this contribution item.
     * Used by <code>MenuManager</code>.
     *
     * @param parent the parent menu
     * @param index the index where the controls are inserted,
     *   or <code>-1</code> to insert at the end
     */
    virtual void Fill(SmartPointer<IMenu> parent, int index) = 0;

    /**
     * Fills the given tool bar with controls representing this contribution item.
     * Used by <code>ToolBarManager</code>.
     *
     * @param parent the parent tool bar
     * @param index the index where the controls are inserted,
     *   or <code>-1</code> to insert at the end
     */
    virtual void Fill(SmartPointer<IToolBar> parent, int index) = 0;

    /**
     * Fills the given cool bar with controls representing this contribution item.
     * Used by <code>CoolBarManager</code>.
     *
     * @param parent the parent cool bar
     * @param index the index where the controls are inserted,
     *   or <code>-1</code> to insert at the end
     * @since 3.0
     */
    // void fill(CoolBar parent, int index);

    /**
     * Returns the identifier of this contribution item.
     * The id is used for retrieving an item from its manager.
     *
     * @return the contribution item identifier, or <code>null</code>
     *   if none
     */
    virtual std::string GetId() const = 0;

    /**
     * Returns whether this contribution item is enabled.
     *
     * @return <code>true</code> if this item is enabled
     */
    virtual bool IsEnabled() = 0;

    /**
     * Returns whether this contribution item is dirty. A dirty item will be
     * recreated when the action bar is updated.
     *
     * @return <code>true</code> if this item is dirty
     */
    virtual bool IsDirty() = 0;

    /**
     * Returns whether this contribution item is dynamic. A dynamic contribution
     * item contributes items conditionally, dependent on some internal state.
     *
     * @return <code>true</code> if this item is dynamic, and
     *  <code>false</code> for normal items
     */
    virtual bool IsDynamic() = 0;

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
    virtual bool IsGroupMarker() = 0;

    /**
     * Returns whether this contribution item is a separator.
     * This information is used to enable hiding of unnecessary separators.
     *
     * @return <code>true</code> if this item is a separator, and
     *  <code>false</code> for normal items
     * @see Separator
     */
    virtual bool IsSeparator() = 0;

    /**
     * Returns whether this contribution item is visibile within its manager.
     *
     * @return <code>true</code> if this item is visible, and
     *  <code>false</code> otherwise
     */
    virtual bool IsVisible() = 0;

    /**
     * Saves any state information of the control(s) owned by this contribution item.
     * The contribution manager calls this method before disposing of the controls.
     *
     * @since 3.0
     */
    virtual void SaveWidgetState() = 0;

    /**
     * Sets the parent manager of this item
     *
     * @param parent the parent contribution manager
     * @since 2.0
     */
    virtual void SetParent(SmartPointer<IContributionManager> parent) = 0;

    /**
     * Sets whether this contribution item is visibile within its manager.
     *
     * @param visible <code>true</code> if this item should be visible, and
     *  <code>false</code> otherwise
     */
    virtual void SetVisible(bool visible) = 0;

    /**
     * Updates any SWT controls cached by this contribution item with any
     * changes which have been made to this contribution item since the last update.
     * Called by contribution manager update methods.
     */
    virtual void Update() = 0;

    /**
     * Updates any SWT controls cached by this contribution item with changes
     * for the the given property.
     *
     * @param id the id of the changed property
     * @since 2.0
     */
    virtual void Update(const std::string& id) = 0;
};

}

#endif /* CHERRYICONTRIBUTIONITEM_H_ */
