/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYILAYOUTCONTAINER_H_
#define BERRYILAYOUTCONTAINER_H_

#include "berryLayoutPart.h"

#include "berryPartPane.h"


namespace berry {


/**
 * \ingroup org_blueberry_ui_internal
 *
 */
struct BERRY_UI_QT ILayoutContainer : virtual public Object {

  berryObjectMacro(ILayoutContainer);

  ~ILayoutContainer() override;

  typedef QList<LayoutPart::Pointer> ChildrenType;

  virtual bool AllowsAdd(LayoutPart::Pointer toAdd) = 0;

    /**
     * Add a child to the container.
     */
  virtual void Add(LayoutPart::Pointer newPart) = 0;

    /**
     * Returns a list of layout children.
     */
  virtual ChildrenType GetChildren() const = 0;

    /**
     * Remove a child from the container.
     */
  virtual void Remove(LayoutPart::Pointer part) = 0;

    /**
     * Replace one child with another
     */
  virtual void Replace(LayoutPart::Pointer oldPart, LayoutPart::Pointer newPart) = 0;

  virtual void FindSashes(LayoutPart::Pointer toFind, PartPane::Sashes& result) = 0;

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
    virtual void ResizeChild(LayoutPart::Pointer childThatChanged) = 0;

};

}

#endif /*BERRYILAYOUTCONTAINER_H_*/
