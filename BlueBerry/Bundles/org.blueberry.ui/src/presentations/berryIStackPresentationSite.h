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

#ifndef BERRYISTACKPRESENTATIONSITE_H_
#define BERRYISTACKPRESENTATIONSITE_H_

#include <berryMacros.h>

#include <list>

#include "berryIPresentablePart.h"

#include "../berryUiDll.h"
#include "../berryPoint.h"

namespace berry
{

/**
 * Represents the main interface between a StackPresentation and the workbench.
 *
 * Not intended to be implemented by clients.
 *
 * @since 3.0
 * @noimplement This interface is not intended to be implemented by clients.
 */
struct BERRY_UI IStackPresentationSite : public Object
{

  berryInterfaceMacro(IStackPresentationSite, berry);

  static int STATE_MINIMIZED; // = 0;

  static int STATE_MAXIMIZED; // = 1;

  static int STATE_RESTORED; // = 2;

  /**
   * Sets the state of the container. Called by the presentation when the
   * user causes the the container to be minimized, maximized, etc.
   *
   * @param newState one of the STATE_* constants
   */
  virtual void SetState(int newState) = 0;

  /**
   * Returns the current state of the site (one of the STATE_* constants)
   *
   * @return the current state of the site (one of the STATE_* constants)
   */
  virtual int GetState() = 0;

  /**
   * Returns true iff the site supports the given state
   *
   * @param state one of the STATE_* constants, above
   * @return true iff the site supports the given state
   */
  virtual bool SupportsState(int state) = 0;

  /**
   * Begins dragging the given part
   *
   * @param beingDragged the part to drag (not null)
   * @param initialPosition the mouse position at the time of the initial mousedown
   * (display coordinates, not null)
   * @param keyboard true iff the drag was initiated via mouse dragging,
   * and false if the drag may be using the keyboard
   */
  virtual void DragStart(IPresentablePart::Pointer beingDragged,
      Point& initialPosition, bool keyboard) = 0;

  /**
   * Closes the given set of parts.
   *
   * @param toClose the set of parts to close (Not null. All of the entries must be non-null)
   */
  virtual void Close(const std::vector<IPresentablePart::Pointer>& toClose) = 0;

  /**
   * Begins dragging the entire stack of parts
   *
   * @param initialPosition the mouse position at the time of the initial mousedown (display coordinates,
   * not null)
   * @param keyboard true iff the drag was initiated via mouse dragging,
   * and false if the drag may be using the keyboard
   */
  virtual void DragStart(Point& initialPosition, bool keyboard) = 0;

  /**
   * Returns true iff this site will allow the given part to be closed
   *
   * @param toClose part to test (not null)
   * @return true iff the part may be closed
   */
  virtual bool IsCloseable(IPresentablePart::Pointer toClose) = 0;

  /**
   * Returns true iff the given part can be dragged. If this
   * returns false, the given part should not trigger a drag.
   *
   * @param toMove part to test (not null)
   * @return true iff this part is a valid drag source
   */
  virtual bool IsPartMoveable(IPresentablePart::Pointer toMove) = 0;

  /**
   * Returns true iff this entire stack can be dragged
   *
   * @return true iff the stack can be dragged
   */
  virtual bool IsStackMoveable() = 0;

  /**
   * Makes the given part active
   *
   * @param toSelect
   */
  virtual void SelectPart(IPresentablePart::Pointer toSelect) = 0;

  /**
   * Returns the currently selected part or null if the stack is empty
   *
   * @return the currently selected part or null if the stack is empty
   */
  virtual IPresentablePart::Pointer GetSelectedPart() = 0;

  /**
   * Adds system actions to the given menu manager. The site may
   * make use of the following group ids:
   * <ul>
   * <li><code>close</code>, for close actions</li>
   * <li><code>size</code>, for resize actions</li>
   * <li><code>misc</code>, for miscellaneous actions</li>
   * </ul>
   * The presentation can control the insertion position by creating
   * these group IDs where appropriate.
   *
   * @param menuManager the menu manager to populate
   */
  //virtual void AddSystemActions(IMenuManager menuManager);

  /**
   * Notifies the workbench that the preferred size of the presentation has
   * changed. Hints to the workbench that it should trigger a layout at the
   * next opportunity.
   *
   * @since 3.1
   */
  virtual void FlushLayout() = 0;

  /**
   * Returns the list of presentable parts currently in this site
   *
   * @return the list of presentable parts currently in this site
   * @since 3.1
   */
  virtual std::list<IPresentablePart::Pointer> GetPartList() = 0;

  /**
   * Returns the property with the given id or <code>null</code>. Folder
   * properties are an extensible mechanism for perspective authors to
   * customize the appearance of view stacks. The list of customizable
   * properties is determined by the presentation factory, and set in the
   * perspective factory.
   *
   * @param id
   *            Must not be <code>null</code>.
   * @return property value, or <code>null</code> if the property is not
   *         set.
   * @since 3.3
   */
  virtual std::string GetProperty(const std::string& id) = 0;
};

}

#endif /* BERRYISTACKPRESENTATIONSITE_H_ */
