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

#ifndef BERRYCONTAINERPLACEHOLDER_H_
#define BERRYCONTAINERPLACEHOLDER_H_

#include "berryIStackableContainer.h"
#include "berryLayoutPart.h"

#include <Poco/SharedPtr.h>

#include <list>

namespace berry {

/**
 * \ingroup org_blueberry_ui_internal
 *
 */
class ContainerPlaceholder : public LayoutPart, public IStackableContainer {

private:

  static int nextId;

  IStackableContainer::Pointer realContainer;

public:

  osgiObjectMacro(ContainerPlaceholder);

    /**
     * ContainerPlaceholder constructor comment.
     * @param id java.lang.String
     * @param label java.lang.String
     */
    ContainerPlaceholder(const std::string& id);

    /**
     * Creates the SWT control
     */
    void CreateControl(void* parent);

    /**
     * Get the part control.  This method may return null.
     */
    void* GetControl();

    /**
     * add method comment.
     */
    void Add(StackablePart::Pointer child);

    bool AllowsAdd(StackablePart::Pointer toAdd);

    /**
     * getChildren method comment.
     */
    std::list<StackablePart::Pointer> GetChildren() const;

    std::string GetID() const;

    /**
     * getFocus method comment.
     */
    IStackableContainer::Pointer GetRealContainer();

    /**
     * remove method comment.
     */
    void Remove(StackablePart::Pointer child);

    /**
     * replace method comment.
     */
    void Replace(StackablePart::Pointer oldChild, StackablePart::Pointer newChild);

    void SetRealContainer(IStackableContainer::Pointer container);

    void FindSashes(PartPane::Sashes& sashes);

    void ResizeChild(StackablePart::Pointer childThatChanged);

    /* (non-Javadoc)
     * @see org.blueberry.ui.internal.ILayoutContainer#allowsAutoFocus()
     */
    bool AllowsAutoFocus();

    /* (non-Javadoc)
     * @see org.blueberry.ui.internal.ILayoutContainer#isZoomed(org.blueberry.ui.internal.LayoutPart)
     */
//    bool childIsZoomed(LayoutPart toTest) {
//        return false;
//    }
};

}

#endif /*BERRYCONTAINERPLACEHOLDER_H_*/
