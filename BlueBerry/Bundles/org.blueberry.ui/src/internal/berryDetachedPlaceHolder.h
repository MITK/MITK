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


#ifndef BERRYDETACHEDPLACEHOLDER_H_
#define BERRYDETACHEDPLACEHOLDER_H_

#include "berryContainerPlaceholder.h"

#include "../berryIMemento.h"
#include "../berryRectangle.h"

#include <list>

namespace berry {

/**
 * DetachedPlaceHolder is the placeholder for detached views.
 *
 */
class DetachedPlaceHolder : public ContainerPlaceholder
{

private:

    std::list<StackablePart::Pointer> children;

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
    void Add(StackablePart::Pointer newPart);

    /**
     * Return true if the container allows its
     * parts to show a border if they choose to,
     * else false if the container does not want
     * its parts to show a border.
     * @return boolean
     */
    bool AllowsBorder();

    Rectangle GetBounds();

    /**
     * Returns a list of layout children.
     */
    std::list<StackablePart::Pointer> GetChildren();
    /**
     * Remove a child from the container.
     */
    void Remove(StackablePart::Pointer part);

    /**
     * Replace one child with another
     */
    void Replace(StackablePart::Pointer oldPart, StackablePart::Pointer newPart);


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

};

}

#endif /* BERRYDETACHEDPLACEHOLDER_H_ */
