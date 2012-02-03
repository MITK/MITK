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


#ifndef BERRYSTACKPRESENTATION_H_
#define BERRYSTACKPRESENTATION_H_

#include <berryMacros.h>

#include <org_blueberry_ui_Export.h>

#include "berryIStackPresentationSite.h"
#include "berryIPresentationSerializer.h"
#include "berryStackDropResult.h"

#include "../berryISizeProvider.h"
#include "../berryIMemento.h"

namespace berry
{

/**
 * This represents an object that can supply trim around a IPresentablePart.
 * Clients can implement subclasses to provide the appearance for editor workbooks,
 * view folders, fast views, and detached windows.
 * <p>
 * StackPresentations do not store any persistent state and cannot
 * directly make changes to the workbench. They are given an IStackPresentationSite
 * reference on creation, which allows them to send events and requests to the workbench.
 * However, the workbench is free to ignore these requests. The workbench will call one
 * of the public methods on StackPresentation when (and if) the presentation is expected to
 * change state.
 * </p>
 * <p>
 * For example, if the user clicks a button that is intended to close a part, the
 * StackPresentation will send a close request to its site, but should not assume
 * that the part has been closed until the workbench responds with a call
 * <code>StackPresentation.remove</code>.
 * </p>
 *
 * @since 3.0
 */
class BERRY_UI StackPresentation : public Object, public ISizeProvider {

public:

  berryObjectMacro(StackPresentation);

private:

  /**
    * The presentation site.
    */
  IStackPresentationSite::WeakPtr site;

protected:

  /**
   * Constructs a new stack presentation with the given site.
   *
   * @param stackSite the stack site
   */
  StackPresentation(IStackPresentationSite::Pointer stackSite);

  ~StackPresentation();

  /**
   * Returns the presentation site (not null).
   * @return  IStackPresentationSite
   */
  IStackPresentationSite::Pointer GetSite();

public:

    /**
     * Inactive state. This is the default state for deselected presentations.
     */
    static const int AS_INACTIVE; // = 0;

    /**
     * Activation state indicating that one of the parts in the presentation currently has focus
     */
    static const int AS_ACTIVE_FOCUS; // = 1;

    /**
     * Activation state indicating that none of the parts in the presentation have focus, but
     * one of the parts is being used as the context for global menus and toolbars
     */
    static const int AS_ACTIVE_NOFOCUS; // = 2;

    /**
     * Sets the bounding rectangle for this presentation.
     *
     * @param bounds new bounding rectangle (not null)
     */
    virtual void SetBounds(const Rectangle& bounds) = 0;

    /**
     * Returns the minimum size for this stack. The stack is prevented
     * from being resized smaller than this amount, and this is used as
     * the default size for the stack when it is minimized. Typically,
     * this is the amount of space required to fit the minimize, close,
     * and maximize buttons and one tab.
     *
     * @return the minimum size for this stack (not null)
     *
     * @deprecated replaced by computePreferredSize
     */
    virtual Point ComputeMinimumSize();

    /*
     * @see ISizeProvider#getSizeFlags(boolean)
     */
    virtual int GetSizeFlags(bool width);

    /*
     * @see ISizeProvider#computePreferredSize(boolean, int, int, int)
     */
    virtual int ComputePreferredSize(bool width, int availableParallel, int availablePerpendicular, int preferredResult);

    /**
     * This is invoked to notify the presentation that its activation
     * state has changed. StackPresentations can have three possible activation
     * states (see the AS_* constants above)
     *
     * @param newState one of AS_INACTIVE, AS_ACTIVE, or AS_ACTIVE_NOFOCUS
     */
    virtual void SetActive(int newState) = 0;

    /**
     * This causes the presentation to become visible or invisible.
     * When a presentation is invisible, it must not respond to user
     * input or modify its parts. For example, a presentations will
     * be made invisible if it belongs to a perspective and the user
     * switches to another perspective.
     *
     * @param isVisible the state to set visibility to
     *
     * @since 3.0
     */
    virtual void SetVisible(bool isVisible) = 0;

    /**
     * Sets the state of the presentation. That is, notifies the presentation
     * that is has been minimized, maximized, or restored. Note that this method
     * is the only way that a presentation is allowed to change its state.
     * <p>
     * If a presentation wishes to minimize itself, it must call setState
     * on its associated IStackPresentationSite. If the site chooses to respond
     * to the state change, it will call this method at the correct time.
     * The presentation should not call this method directly.
     * </p>
     *
     * @param state one of the IStackPresentationSite.STATE_* constants.
     */
    virtual void SetState(int state) = 0;

    /**
     * Returns the control for this presentation
     *
     * @return the control for this presentation (not null)
     */
    virtual void* GetControl() = 0;

    /**
     * Adds the given part to the stack. The presentation is free to determine
     * where the part should be inserted. If the part is being inserted as the
     * result of a drag/drop operation, it will be given a cookie
     * identifying the drop location. Has no effect if an identical part is
     * already in the presentation.
     *
     * @param newPart the new part to add (not null)
     * @param cookie an identifier for a drop location, or null. When the presentation
     * attaches a cookie to a StackDropResult, that cookie is passed back into
     * addPart when a part is actually dropped in that location.
     */
    virtual void AddPart(IPresentablePart::Pointer newPart, Object::Pointer cookie) = 0;

    /**
     * Removes the given part from the stack.
     *
     * @param oldPart the part to remove (not null)
     */
    virtual void RemovePart(IPresentablePart::Pointer oldPart) = 0;

    /**
     * Moves a part to a new location as the result of a drag/drop
     * operation within this presentation.
     *
     * @param toMove a part that already belongs to this presentation
     * @param cookie a drop cookie returned by <code>StackPresentation#dragOver</code>
     * @since 3.1
     */
    virtual void MovePart(IPresentablePart::Pointer toMove, Object::Pointer cookie);

    /**
     * Brings the specified part to the foreground. This should not affect
     * the current focus.
     *
     * @param toSelect the new active part (not null)
     */
    virtual void SelectPart(IPresentablePart::Pointer toSelect) = 0;

    /**
     * This method is invoked whenever a part is dragged over the stack's control.
     * It returns a StackDropResult if and only if the part may be dropped in this
     * location.
     *
     * @param currentControl the control being dragged over
     * @param location cursor location (display coordinates)
     * @return a StackDropResult or null if the presentation does not have
     * a drop target in this location.
     */
    virtual StackDropResult::Pointer DragOver(void* currentControl, const Point& location) = 0;

    /**
     * Instructs the presentation to display the system menu
     *
     */
 //   virtual void ShowSystemMenu() = 0;

    /**
     * Instructs the presentation to display the pane menu
     */
 //   virtual void ShowPaneMenu() = 0;

    /**
     * Instructs the presentation to display a list of all parts in the stack, and
     * allow the user to change the selection using the keyboard.
     */
    virtual void ShowPartList();

    /**
     * Saves the state of this presentation to the given memento.
     *
     * @param context object that can be used to generate unique IDs for IPresentableParts (this
     * may be a temporary object - the presentation should not keep any references to it)
     * @param memento memento where the data will be saved
     */
    virtual void SaveState(IPresentationSerializer* context, IMemento::Pointer memento);

    /**
     * Restores the state of this presentation to a previously saved state.
     *
     * @param context object that can be used to find IPresentableParts given string IDs (this
     * may be a temporary object - the presentation should not keep any references to it)
     * @param memento memento where the data will be saved
     */
    virtual void RestoreState(IPresentationSerializer* context, IMemento::Pointer memento);

    /**
     * Returns the tab-key traversal order for the given <code>IPresentablePart</code>.
     *
     * @param part the part
     * @return the tab-key traversal order
     */
    virtual std::vector<void*> GetTabList(IPresentablePart::Pointer part) = 0;
};

}

#endif /* BERRYSTACKPRESENTATION_H_ */
