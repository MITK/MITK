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


#ifndef BERRYSUBCONTRIBUTIONITEM_H_
#define BERRYSUBCONTRIBUTIONITEM_H_

#include "berryIContributionItem.h"

#include <org_blueberry_ui_qt_Export.h>

namespace berry {

/**
 * A <code>SubContributionItem</code> is a wrapper for an <code>IContributionItem</code>.
 * It is used within a <code>SubContributionManager</code> to control the visibility
 * of items.
 * <p>
 * This class is not intended to be subclassed.
 * </p>
 * @noextend This class is not intended to be subclassed by clients.
 */
class BERRY_UI_QT SubContributionItem : public IContributionItem
{

public:

  berryObjectMacro(SubContributionItem)

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

    using IContributionItem::Fill;

    /*
     * Method declared on IContributionItem.
     */
    void Fill(QStatusBar* parent);

    /*
     * Method declared on IContributionItem.
     */
    void Fill(QMenu* parent, QAction* before);

    /*
     * Method declared on IContributionItem.
     */
    void Fill(QToolBar* parent, QAction* before);

    /*
     * Method declared on IContributionItem.
     */
    QString GetId() const;

    /**
     * Returns the inner contribution item.
     *
     * @return the inner contribution item
     */
    IContributionItem::Pointer GetInnerItem() const;

    /*
     * Method declared on IContributionItem.
     */
    bool IsEnabled() const;

    /*
     * Method declared on IContributionItem.
     */
    bool IsDirty() const;

    /*
     * Method declared on IContributionItem.
     */
    bool IsDynamic() const;

    /*
     * Method declared on IContributionItem.
     */
    bool IsGroupMarker() const;

    /*
     * Method declared on IContributionItem.
     */
    bool IsSeparator() const;

    /*
     * Method declared on IContributionItem.
     */
    bool IsVisible() const;

    /*
     * Method declared on IContributionItem.
     */
    void SetParent(IContributionManager* parent);

    /*
     * Method declared on IContributionItem.
     */
    void SetVisible(bool visible);

    /*
     * Method declared on IContributionItem.
     */
    void Update();

    /*
     * Method declared on IContributionItem.
     */
    void Update(const QString& id);

    /*
     * @see IContributionItem#SaveWidgetState()
     */
    void SaveWidgetState();

};

}

#endif /* BERRYSUBCONTRIBUTIONITEM_H_ */
