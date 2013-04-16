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


#ifndef BERRYTABORDER_H_
#define BERRYTABORDER_H_

#include <berryIPresentablePart.h>
#include <berryIPresentationSerializer.h>
#include <berryIMemento.h>

#include <org_blueberry_ui_qt_Export.h>

namespace berry {

struct BERRY_UI_QT TabOrder {

    virtual ~TabOrder();

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


#endif /* BERRYTABORDER_H_ */
