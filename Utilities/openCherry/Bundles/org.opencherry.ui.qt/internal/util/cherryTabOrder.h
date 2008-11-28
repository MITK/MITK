/*=========================================================================

 Program:   openCherry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision$

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY = 0; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/


#ifndef CHERRYTABORDER_H_
#define CHERRYTABORDER_H_

#include <presentations/cherryIPresentablePart.h>
#include <presentations/cherryIPresentationSerializer.h>
#include <cherryIMemento.h>

namespace cherry {

struct TabOrder {
    /**
     * Adds a part due to a user action that opened a part
     *
     * @param newPart part being added
     */
    virtual void Add(IPresentablePart::Pointer newPart) = 0;

    /**
     * Adds a part at initialization-time (the part was added as
     * part of a perspective, rather than by a user action)
     *
     * @param newPart the part being added
     */
    virtual void AddInitial(IPresentablePart::Pointer newPart) = 0;

    virtual void RestoreState(IPresentationSerializer* serializer,
            IMemento::Pointer savedState) = 0;

    virtual void SaveState(IPresentationSerializer* serializer, IMemento::Pointer memento) = 0;

    /**
     * Adds a part at a particular index due to a drag/drop operation.
     *
     * @param added part being added
     * @param index index where the part is added at
     */
    virtual void Insert(IPresentablePart::Pointer added, int index) = 0;

    virtual void Move(IPresentablePart::Pointer toMove, int newIndex) = 0;

    /**
     * Removes a part
     *
     * @param removed part being removed
     */
    virtual void Remove(IPresentablePart::Pointer removed) = 0;

    /**
     * Selects a part
     *
     * @param selected part being selected
     */
    virtual void Select(IPresentablePart::Pointer selected) = 0;

    virtual std::vector<IPresentablePart::Pointer> GetPartList() = 0;
};

}


#endif /* CHERRYTABORDER_H_ */
