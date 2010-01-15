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


#ifndef CHERRYLEFTTORIGHTTABORDER_H_
#define CHERRYLEFTTORIGHTTABORDER_H_

#include "cherryTabOrder.h"

#include "cherryIPresentablePartList.h"

namespace cherry {

class LeftToRightTabOrder : public TabOrder {

private:

  IPresentablePartList* list;

public:

  LeftToRightTabOrder(IPresentablePartList* list);

    /* (non-Javadoc)
     * @see org.opencherry.ui.internal.presentations.util.TabOrder#add(org.opencherry.ui.presentations.IPresentablePart)
     */
    void Add(IPresentablePart::Pointer newPart);

    /* (non-Javadoc)
     * @see org.opencherry.ui.internal.presentations.util.TabOrder#addInitial(org.opencherry.ui.presentations.IPresentablePart)
     */
    void AddInitial(IPresentablePart::Pointer newPart);

    /* (non-Javadoc)
     * @see org.opencherry.ui.internal.presentations.util.TabOrder#insert(org.opencherry.ui.presentations.IPresentablePart, int)
     */
    void Insert(IPresentablePart::Pointer newPart, int index);

    /* (non-Javadoc)
     * @see org.opencherry.ui.internal.presentations.util.TabOrder#remove(org.opencherry.ui.presentations.IPresentablePart)
     */
    void Remove(IPresentablePart::Pointer removed);

    /* (non-Javadoc)
     * @see org.opencherry.ui.internal.presentations.util.TabOrder#select(org.opencherry.ui.presentations.IPresentablePart)
     */
    void Select(IPresentablePart::Pointer selected);

    /* (non-Javadoc)
     * @see org.opencherry.ui.internal.presentations.util.TabOrder#move(org.opencherry.ui.presentations.IPresentablePart, int)
     */
    void Move(IPresentablePart::Pointer toMove, int newIndex);


    /* (non-Javadoc)
     * @see org.opencherry.ui.internal.presentations.util.TabOrder#getPartList()
     */
    std::vector<IPresentablePart::Pointer> GetPartList();

    /**
     * Restores a presentation from a previously stored state
     *
     * @param serializer (not null)
     * @param savedState (not null)
     */
    void RestoreState(IPresentationSerializer* serializer,
            IMemento::Pointer savedState);

    /* (non-Javadoc)
     * @see org.opencherry.ui.presentations.StackPresentation#saveState(org.opencherry.ui.presentations.IPresentationSerializer, org.opencherry.ui.IMemento)
     */
    void SaveState(IPresentationSerializer* context, IMemento::Pointer memento);
};

}

#endif /* CHERRYLEFTTORIGHTTABORDER_H_ */
