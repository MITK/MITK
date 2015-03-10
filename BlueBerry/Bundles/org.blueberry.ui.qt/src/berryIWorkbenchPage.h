/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef BERRYIWORKBENCHPAGE_H_
#define BERRYIWORKBENCHPAGE_H_

#include <berryMacros.h>

#include "berryIEditorReference.h"
#include "berryIViewReference.h"
#include "berryIPerspectiveDescriptor.h"
#include "berryIEditorPart.h"
#include "berryIViewPart.h"
#include "berryIEditorInput.h"
#include "berryIPartService.h"
#include "berryISelectionService.h"
#include "berryIReusableEditor.h"

#include "berryIWorkbenchWindow.h"


/**
 * \ingroup org_blueberry_ui_qt
 *
 */
namespace berry {

/**
 * A workbench page consists of an arrangement of views and editors intended to
 * be presented together to the user in a single workbench window.
 * <p>
 * A page can contain 0 or more views and 0 or more editors. These views and
 * editors are contained wholly within the page and are not shared with other
 * pages. The layout and visible action set for the page is defined by a
 * perspective.
 * <p>
 * The number of views and editors within a page is restricted to simplify part
 * management for the user. In particular:
 * <ul>
 * <li>Unless a view explicitly allows for multiple instances in its plugin
 * declaration there will be only one instance in a given workbench page.</li>
 * <li>Only one editor can exist for each editor input within a page.
 * <li>
 * </ul>
 * </p>
 * <p>
 * This interface is not intended to be implemented by clients.
 * </p>
 *
 * @see IPerspectiveDescriptor
 * @see IEditorPart
 * @see IViewPart
 */
struct BERRY_UI_QT IWorkbenchPage : public IPartService, public ISelectionService, public Object
{

  berryObjectMacro(berry::IWorkbenchPage)

  ~IWorkbenchPage();

  /**
   * An optional attribute within a workspace marker (<code>IMarker</code>)
   * which identifies the preferred editor type to be opened when
   * <code>openEditor</code> is called.
   *
   * @see #openEditor(IEditorInput, String)
   * @see #openEditor(IEditorInput, String, boolean)
   * @deprecated in 3.0 since the notion of markers this is not generally
   *             applicable. Use the IDE-specific constant
   *             <code>IDE.EDITOR_ID_ATTR</code>.
   */
  static const QString EDITOR_ID_ATTR; // = "org.blueberry.ui.editorID";

  /**
   * Change event id when the perspective is reset to its original state.
   *
   * @see IPerspectiveListener
   */
  static const QString CHANGE_RESET; // = "reset";

  /**
   * Change event id when the perspective has completed a reset to its
   * original state.
   *
   * @since 3.0
   * @see IPerspectiveListener
   */
  static const QString CHANGE_RESET_COMPLETE; // = "resetComplete";

  /**
   * Change event id when one or more views are shown in a perspective.
   *
   * @see IPerspectiveListener
   */
  static const QString CHANGE_VIEW_SHOW; // = "viewShow";

  /**
   * Change event id when one or more views are hidden in a perspective.
   *
   * @see IPerspectiveListener
   */
  static const QString CHANGE_VIEW_HIDE; // = "viewHide";

  /**
   * Change event id when one or more editors are opened in a perspective.
   *
   * @see IPerspectiveListener
   */
  static const QString CHANGE_EDITOR_OPEN; // = "editorOpen";

  /**
   * Change event id when one or more editors are closed in a perspective.
   *
   * @see IPerspectiveListener
   */
  static const QString CHANGE_EDITOR_CLOSE; // = "editorClose";

  /**
   * Change event id when the editor area is shown in a perspective.
   *
   * @see IPerspectiveListener
   */
  static const QString CHANGE_EDITOR_AREA_SHOW; // = "editorAreaShow";

  /**
   * Change event id when the editor area is hidden in a perspective.
   *
   * @see IPerspectiveListener
   */
  static const QString CHANGE_EDITOR_AREA_HIDE; // = "editorAreaHide";

  /**
   * Change event id when an action set is shown in a perspective.
   *
   * @see IPerspectiveListener
   */
  static const QString CHANGE_ACTION_SET_SHOW; // = "actionSetShow";

  /**
   * Change event id when an action set is hidden in a perspective.
   *
   * @see IPerspectiveListener
   */
  static const QString CHANGE_ACTION_SET_HIDE; // = "actionSetHide";

  /**
   * Show view mode that indicates the view should be made visible and
   * activated. Use of this mode has the same effect as calling
   * {@link #showView(String)}.
   */
  static const int VIEW_ACTIVATE; // = 1;

  /**
   * Show view mode that indicates the view should be made visible. If the
   * view is opened in the container that contains the active view then this
   * has the same effect as <code>VIEW_CREATE</code>.
   */
  static const int VIEW_VISIBLE; // = 2;

  /**
   * Show view mode that indicates the view should be made created but not
   * necessarily be made visible. It will only be made visible in the event
   * that it is opened in its own container. In other words, only if it is not
   * stacked with another view.
   */
  static const int VIEW_CREATE; // = 3;

  /**
   * Editor opening match mode specifying that no matching against existing
   * editors should be done.
   */
  static const int MATCH_NONE; // = 0;

  /**
   * Editor opening match mode specifying that the editor input should be
   * considered when matching against existing editors.
   */
  static const int MATCH_INPUT; // = 1;

  /**
   * Editor opening match mode specifying that the editor id should be
   * considered when matching against existing editors.
   */
  static const int MATCH_ID; // = 2;

  /**
   * Activates the given part. The part will be brought to the front and given
   * focus. The part must belong to this page.
   *
   * @param part
   *            the part to activate
   */
  virtual void Activate(IWorkbenchPart::Pointer part) = 0;

  /**
   * Adds a property change listener.
   *
   * @param listener
   *            the property change listener to add
   */
  //virtual void addPropertyChangeListener(IPropertyChangeListener listener);

  /**
   * Moves the given part forward in the Z order of this page so as to make it
   * visible, without changing which part has focus. The part must belong to
   * this page.
   *
   * @param part
   *            the part to bring forward
   */
  virtual void BringToTop(IWorkbenchPart::Pointer part) = 0;

  /**
   * Closes this workbench page. If this page is the active one, this honor is
   * passed along to one of the window's other pages if possible.
   * <p>
   * If the page has an open editor with unsaved content, the user will be
   * given the opportunity to save it.
   * </p>
   *
   * @return <code>true</code> if the page was successfully closed, and
   *         <code>false</code> if it is still open
   */
  virtual bool Close() = 0;

  /**
   * Closes all of the editors belonging to this workbench page.
   * <p>
   * If the page has open editors with unsaved content and <code>save</code>
   * is <code>true</code>, the user will be given the opportunity to save
   * them.
   * </p>
   *
   * @param save
   *
   * @return <code>true</code> if all editors were successfully closed, and
   *         <code>false</code> if at least one is still open
   */
  virtual bool CloseAllEditors(bool save) = 0;

  /**
   * Closes the given <code>Array</code> of editor references. The editors
   * must belong to this workbench page.
   * <p>
   * If any of the editors have unsaved content and <code>save</code> is
   * <code>true</code>, the user will be given the opportunity to save
   * them.
   * </p>
   *
   * @param editorRefs
   *            the editors to close
   * @param save
   *            <code>true</code> to save the editor contents if required
   *            (recommended), and <code>false</code> to discard any unsaved
   *            changes
   * @return <code>true</code> if the editors were successfully closed, and
   *         <code>false</code> if the editors are still open
   */
  virtual bool CloseEditors(const QList<IEditorReference::Pointer>& editorRefs, bool save) = 0;

  /**
   * Closes the given editor. The editor must belong to this workbench page.
   * <p>
   * If the editor has unsaved content and <code>save</code> is
   * <code>true</code>, the user will be given the opportunity to save it.
   * </p>
   *
   * @param editor
   *            the editor to close
   * @param save
   *            <code>true</code> to save the editor contents if required
   *            (recommended), and <code>false</code> to discard any unsaved
   *            changes
   * @return <code>true</code> if the editor was successfully closed, and
   *         <code>false</code> if the editor is still open
   */
  virtual bool CloseEditor(IEditorPart::Pointer editor, bool save) = 0;

  /**
   * Returns the view in this page with the specified id. There is at most one
   * view in the page with the specified id.
   *
   * @param viewId
   *            the id of the view extension to use
   * @return the view, or <code>null</code> if none is found
   */
  virtual IViewPart::Pointer FindView(const QString& viewId) = 0;

  /**
   * Returns the view reference with the specified id.
   *
   * @param viewId
   *            the id of the view extension to use
   * @return the view reference, or <code>null</code> if none is found
   */
  virtual IViewReference::Pointer FindViewReference(const QString& viewId) = 0;

  /**
   * Returns the view reference with the specified id and secondary id.
   *
   * @param viewId
   *            the id of the view extension to use
   * @param secondaryId
   *            the secondary id to use, or <code>null</code> for no
   *            secondary id
   * @return the view reference, or <code>null</code> if none is found
   */
  virtual IViewReference::Pointer FindViewReference(const QString& viewId, const QString& secondaryId) = 0;

  /**
   * Returns the active editor open in this page.
   * <p>
   * This is the visible editor on the page, or, if there is more than one
   * visible editor, this is the one most recently brought to top.
   * </p>
   *
   * @return the active editor, or <code>null</code> if no editor is active
   */
  virtual IEditorPart::Pointer GetActiveEditor() = 0;

  /**
   * Returns the editor with the specified input. Returns null if there is no
   * opened editor with that input.
   *
   * @param input
   *            the editor input
   * @return an editor with input equals to <code>input</code>
   */
  virtual IEditorPart::Pointer FindEditor(IEditorInput::Pointer input) = 0;

  /**
   * Returns an array of editor references that match the given input and/or
   * editor id, as specified by the given match flags. Returns an empty array
   * if there are no matching editors, or if matchFlags is MATCH_NONE.
   *
   * @param input
   *            the editor input, or <code>null</code> if MATCH_INPUT is not
   *            specified in matchFlags
   * @param editorId
   *            the editor id, or <code>null</code> if MATCH_ID is not
   *            specified in matchFlags
   * @param matchFlags
   *            a bit mask consisting of zero or more of the MATCH_* constants
   *            OR-ed together
   * @return the references for the matching editors
   *
   * @see #MATCH_NONE
   * @see #MATCH_INPUT
   * @see #MATCH_ID
   */
  virtual QList<IEditorReference::Pointer> FindEditors(IEditorInput::Pointer input, const QString& editorId,
      int matchFlags) = 0;

  /**
   * Returns a list of the editors open in this page.
   * <p>
   * Note that each page has its own editors; editors are never shared between
   * pages.
   * </p>
   *
   * @return a list of open editors
   *
   * @deprecated use #getEditorReferences() instead
   */
  virtual QList<IEditorPart::Pointer> GetEditors() = 0;

  /**
   * Returns an array of references to open editors in this page.
   * <p>
   * Note that each page has its own editors; editors are never shared between
   * pages.
   * </p>
   *
   * @return a list of open editors
   */
  virtual QList<IEditorReference::Pointer> GetEditorReferences() = 0;

  /**
   * Returns a list of dirty editors in this page.
   *
   * @return a list of dirty editors
   */
  virtual QList<IEditorPart::Pointer> GetDirtyEditors() = 0;

  /**
   * Returns the input for this page.
   *
   * @return the input for this page, or <code>null</code> if none
   */
  virtual IAdaptable* GetInput() = 0;

  /**
   * Returns the page label. This will be a unique identifier within the
   * containing workbench window.
   *
   * @return the page label
   */
  virtual QString GetLabel() = 0;

  /**
   * Returns the current perspective descriptor for this page, or
   * <code>null</code> if there is no current perspective.
   *
   * @return the current perspective descriptor or <code>null</code>
   * @see #setPerspective
   * @see #savePerspective
   */
  virtual IPerspectiveDescriptor::Pointer GetPerspective() = 0;

  /**
   * Returns a list of the reference to views visible on this page.
   * <p>
   * Note that each page has its own views; views are never shared between
   * pages.
   * </p>
   *
   * @return a list of references to visible views
   */
  virtual QList<IViewReference::Pointer> GetViewReferences() = 0;

  /**
   * Returns a list of the views visible on this page.
   * <p>
   * Note that each page has its own views; views are never shared between
   * pages.
   * </p>
   *
   * @return a list of visible views
   *
   * @deprecated use #getViewReferences() instead.
   */
  virtual QList<IViewPart::Pointer> GetViews() = 0;

  /**
   * Returns the workbench window of this page.
   *
   * @return the workbench window
   */
  virtual IWorkbenchWindow::Pointer GetWorkbenchWindow() = 0;

  /**
   * Hides the given view. The view must belong to this page.
   *
   * @param view
   *            the view to hide
   */
  virtual void HideView(IViewPart::Pointer view) = 0;

  /**
   * Hides the given view that belongs to the reference, if any.
   *
   * @param view
   *            the references whos view is to be hidden
   */
  virtual void HideView(IViewReference::Pointer view) = 0;

  /**
   * Returns true if perspective with given id contains view with given id
   */
  virtual bool HasView(const QString& perspectiveId, const QString& viewId) = 0;

  /**
   * Returns whether the specified part is visible.
   *
   * @param part
   *            the part to test
   * @return boolean <code>true</code> if part is visible
   */
  virtual bool IsPartVisible(IWorkbenchPart::Pointer part) = 0;

  /**
   * Removes the perspective specified by desc.
   *
   * @param desc
   *            the perspective that will be removed
   */
  virtual void RemovePerspective(IPerspectiveDescriptor::Pointer desc) = 0;

  /**
   * Reuses the specified editor by setting its new input.
   *
   * @param editor
   *            the editor to be reused
   * @param input
   *            the new input for the reusable editor
   */
  virtual void ReuseEditor(IReusableEditor::Pointer editor, IEditorInput::Pointer input) = 0;

  /**
   * Opens an editor on the given input.
   * <p>
   * If this page already has an editor open on the target input that editor
   * is activated; otherwise, a new editor is opened. Two editor inputs,
   * input1 and input2, are considered the same if
   *
   * <pre>
   * input1.equals(input2) == true
   * </pre>.
   * </p>
   * <p>
   * The editor type is determined by mapping <code>editorId</code> to an
   * editor extension registered with the workbench. An editor id is passed
   * rather than an editor object to prevent the accidental creation of more
   * than one editor for the same input. It also guarantees a consistent
   * lifecycle for editors, regardless of whether they are created by the user
   * or restored from saved data.
   * </p>
   *
   * @param input
   *            the editor input
   * @param editorId
   *            the id of the editor extension to use
   * @return an open and active editor, or <code>null</code> if an external
   *         editor was opened
   * @exception PartInitException
   *                if the editor could not be created or initialized
   */
  virtual IEditorPart::Pointer OpenEditor(IEditorInput::Pointer input, const QString& editorId) = 0;

  /**
   * Opens an editor on the given input.
   * <p>
   * If this page already has an editor open on the target input that editor
   * is brought to the front; otherwise, a new editor is opened. Two editor
   * inputs are considered the same if they equal. See
   * <code>Object.equals(Object)<code>
   * and <code>IEditorInput</code>. If <code>activate == true</code> the editor
   * will be activated.
   * </p><p>
   * The editor type is determined by mapping <code>editorId</code> to an editor
   * extension registered with the workbench.  An editor id is passed rather than
   * an editor object to prevent the accidental creation of more than one editor
   * for the same input. It also guarantees a consistent lifecycle for editors,
   * regardless of whether they are created by the user or restored from saved
   * data.
   * </p>
   *
   * @param input the editor input
   * @param editorId the id of the editor extension to use
   * @param activate if <code>true</code> the editor will be activated
   * @return an open editor, or <code>null</code> if an external editor was opened
   * @exception PartInitException if the editor could not be created or initialized
   */
  virtual IEditorPart::Pointer OpenEditor(IEditorInput::Pointer input, const QString& editorId,
      bool activate) = 0;

  /**
   * Opens an editor on the given input.
   * <p>
   * If this page already has an editor open that matches the given input
   * and/or editor id (as specified by the matchFlags argument), that editor
   * is brought to the front; otherwise, a new editor is opened. Two editor
   * inputs are considered the same if they equal. See
   * <code>Object.equals(Object)<code>
   * and <code>IEditorInput</code>. If <code>activate == true</code> the editor
   * will be activated.
   * </p><p>
   * The editor type is determined by mapping <code>editorId</code> to an editor
   * extension registered with the workbench.  An editor id is passed rather than
   * an editor object to prevent the accidental creation of more than one editor
   * for the same input. It also guarantees a consistent lifecycle for editors,
   * regardless of whether they are created by the user or restored from saved
   * data.
   * </p>
   *
   * @param input the editor input
   * @param editorId the id of the editor extension to use
   * @param activate if <code>true</code> the editor will be activated
   * @param matchFlags a bit mask consisting of zero or more of the MATCH_* constants OR-ed together
   * @return an open editor, or <code>null</code> if an external editor was opened
   * @exception PartInitException if the editor could not be created or initialized
   *
   * @see #MATCH_NONE
   * @see #MATCH_INPUT
   * @see #MATCH_ID
   */
  virtual IEditorPart::Pointer OpenEditor(IEditorInput::Pointer input,
      const QString& editorId, bool activate, int matchFlags) = 0;

  /**
   * Removes the property change listener.
   *
   * @param listener
   *            the property change listener to remove
   */
  //virtual void removePropertyChangeListener(IPropertyChangeListener listener);

  /**
   * Changes the visible views, their layout, and the visible action sets
   * within the page to match the current perspective descriptor. This is a
   * rearrangement of components and not a replacement. The contents of the
   * current perspective descriptor are unaffected.
   * <p>
   * For more information on perspective change see
   * <code>setPerspective()</code>.
   * </p>
   */
  virtual void ResetPerspective() = 0;

  /**
   * Saves the contents of all dirty editors belonging to this workbench page.
   * If there are no dirty editors this method returns without effect.
   * <p>
   * If <code>confirm</code> is <code>true</code> the user is prompted to
   * confirm the command.
   * </p>
   * <p>
   * Note that as of 3.2, this method also saves views that implement
   * ISaveablePart and are dirty.
   * </p>
   *
   * @param confirm <code>true</code> to ask the user before saving unsaved
   *            changes (recommended), and <code>false</code> to save
   *            unsaved changes without asking
   * @return <code>true</code> if the command succeeded, and
   *         <code>false</code> if the operation was canceled by the user or
   *         an error occurred while saving
   */
  virtual bool SaveAllEditors(bool confirm) = 0;

  /**
   * Saves the contents of the given editor if dirty. If not, this method
   * returns without effect.
   * <p>
   * If <code>confirm</code> is <code>true</code> the user is prompted to
   * confirm the command. Otherwise, the save happens without prompt.
   * </p>
   * <p>
   * The editor must belong to this workbench page.
   * </p>
   *
   * @param editor
   *            the editor to close
   * @param confirm
   *            <code>true</code> to ask the user before saving unsaved
   *            changes (recommended), and <code>false</code> to save
   *            unsaved changes without asking
   * @return <code>true</code> if the command succeeded, and
   *         <code>false</code> if the editor was not saved
   */
  virtual bool SaveEditor(IEditorPart::Pointer editor, bool confirm) = 0;

  /**
   * Saves the visible views, their layout, and the visible action sets for
   * this page to the current perspective descriptor. The contents of the
   * current perspective descriptor are overwritten.
   */
  virtual void SavePerspective() = 0;

  /**
   * Saves the visible views, their layout, and the visible action sets for
   * this page to the given perspective descriptor. The contents of the given
   * perspective descriptor are overwritten and it is made the current one for
   * this page.
   *
   * @param perspective
   *            the perspective descriptor to save to
   */
  virtual void SavePerspectiveAs(IPerspectiveDescriptor::Pointer perspective) = 0;


  /**
   * Changes the visible views, their layout, and the visible action sets
   * within the page to match the given perspective descriptor. This is a
   * rearrangement of components and not a replacement. The contents of the
   * old perspective descriptor are unaffected.
   * <p>
   * When a perspective change occurs the old perspective is deactivated
   * (hidden) and cached for future reference. Then the new perspective is
   * activated (shown). The views within the page are shared by all existing
   * perspectives to make it easy for the user to switch between one
   * perspective and another quickly without loss of context.
   * </p>
   * <p>
   * During activation the action sets are modified. If an action set is
   * specified in the new perspective which is not visible in the old one it
   * will be created. If an old action set is not specified in the new
   * perspective it will be disposed.
   * </p>
   * <p>
   * The visible views and their layout within the page also change. If a view
   * is specified in the new perspective which is not visible in the old one a
   * new instance of the view will be created. If an old view is not specified
   * in the new perspective it will be hidden. This view may reappear if the
   * user selects it from the View menu or if they switch to a perspective
   * (which may be the old one) where the view is visible.
   * </p>
   * <p>
   * The open editors are not modified by this method.
   * </p>
   *
   * @param perspective
   *            the perspective descriptor
   */
  virtual void SetPerspective(IPerspectiveDescriptor::Pointer perspective) = 0;

  /**
   * Shows the view identified by the given view id in this page and gives it
   * focus. If there is a view identified by the given view id (and with no
   * secondary id) already open in this page, it is given focus.
   *
   * @param viewId
   *            the id of the view extension to use
   * @return the shown view
   * @exception PartInitException
   *                if the view could not be initialized
   */
  virtual IViewPart::Pointer ShowView(const QString& viewId) = 0;

  /**
   * Shows a view in this page with the given id and secondary id. The
   * behaviour of this method varies based on the supplied mode. If
   * <code>VIEW_ACTIVATE</code> is supplied, the view is focus. If
   * <code>VIEW_VISIBLE</code> is supplied, then it is made visible but not
   * given focus. Finally, if <code>VIEW_CREATE</code> is supplied the view
   * is created and will only be made visible if it is not created in a folder
   * that already contains visible views.
   * <p>
   * This allows multiple instances of a particular view to be created. They
   * are disambiguated using the secondary id. If a secondary id is given, the
   * view must allow multiple instances by having specified
   * allowMultiple="true" in its extension.
   * </p>
   *
   * @param viewId
   *            the id of the view extension to use
   * @param secondaryId
   *            the secondary id to use, or <code>null</code> for no
   *            secondary id
   * @param mode
   *            the activation mode. Must be {@link #VIEW_ACTIVATE},
   *            {@link #VIEW_VISIBLE} or {@link #VIEW_CREATE}
   * @return a view
   * @exception PartInitException
   *                if the view could not be initialized
   * @exception IllegalArgumentException
   *                if the supplied mode is not valid
   */
  virtual IViewPart::Pointer ShowView(const QString& viewId, const QString& secondaryId, int mode) = 0;

  /**
   * Returns <code>true</code> if the editor is pinned and should not be
   * reused.
   *
   * @param editor
   *            the editor to test
   * @return boolean whether the editor is pinned
   */
  virtual bool IsEditorPinned(IEditorPart::Pointer editor) = 0;

  /**
   * Returns the perspective shortcuts associated with the current
   * perspective. Returns an empty array if there is no current perspective.
   *
   * @see IPageLayout#addPerspectiveShortcut(String)
   * @return an array of perspective identifiers
   */
  virtual QList<QString> GetPerspectiveShortcuts() = 0;

  /**
   * Returns the show view shortcuts associated with the current perspective.
   * Returns an empty array if there is no current perspective.
   *
   * @see IPageLayout#addShowViewShortcut(String)
   * @return an array of view identifiers
   */
  virtual QList<QString> GetShowViewShortcuts() = 0;

  /**
   * Returns the descriptors for the perspectives that are open in this page,
   * in the order in which they were opened.
   *
   * @return the open perspective descriptors, in order of opening
   */
  virtual QList<IPerspectiveDescriptor::Pointer> GetOpenPerspectives() = 0;

  /**
   * Returns the descriptors for the perspectives that are open in this page,
   * in the order in which they were activated (oldest first).
   *
   * @return the open perspective descriptors, in order of activation
   */
  virtual QList<IPerspectiveDescriptor::Pointer> GetSortedPerspectives() = 0;

  /**
   * Closes current perspective. If last perspective, then entire page
   * is closed.
   *
   * @param saveParts
   *            whether the page's parts should be saved if closed
   * @param closePage
   *            whether the page itself should be closed if last perspective
   */
  virtual void CloseCurrentPerspective(bool saveParts, bool closePage) = 0;

  /**
   * Closes the specified perspective in this page. If the last perspective in
   * this page is closed, then all editors are closed. Views that are not
   * shown in other perspectives are closed as well. If <code>saveParts</code>
   * is <code>true</code>, the user will be prompted to save any unsaved
   * changes for parts that are being closed. The page itself is closed if
   * <code>closePage</code> is <code>true</code>.
   *
   * @param desc
   *            the descriptor of the perspective to be closed
   * @param saveParts
   *            whether the page's parts should be saved if closed
   * @param closePage
   *            whether the page itself should be closed if last perspective
   */
  virtual void ClosePerspective(IPerspectiveDescriptor::Pointer desc,
                                bool saveParts, bool closePage) = 0;

  /**
   * Closes all perspectives in this page. All editors are closed, prompting
   * to save any unsaved changes if <code>saveEditors</code> is
   * <code>true</code>. The page itself is closed if <code>closePage</code>
   * is <code>true</code>.
   *
   * @param saveEditors
   *            whether the page's editors should be saved
   * @param closePage
   *            whether the page itself should be closed
   */
  virtual void CloseAllPerspectives(bool saveEditors, bool closePage) = 0;

  /**
   * Find the part reference for the given part. A convenience method to
   * quickly go from part to part reference.
   *
   * @param part
   *            The part to search for. It can be <code>null</code>.
   * @return The reference for the given part, or <code>null</code> if no
   *         reference can be found.
   */
  virtual IWorkbenchPartReference::Pointer GetReference(IWorkbenchPart::Pointer part) = 0;
};

}  // namespace berry

#endif /*BERRYIWORKBENCHPAGE_H_*/
