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

#include "cherryContainerPlaceholder.h"

namespace cherry {

int ContainerPlaceholder::nextId = 0;

ContainerPlaceholder::ContainerPlaceholder(const std::string& id) {
        super(((id == null) ? "Container Placeholder " + nextId++ : id)); //$NON-NLS-1$
    }

    void ContainerPlaceholder::Add(LayoutPart::Pointer child) {
        if (!(child instanceof PartPlaceholder)) {
      return;
    }
        realContainer.add(child);
    }

    bool ContainerPlaceholder::AllowsBorder() {
        return true;
    }

    std::vector<LayoutPart::Pointer> ContainerPlaceholder::GetChildren() {
        return realContainer.getChildren();
    }

    LayoutPart::Pointer ContainerPlaceholder::GetFocus() {
        return null;
    }

    LayoutPart::Pointer ContainerPlaceholder::GetRealContainer() {
        return (LayoutPart) realContainer;
    }

    bool ContainerPlaceholder::IsChildVisible(LayoutPart::Pointer child) {
        return false;
    }

    void ContainerPlaceholder::Remove(LayoutPart::Pointer child) {
        if (!(child instanceof PartPlaceholder)) {
      return;
    }
        realContainer.remove(child);
    }

    void ContainerPlaceholder::Replace(LayoutPart::Pointer oldChild, LayoutPart::Pointer newChild) {
        if (!(oldChild instanceof PartPlaceholder)
                && !(newChild instanceof PartPlaceholder)) {
      return;
    }
        realContainer.replace(oldChild, newChild);
    }

    void ContainerPlaceholder::SetChildVisible(LayoutPart::Pointer child, bool visible) {
    }

    void ContainerPlaceholder::SetFocus(LayoutPart::Pointer child) {
    }

    void ContainerPlaceholder::SetRealContainer(ILayoutContainer::Pointer container) {

        if (container == null) {
            // set the parent container of the children back to the real container
            if (realContainer != null) {
                LayoutPart[] children = realContainer.getChildren();
                if (children != null) {
                    for (int i = 0, length = children.length; i < length; i++) {
                        children[i].setContainer(realContainer);
                    }
                }
            }
        } else {
            // replace the real container with this place holder
            LayoutPart[] children = container.getChildren();
            if (children != null) {
                for (int i = 0, length = children.length; i < length; i++) {
                    children[i].setContainer(this);
                }
            }
        }

        this.realContainer = container;
    }

    bool ContainerPlaceholder::AllowsAutoFocus() {
        return false;
    }

}
