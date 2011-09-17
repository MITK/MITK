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


#ifndef BERRYISTACKABLECONTAINER_H_
#define BERRYISTACKABLECONTAINER_H_

#include "berryStackablePart.h"

#include "berryPartPane.h"

#include <list>

namespace berry {

/**
 * \ingroup org_blueberry_ui_internal
 *
 */
struct IStackableContainer : virtual public Object {

  berryObjectMacro(IStackableContainer);

  typedef std::list<StackablePart::Pointer> ChildrenType;

  virtual bool AllowsAdd(StackablePart::Pointer toAdd) = 0;

    /**
     * Add a child to the container.
     */
  virtual void Add(StackablePart::Pointer newPart) = 0;

  /**
   * Returnd the id for this stackable container
   */
  virtual std::string GetID() const = 0;

    /**
     * Returns a list of layout children.
     */
  virtual ChildrenType GetChildren() const = 0;

    /**
     * Remove a child from the container.
     */
  virtual void Remove(StackablePart::Pointer part) = 0;

    /**
     * Replace one child with another
     */
  virtual void Replace(StackablePart::Pointer oldPart, StackablePart::Pointer newPart) = 0;

  virtual void FindSashes(PartPane::Sashes& result) = 0;

    /**
     * When a layout part closes, focus will return to the previously active part.
     * This method determines whether the parts in this container should participate
     * in this behavior. If this method returns true, its parts may automatically be
     * given focus when another part is closed.
     *
     * @return true iff the parts in this container may be given focus when the active
     * part is closed
     */
  virtual bool AllowsAutoFocus() = 0;

    /**
     * Called by child parts to request a zoom in, given an immediate child
     *
     * @param toZoom
     * @since 3.1
     */
    //public void childRequestZoomIn(LayoutPart toZoom);

    /**
     * Called by child parts to request a zoom out
     *
     * @since 3.1
     */
    //public void childRequestZoomOut();

    /**
     * Returns true iff the given child is obscured due to the fact that the container is zoomed into
     * another part.
     *
     * @param toTest
     * @return
     * @since 3.1
     */
    //public boolean childObscuredByZoom(LayoutPart toTest);

    /**
     * Returns true iff we are zoomed into the given part, given an immediate child of this container.
     *
     * @param toTest
     * @return
     * @since 3.1
     */
    //public boolean childIsZoomed(LayoutPart toTest);

    /**
     * Called when the preferred size of the given child has changed, requiring a
     * layout to be triggered.
     *
     * @param childThatChanged the child that triggered the new layout
     */
    virtual void ResizeChild(StackablePart::Pointer childThatChanged) = 0;

};

}

#endif /* BERRYISTACKABLECONTAINER_H_ */
