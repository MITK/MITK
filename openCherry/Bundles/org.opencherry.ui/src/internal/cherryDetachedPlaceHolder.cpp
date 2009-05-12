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

#include "cherryDetachedPlaceHolder.h"

#include "cherryILayoutContainer.h"
#include "cherryPartPlaceholder.h"

namespace cherry
{

DetachedPlaceHolder::DetachedPlaceHolder(const std::string& id,
    const Rectangle& b) :
  ContainerPlaceholder(id), bounds(b)
{

}

void DetachedPlaceHolder::Add(StackablePart::Pointer newPart)
{
  if (newPart.Cast<PartPlaceholder> () == 0)
  {
    return;
  }
  children.push_back(newPart);
}

bool DetachedPlaceHolder::AllowsBorder()
{
  return false;
}

Rectangle DetachedPlaceHolder::GetBounds()
{
  return bounds;
}

std::list<StackablePart::Pointer> DetachedPlaceHolder::GetChildren()
{
  return children;
}

void DetachedPlaceHolder::Remove(StackablePart::Pointer part)
{
  children.remove(part);
}

void DetachedPlaceHolder::Replace(StackablePart::Pointer oldPart,
    StackablePart::Pointer newPart)
{
  this->Remove(oldPart);
  this->Add(newPart);
}

void DetachedPlaceHolder::RestoreState(IMemento::Pointer memento)
{

  //TODO DetachedPlaceHolder restore state
  //     // Read the bounds.
  //     Integer bigInt;
  //     bigInt = memento.getInteger(IWorkbenchConstants.TAG_X);
  //     int x = bigInt.intValue();
  //     bigInt = memento.getInteger(IWorkbenchConstants.TAG_Y);
  //     int y = bigInt.intValue();
  //     bigInt = memento.getInteger(IWorkbenchConstants.TAG_WIDTH);
  //     int width = bigInt.intValue();
  //     bigInt = memento.getInteger(IWorkbenchConstants.TAG_HEIGHT);
  //     int height = bigInt.intValue();
  //
  //     bounds = new Rectangle(x, y, width, height);
  //
  //     // Restore the placeholders.
  //     IMemento childrenMem[] = memento
  //             .getChildren(IWorkbenchConstants.TAG_VIEW);
  //     for (int i = 0; i < childrenMem.length; i++) {
  //         PartPlaceholder holder = new PartPlaceholder(childrenMem[i]
  //                 .getString(IWorkbenchConstants.TAG_ID));
  //         holder.setContainer(this);
  //         children.add(holder);
  //     }
}

void DetachedPlaceHolder::SaveState(IMemento::Pointer memento)
{
  //TODO DetachedPlaceHolder save state
  //     // Save the bounds.
  //     memento.putInteger(IWorkbenchConstants.TAG_X, bounds.x);
  //     memento.putInteger(IWorkbenchConstants.TAG_Y, bounds.y);
  //     memento.putInteger(IWorkbenchConstants.TAG_WIDTH, bounds.width);
  //     memento.putInteger(IWorkbenchConstants.TAG_HEIGHT, bounds.height);
  //
  //     // Save the views.
  //     for (int i = 0; i < children.size(); i++) {
  //         IMemento childMem = memento
  //                 .createChild(IWorkbenchConstants.TAG_VIEW);
  //         LayoutPart child = (LayoutPart) children.get(i);
  //         childMem.putString(IWorkbenchConstants.TAG_ID, child.getID());
  //     }
}

void DetachedPlaceHolder::FindSashes(LayoutPart::Pointer part,
    PartPane::Sashes& sashes)
{
  ILayoutContainer::Pointer container = this->GetContainer();

  if (container != 0)
  {
    container->FindSashes(LayoutPart::Pointer(this), sashes);
  }
}


}
