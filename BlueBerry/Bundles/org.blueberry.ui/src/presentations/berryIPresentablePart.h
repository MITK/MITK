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


#ifndef BERRYIPRESENTABLEPART_H_
#define BERRYIPRESENTABLEPART_H_

#include <berryMacros.h>

#include "../berryISizeProvider.h"
#include "../berryRectangle.h"
#include "../berryIPropertyChangeListener.h"

namespace berry {

/**
 * This is a skin's interface to the contents of a view or editor. Note that this
 * is essentially the same as IWorkbenchPart, except it does not provide access
 * to lifecycle events and allows repositioning of the part.
 *
 * Not intended to be implemented by clients.
 *
 * @since 3.0
 * @since 3.4 now extends {@link org.blueberry.ui.ISizeProvider}
 * @noimplement This interface is not intended to be implemented by clients.
 */
struct BERRY_UI IPresentablePart : public Object, public ISizeProvider {

  berryInterfaceMacro(IPresentablePart, berry);

    /**
     * The property id for <code>isDirty</code>.
     */
    static const int PROP_DIRTY; // = IWorkbenchPartConstants.PROP_DIRTY;

    /**
     * The property id for <code>getEditorInput</code>.
     */
    static const int PROP_INPUT; // = IWorkbenchPartConstants.PROP_INPUT;

    /**
     * The property id for <code>getTitle</code>, <code>getTitleImage</code>
     * and <code>getTitleToolTip</code>.
     */
    static const int PROP_TITLE; // = IWorkbenchPartConstants.PROP_TITLE;

    /**
     * The property id for <code>IWorkbenchPart2.getContentDescription()</code>
     */
    static const int PROP_CONTENT_DESCRIPTION; // = IWorkbenchPartConstants.PROP_CONTENT_DESCRIPTION;

    /**
     * The property id for <code>IWorkbenchPart2.getContentDescription()</code>
     */
    static const int PROP_PART_NAME; // = IWorkbenchPartConstants.PROP_PART_NAME;

    /**
     * The property id for <code>isBusy</code>.
     */
    static const int PROP_BUSY; // = 0x92;

    /**
     * The property id for toolbar changes
     */
    static const int PROP_TOOLBAR; // = 0x93;

    /**
     * The property id for highlighting the
     * part if it is not in front.
     */
    static const int PROP_HIGHLIGHT_IF_BACK; // = 0x94;

    /**
     * The property id for pane menu changes
     */
    static const int PROP_PANE_MENU; // = 0x302;

    /**
     * The property id for preferred size changes
     * @since 3.4
     */
    static const int PROP_PREFERRED_SIZE; // = IWorkbenchPartConstants.PROP_PREFERRED_SIZE;

    /**
     * Sets the bounds of this part.
     *
     * @param bounds bounding rectangle (not null)
     */
    virtual void SetBounds(const Rectangle& bounds) = 0;

    /**
     * Notifies the part whether or not it is visible in the current
     * perspective. A part is visible iff any part of its widgetry can
     * be seen.
     *
     * @param isVisible true if the part has just become visible, false
     * if the part has just become hidden
     */
    virtual void SetVisible(bool isVisible) = 0;

    /**
     * Forces this part to have focus.
     */
    virtual void SetFocus() = 0;

    /**
     * Adds a listener for changes to properties of this workbench part.
     * Has no effect if an identical listener is already registered.
     * <p>
     * The properties ids are defined by the PROP_* constants, above.
     * </p>
     *
     * @param listener a property listener (not null)
     */
    virtual void AddPropertyListener(IPropertyChangeListener::Pointer listener) = 0;

    /**
     * Remove a listener that was previously added using addPropertyListener.
     *
     * @param listener a property listener (not null)
     */
    virtual void RemovePropertyListener(IPropertyChangeListener::Pointer listener) = 0;

    /**
     * Returns the short name of the part. This is used as the text on
     * the tab when this part is stacked on top of other parts.
     *
     * @return the short name of the part (not null)
     */
    virtual std::string GetName() const = 0;

    /**
     * Returns the title of this workbench part. If this value changes
     * the part must fire a property listener event with
     * <code>PROP_TITLE</code>.
     * <p>
     * The title is used to populate the title bar of this part's visual
     * container.
     * </p>
     *
     * @return the workbench part title (not null)
     */
    virtual std::string GetTitle() const = 0;

    /**
     * Returns the status message from the part's title, or the empty string if none.
     * This is a substring of the part's title. A typical title will consist of
     * the part name, a separator, and a status message describing the current contents.
     * <p>
     * Presentations can query getName() and getTitleStatus() if they want to display
     * the status message and name separately, or they can use getTitle() if they want
     * to display the entire title.
     * </p>
     *
     * @return the status message or the empty string if none (not null)
     */
    virtual std::string GetTitleStatus() const = 0;

    /**
     * Returns the title image of this workbench part.  If this value changes
     * the part must fire a property listener event with
     * <code>PROP_TITLE</code>.
     * <p>
     * The title image is usually used to populate the title bar of this part's
     * visual container. Since this image is managed by the part itself, callers
     * must <b>not</b> dispose the returned image.
     * </p>
     *
     * @return the title image
     */
    virtual void* GetTitleImage() = 0;

    /**
     * Returns the title tool tip text of this workbench part. If this value
     * changes the part must fire a property listener event with
     * <code>PROP_TITLE</code>.
     * <p>
     * The tool tip text is used to populate the title bar of this part's
     * visual container.
     * </p>
     *
     * @return the workbench part title tool tip (not null)
     */
    virtual std::string GetTitleToolTip() const = 0;

    /**
     * Returns true iff the contents of this part have changed recently. For
     * editors, this indicates that the part has changed since the last save.
     * For views, this indicates that the view contains interesting changes
     * that it wants to draw the user's attention to.
     *
     * @return true iff the part is dirty
     */
    virtual bool IsDirty() const = 0;

    /**
     * Return true if the the receiver is currently in a busy state.
     * @return boolean true if busy
     */
    virtual bool IsBusy() const = 0;

    /**
     * Returns true iff this part can be closed
     *
     * @return true iff this part can be closed
     * @since 3.1
     */
    virtual bool IsCloseable() const = 0;

    /**
     * Returns the local toolbar for this part, or null if this part does not
     * have a local toolbar. Callers must not dispose or downcast the return value.
     *
     * @return the local toolbar for the part, or null if none
     */
    virtual void* GetToolBar() = 0;

    /**
     * Returns the menu for this part or null if none
     *
     * @return the menu for this part or null if none
     */
    //virtual IPartMenu getMenu();

    /**
     * Returns an SWT control that can be used to indicate the tab order for
     * this part. This can be returned as part of the result to
     * {@link StackPresentation#getTabList(IPresentablePart)}. Any other use of this control is
     * unsupported. This may return a placeholder control that is only
     * meaningful in the context of <code>getTabList</code>.
     *
     * @return the part's control (not null)
     */
    virtual void* GetControl() = 0;

    /**
   * Get a property from the part's arbitrary property set.
   * <p>
   * <b>Note:</b> this is a different set of properties than the ones covered
   * by the PROP_* constants.
   * </p>
   *
   * @param key
   *            The property key to retrieve. Must not be <code>null</code>.
   * @return the property, or <code>null</code> if that property is not set.
   * @since 3.3
   */
  virtual std::string GetPartProperty(const std::string& key) const = 0;

};

}

#endif /* BERRYIPRESENTABLEPART_H_ */
