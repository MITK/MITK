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

#include "cherryILayoutContainer.h"
#include "cherryPartPlaceholder.h"

#include <Poco/SharedPtr.h>

namespace cherry {

/**
 * \ingroup org_opencherry_ui_internal
 * 
 */
class ContainerPlaceholder : public PartPlaceholder, public ILayoutContainer {
    
private:
  
  static int nextId;

  Poco::SharedPtr<ILayoutContainer> realContainer;

public:
  
  cherryClassMacro(ContainerPlaceholder);
  
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

    /**
     * See ILayoutContainer::allowBorder
     */
    bool AllowsBorder();

    /**
     * getChildren method comment.
     */
    std::vector<LayoutPart::Pointer> GetChildren();

    /**
     * getFocus method comment.
     */
    LayoutPart::Pointer GetFocus();

    /**
     * getFocus method comment.
     */
    LayoutPart::Pointer GetRealContainer();

    /**
     * isChildVisible method comment.
     */
    bool IsChildVisible(LayoutPart::Pointer child);

    /**
     * remove method comment.
     */
    void Remove(LayoutPart::Pointer child);

    /**
     * replace method comment.
     */
    void Replace(LayoutPart::Pointer oldChild, LayoutPart::Pointer newChild);

    /**
     * setChildVisible method comment.
     */
    void SetChildVisible(LayoutPart::Pointer child, bool visible);

    /**
     * setFocus method comment.
     */
    void SetFocus(LayoutPart::Pointer child);

    void SetRealContainer(Poco::SharedPtr<ILayoutContainer> container);

//    void FindSashes(LayoutPart::Pointer part, PartPane.Sashes sashes) {
//        ILayoutContainer container = getContainer();
//
//        if (container != null) {
//            container.findSashes(this, sashes);
//        }
//    }

    /* (non-Javadoc)
     * @see org.eclipse.ui.internal.ILayoutContainer#allowsAutoFocus()
     */
    bool AllowsAutoFocus();

    /* (non-Javadoc)
     * @see org.eclipse.ui.internal.ILayoutContainer#isZoomed(org.eclipse.ui.internal.LayoutPart)
     */
//    bool childIsZoomed(LayoutPart toTest) {
//        return false;
//    }
};

}

#endif /*CHERRYCONTAINERPLACEHOLDER_H_*/
