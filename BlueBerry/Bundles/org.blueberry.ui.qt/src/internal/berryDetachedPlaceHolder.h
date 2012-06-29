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


#ifndef BERRYDETACHEDPLACEHOLDER_H_
#define BERRYDETACHEDPLACEHOLDER_H_

#include "berryPartPlaceholder.h"
#include "berryILayoutContainer.h"

#include "berryIMemento.h"
#include "berryRectangle.h"

#include <list>

namespace berry {

/**
 * DetachedPlaceHolder is the placeholder for detached views.
 *
 */
class DetachedPlaceHolder : public PartPlaceholder, public ILayoutContainer
{

private:

    std::list<LayoutPart::Pointer> children;

    Rectangle bounds;

public:

  berryObjectMacro(DetachedPlaceHolder);

    /**
     * DetachedPlaceHolder constructor comment.
     * @param id java.lang.String
     * @param bounds the size of the placeholder
     */
    DetachedPlaceHolder(const std::string& id, const Rectangle& b);

    /**
     * Add a child to the container.
     */
    void Add(LayoutPart::Pointer newPart);

    /**
     * Return true if the container allows its
     * parts to show a border if they choose to,
     * else false if the container does not want
     * its parts to show a border.
     * @return boolean
     */
    bool AllowsBorder();

    bool AllowsAdd(LayoutPart::Pointer toAdd);

    bool AllowsAutoFocus();

    Rectangle GetBounds();

    /**
     * Returns a list of layout children.
     */
    std::list<LayoutPart::Pointer> GetChildren();
    /**
     * Remove a child from the container.
     */
    void Remove(LayoutPart::Pointer part);

    /**
     * Replace one child with another
     */
    void Replace(LayoutPart::Pointer oldPart, LayoutPart::Pointer newPart);


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

    void FindSashes(LayoutPart::Pointer part, PartPane::Sashes& sashes);

    ILayoutContainer::ChildrenType GetChildren() const;

    void ResizeChild(LayoutPart::Pointer childThatChanged);

};

}

#endif /* BERRYDETACHEDPLACEHOLDER_H_ */
