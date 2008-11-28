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

#ifndef IWORKBENCHPART_H_
#define IWORKBENCHPART_H_

#include "cherryUiDll.h"

#include "cherryIPropertyChangeListener.h"

#include <cherryMacros.h>

#include <map>

namespace cherry {

struct IWorkbenchPartSite;


/**
 * \ingroup org_opencherry_ui
 *
 * A workbench part is a visual component within a workbench page.  There
 * are two subtypes: view and editor, as defined by <code>IViewPart</code> and
 * <code>IEditorPart</code>.
 * <p>
 * A view is typically used to navigate a hierarchy of information (like the
 * workspace), open an editor, or display properties for the active editor.
 * Modifications made in a view are saved immediately.
 * </p><p>
 * An editor is typically used to edit or browse a document or input object.
 * The input is identified using an <code>IEditorInput</code>.  Modifications made
 * in an editor part follow an open-save-close lifecycle model.
 * </p><p>
 * This interface may be implemented directly.  For convenience, a base
 * implementation is defined in <code>WorkbenchPart</code>.
 * </p><p>
 * The lifecycle of a workbench part is as follows:
 * <ul>
 *  <li>When a part extension is created:
 *    <ul>
 *    <li>instantiate the part</li>
 *    <li>create a part site</li>
 *    <li>call <code>part.init(site)</code></li>
 *    </ul>
 *  <li>When a part becomes visible in the workbench:
 *    <ul>
 *    <li>add part to presentation by calling
 *        <code>part.createControl(parent)</code> to create actual widgets</li>
 *    <li>fire <code>partOpened</code> event to all listeners</li>
 *    </ul>
 *   </li>
 *  <li>When a part is activated or gets focus:
 *    <ul>
 *    <li>call <code>part.setFocus()</code></li>
 *    <li>fire <code>partActivated</code> event to all listeners</li>
 *    </ul>
 *   </li>
 *  <li>When a part is closed:
 *    <ul>
 *    <li>if save is needed, do save; if it fails or is canceled return</li>
 *    <li>if part is active, deactivate part</li>
 *    <li>fire <code>partClosed</code> event to all listeners</li>
 *    <li>remove part from presentation; part controls are disposed as part
 *         of the SWT widget tree
 *    <li>call <code>part.dispose()</code></li>
 *    </ul>
 *   </li>
 * </ul>
 * </p>
 * <p>
 * After <code>createPartControl</code> has been called, the implementor may
 * safely reference the controls created.  When the part is closed
 * these controls will be disposed as part of an SWT composite.  This
 * occurs before the <code>IWorkbenchPart.dispose</code> method is called.
 * If there is a need to free SWT resources the part should define a dispose
 * listener for its own control and free those resources from the dispose
 * listener.  If the part invokes any method on the disposed SWT controls
 * after this point an <code>SWTError</code> will be thrown.
 * </p>
 * <p>
 * The last method called on <code>IWorkbenchPart</code> is <code>dispose</code>.
 * This signals the end of the part lifecycle.
 * </p>
 * <p>
 * An important point to note about this lifecycle is that following
 * a call to init, createControl may never be called. Thus in the dispose
 * method, implementors must not assume controls were created.
 * </p>
 * <p>
 * Workbench parts implement the <code>IAdaptable</code> interface; extensions
 * are managed by the platform's adapter manager.
 * </p>
 *
 * @see IViewPart
 * @see IEditorPart
 */
struct CHERRY_UI IWorkbenchPart : public virtual Object { // public IAdaptable {

  cherryClassMacro(IWorkbenchPart)

  virtual ~IWorkbenchPart() {}

    /**
     * The property id for <code>getTitle</code>, <code>getTitleImage</code>
     * and <code>getTitleToolTip</code>.
     */
    //static const int PROP_TITLE = IWorkbenchPartConstants.PROP_TITLE;

    /**
     * Adds a listener for changes to properties of this workbench part.
     * Has no effect if an identical listener is already registered.
     * <p>
     * The property ids are defined in {@link IWorkbenchPartConstants}.
     * </p>
     *
     * @param listener a property listener
     */
    virtual void AddPropertyListener(IPropertyChangeListener::Pointer listener) = 0;

    /**
     * Creates the controls for this workbench part.
     * <p>
     * Clients should not call this method (the workbench calls this method when
     * it needs to, which may be never).
     * </p>
     * <p>
     * For implementors this is a multi-step process:
     * <ol>
     *   <li>Create one or more controls within the parent.</li>
     *   <li>Set the parent layout as needed.</li>
     *   <li>Register any global actions with the site's <code>IActionBars</code>.</li>
     *   <li>Register any context menus with the site.</li>
     *   <li>Register a selection provider with the site, to make it available to
     *     the workbench's <code>ISelectionService</code> (optional). </li>
     * </ol>
     * </p>
     *
     * @param parent the parent control
     */
    virtual void CreatePartControl(void* parent) = 0;

    /**
     * Returns the site for this workbench part. The site can be
     * <code>null</code> while the workbench part is being initialized. After
     * the initialization is complete, this value must be non-<code>null</code>
     * for the remainder of the part's life cycle.
     *
     * @return The part site; this value may be <code>null</code> if the part
     *         has not yet been initialized
     */
    virtual SmartPointer<IWorkbenchPartSite> GetSite() = 0;


    /**
     * Returns the name of this part. If this value changes the part must fire a
     * property listener event with {@link IWorkbenchPartConstants#PROP_PART_NAME}.
     *
     * @return the name of this view, or the empty string if the name is being managed
     * by the workbench (not <code>null</code>)
     */
    virtual std::string GetPartName() = 0;

    /**
     * Returns the content description of this part. The content description is an optional
     * user-readable string that describes what is currently being displayed in the part.
     * By default, the workbench will display the content description in a line
     * near the top of the view or editor.
     * An empty string indicates no content description
     * text. If this value changes the part must fire a property listener event
     * with {@link IWorkbenchPartConstants#PROP_CONTENT_DESCRIPTION}.
     *
     * @return the content description of this part (not <code>null</code>)
     */
    virtual std::string GetContentDescription() = 0;

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
    //virtual Image GetTitleImage() = 0;

    /**
     * Returns the title tool tip text of this workbench part.
     * An empty string result indicates no tool tip.
     * If this value changes the part must fire a property listener event with
     * <code>PROP_TITLE</code>.
     * <p>
     * The tool tip text is used to populate the title bar of this part's
     * visual container.
     * </p>
     *
     * @return the workbench part title tool tip (not <code>null</code>)
     */
    virtual std::string GetTitleToolTip() = 0;

    /**
     * Removes the given property listener from this workbench part.
     * Has no affect if an identical listener is not registered.
     *
     * @param listener a property listener
     */
    virtual void RemovePropertyListener(IPropertyChangeListener::Pointer listener) = 0;

    /**
     * Return the value for the arbitrary property key, or <code>null</code>.
     *
     * @param key
     *            the arbitrary property. Must not be <code>null</code>.
     * @return the property value, or <code>null</code>.
     */
    virtual std::string GetPartProperty(const std::string& key) = 0;

    /**
     * Set an arbitrary property on the part. It is the implementor's
     * responsibility to fire the corresponding PropertyChangeEvent.
     * <p>
     * A default implementation has been added to WorkbenchPart.
     * </p>
     *
     * @param key
     *            the arbitrary property. Must not be <code>null</code>.
     * @param value
     *            the property value. A <code>null</code> value will remove
     *            that property.
     */
    virtual void SetPartProperty(const std::string& key, const std::string& value) = 0;

    /**
     * Return an unmodifiable map of the arbitrary properties. This method can
     * be used to save the properties during workbench save/restore.
     *
     * @return A Map of the properties. Must not be <code>null</code>.
     */
    virtual const std::map<std::string, std::string>& GetPartProperties() = 0;

    /**
     * Asks this part to take focus within the workbench.
     * <p>
     * Clients should not call this method (the workbench calls this method at
     * appropriate times).  To have the workbench activate a part, use
     * <code>IWorkbenchPage.activate(IWorkbenchPart) instead</code>.
     * </p>
     */
    virtual void SetFocus() = 0;
};

}  // namespace cherry

#endif /*IWORKBENCHPART_H_*/
