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


#ifndef BERRYVIEWSASHCONTAINER_H_
#define BERRYVIEWSASHCONTAINER_H_

#include "berryPartSashContainer.h"
#include "berryPartStack.h"

namespace berry {

/**
 * Represents the top level container.
 */
class ViewSashContainer : public PartSashContainer {

public:

  berryObjectMacro(ViewSashContainer);

  ViewSashContainer(WorkbenchPage* page, void* parent);

    /**
     * Gets root container for this part.
     */
    ViewSashContainer::Pointer GetRootContainer();

    /**
     * Get the part control.  This method may return null.
     */
    void* GetControl();

    /**
     * @see IPersistablePart
     */
    bool RestoreState(IMemento::Pointer memento);

    /**
     * @see IPersistablePart
     */
    bool SaveState(IMemento::Pointer memento);

    /* (non-Javadoc)
     * @see org.blueberry.ui.internal.PartSashContainer#isStackType(org.blueberry.ui.internal.LayoutPart)
     */
    bool IsStackType(IStackableContainer::Pointer toTest);

    /* (non-Javadoc)
     * @see org.blueberry.ui.internal.PartSashContainer#isPaneType(org.blueberry.ui.internal.LayoutPart)
     */
    bool IsPaneType(StackablePart::Pointer toTest);

    /* (non-Javadoc)
     * @see org.blueberry.ui.internal.ILayoutContainer#replace(org.blueberry.ui.internal.LayoutPart, org.blueberry.ui.internal.LayoutPart)
     */
    //void Replace(LayoutPart::Pointer oldChild, LayoutPart::Pointer newChild);

    bool AllowsAdd(LayoutPart::Pointer layoutPart);


protected:

    /**
     * Subclasses override this method to specify
     * the composite to use to parent all children
     * layout parts it contains.
     */
    void* CreateParent(void* parentWidget);

    /**
     * Subclasses override this method to dispose
     * of any swt resources created during createParent.
     */
    void DisposeParent();

    /* (non-Javadoc)
     * @see org.blueberry.ui.internal.PartSashContainer#getDockingRatio(org.blueberry.ui.internal.LayoutPart, org.blueberry.ui.internal.LayoutPart)
     */
    float GetDockingRatio(Object::Pointer dragged, IStackableContainer::Pointer target);

    /* (non-Javadoc)
     * @see org.blueberry.ui.internal.PartSashContainer#createStack(org.blueberry.ui.internal.LayoutPart)
     */
    PartStack::Pointer CreateStack();

    /* (non-Javadoc)
     * @see org.blueberry.ui.internal.PartSashContainer#setVisiblePart(org.blueberry.ui.internal.ILayoutContainer, org.blueberry.ui.internal.LayoutPart)
     */
    void SetVisiblePart(IStackableContainer::Pointer container,
            PartPane::Pointer visiblePart);

    /* (non-Javadoc)
     * @see org.blueberry.ui.internal.PartSashContainer#getVisiblePart(org.blueberry.ui.internal.ILayoutContainer)
     */
    StackablePart::Pointer GetVisiblePart(IStackableContainer::Pointer container);

    /* (non-Javadoc)
     * @see org.blueberry.ui.internal.PartSashContainer#derefPart(org.blueberry.ui.internal.LayoutPart)
     */
    void DerefPart(StackablePart::Pointer sourcePart);

    /* (non-Javadoc)
     * @see org.blueberry.ui.internal.PartSashContainer#addChild(org.blueberry.ui.internal.PartSashContainer.RelationshipInfo)
     */
    //void AddChild(const RelationshipInfo& info);

};

}

#endif /* BERRYVIEWSASHCONTAINER_H_ */
