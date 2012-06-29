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

#ifndef BERRYCONTAINERPLACEHOLDER_H_
#define BERRYCONTAINERPLACEHOLDER_H_

#include "berryILayoutContainer.h"
#include "berryPartPlaceholder.h"

#include <Poco/SharedPtr.h>

#include <list>

namespace berry {

/**
 * \ingroup org_blueberry_ui_internal
 *
 */
class ContainerPlaceholder : public PartPlaceholder, public ILayoutContainer {

private:

  static int nextId;

  ILayoutContainer::Pointer realContainer;

public:

  berryObjectMacro(ContainerPlaceholder);

    /**
     * ContainerPlaceholder constructor comment.
     * @param id java.lang.String
     * @param label java.lang.String
     */
    ContainerPlaceholder(const std::string& id);

    /**
     * add method comment.
     */
    void Add(LayoutPart::Pointer child);

    bool AllowsAdd(LayoutPart::Pointer toAdd);

    /**
     * getChildren method comment.
     */
    std::list<LayoutPart::Pointer> GetChildren() const;

    std::string GetID() const;

    /**
     * getFocus method comment.
     */
    LayoutPart::Pointer GetRealContainer();

    /**
     * remove method comment.
     */
    void Remove(LayoutPart::Pointer child);

    /**
     * replace method comment.
     */
    void Replace(LayoutPart::Pointer oldChild, LayoutPart::Pointer newChild);

    void SetRealContainer(ILayoutContainer::Pointer container);

    void FindSashes(LayoutPart::Pointer part, PartPane::Sashes& sashes);

    void ResizeChild(LayoutPart::Pointer childThatChanged);

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
