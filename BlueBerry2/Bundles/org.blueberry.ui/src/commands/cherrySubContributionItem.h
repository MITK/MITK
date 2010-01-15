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


#ifndef CHERRYSUBCONTRIBUTIONITEM_H_
#define CHERRYSUBCONTRIBUTIONITEM_H_

#include "cherryIContributionItem.h"


namespace cherry {

/**
 * A <code>SubContributionItem</code> is a wrapper for an <code>IContributionItem</code>.
 * It is used within a <code>SubContributionManager</code> to control the visibility
 * of items.
 * <p>
 * This class is not intended to be subclassed.
 * </p>
 * @noextend This class is not intended to be subclassed by clients.
 */
class CHERRY_UI SubContributionItem : public IContributionItem {

public:

  osgiObjectMacro(SubContributionItem);

private:

    /**
     * The visibility of the item.
     */
    bool visible;

    /**
     * The inner item for this contribution.
     */
    IContributionItem::Pointer innerItem;

public:

    /**
     * Creates a new <code>SubContributionItem</code>.
     * @param item the contribution item to be wrapped
     */
    SubContributionItem(IContributionItem::Pointer item);

    /**
     * The default implementation of this <code>IContributionItem</code>
     * delegates to the inner item. Subclasses may override.
     */
    void Dispose();

    /* (non-Javadoc)
     * Method declared on IContributionItem.
     */
    void Fill(void* parent);

    /* (non-Javadoc)
     * Method declared on IContributionItem.
     */
    void Fill(SmartPointer<IMenu> parent, int index);

    /* (non-Javadoc)
     * Method declared on IContributionItem.
     */
    void Fill(SmartPointer<IToolBar> parent, int index);

    /* (non-Javadoc)
     * Method declared on IContributionItem.
     */
    std::string GetId() const;

    /**
     * Returns the inner contribution item.
     *
     * @return the inner contribution item
     */
    IContributionItem::Pointer GetInnerItem();

    /* (non-Javadoc)
     * Method declared on IContributionItem.
     */
    bool IsEnabled();

    /* (non-Javadoc)
     * Method declared on IContributionItem.
     */
    bool IsDirty();

    /* (non-Javadoc)
     * Method declared on IContributionItem.
     */
    bool IsDynamic();

    /* (non-Javadoc)
     * Method declared on IContributionItem.
     */
    bool IsGroupMarker();

    /* (non-Javadoc)
     * Method declared on IContributionItem.
     */
    bool IsSeparator();

    /* (non-Javadoc)
     * Method declared on IContributionItem.
     */
    bool IsVisible();

    /* (non-Javadoc)
     * Method declared on IContributionItem.
     */
    void SetParent(SmartPointer<IContributionManager> parent);

    /* (non-Javadoc)
     * Method declared on IContributionItem.
     */
    void SetVisible(bool visible);

    /* (non-Javadoc)
     * Method declared on IContributionItem.
     */
    void Update();

    /* (non-Javadoc)
     * Method declared on IContributionItem.
     */
    void Update(const std::string& id);

    /* (non-Javadoc)
     * @see org.eclipse.jface.action.IContributionItem#saveWidgetState()
     */
    void SaveWidgetState();

};

}

#endif /* CHERRYSUBCONTRIBUTIONITEM_H_ */
