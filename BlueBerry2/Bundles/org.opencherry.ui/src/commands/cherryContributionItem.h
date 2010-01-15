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


#ifndef CHERRYCONTRIBUTIONITEM_H_
#define CHERRYCONTRIBUTIONITEM_H_

#include "cherryIContributionItem.h"

#include "cherryIContributionManager.h"

namespace cherry {

using namespace osgi::framework;

/**
 * An abstract base implementation for contribution items.
 */
class CHERRY_UI ContributionItem : public IContributionItem {

private:

    /**
     * The identifier for this contribution item, of <code>null</code> if none.
     */
    std::string id; // = "";

    /**
     * Indicates this item is visible in its manager; <code>true</code>
     * by default.
     */
    bool visible; // = true;

    /**
     * The parent contribution manager for this item
     */
    IContributionManager::WeakPtr parent;

protected:

    /**
     * Creates a contribution item with the given (optional) id.
     * The given id is used to find items in a contribution manager,
     * and for positioning items relative to other items.
     *
     * @param id the contribution item identifier, or <code>null</code>
     */
    ContributionItem(const std::string& id = "");

public:

    /**
     * The default implementation of this <code>IContributionItem</code>
     * method does nothing. Subclasses may override.
     */
    void Dispose();

    /**
     * The default implementation of this <code>IContributionItem</code>
     * method does nothing. Subclasses may override.
     */
    void Fill(void* parent);

    /**
     * The default implementation of this <code>IContributionItem</code>
     * method does nothing. Subclasses may override.
     */
    void Fill(SmartPointer<IMenu> menu, int index);

    /**
     * The default implementation of this <code>IContributionItem</code>
     * method does nothing. Subclasses may override.
     */
    void Fill(SmartPointer<IToolBar> parent, int index);

    /**
     * The default implementation of this <code>IContributionItem</code>
     * method does nothing. Subclasses may override.
     *
     * @since 3.0
     */
//    void Fill(CoolBar parent, int index) {
//    }

    /**
     * The default implementation of this <code>IContributionItem</code>
     * method does nothing. Subclasses may override.
     *
     * @since 3.0
     */
    void SaveWidgetState();

    /* (non-Javadoc)
     * Method declared on IContributionItem.
     */
    std::string GetId() const;

    /**
     * Returns the parent contribution manager, or <code>null</code> if this
     * contribution item is not currently added to a contribution manager.
     *
     * @return the parent contribution manager, or <code>null</code>
     * @since 2.0
     */
    SmartPointer<IContributionManager> GetParent();

    /**
     * The default implementation of this <code>IContributionItem</code>
     * method returns <code>false</code>. Subclasses may override.
     */
    bool IsDirty();

    /**
     * The default implementation of this <code>IContributionItem</code>
     * method returns <code>true</code>. Subclasses may override.
     */
    bool IsEnabled();

    /**
     * The default implementation of this <code>IContributionItem</code>
     * method returns <code>false</code>. Subclasses may override.
     */
    bool IsDynamic();

    /**
     * The default implementation of this <code>IContributionItem</code>
     * method returns <code>false</code>. Subclasses may override.
     */
    bool IsGroupMarker();

    /**
     * The default implementation of this <code>IContributionItem</code>
     * method returns <code>false</code>. Subclasses may override.
     */
    bool IsSeparator();

    /**
     * The default implementation of this <code>IContributionItem</code>
     * method returns the value recorded in an internal state variable,
     * which is <code>true</code> by default. <code>setVisible</code>
     * should be used to change this setting.
     */
    bool IsVisible();

    /**
     * The default implementation of this <code>IContributionItem</code>
     * method stores the value in an internal state variable,
     * which is <code>true</code> by default.
     */
    void SetVisible(bool visible);

    /**
     * Returns a string representation of this contribution item
     * suitable only for debugging.
     */
    void PrintSelf(std::ostream& os, Indent indent) const;

    /**
     * The default implementation of this <code>IContributionItem</code>
     * method does nothing. Subclasses may override.
     */
    void Update();

    /* (non-Javadoc)
     * Method declared on IContributionItem.
     */
    void SetParent(SmartPointer<IContributionManager> parent);

    /**
     * The <code>ContributionItem</code> implementation of this
     * method declared on <code>IContributionItem</code> does nothing.
     * Subclasses should override to update their state.
     */
    void Update(const std::string& id);

    /**
   * The ID for this contribution item. It should be set once either in the
   * constructor or using this method.
   *
   * @param itemId
   * @see #GetId()
   */
    void SetId(const std::string& itemId);
};

}

#endif /* CHERRYCONTRIBUTIONITEM_H_ */
