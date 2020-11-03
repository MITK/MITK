/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYLEFTTORIGHTTABORDER_H_
#define BERRYLEFTTORIGHTTABORDER_H_

#include "berryTabOrder.h"

#include "berryIPresentablePartList.h"

namespace berry {

class LeftToRightTabOrder : public TabOrder {

private:

  IPresentablePartList* list;

public:

  LeftToRightTabOrder(IPresentablePartList* list);

    /* (non-Javadoc)
     * @see org.blueberry.ui.internal.presentations.util.TabOrder#add(org.blueberry.ui.presentations.IPresentablePart)
     */
    void Add(IPresentablePart::Pointer newPart) override;

    /* (non-Javadoc)
     * @see org.blueberry.ui.internal.presentations.util.TabOrder#addInitial(org.blueberry.ui.presentations.IPresentablePart)
     */
    void AddInitial(IPresentablePart::Pointer newPart) override;

    /* (non-Javadoc)
     * @see org.blueberry.ui.internal.presentations.util.TabOrder#insert(org.blueberry.ui.presentations.IPresentablePart, int)
     */
    void Insert(IPresentablePart::Pointer newPart, int index) override;

    /* (non-Javadoc)
     * @see org.blueberry.ui.internal.presentations.util.TabOrder#remove(org.blueberry.ui.presentations.IPresentablePart)
     */
    void Remove(IPresentablePart::Pointer removed) override;

    /* (non-Javadoc)
     * @see org.blueberry.ui.internal.presentations.util.TabOrder#select(org.blueberry.ui.presentations.IPresentablePart)
     */
    void Select(IPresentablePart::Pointer selected) override;

    /* (non-Javadoc)
     * @see org.blueberry.ui.internal.presentations.util.TabOrder#move(org.blueberry.ui.presentations.IPresentablePart, int)
     */
    void Move(IPresentablePart::Pointer toMove, int newIndex) override;


    /* (non-Javadoc)
     * @see org.blueberry.ui.internal.presentations.util.TabOrder#getPartList()
     */
    QList<IPresentablePart::Pointer> GetPartList() override;

    /**
     * Restores a presentation from a previously stored state
     *
     * @param serializer (not null)
     * @param savedState (not null)
     */
    void RestoreState(IPresentationSerializer* serializer,
            IMemento::Pointer savedState) override;

    /* (non-Javadoc)
     * @see org.blueberry.ui.presentations.StackPresentation#saveState(org.blueberry.ui.presentations.IPresentationSerializer, org.blueberry.ui.IMemento)
     */
    void SaveState(IPresentationSerializer* context, IMemento::Pointer memento) override;
};

}

#endif /* BERRYLEFTTORIGHTTABORDER_H_ */
