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

#ifndef BERRYWORKBENCHPAGE_H_
#define BERRYWORKBENCHPAGE_H_

#include <berryIAdaptable.h>
#include <berryIExtensionPoint.h>

#include "berryIWorkbenchPage.h"
#include "berryIWorkbenchPartReference.h"
#include "berryIReusableEditor.h"

#include "berryILayoutContainer.h"
#include "berryIStickyViewManager.h"
#include "berryWorkbenchPagePartList.h"
#include "berryWorkbenchPartReference.h"
#include "berryPageSelectionService.h"
#include "berryEditorManager.h"
#include "berryViewFactory.h"

#include "berryPartPane.h"

#include <list>

namespace berry
{
//class PartPane;
//class PartPane::Sashes;
class EditorAreaHelper;
class WorkbenchWindow;
class Perspective;
class PerspectiveHelper;
class PerspectiveDescriptor;
class LayoutPartSash;
class LayoutTree;
class LayoutTreeNode;
class PartService;

/**
 * \ingroup org_blueberry_ui_internal
 *
 * A collection of views and editors in a workbench.
 */
class BERRY_UI WorkbenchPage: public IWorkbenchPage
{
public:
  berryObjectMacro(WorkbenchPage);

protected:

  //TODO Weakpointer
  WorkbenchWindow* window;

  friend class ViewFactory;
  friend class WorkbenchWindow;
  friend class EditorAreaHelper;
  friend class WWinPartService;

private:

  /**
   * Manages editor contributions and action set part associations.
   */
  class ActionSwitcher
  {
  private:
    IWorkbenchPart::WeakPtr activePart;

    IEditorPart::WeakPtr topEditor;

    /**
     * Updates the contributions given the new part as the active part.
     *
     * @param newPart
     *            the new active part, may be <code>null</code>
     */
  public:
    void UpdateActivePart(IWorkbenchPart::Pointer newPart);
    /**
     * Updates the contributions given the new part as the topEditor.
     *
     * @param newEditor
     *            the new top editor, may be <code>null</code>
     */
  public:
    void UpdateTopEditor(IEditorPart::Pointer newEditor);

    /**
     * Activates the contributions of the given part. If <code>enable</code>
     * is <code>true</code> the contributions are visible and enabled,
     * otherwise they are disabled.
     *
     * @param part
     *            the part whose contributions are to be activated
     * @param enable
     *            <code>true</code> the contributions are to be enabled,
     *            not just visible.
     */
  private:
    void ActivateContributions(IWorkbenchPart::Pointer part, bool enable);

    /**
     * Deactivates the contributions of the given part. If <code>remove</code>
     * is <code>true</code> the contributions are removed, otherwise they
     * are disabled.
     *
     * @param part
     *            the part whose contributions are to be deactivated
     * @param remove
     *            <code>true</code> the contributions are to be removed,
     *            not just disabled.
     */
  private:
    void DeactivateContributions(IWorkbenchPart::Pointer part, bool remove);
  };

  class ActivationList
  {
  public:

    //List of parts in the activation order (oldest first)
    typedef std::deque<IWorkbenchPartReference::Pointer> PartListType;
    typedef std::deque<IWorkbenchPartReference::Pointer>::iterator PartListIter;
    typedef std::deque<IWorkbenchPartReference::Pointer>::reverse_iterator PartListReverseIter;

  private:

    PartListType parts;

    WorkbenchPage* page;

  public:
    ActivationList(WorkbenchPage* page);

    /*
     * Add/Move the active part to end of the list;
     */
    void SetActive(SmartPointer<IWorkbenchPart> part);

    /*
     * Ensures that the given part appears AFTER any other part in the same
     * container.
     */
    void BringToTop(SmartPointer<IWorkbenchPartReference> ref);

    /*
     * Returns the last (most recent) iterator (index) of the given container in the activation list, or returns
     * end() if the given container does not appear in the activation list.
     */
    PartListIter LastIndexOfContainer(SmartPointer<ILayoutContainer> container);

    /*
     * Add/Move the active part to end of the list;
     */
    void SetActive(SmartPointer<IWorkbenchPartReference> ref);

    /*
     * Add the active part to the beginning of the list.
     */
    void Add(SmartPointer<IWorkbenchPartReference> ref);

    /*
     * Return the active part. Filter fast views.
     */
    SmartPointer<IWorkbenchPart> GetActive();

    /*
     * Return the previously active part. Filter fast views.
     */
    SmartPointer<IWorkbenchPart> GetPreviouslyActive();

    SmartPointer<IWorkbenchPartReference> GetActiveReference(bool editorsOnly);

    /*
     * Retuns the index of the part within the activation list. The higher
     * the index, the more recently it was used.
     */
    PartListIter IndexOf(SmartPointer<IWorkbenchPart> part);

    /*
     * Returns the index of the part reference within the activation list.
     * The higher the index, the more recent it was used.
     */
    PartListIter IndexOf(SmartPointer<IWorkbenchPartReference> ref);

    /*
     * Remove a part from the list
     */
    bool Remove(SmartPointer<IWorkbenchPartReference> ref);

    /*
     * Returns the topmost editor on the stack, or null if none.
     */
    SmartPointer<IEditorPart> GetTopEditor();

    /*
     * Returns the editors in activation order (oldest first).
     */
    std::vector<SmartPointer<IEditorReference> > GetEditors();

    /*
     * Return a list with all parts (editors and views).
     */
    std::vector<SmartPointer<IWorkbenchPartReference> > GetParts();

  private:

    SmartPointer<IWorkbenchPart> GetActive(PartListIter start);

    SmartPointer<IWorkbenchPartReference> GetActiveReference(PartListIter start, bool editorsOnly);

    /*
     * Find a part in the list starting from the end and filter
     * and views from other perspectives. Will filter fast views
     * unless 'includeActiveFastViews' is true;
     */
    SmartPointer<IWorkbenchPartReference> GetActiveReference(PartListIter start, bool editorsOnly, bool skipPartsObscuredByZoom);
  };

  /**
   * Helper class to keep track of all opened perspective. Both the opened
   * and used order is kept.
   */
  struct PerspectiveList
  {
  public:
    typedef std::list<SmartPointer<Perspective> > PerspectiveListType;
    typedef PerspectiveListType::iterator iterator;

  private:

    /**
     * List of perspectives in the order they were opened;
     */
    PerspectiveListType openedList;

    /**
     * List of perspectives in the order they were used. Last element is
     * the most recently used, and first element is the least recently
     * used.
     */
    PerspectiveListType usedList;

    /**
     * The perspective explicitly set as being the active one
     */
    SmartPointer<Perspective> active;

    void UpdateActionSets(SmartPointer<Perspective> oldPersp,
        SmartPointer<Perspective> newPersp);

  public:

    /**
     * Creates an empty instance of the perspective list
     */
    PerspectiveList();

    /**
     * Update the order of the perspectives in the opened list
     *
     * @param perspective
     * @param newLoc
     */
    void Reorder(IPerspectiveDescriptor::Pointer perspective, int newLoc);

    /**
     * Return all perspectives in the order they were activated.
     *
     * @return an array of perspectives sorted by activation order, least
     *         recently activated perspective last.
     */
    PerspectiveListType GetSortedPerspectives();

    /**
     * Adds a perspective to the list. No check is done for a duplicate when
     * adding.
     * @param perspective the perspective to add
     * @return boolean <code>true</code> if the perspective was added
     */
    bool Add(SmartPointer<Perspective> perspective);

    /**
     * Returns an iterator on the perspective list in the order they were
     * opened.
     */
    PerspectiveListType::iterator Begin();

    PerspectiveListType::iterator End();

    /**
     * Returns an array with all opened perspectives
     */
    PerspectiveListType GetOpenedPerspectives();

    /**
     * Removes a perspective from the list.
     */
    bool Remove(SmartPointer<Perspective> perspective);

    /**
     * Swap the opened order of old perspective with the new perspective.
     */
    void Swap(SmartPointer<Perspective> oldPerspective,
        SmartPointer<Perspective> newPerspective);

    /**
     * Returns whether the list contains any perspectives
     */
    bool IsEmpty();

    /**
     * Returns the most recently used perspective in the list.
     */
    SmartPointer<Perspective> GetActive();

    /**
     * Returns the next most recently used perspective in the list.
     */
    SmartPointer<Perspective> GetNextActive();

    /**
     * Returns the number of perspectives opened
     */
    PerspectiveListType::size_type Size();

    /**
     * Marks the specified perspective as the most recently used one in the
     * list.
     */
    void SetActive(SmartPointer<Perspective> perspective);
  };

  IAdaptable* input;

  void* composite;

  //Could be delete. This information is in the active part list;
  ActivationList* activationList;

  EditorManager* editorMgr;

  EditorAreaHelper* editorPresentation;

  //ListenerList propertyChangeListeners = new ListenerList();

  PageSelectionService* selectionService;

  WorkbenchPagePartList::Pointer partList; // = new WorkbenchPagePartList(selectionService);

  //IActionBars actionBars;

  ViewFactory* viewFactory;

  PerspectiveList perspList;

  SmartPointer<PerspectiveDescriptor> deferredActivePersp;

  //NavigationHistory navigationHistory = new NavigationHistory(this);

  IStickyViewManager::Pointer stickyViewMan;



  /**
   * Returns true if perspective with given id contains view with given id
   */
  bool HasView(const std::string& perspectiveId, const std::string& viewId);

  /**
   * If we're in the process of activating a part, this points to the new part.
   * Otherwise, this is null.
   */
  IWorkbenchPartReference::Pointer partBeingActivated;

  /**
   * Contains a list of perspectives that may be dirty due to plugin
   * installation and removal.
   */
  std::set<std::string> dirtyPerspectives;

  ActionSwitcher actionSwitcher;

  //IExtensionTracker tracker;

  // Deferral count... delays disposing parts and sending certain events if nonzero
  int deferCount;
  // Parts waiting to be disposed
  std::vector<WorkbenchPartReference::Pointer> pendingDisposals;

  const IExtensionPoint* GetPerspectiveExtensionPoint();

public:

  /**
   * Constructs a new page with a given perspective and input.
   *
   * @param w
   *            the parent window
   * @param layoutID
   *            must not be <code>null</code>
   * @param input
   *            the page input
   * @throws WorkbenchException
   *             on null layout id
   */
  WorkbenchPage(WorkbenchWindow* w, const std::string& layoutID,
      IAdaptable* input);

  /**
   * Constructs a page. <code>restoreState(IMemento)</code> should be
   * called to restore this page from data stored in a persistance file.
   *
   * @param w
   *            the parent window
   * @param input
   *            the page input
   * @throws WorkbenchException
   */
  WorkbenchPage(WorkbenchWindow* w, IAdaptable* input);

  ~WorkbenchPage();

  /**
   * Activates a part. The part will be brought to the front and given focus.
   *
   * @param part
   *            the part to activate
   */
  void Activate(IWorkbenchPart::Pointer part);

  /**
   * Activates a part. The part is given focus, the pane is hilighted.
   */
private:
  void ActivatePart(const IWorkbenchPart::Pointer part);

  /**
   * Adds an IPartListener to the part service.
   */
public:
  void AddPartListener(IPartListener::Pointer l);

  /*
   * (non-Javadoc) Method declared on ISelectionListener.
   */
public:
  void AddSelectionListener(ISelectionListener::Pointer listener);

  /*
   * (non-Javadoc) Method declared on ISelectionListener.
   */
public:
  void AddSelectionListener(const std::string& partId,
      ISelectionListener::Pointer listener);

  /*
   * (non-Javadoc) Method declared on ISelectionListener.
   */
public:
  void AddPostSelectionListener(ISelectionListener::Pointer listener);

  /*
   * (non-Javadoc) Method declared on ISelectionListener.
   */
public:
  void AddPostSelectionListener(const std::string& partId,
      ISelectionListener::Pointer listener);

private:
  ILayoutContainer::Pointer GetContainer(IWorkbenchPart::Pointer part);

private:
  ILayoutContainer::Pointer GetContainer(IWorkbenchPartReference::Pointer part);

private:
  SmartPointer<PartPane> GetPane(IWorkbenchPart::Pointer part);

private:
  SmartPointer<PartPane> GetPane(IWorkbenchPartReference::Pointer part);

  /**
   * Brings a part to the front of its stack. Does not update the active part or
   * active editor. This should only be called if the caller knows that the part
   * is not in the same stack as the active part or active editor, or if the caller
   * is prepared to update activation after the call.
   *
   * @param part
   */
private:
  bool InternalBringToTop(IWorkbenchPartReference::Pointer part);

  /**
   * Moves a part forward in the Z order of a perspective so it is visible.
   * If the part is in the same stack as the active part, the new part is
   * activated.
   *
   * @param part
   *            the part to bring to move forward
   */
public:
  void BringToTop(IWorkbenchPart::Pointer part);

  /**
   * Resets the layout for the perspective. The active part in the old layout
   * is activated in the new layout for consistent user context.
   *
   * Assumes the busy cursor is active.
   */
private:
  void BusyResetPerspective();

  /**
   * Implements <code>setPerspective</code>.
   *
   * Assumes that busy cursor is active.
   *
   * @param desc
   *            identifies the new perspective.
   */
private:
  void BusySetPerspective(IPerspectiveDescriptor::Pointer desc);

  /**
   * Removes the perspective which match the given description.
   *
   * @param desc
   *            identifies the perspective to be removed.
   */
  public:
    void RemovePerspective(IPerspectiveDescriptor::Pointer desc);
  /**
   * Shows a view.
   *
   * Assumes that a busy cursor is active.
   */
protected:
  IViewPart::Pointer BusyShowView(const std::string& viewID,
      const std::string& secondaryID, int mode);

  /*
   * Performs showing of the view in the given mode.
   */
private:
  void BusyShowView(IViewPart::Pointer part, int mode);

  /**
   * Returns whether a part exists in the current page.
   */
private:
  bool CertifyPart(IWorkbenchPart::Pointer part);

  /**
   * Closes the perspective.
   */
public:
  bool Close();

  /**
   * See IWorkbenchPage
   */
public:
  bool CloseAllSavedEditors();

  /**
   * See IWorkbenchPage
   */
public:
  bool CloseAllEditors(bool save);

private:
  void UpdateActivePart();

  /**
   * Makes the given part active. Brings it in front if necessary. Permits null
   * (indicating that no part should be active).
   *
   * @since 3.1
   *
   * @param ref new active part (or null)
   */
private:
  void MakeActive(IWorkbenchPartReference::Pointer ref);

  /**
   * Makes the given editor active. Brings it to front if necessary. Permits <code>null</code>
   * (indicating that no editor is active).
   *
   * @since 3.1
   *
   * @param ref the editor to make active, or <code>null</code> for no active editor
   */
private:
  void MakeActiveEditor(IEditorReference::Pointer ref);

  /**
   * See IWorkbenchPage
   */
public:
  bool CloseEditors(const std::list<IEditorReference::Pointer>& refArray,
      bool save);

  /**
   * Enables or disables listener notifications. This is used to delay listener notifications until the
   * end of a public method.
   *
   * @param shouldDefer
   */
private:
  void DeferUpdates(bool shouldDefer);

private:
  void StartDeferring();

private:
  void HandleDeferredEvents();

private:
  bool IsDeferred();

  /**
   * See IWorkbenchPage#closeEditor
   */
public:
  bool CloseEditor(IEditorReference::Pointer editorRef, bool save);

  /**
   * See IWorkbenchPage#closeEditor
   */
public:
  bool CloseEditor(IEditorPart::Pointer editor, bool save);

  /**
   * @see IWorkbenchPage#closePerspective(IPerspectiveDescriptor, boolean, boolean)
   */
public:

  void CloseCurrentPerspective(bool saveParts, bool closePage);
  /**
   * Closes current perspective. If last perspective, then entire page
   * is closed.
   *
   * @param saveParts
   *            whether the page's parts should be saved if closed
   * @param closePage
   *            whether the page itself should be closed if last perspective
   */

  void ClosePerspective(IPerspectiveDescriptor::Pointer desc, bool saveParts,
      bool closePage);

  /**
   * Closes the specified perspective. If last perspective, then entire page
   * is closed.
   *
   * @param persp
   *            the perspective to be closed
   * @param saveParts
   *            whether the parts that are being closed should be saved
   *            (editors if last perspective, views if not shown in other
   *            parspectives)
   */
  /* package */
protected:
  void ClosePerspective(SmartPointer<Perspective> persp, bool saveParts,
      bool closePage);

  /**
   * @see IWorkbenchPage#closeAllPerspectives(boolean, boolean)
   */
public:
  void CloseAllPerspectives(bool saveEditors, bool closePage);

  /**
   * Creates the client composite.
   */
private:
  void CreateClientComposite();

  /**
   * Creates a new view set. Return null on failure.
   *
   * @param desc the perspective descriptor
   * @param notify whether to fire a perspective opened event
   */
private:
  SmartPointer<Perspective> CreatePerspective(SmartPointer<PerspectiveDescriptor> desc,
      bool notify);

  /**
   * This is called by child objects after a part has been added to the page.
   * The page will in turn notify its listeners.
   */
  /* package */
protected:
  void PartAdded(WorkbenchPartReference::Pointer ref);

  /**
   * This is called by child objects after a part has been added to the page.
   * The part will be queued for disposal after all listeners have been notified
   */
  /* package */
protected:
  void PartRemoved(WorkbenchPartReference::Pointer ref);

private:
  void DisposePart(WorkbenchPartReference::Pointer ref);

  /**
   * Deactivates a part. The pane is unhilighted.
   */
private:
  void DeactivatePart(IWorkbenchPart::Pointer part);

  /**
   * Detaches a view from the WorkbenchWindow.
   */
public:
  void DetachView(IViewReference::Pointer ref);

  /**
   * Removes a detachedwindow.
   */
public:
  void AttachView(IViewReference::Pointer ref);

  /**
   * Dispose a perspective.
   *
   * @param persp the perspective descriptor
   * @param notify whether to fire a perspective closed event
   */
private:
  void DisposePerspective(SmartPointer<Perspective> persp, bool notify);

  /**
   * Returns the first view manager with given ID.
   */
public:
  SmartPointer<Perspective> FindPerspective(IPerspectiveDescriptor::Pointer desc);

  /**
   * See IWorkbenchPage@findView.
   */
public:
  IViewPart::Pointer FindView(const std::string& id);

  /*
   * (non-Javadoc)
   *
   * @see org.blueberry.ui.IWorkbenchPage
   */
public:
  IViewReference::Pointer FindViewReference(const std::string& viewId);

  /*
   * (non-Javadoc)
   *
   * @see org.blueberry.ui.IWorkbenchPage
   */
public:
  IViewReference::Pointer FindViewReference(const std::string& viewId,
      const std::string& secondaryId);

  /**
   * Notify property change listeners about a property change.
   *
   * @param changeId
   *            the change id
   * @param oldValue
   *            old property value
   * @param newValue
   *            new property value
   */
  //private: void FirePropertyChange(String changeId, Object oldValue,
  //            Object newValue) {
  //
  //        UIListenerLogging.logPagePropertyChanged(this, changeId, oldValue, newValue);
  //
  //        Object[] listeners = propertyChangeListeners.getListeners();
  //        PropertyChangeEvent event = new PropertyChangeEvent(this, changeId,
  //                oldValue, newValue);
  //
  //        for (int i = 0; i < listeners.length; i++) {
  //            ((IPropertyChangeListener) listeners[i]).propertyChange(event);
  //        }
  //    }


  /**
   * @see IWorkbenchPage
   */
public:
  IEditorPart::Pointer GetActiveEditor();

  /**
   * Returns the reference for the active editor, or <code>null</code>
   * if there is no active editor.
   *
   * @return the active editor reference or <code>null</code>
   */
public:
  IEditorReference::Pointer GetActiveEditorReference();

  /*
   * (non-Javadoc) Method declared on IPartService
   */
public:
  IWorkbenchPart::Pointer GetActivePart();

  /*
   * (non-Javadoc) Method declared on IPartService
   */
public:
  IWorkbenchPartReference::Pointer GetActivePartReference();

  /**
   * Returns the active perspective for the page, <code>null</code> if
   * none.
   */
public:
  SmartPointer<Perspective> GetActivePerspective();

  /**
   * Returns the client composite.
   */
public:
  void* GetClientComposite();

  //  for dynamic UI - change access from private to protected
  // for testing purposes only, changed from protected to public
  /**
   * Answer the editor manager for this window.
   */
public:
  EditorManager* GetEditorManager();

  /**
   * Answer the perspective presentation.
   */
public:
  PerspectiveHelper* GetPerspectivePresentation();

  /**
   * Answer the editor presentation.
   */
public:
  EditorAreaHelper* GetEditorPresentation();

  /**
   * Allow access to the part service for this page ... used internally to
   * propogate certain types of events to the page part listeners.
   * @return the part service for this page.
   */
public: PartService* GetPartService();

  /**
   * See IWorkbenchPage.
   */
public:
  std::vector<IEditorPart::Pointer> GetEditors();

public:
  std::vector<IEditorPart::Pointer> GetDirtyEditors();

public:
  std::vector<ISaveablePart::Pointer> GetDirtyParts();

  /**
   * See IWorkbenchPage.
   */
public:
  IEditorPart::Pointer FindEditor(IEditorInput::Pointer input);

  /**
   * See IWorkbenchPage.
   */
public:
  std::vector<IEditorReference::Pointer> FindEditors(
      IEditorInput::Pointer input, const std::string& editorId, int matchFlags);

  /**
   * See IWorkbenchPage.
   */
public:
  std::list<IEditorReference::Pointer> GetEditorReferences();

  /**
   * @see IWorkbenchPage
   */
public:
  IAdaptable* GetInput();

  /**
   * Returns the page label. This is a combination of the page input and
   * active perspective.
   */
public:
  std::string GetLabel();

  /**
   * Returns the perspective.
   */
public:
  IPerspectiveDescriptor::Pointer GetPerspective();

  /*
   * (non-Javadoc) Method declared on ISelectionService
   */
public:
  ISelection::ConstPointer GetSelection() const;

  /*
   * (non-Javadoc) Method declared on ISelectionService
   */
public:
  ISelection::ConstPointer GetSelection(const std::string& partId);

//public:
//  SelectionEvents& GetSelectionEvents(const std::string& partId = "");

  /*
   * Returns the view factory.
   */
public:
  ViewFactory* GetViewFactory();

  /**
   * See IWorkbenchPage.
   */
public:
  std::vector<IViewReference::Pointer> GetViewReferences();

  /**
   * See IWorkbenchPage.
   */
public:
  std::vector<IViewPart::Pointer> GetViews();

  /**
   * Returns all view parts in the specified perspective
   *
   * @param persp the perspective
   * @return an array of view parts
   * @since 3.1
   */
  /*package*/
protected:
  std::vector<IViewPart::Pointer> GetViews(SmartPointer<Perspective> persp,
      bool restore);

  /**
   * See IWorkbenchPage.
   */
public:
  IWorkbenchWindow::Pointer GetWorkbenchWindow();

  /*
   * (non-Javadoc)
   *
   * @see org.blueberry.ui.IWorkbenchPage#hideView(org.blueberry.ui.IViewReference)
   */
public:
  void HideView(IViewReference::Pointer ref);

  /* package */
protected:
  void RefreshActiveView();

  /**
   * See IPerspective
   */
public:
  void HideView(IViewPart::Pointer view);

  /**
   * Initialize the page.
   *
   * @param w
   *            the parent window
   * @param layoutID
   *            may be <code>null</code> if restoring from file
   * @param input
   *            the page input
   * @param openExtras
   *            whether to process the perspective extras preference
   */
private:
  void Init(WorkbenchWindow* w, const std::string& layoutID,
      IAdaptable* input, bool openExtras);

  /**
   * Opens the perspectives specified in the PERSPECTIVE_BAR_EXTRAS preference (see bug 84226).
   */
public:
  void OpenPerspectiveExtras();

  /**
   * See IWorkbenchPage.
   */
public:
  bool IsPartVisible(IWorkbenchPart::Pointer part);

  /**
   * See IWorkbenchPage.
   */
public:
  bool IsEditorAreaVisible();

  /**
   * Returns whether the view is fast.
   */
public:
  bool IsFastView(IViewReference::Pointer ref);

  /**
   * Return whether the view is closeable or not.
   *
   * @param ref the view reference to check.  Must not be <code>null</code>.
   * @return true if the part is closeable.
   * @since 3.1.1
   */
public:
  bool IsCloseable(IViewReference::Pointer ref);

  /**
   * Return whether the view is moveable or not.
   *
   * @param ref the view reference to check.  Must not be <code>null</code>.
   * @return true if the part is moveable.
   * @since 3.1.1
   */
public:
  bool IsMoveable(IViewReference::Pointer ref);

  /**
   * Returns whether the layout of the active
   * perspective is fixed.
   */
public:
  bool IsFixedLayout();

  /**
   * Return true if the perspective has a dirty editor.
   */
protected:
  bool IsSaveNeeded();

  /**
   * This method is called when the page is activated.
   */
protected:
  void OnActivate();

  /**
   * This method is called when the page is deactivated.
   */
protected:
  void OnDeactivate();

  /**
   * See IWorkbenchPage.
   */
public:
  void
  ReuseEditor(IReusableEditor::Pointer editor, IEditorInput::Pointer input);

  /**
   * See IWorkbenchPage.
   */
public:
  IEditorPart::Pointer OpenEditor(IEditorInput::Pointer input,
      const std::string& editorID);

  /**
   * See IWorkbenchPage.
   */
public:
  IEditorPart::Pointer OpenEditor(IEditorInput::Pointer input,
      const std::string& editorID, bool activate);

  /**
   * See IWorkbenchPage.
   */
public:
  IEditorPart::Pointer OpenEditor(IEditorInput::Pointer input,
      const std::string& editorID, bool activate, int matchFlags);

  /**
   * This is not public API but for use internally.  editorState can be <code>null</code>.
   */
public:
  IEditorPart::Pointer OpenEditor(IEditorInput::Pointer input,
      const std::string& editorID, bool activate, int matchFlags,
      IMemento::Pointer editorState);

  /*
   * Added to fix Bug 178235 [EditorMgmt] DBCS 3.3 - Cannot open file with external program.
   * Opens a new editor using the given input and descriptor. (Normally, editors are opened using
   * an editor ID and an input.)
   */
public:
  IEditorPart::Pointer OpenEditorFromDescriptor(IEditorInput::Pointer input,
      IEditorDescriptor::Pointer editorDescriptor, bool activate,
      IMemento::Pointer editorState);

  /**
   * @see #openEditor(IEditorInput, String, boolean, int)
   */
private:
  IEditorPart::Pointer BusyOpenEditor(IEditorInput::Pointer input,
      const std::string& editorID, bool activate, int matchFlags,
      IMemento::Pointer editorState);

  /*
   * Added to fix Bug 178235 [EditorMgmt] DBCS 3.3 - Cannot open file with external program.
   * See openEditorFromDescriptor().
   */
private:
  IEditorPart::Pointer BusyOpenEditorFromDescriptor(
      IEditorInput::Pointer input,
      EditorDescriptor::Pointer editorDescriptor, bool activate,
      IMemento::Pointer editorState);

  /**
   * Do not call this method.  Use <code>busyOpenEditor</code>.
   *
   * @see IWorkbenchPage#openEditor(IEditorInput, String, boolean)
   */
protected:
  IEditorPart::Pointer BusyOpenEditorBatched(IEditorInput::Pointer input,
      const std::string& editorID, bool activate, int matchFlags,
      IMemento::Pointer editorState);

  /*
   * Added to fix Bug 178235 [EditorMgmt] DBCS 3.3 - Cannot open file with external program.
   * See openEditorFromDescriptor().
   */
private:
  IEditorPart::Pointer BusyOpenEditorFromDescriptorBatched(
      IEditorInput::Pointer input, EditorDescriptor::Pointer editorDescriptor,
      bool activate, IMemento::Pointer editorState);

public:
  void OpenEmptyTab();

protected:
  void ShowEditor(bool activate, IEditorPart::Pointer editor);

  /**
   * See IWorkbenchPage.
   */
public:
  bool IsEditorPinned(IEditorPart::Pointer editor);

  /**
   * Removes an IPartListener from the part service.
   */
public:
  void RemovePartListener(IPartListener::Pointer l);

  /*
   * (non-Javadoc) Method declared on ISelectionListener.
   */
public:
  void RemoveSelectionListener(ISelectionListener::Pointer listener);

  /*
   * (non-Javadoc) Method declared on ISelectionListener.
   */
public:
  void RemoveSelectionListener(const std::string& partId,
      ISelectionListener::Pointer listener);

  /*
   * (non-Javadoc) Method declared on ISelectionListener.
   */
public:
  void RemovePostSelectionListener(ISelectionListener::Pointer listener);

  /*
   * (non-Javadoc) Method declared on ISelectionListener.
   */
public:
  void RemovePostSelectionListener(const std::string& partId,
      ISelectionListener::Pointer listener);

  /**
   * This method is called when a part is activated by clicking within it. In
   * response, the part, the pane, and all of its actions will be activated.
   *
   * In the current design this method is invoked by the part pane when the
   * pane, the part, or any children gain focus.
   */
public:
  void RequestActivation(IWorkbenchPart::Pointer part);

  /**
   * Resets the layout for the perspective. The active part in the old layout
   * is activated in the new layout for consistent user context.
   */
public:
  void ResetPerspective();

  /**
   * Restore this page from the memento and ensure that the active
   * perspective is equals the active descriptor otherwise create a new
   * perspective for that descriptor. If activeDescriptor is null active the
   * old perspective.
   */
public:
  /*IStatus*/bool RestoreState(IMemento::Pointer memento,
      IPerspectiveDescriptor::Pointer activeDescriptor);

  /**
   * See IWorkbenchPage
   */
public:
  bool SaveAllEditors(bool confirm);

  /**
   * @param confirm
   * @param addNonPartSources true if saveables from non-part sources should be saved too
   * @return false if the user cancelled
   *
   */
public:
  bool SaveAllEditors(bool confirm, bool addNonPartSources);

  /*
   * Saves the workbench part.
   */
protected:
  bool SavePart(ISaveablePart::Pointer saveable, IWorkbenchPart::Pointer part,
      bool confirm);

  /**
   * Saves an editors in the workbench. If <code>confirm</code> is <code>true</code>
   * the user is prompted to confirm the command.
   *
   * @param confirm
   *            if user confirmation should be sought
   * @return <code>true</code> if the command succeeded, or <code>false</code>
   *         if the user cancels the command
   */
public:
  bool SaveEditor(IEditorPart::Pointer editor, bool confirm);

  /**
   * Saves the current perspective.
   */
public:
  void SavePerspective();

  /**
   * Saves the perspective.
   */
public:
  void SavePerspectiveAs(IPerspectiveDescriptor::Pointer newDesc);

  /**
   * Save the state of the page.
   */
public:
  /*IStatus*/bool SaveState(IMemento::Pointer memento);

private:
  std::string GetId(IWorkbenchPart::Pointer part);

private:
  std::string GetId(IWorkbenchPartReference::Pointer ref);

  /**
   * Sets the active part.
   */
private:
  void SetActivePart(IWorkbenchPart::Pointer newPart);

  /**
   * See IWorkbenchPage.
   */
public:
  void SetEditorAreaVisible(bool showEditorArea);

  /**
   * Sets the layout of the page. Assumes the new perspective is not null.
   * Keeps the active part if possible. Updates the window menubar and
   * toolbar if necessary.
   */
private:
  void SetPerspective(SmartPointer<Perspective> newPersp);

  /*
   * Update visibility state of all views.
   */
private:
  void UpdateVisibility(SmartPointer<Perspective> oldPersp,
      SmartPointer<Perspective> newPersp);

  /**
   * Sets the perspective.
   *
   * @param desc
   *            identifies the new perspective.
   */
public:
  void SetPerspective(IPerspectiveDescriptor::Pointer desc);

  /**
   * Restore the toolbar layout for the active perspective.
   */
protected:
  void ResetToolBarLayout();

  /**
   * See IWorkbenchPage.
   */
public:
  IViewPart::Pointer ShowView(const std::string& viewID);

  /*
   * (non-Javadoc)
   *
   * @see org.blueberry.ui.IWorkbenchPage#showView(java.lang.String,
   *      java.lang.String, int)
   */
public:
  IViewPart::Pointer ShowView(const std::string& viewID,
      const std::string& secondaryID, int mode);

  /**
   * @param mode the mode to test
   * @return whether the mode is recognized
   * @since 3.0
   */
private:
  bool CertifyMode(int mode);

  /*
   * Returns the editors in activation order (oldest first).
   */
public:
  std::vector<IEditorReference::Pointer> GetSortedEditors();

  /**
   * @see IWorkbenchPage#getOpenPerspectives()
   */
public:
  std::vector<IPerspectiveDescriptor::Pointer> GetOpenPerspectives();

  /**
   * Return all open Perspective objects.
   *
   * @return all open Perspective objects
   * @since 3.1
   */
  /*package*/
protected:
  std::list<SmartPointer<Perspective> > GetOpenInternalPerspectives();

  /**
   * Checks perspectives in the order they were activiated
   * for the specfied part.  The first sorted perspective
   * that contains the specified part is returned.
   *
   * @param part specified part to search for
   * @return the first sorted perspespective containing the part
   * @since 3.1
   */
  /*package*/
protected:
  SmartPointer<Perspective> GetFirstPerspectiveWithView(IViewPart::Pointer part);

  /**
   * Returns the perspectives in activation order (oldest first).
   */
public:
  std::vector<IPerspectiveDescriptor::Pointer> GetSortedPerspectives();

  /*
   * Returns the parts in activation order (oldest first).
   */
public:
  std::vector<IWorkbenchPartReference::Pointer> GetSortedParts();

  /**
   * Returns the reference to the given part, or <code>null</code> if it has no reference
   * (i.e. it is not a top-level part in this workbench page).
   *
   * @param part the part
   * @return the part's reference or <code>null</code> if the given part does not belong
   * to this workbench page
   */
public:
  IWorkbenchPartReference::Pointer GetReference(IWorkbenchPart::Pointer part);

  //  private: class ActivationList {
  //        //List of parts in the activation order (oldest first)
  //        List parts = new ArrayList();
  //
  //        /*
  //         * Add/Move the active part to end of the list;
  //         */
  //        void setActive(IWorkbenchPart part) {
  //            if (parts.size() <= 0) {
  //        return;
  //      }
  //      IWorkbenchPartReference ref = getReference(part);
  //      if (ref != null) {
  //        if (ref == parts.get(parts.size() - 1)) {
  //          return;
  //        }
  //        parts.remove(ref);
  //        parts.add(ref);
  //      }
  //        }
  //
  //        /*
  //     * Ensures that the given part appears AFTER any other part in the same
  //     * container.
  //     */
  //        void bringToTop(IWorkbenchPartReference ref) {
  //            ILayoutContainer targetContainer = getContainer(ref);
  //
  //            int newIndex = lastIndexOfContainer(targetContainer);
  //
  //            //New index can be -1 if there is no last index
  //            if (newIndex >= 0 && ref == parts.get(newIndex))
  //        return;
  //
  //            parts.remove(ref);
  //            if(newIndex >= 0)
  //              parts.add(newIndex, ref);
  //            else
  //              parts.add(ref);
  //        }
  //
  //        /*
  //         * Returns the last (most recent) index of the given container in the activation list, or returns
  //         * -1 if the given container does not appear in the activation list.
  //         */
  //        int lastIndexOfContainer(ILayoutContainer container) {
  //            for (int i = parts.size() - 1; i >= 0; i--) {
  //                IWorkbenchPartReference ref = (IWorkbenchPartReference)parts.get(i);
  //
  //                ILayoutContainer cnt = getContainer(ref);
  //                if (cnt == container) {
  //                    return i;
  //                }
  //            }
  //
  //            return -1;
  //        }
  //
  //        /*
  //         * Add/Move the active part to end of the list;
  //         */
  //        void setActive(IWorkbenchPartReference ref) {
  //            setActive(ref.getPart(true));
  //        }
  //
  //        /*
  //         * Add the active part to the beginning of the list.
  //         */
  //        void add(IWorkbenchPartReference ref) {
  //            if (parts.indexOf(ref) >= 0) {
  //        return;
  //      }
  //
  //            IWorkbenchPart part = ref.getPart(false);
  //            if (part != null) {
  //                PartPane pane = ((PartSite) part.getSite()).getPane();
  //                if (pane instanceof MultiEditorInnerPane) {
  //                    MultiEditorInnerPane innerPane = (MultiEditorInnerPane) pane;
  //                    add(innerPane.getParentPane().getPartReference());
  //                    return;
  //                }
  //            }
  //            parts.add(0, ref);
  //        }
  //
  //        /*
  //         * Return the active part. Filter fast views.
  //         */
  //        IWorkbenchPart getActive() {
  //            if (parts.isEmpty()) {
  //        return null;
  //      }
  //            return getActive(parts.size() - 1);
  //        }
  //
  //        /*
  //         * Return the previously active part. Filter fast views.
  //         */
  //        IWorkbenchPart getPreviouslyActive() {
  //            if (parts.size() < 2) {
  //        return null;
  //      }
  //            return getActive(parts.size() - 2);
  //        }
  //
  //  private: IWorkbenchPart getActive(int start) {
  //            IWorkbenchPartReference ref = getActiveReference(start, false);
  //
  //            if (ref == null) {
  //                return null;
  //            }
  //
  //            return ref.getPart(true);
  //        }
  //
  //  public: IWorkbenchPartReference getActiveReference(boolean editorsOnly) {
  //            return getActiveReference(parts.size() - 1, editorsOnly);
  //        }
  //
  //  private: IWorkbenchPartReference getActiveReference(int start, boolean editorsOnly) {
  //            // First look for parts that aren't obscured by the current zoom state
  //            IWorkbenchPartReference nonObscured = getActiveReference(start, editorsOnly, true);
  //
  //            if (nonObscured != null) {
  //                return nonObscured;
  //            }
  //
  //            // Now try all the rest of the parts
  //            return getActiveReference(start, editorsOnly, false);
  //        }
  //
  //        /*
  //         * Find a part in the list starting from the end and filter
  //         * and views from other perspectives. Will filter fast views
  //         * unless 'includeActiveFastViews' is true;
  //         */
  //  private: IWorkbenchPartReference getActiveReference(int start, boolean editorsOnly, boolean skipPartsObscuredByZoom) {
  //            IWorkbenchPartReference[] views = getViewReferences();
  //            for (int i = start; i >= 0; i--) {
  //                WorkbenchPartReference ref = (WorkbenchPartReference) parts
  //                        .get(i);
  //
  //                if (editorsOnly && !(ref instanceof IEditorReference)) {
  //                    continue;
  //                }
  //
  //                // Skip parts whose containers have disabled auto-focus
  //                PartPane pane = ref.getPane();
  //
  //                if (pane != null) {
  //                    if (!pane.allowsAutoFocus()) {
  //                        continue;
  //                    }
  //
  //                    if (skipPartsObscuredByZoom) {
  //                        if (pane.isObscuredByZoom()) {
  //                            continue;
  //                        }
  //                    }
  //                }
  //
  //                // Skip fastviews (unless overridden)
  //                if (ref instanceof IViewReference) {
  //                    if (ref == getActiveFastView() || !((IViewReference) ref).isFastView()) {
  //                        for (int j = 0; j < views.length; j++) {
  //                            if (views[j] == ref) {
  //                                return ref;
  //                            }
  //                        }
  //                    }
  //                } else {
  //                    return ref;
  //                }
  //            }
  //            return null;
  //        }
  //
  //        /*
  //         * Retuns the index of the part within the activation list. The higher
  //         * the index, the more recently it was used.
  //         */
  //        int indexOf(IWorkbenchPart part) {
  //          IWorkbenchPartReference ref = getReference(part);
  //          if (ref == null) {
  //            return -1;
  //          }
  //            return parts.indexOf(ref);
  //        }
  //
  //        /*
  //         * Returns the index of the part reference within the activation list.
  //         * The higher the index, the more recent it was used.
  //         */
  //        int indexOf(IWorkbenchPartReference ref) {
  //            return parts.indexOf(ref);
  //        }
  //
  //        /*
  //         * Remove a part from the list
  //         */
  //        boolean remove(IWorkbenchPartReference ref) {
  //            return parts.remove(ref);
  //        }
  //
  //        /*
  //         * Returns the editors in activation order (oldest first).
  //         */
  //  private: IEditorReference[] getEditors() {
  //            ArrayList editors = new ArrayList(parts.size());
  //            for (Iterator i = parts.iterator(); i.hasNext();) {
  //                IWorkbenchPartReference part = (IWorkbenchPartReference) i
  //                        .next();
  //                if (part instanceof IEditorReference) {
  //                    editors.add(part);
  //                }
  //            }
  //            return (IEditorReference[]) editors
  //                    .toArray(new IEditorReference[editors.size()]);
  //        }
  //
  //        /*
  //         * Return a list with all parts (editors and views).
  //         */
  //        private: IWorkbenchPartReference[] getParts() {
  //            IWorkbenchPartReference[] views = getViewReferences();
  //            ArrayList resultList = new ArrayList(parts.size());
  //            for (Iterator iterator = parts.iterator(); iterator.hasNext();) {
  //                IWorkbenchPartReference ref = (IWorkbenchPartReference) iterator
  //                        .next();
  //                if (ref instanceof IViewReference) {
  //                    //Filter views from other perspectives
  //                    for (int i = 0; i < views.length; i++) {
  //                        if (views[i] == ref) {
  //                            resultList.add(ref);
  //                            break;
  //                        }
  //                    }
  //                } else {
  //                    resultList.add(ref);
  //                }
  //            }
  //            IWorkbenchPartReference[] result = new IWorkbenchPartReference[resultList
  //                    .size()];
  //            return (IWorkbenchPartReference[]) resultList.toArray(result);
  //        }
  //
  //        /*
  //         * Returns the topmost editor on the stack, or null if none.
  //         */
  //        IEditorPart getTopEditor() {
  //            IEditorReference editor = (IEditorReference)getActiveReference(parts.size() - 1, true);
  //
  //            if (editor == null) {
  //                return null;
  //            }
  //
  //            return editor.getEditor(true);
  //        }
  //    };


  // for dynamic UI
protected:
  void AddPerspective(SmartPointer<Perspective> persp);

  /**
   * Find the stack of view references stacked with this view part.
   *
   * @param part
   *            the part
   * @return the stack of references
   * @since 3.0
   */
private:
  std::vector<IViewReference::Pointer> GetViewReferenceStack(
      IViewPart::Pointer part);

  /*
   * (non-Javadoc)
   *
   * @see org.blueberry.ui.IWorkbenchPage#getViewStack(org.blueberry.ui.IViewPart)
   */
public:
  std::vector<IViewPart::Pointer> GetViewStack(IViewPart::Pointer part);

  /**
   * Allow for programmatically resizing a part.
   * <p>
   * <em>EXPERIMENTAL</em>
   * </p>
   * <p>
   * Known limitations:
   * <ul>
   * <li>currently applies only to views</li>
   * <li>has no effect when view is zoomed</li>
   * </ul>
   */
public:
  void ResizeView(IViewPart::Pointer part, int width, int height);

private:

  struct ActivationOrderPred : std::binary_function<IViewReference::Pointer,
  IViewReference::Pointer, bool>
  {
    ActivationOrderPred(ActivationList* partList);

    ActivationList* activationList;

    bool operator()(const IViewReference::Pointer o1, const IViewReference::Pointer o2) const;
  };

  // provides sash information for the given pane
  struct SashInfo
  {
    SmartPointer<LayoutPartSash> right;

    SmartPointer<LayoutPartSash> left;

    SmartPointer<LayoutPartSash> top;

    SmartPointer<LayoutPartSash> bottom;

    SmartPointer<LayoutTreeNode> rightNode;

    SmartPointer<LayoutTreeNode> leftNode;

    SmartPointer<LayoutTreeNode> topNode;

    SmartPointer<LayoutTreeNode> bottomNode;
  };

  void FindSashParts(SmartPointer<LayoutTree> tree, const PartPane::Sashes& sashes,
      SashInfo& info);

  /**
   * Returns all parts that are owned by this page
   *
   * @return
   */
protected:
  std::vector<IWorkbenchPartReference::Pointer> GetAllParts();

  /**
   * Returns all open parts that are owned by this page (that is, all parts
   * for which a part opened event would have been sent -- these would be
   * activated parts whose controls have already been created.
   */
protected:
  std::vector<IWorkbenchPartReference::Pointer> GetOpenParts();

  /**
   * Sanity-checks the objects in this page. Throws an Assertation exception
   * if an object's internal state is invalid. ONLY INTENDED FOR USE IN THE
   * UI TEST SUITES.
   */
public:
  void TestInvariants();

  /* (non-Javadoc)
   * @see org.blueberry.ui.IWorkbenchPage#getExtensionTracker()
   */
  //public: IExtensionTracker GetExtensionTracker();

  /*
   * (non-Javadoc)
   *
   * @see org.blueberry.ui.IWorkbenchPage#getPerspectiveShortcuts()
   */
public:
  std::vector<std::string> GetPerspectiveShortcuts();

  /*
   * (non-Javadoc)
   *
   * @see org.blueberry.ui.IWorkbenchPage#getShowViewShortcuts()
   */
public:
  std::vector<std::string> GetShowViewShortcuts();

  /**
   * @since 3.1
   */
private:
  void SuggestReset();

public:
  bool IsPartVisible(IWorkbenchPartReference::Pointer reference);
};
}
#endif /*BERRYWORKBENCHPAGE_H_*/
