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

#ifndef CHERRYCONTAINERPLACEHOLDER_H_
#define CHERRYCONTAINERPLACEHOLDER_H_

#include "cherryIStackableContainer.h"
#include "cherryLayoutPart.h"

#include <Poco/SharedPtr.h>

#include <list>

namespace cherry {

/**
 * \ingroup org_opencherry_ui_internal
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
     * @see org.opencherry.ui.internal.ILayoutContainer#allowsAutoFocus()
     */
    bool AllowsAutoFocus();

    /* (non-Javadoc)
     * @see org.opencherry.ui.internal.ILayoutContainer#isZoomed(org.opencherry.ui.internal.LayoutPart)
     */
//    bool childIsZoomed(LayoutPart toTest) {
//        return false;
//    }
};

}

#endif /*CHERRYCONTAINERPLACEHOLDER_H_*/
