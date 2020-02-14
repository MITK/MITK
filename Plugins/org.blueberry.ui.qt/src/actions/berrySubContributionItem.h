/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


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

  berryObjectMacro(SubContributionItem);

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
    void Fill(QStatusBar* parent) override;

    /*
     * Method declared on IContributionItem.
     */
    void Fill(QMenu* parent, QAction* before) override;

    /*
     * Method declared on IContributionItem.
     */
    void Fill(QToolBar* parent, QAction* before) override;

    /*
     * Method declared on IContributionItem.
     */
    QString GetId() const override;

    /**
     * Returns the inner contribution item.
     *
     * @return the inner contribution item
     */
    IContributionItem::Pointer GetInnerItem() const;

    /*
     * Method declared on IContributionItem.
     */
    bool IsEnabled() const override;

    /*
     * Method declared on IContributionItem.
     */
    bool IsDirty() const override;

    /*
     * Method declared on IContributionItem.
     */
    bool IsDynamic() const override;

    /*
     * Method declared on IContributionItem.
     */
    bool IsGroupMarker() const override;

    /*
     * Method declared on IContributionItem.
     */
    bool IsSeparator() const override;

    /*
     * Method declared on IContributionItem.
     */
    bool IsVisible() const override;

    /*
     * Method declared on IContributionItem.
     */
    void SetParent(IContributionManager* parent) override;

    /*
     * Method declared on IContributionItem.
     */
    void SetVisible(bool visible) override;

    /*
     * Method declared on IContributionItem.
     */
    void Update() override;

    /*
     * Method declared on IContributionItem.
     */
    void Update(const QString& id) override;

    /*
     * @see IContributionItem#SaveWidgetState()
     */
    void SaveWidgetState() override;

};

}

#endif /* BERRYSUBCONTRIBUTIONITEM_H_ */
