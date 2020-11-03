/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
    ContainerPlaceholder(const QString& id);

    /**
     * add method comment.
     */
    void Add(LayoutPart::Pointer child) override;

    bool AllowsAdd(LayoutPart::Pointer toAdd) override;

    /**
     * getChildren method comment.
     */
    QList<LayoutPart::Pointer> GetChildren() const override;

    QString GetID() const override;

    /**
     * getFocus method comment.
     */
    LayoutPart::Pointer GetRealContainer();

    /**
     * remove method comment.
     */
    void Remove(LayoutPart::Pointer child) override;

    /**
     * replace method comment.
     */
    void Replace(LayoutPart::Pointer oldChild, LayoutPart::Pointer newChild) override;

    void SetRealContainer(ILayoutContainer::Pointer container);

    void FindSashes(LayoutPart::Pointer part, PartPane::Sashes& sashes) override;

    void ResizeChild(LayoutPart::Pointer childThatChanged) override;

    /* (non-Javadoc)
     * @see org.blueberry.ui.internal.ILayoutContainer#allowsAutoFocus()
     */
    bool AllowsAutoFocus() override;

    /* (non-Javadoc)
     * @see org.blueberry.ui.internal.ILayoutContainer#isZoomed(org.blueberry.ui.internal.LayoutPart)
     */
//    bool childIsZoomed(LayoutPart toTest) {
//        return false;
//    }
};

}

#endif /*BERRYCONTAINERPLACEHOLDER_H_*/
