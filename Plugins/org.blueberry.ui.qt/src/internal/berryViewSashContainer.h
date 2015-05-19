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

  ViewSashContainer(WorkbenchPage* page, QWidget* parent);

    /**
     * Gets root container for this part.
     */
    ViewSashContainer::Pointer GetRootContainer();

    /**
     * Get the part control.  This method may return null.
     */
    QWidget* GetControl() override;

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
    bool IsStackType(ILayoutContainer::Pointer toTest) override;

    /* (non-Javadoc)
     * @see org.blueberry.ui.internal.PartSashContainer#isPaneType(org.blueberry.ui.internal.LayoutPart)
     */
    bool IsPaneType(LayoutPart::Pointer toTest) override;

    /* (non-Javadoc)
     * @see org.blueberry.ui.internal.ILayoutContainer#replace(org.blueberry.ui.internal.LayoutPart, org.blueberry.ui.internal.LayoutPart)
     */
    //void Replace(LayoutPart::Pointer oldChild, LayoutPart::Pointer newChild);

    bool AllowsAdd(LayoutPart::Pointer layoutPart) override;

    void Replace(LayoutPart::Pointer oldChild, LayoutPart::Pointer newChild) override;


protected:

    /**
     * Subclasses override this method to specify
     * the composite to use to parent all children
     * layout parts it contains.
     */
    QWidget* CreateParent(QWidget* parentWidget) override;

    /**
     * Subclasses override this method to dispose
     * of any swt resources created during createParent.
     */
    void DisposeParent() override;

    /* (non-Javadoc)
     * @see org.blueberry.ui.internal.PartSashContainer#getDockingRatio(org.blueberry.ui.internal.LayoutPart, org.blueberry.ui.internal.LayoutPart)
     */
    float GetDockingRatio(Object::Pointer dragged, ILayoutContainer::Pointer target) override;

    /* (non-Javadoc)
     * @see org.blueberry.ui.internal.PartSashContainer#createStack(org.blueberry.ui.internal.LayoutPart)
     */
    PartStack::Pointer CreateStack() override;

    /* (non-Javadoc)
     * @see org.blueberry.ui.internal.PartSashContainer#setVisiblePart(org.blueberry.ui.internal.ILayoutContainer, org.blueberry.ui.internal.LayoutPart)
     */
    void SetVisiblePart(ILayoutContainer::Pointer container,
            PartPane::Pointer visiblePart) override;

    /* (non-Javadoc)
     * @see org.blueberry.ui.internal.PartSashContainer#getVisiblePart(org.blueberry.ui.internal.ILayoutContainer)
     */
    LayoutPart::Pointer GetVisiblePart(ILayoutContainer::Pointer container) override;

    /* (non-Javadoc)
     * @see org.blueberry.ui.internal.PartSashContainer#derefPart(org.blueberry.ui.internal.LayoutPart)
     */
    void DerefPart(LayoutPart::Pointer sourcePart) override;

    /* (non-Javadoc)
     * @see org.blueberry.ui.internal.PartSashContainer#addChild(org.blueberry.ui.internal.PartSashContainer.RelationshipInfo)
     */
    //void AddChild(const RelationshipInfo& info);

};

}

#endif /* BERRYVIEWSASHCONTAINER_H_ */
