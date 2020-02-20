/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYDETACHEDPLACEHOLDER_H_
#define BERRYDETACHEDPLACEHOLDER_H_

#include "berryPartPlaceholder.h"
#include "berryILayoutContainer.h"

#include "berryIMemento.h"

namespace berry {

/**
 * DetachedPlaceHolder is the placeholder for detached views.
 *
 */
class DetachedPlaceHolder : public PartPlaceholder, public ILayoutContainer
{

private:

    QList<LayoutPart::Pointer> children;

    QRect bounds;

public:

  berryObjectMacro(DetachedPlaceHolder);

    /**
     * DetachedPlaceHolder constructor comment.
     * @param id java.lang.String
     * @param bounds the size of the placeholder
     */
    DetachedPlaceHolder(const QString& id, const QRect& b);

    /**
     * Add a child to the container.
     */
    void Add(LayoutPart::Pointer newPart) override;

    /**
     * Return true if the container allows its
     * parts to show a border if they choose to,
     * else false if the container does not want
     * its parts to show a border.
     * @return boolean
     */
    bool AllowsBorder();

    bool AllowsAdd(LayoutPart::Pointer toAdd) override;

    bool AllowsAutoFocus() override;

    QRect GetBounds();

    /**
     * Returns a list of layout children.
     */
    QList<LayoutPart::Pointer> GetChildren();
    /**
     * Remove a child from the container.
     */
    void Remove(LayoutPart::Pointer part) override;

    /**
     * Replace one child with another
     */
    void Replace(LayoutPart::Pointer oldPart, LayoutPart::Pointer newPart) override;


    /**
     * Restore the state from the memento.
     * @param memento
     */
    void RestoreState(IMemento::Pointer memento);

    /**
     * Save state to the memento.
     * @param memento
     */
    void SaveState(IMemento::Pointer memento);

    void FindSashes(LayoutPart::Pointer part, PartPane::Sashes& sashes) override;

    ILayoutContainer::ChildrenType GetChildren() const override;

    void ResizeChild(LayoutPart::Pointer childThatChanged) override;

};

}

#endif /* BERRYDETACHEDPLACEHOLDER_H_ */
