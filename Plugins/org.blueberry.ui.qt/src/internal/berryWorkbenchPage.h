/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYWORKBENCHPAGE_H_
#define BERRYWORKBENCHPAGE_H_

#include <berryIAdaptable.h>

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

struct IExtensionPoint;
struct IExtensionTracker;

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
class BERRY_UI_QT WorkbenchPage: public IWorkbenchPage
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
  public:

    ActionSwitcher(WorkbenchPage* page);

    /**
     * Updates the contributions given the new part as the active part.
     *
     * @param newPart
     *            the new active part, may be <code>null</code>
     */
    void UpdateActivePart(IWorkbenchPart::Pointer newPart);

    /**
     * Updates the contributions given the new part as the topEditor.
     *
     * @param newEditor
     *            the new top editor, may be <code>null</code>
     */
    void UpdateTopEditor(IEditorPart::Pointer newEditor);

  private:

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
    void DeactivateContributions(IWorkbenchPart::Pointer part, bool remove);

    WorkbenchPage* page;

    IWorkbenchPart::WeakPtr activePart;

    IEditorPart::WeakPtr topEditor;

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
    QList<SmartPointer<IEditorReference> > GetEditors();

    /*
     * Return a list with all parts (editors and views).
     */
    QList<SmartPointer<IWorkbenchPartReference> > GetParts();

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
    typedef QList<SmartPointer<Perspective> > PerspectiveListType;
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

  QWidget* composite;

  //Could be delete. This information is in the active part list;
  ActivationList* activationList;

  EditorManager* editorMgr;

  EditorAreaHelper* editorPresentation;

  //ListenerList propertyChangeListeners = new ListenerList();

  PageSelectionService* selectionService;

  QScopedPointer<WorkbenchPagePartList> partList; // = new WorkbenchPagePartList(selectionService);

  //IActionBars actionBars;

  ViewFactory* viewFactory;

  PerspectiveList perspList;

  SmartPointer<PerspectiveDescriptor> deferredActivePersp;

  //NavigationHistory navigationHistory = new NavigationHistory(this);

  IStickyViewManager::Pointer stickyViewMan;

  /**
   * Returns true if perspective with given id contains view with given id
   */
  bool HasView(const QString& perspectiveId, const QString& viewId);

  /**
   * If we're in the process of activating a part, this points to the new part.
   * Otherwise, this is null.
   */
  IWorkbenchPartReference::Pointer partBeingActivated;

  /**
   * Contains a list of perspectives that may be dirty due to plugin
   * installation and removal.
   */
  std::set<QString> dirtyPerspectives;

  ActionSwitcher actionSwitcher;

  mutable QScopedPointer<IExtensionTracker> tracker;

  // Deferral count... delays disposing parts and sending certain events if nonzero
  int deferCount;
  // Parts waiting to be disposed
  QList<WorkbenchPartReference::Pointer> pendingDisposals;

  SmartPointer<IExtensionPoint> GetPerspectiveExtensionPoint();

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
  WorkbenchPage(WorkbenchWindow* w, const QString& layoutID,
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

  ~WorkbenchPage() override;

  /**
   * Activates a part. The part will be brought to the front and given focus.
   *
   * @param part
   *            the part to activate
   */
  void Activate(IWorkbenchPart::Pointer part) override;

  /**
   * Adds an IPartListener to the part service.
   */
  void AddPartListener(IPartListener* l) override;

  /*
   * (non-Javadoc) Method declared on ISelectionListener.
   */
  void AddSelectionListener(ISelectionListener* listener) override;

  /*
   * (non-Javadoc) Method declared on ISelectionListener.
   */
  void AddSelectionListener(const QString& partId,
      ISelectionListener* listener) override;

  /*
   * (non-Javadoc) Method declared on ISelectionListener.
   */
  void AddPostSelectionListener(ISelectionListener* listener) override;

  /*
   * (non-Javadoc) Method declared on ISelectionListener.
   */
  void AddPostSelectionListener(const QString& partId,
      ISelectionListener* listener) override;

  /**
   * Moves a part forward in the Z order of a perspective so it is visible.
   * If the part is in the same stack as the active part, the new part is
   * activated.
   *
   * @param part
   *            the part to bring to move forward
   */
  void BringToTop(IWorkbenchPart::Pointer part) override;

private:

  /**
   * Activates a part. The part is given focus, the pane is hilighted.
   */
  void ActivatePart(const IWorkbenchPart::Pointer part);

  ILayoutContainer::Pointer GetContainer(IWorkbenchPart::Pointer part);

  ILayoutContainer::Pointer GetContainer(IWorkbenchPartReference::Pointer part);

  SmartPointer<PartPane> GetPane(IWorkbenchPart::Pointer part);

  SmartPointer<PartPane> GetPane(IWorkbenchPartReference::Pointer part);

  /**
   * Brings a part to the front of its stack. Does not update the active part or
   * active editor. This should only be called if the caller knows that the part
   * is not in the same stack as the active part or active editor, or if the caller
   * is prepared to update activation after the call.
   *
   * @param part
   */
  bool InternalBringToTop(IWorkbenchPartReference::Pointer part);

  /**
   * Resets the layout for the perspective. The active part in the old layout
   * is activated in the new layout for consistent user context.
   *
   * Assumes the busy cursor is active.
   */
  void BusyResetPerspective();

  /**
   * Implements <code>setPerspective</code>.
   *
   * Assumes that busy cursor is active.
   *
   * @param desc
   *            identifies the new perspective.
   */
  void BusySetPerspective(IPerspectiveDescriptor::Pointer desc);

  /*
   * Performs showing of the view in the given mode.
   */
  void BusyShowView(IViewPart::Pointer part, int mode);

  /**
   * Returns whether a part exists in the current page.
   */
  bool CertifyPart(IWorkbenchPart::Pointer part);

protected:

  /**
   * Shows a view.
   *
   * Assumes that a busy cursor is active.
   */
  IViewPart::Pointer BusyShowView(const QString& viewID,
      const QString& secondaryID, int mode);

public:

  void UpdateActionBars();

  /**
   * Removes the perspective which match the given description.
   *
   * @param desc
   *            identifies the perspective to be removed.
   */
  void RemovePerspective(IPerspectiveDescriptor::Pointer desc);

  /**
   * Closes the perspective.
   */
  bool Close() override;

  /**
   * See IWorkbenchPage
   */
  bool CloseAllSavedEditors();

  /**
   * See IWorkbenchPage
   */
  bool CloseAllEditors(bool save) override;

  /**
   * See IWorkbenchPage
   */
  bool CloseEditors(const QList<IEditorReference::Pointer>& refArray,
      bool save) override;

private:

  void UpdateActivePart();

  /**
   * Makes the given part active. Brings it in front if necessary. Permits null
   * (indicating that no part should be active).
   *
   * @param ref new active part (or null)
   */
  void MakeActive(IWorkbenchPartReference::Pointer ref);

  /**
   * Makes the given editor active. Brings it to front if necessary. Permits <code>null</code>
   * (indicating that no editor is active).
   *
   * @param ref the editor to make active, or <code>null</code> for no active editor
   */
  void MakeActiveEditor(IEditorReference::Pointer ref);

  /**
   * Enables or disables listener notifications. This is used to delay listener notifications until the
   * end of a public method.
   *
   * @param shouldDefer
   */
  void DeferUpdates(bool shouldDefer);

  void StartDeferring();

  void HandleDeferredEvents();

  bool IsDeferred();

public:

  /**
   * See IWorkbenchPage#closeEditor
   */
  bool CloseEditor(IEditorReference::Pointer editorRef, bool save);

  /**
   * See IWorkbenchPage#closeEditor
   */
  bool CloseEditor(IEditorPart::Pointer editor, bool save) override;

  /**
   * Closes current perspective. If last perspective, then entire page
   * is closed.
   *
   * @param saveParts
   *            whether the page's parts should be saved if closed
   * @param closePage
   *            whether the page itself should be closed if last perspective
   */
  void CloseCurrentPerspective(bool saveParts, bool closePage);

  /**
   * @see IWorkbenchPage#closePerspective(IPerspectiveDescriptor, boolean, boolean)
   */
  void ClosePerspective(IPerspectiveDescriptor::Pointer desc, bool saveParts,
      bool closePage) override;

  /**
   * @see IWorkbenchPage#closeAllPerspectives(boolean, boolean)
   */
  void CloseAllPerspectives(bool saveEditors, bool closePage) override;

protected:

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
  void ClosePerspective(SmartPointer<Perspective> persp, bool saveParts,
      bool closePage);

  /**
   * This is called by child objects after a part has been added to the page.
   * The page will in turn notify its listeners.
   */
  void PartAdded(WorkbenchPartReference::Pointer ref);

  /**
   * This is called by child objects after a part has been added to the page.
   * The part will be queued for disposal after all listeners have been notified
   */
  void PartRemoved(WorkbenchPartReference::Pointer ref);

private:

  /**
   * Creates the client composite.
   */
  void CreateClientComposite();

  /**
   * Creates a new view set. Return null on failure.
   *
   * @param desc the perspective descriptor
   * @param notify whether to fire a perspective opened event
   */
  SmartPointer<Perspective> CreatePerspective(SmartPointer<PerspectiveDescriptor> desc,
      bool notify);

  void DisposePart(WorkbenchPartReference::Pointer ref);

  /**
   * Deactivates a part. The pane is unhilighted.
   */
  void DeactivatePart(IWorkbenchPart::Pointer part);

  /**
   * Dispose a perspective.
   *
   * @param persp the perspective descriptor
   * @param notify whether to fire a perspective closed event
   */
  void DisposePerspective(SmartPointer<Perspective> persp, bool notify);

public:

  /**
   * Detaches a view from the WorkbenchWindow.
   */
  void DetachView(IViewReference::Pointer ref);

  /**
   * Removes a detachedwindow.
   */
  void AttachView(IViewReference::Pointer ref);

  /**
   * Returns the first view manager with given ID.
   */
  SmartPointer<Perspective> FindPerspective(IPerspectiveDescriptor::Pointer desc);

  /**
   * See IWorkbenchPage@findView.
   */
  IViewPart::Pointer FindView(const QString& id) override;

  /*
   * (non-Javadoc)
   *
   * @see org.blueberry.ui.IWorkbenchPage
   */
  IViewReference::Pointer FindViewReference(const QString& viewId) override;

  /*
   * (non-Javadoc)
   *
   * @see org.blueberry.ui.IWorkbenchPage
   */
  IViewReference::Pointer FindViewReference(const QString& viewId,
      const QString& secondaryId) override;

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
  IEditorPart::Pointer GetActiveEditor() override;

  /**
   * Returns the reference for the active editor, or <code>null</code>
   * if there is no active editor.
   *
   * @return the active editor reference or <code>null</code>
   */
  IEditorReference::Pointer GetActiveEditorReference();

  /*
   * (non-Javadoc) Method declared on IPartService
   */
  IWorkbenchPart::Pointer GetActivePart() override;

  /*
   * (non-Javadoc) Method declared on IPartService
   */
  IWorkbenchPartReference::Pointer GetActivePartReference() override;

  /**
   * Returns the active perspective for the page, <code>null</code> if
   * none.
   */
  SmartPointer<Perspective> GetActivePerspective();

  /**
   * Returns the client composite.
   */
  QWidget* GetClientComposite();

  //  for dynamic UI - change access from private to protected
  // for testing purposes only, changed from protected to public
  /**
   * Answer the editor manager for this window.
   */
  EditorManager* GetEditorManager();

  /**
   * Answer the perspective presentation.
   */
  PerspectiveHelper* GetPerspectivePresentation();

  /**
   * Answer the editor presentation.
   */
  EditorAreaHelper* GetEditorPresentation();

  /**
   * Allow access to the part service for this page ... used internally to
   * propogate certain types of events to the page part listeners.
   * @return the part service for this page.
   */
  PartService* GetPartService();

  /**
   * See IWorkbenchPage.
   */
  QList<IEditorPart::Pointer> GetEditors() override;

  QList<IEditorPart::Pointer> GetDirtyEditors() override;

  QList<ISaveablePart::Pointer> GetDirtyParts();

  /**
   * See IWorkbenchPage.
   */
  IEditorPart::Pointer FindEditor(IEditorInput::Pointer input) override;

  /**
   * See IWorkbenchPage.
   */
  QList<IEditorReference::Pointer> FindEditors(
      IEditorInput::Pointer input, const QString& editorId, int matchFlags) override;

  /**
   * See IWorkbenchPage.
   */
  QList<IEditorReference::Pointer> GetEditorReferences() override;

  /**
   * @see IWorkbenchPage
   */
  IAdaptable* GetInput() override;

  /**
   * Returns the page label. This is a combination of the page input and
   * active perspective.
   */
  QString GetLabel() override;

  /**
   * Returns the perspective.
   */
  IPerspectiveDescriptor::Pointer GetPerspective() override;

  /*
   * (non-Javadoc) Method declared on ISelectionService
   */
  ISelection::ConstPointer GetSelection() const override;

  /*
   * (non-Javadoc) Method declared on ISelectionService
   */
  ISelection::ConstPointer GetSelection(const QString& partId) override;

//public:
//  SelectionEvents& GetSelectionEvents(const QString& partId = "");

  /*
   * Returns the view factory.
   */
  ViewFactory* GetViewFactory();

  /**
   * See IWorkbenchPage.
   */
  QList<IViewReference::Pointer> GetViewReferences() override;

  /**
   * See IWorkbenchPage.
   */
  QList<IViewPart::Pointer> GetViews() override;

protected:

  /**
   * Returns all view parts in the specified perspective
   *
   * @param persp the perspective
   * @return an array of view parts
   */
  /*package*/
  QList<IViewPart::Pointer> GetViews(SmartPointer<Perspective> persp,
      bool restore);

  /* package */
  void RefreshActiveView();

public:

  /**
   * See IWorkbenchPage.
   */
  IWorkbenchWindow::Pointer GetWorkbenchWindow() const override;

  /*
   * (non-Javadoc)
   *
   * @see org.blueberry.ui.IWorkbenchPage#hideView(org.blueberry.ui.IViewReference)
   */
  void HideView(IViewReference::Pointer ref) override;

  /**
   * See IPerspective
   */
  void HideView(IViewPart::Pointer view) override;

private:

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
  void Init(WorkbenchWindow* w, const QString& layoutID,
      IAdaptable* input, bool openExtras);

  /**
   * Opens the perspectives specified in the PERSPECTIVE_BAR_EXTRAS preference (see bug 84226).
   */
public:
  void OpenPerspectiveExtras();

  /**
   * See IWorkbenchPage.
   */
  bool IsPartVisible(IWorkbenchPart::Pointer part) override;

  /**
   * See IWorkbenchPage.
   */
  bool IsEditorAreaVisible();

  /**
   * Returns whether the view is fast.
   */
  bool IsFastView(IViewReference::Pointer ref);

  /**
   * Return whether the view is closeable or not.
   *
   * @param ref the view reference to check.  Must not be <code>null</code>.
   * @return true if the part is closeable.
   */
  bool IsCloseable(IViewReference::Pointer ref);

  /**
   * Return whether the view is moveable or not.
   *
   * @param ref the view reference to check.  Must not be <code>null</code>.
   * @return true if the part is moveable.
   */
  bool IsMoveable(IViewReference::Pointer ref);

  /**
   * Returns whether the layout of the active
   * perspective is fixed.
   */
  bool IsFixedLayout();

protected:

  /**
   * Return true if the perspective has a dirty editor.
   */
  bool IsSaveNeeded();

  /**
   * This method is called when the page is activated.
   */
  void OnActivate();

  /**
   * This method is called when the page is deactivated.
   */
  void OnDeactivate();

public:

  /**
   * See IWorkbenchPage.
   */
  void ReuseEditor(IReusableEditor::Pointer editor, IEditorInput::Pointer input) override;

  /**
   * See IWorkbenchPage.
   */
  IEditorPart::Pointer OpenEditor(IEditorInput::Pointer input,
      const QString& editorID) override;

  /**
   * See IWorkbenchPage.
   */
  IEditorPart::Pointer OpenEditor(IEditorInput::Pointer input,
      const QString& editorID, bool activate) override;

  /**
   * See IWorkbenchPage.
   */
  IEditorPart::Pointer OpenEditor(IEditorInput::Pointer input,
      const QString& editorID, bool activate, int matchFlags) override;

  /**
   * This is not public API but for use internally.  editorState can be <code>null</code>.
   */
  IEditorPart::Pointer OpenEditor(IEditorInput::Pointer input,
      const QString& editorID, bool activate, int matchFlags,
      IMemento::Pointer editorState);

  /*
   * Added to fix Bug 178235 [EditorMgmt] DBCS 3.3 - Cannot open file with external program.
   * Opens a new editor using the given input and descriptor. (Normally, editors are opened using
   * an editor ID and an input.)
   */
  IEditorPart::Pointer OpenEditorFromDescriptor(IEditorInput::Pointer input,
      IEditorDescriptor::Pointer editorDescriptor, bool activate,
      IMemento::Pointer editorState);

private:

  /**
   * @see #openEditor(IEditorInput, String, boolean, int)
   */
  IEditorPart::Pointer BusyOpenEditor(IEditorInput::Pointer input,
      const QString& editorID, bool activate, int matchFlags,
      IMemento::Pointer editorState);

  /*
   * Added to fix Bug 178235 [EditorMgmt] DBCS 3.3 - Cannot open file with external program.
   * See openEditorFromDescriptor().
   */
  IEditorPart::Pointer BusyOpenEditorFromDescriptor(
      IEditorInput::Pointer input,
      EditorDescriptor::Pointer editorDescriptor, bool activate,
      IMemento::Pointer editorState);

  /*
   * Added to fix Bug 178235 [EditorMgmt] DBCS 3.3 - Cannot open file with external program.
   * See openEditorFromDescriptor().
   */
  IEditorPart::Pointer BusyOpenEditorFromDescriptorBatched(
      IEditorInput::Pointer input, EditorDescriptor::Pointer editorDescriptor,
      bool activate, IMemento::Pointer editorState);

public:

  void OpenEmptyTab();

  /**
   * See IWorkbenchPage.
   */
  bool IsEditorPinned(IEditorPart::Pointer editor) override;

  /**
   * Removes an IPartListener from the part service.
   */
  void RemovePartListener(IPartListener* l) override;

  /*
   * (non-Javadoc) Method declared on ISelectionListener.
   */
  void RemoveSelectionListener(ISelectionListener* listener) override;

  /*
   * (non-Javadoc) Method declared on ISelectionListener.
   */
  void RemoveSelectionListener(const QString& partId,
                               ISelectionListener* listener) override;

  /*
   * (non-Javadoc) Method declared on ISelectionListener.
   */
  void RemovePostSelectionListener(ISelectionListener* listener) override;

  /*
   * (non-Javadoc) Method declared on ISelectionListener.
   */
  void RemovePostSelectionListener(const QString& partId,
                                   ISelectionListener* listener) override;

  /**
   * This method is called when a part is activated by clicking within it. In
   * response, the part, the pane, and all of its actions will be activated.
   *
   * In the current design this method is invoked by the part pane when the
   * pane, the part, or any children gain focus.
   */
  void RequestActivation(IWorkbenchPart::Pointer part);

  /**
   * Resets the layout for the perspective. The active part in the old layout
   * is activated in the new layout for consistent user context.
   */
  void ResetPerspective() override;

  /**
   * Restore this page from the memento and ensure that the active
   * perspective is equals the active descriptor otherwise create a new
   * perspective for that descriptor. If activeDescriptor is null active the
   * old perspective.
   */
  /*IStatus*/bool RestoreState(IMemento::Pointer memento,
      IPerspectiveDescriptor::Pointer activeDescriptor);

  /**
   * See IWorkbenchPage
   */
  bool SaveAllEditors(bool confirm) override;

  /**
   * @param confirm
   * @param addNonPartSources true if saveables from non-part sources should be saved too
   * @return false if the user cancelled
   *
   */
  bool SaveAllEditors(bool confirm, bool addNonPartSources);

  /**
   * Saves an editors in the workbench. If <code>confirm</code> is <code>true</code>
   * the user is prompted to confirm the command.
   *
   * @param confirm
   *            if user confirmation should be sought
   * @return <code>true</code> if the command succeeded, or <code>false</code>
   *         if the user cancels the command
   */
  bool SaveEditor(IEditorPart::Pointer editor, bool confirm) override;

  /**
   * Saves the current perspective.
   */
  void SavePerspective() override;

  /**
   * Saves the perspective.
   */
  void SavePerspectiveAs(IPerspectiveDescriptor::Pointer newDesc) override;

  /**
   * Save the state of the page.
   */
  /*IStatus*/bool SaveState(IMemento::Pointer memento);

  /**
   * See IWorkbenchPage.
   */
  void SetEditorAreaVisible(bool showEditorArea);

  /**
   * Sets the perspective.
   *
   * @param desc
   *            identifies the new perspective.
   */
  void SetPerspective(IPerspectiveDescriptor::Pointer desc) override;

  /**
   * See IWorkbenchPage.
   */
  IViewPart::Pointer ShowView(const QString& viewID) override;

  /*
   * (non-Javadoc)
   *
   * @see org.blueberry.ui.IWorkbenchPage#showView(java.lang.String,
   *      java.lang.String, int)
   */
  IViewPart::Pointer ShowView(const QString& viewID,
      const QString& secondaryID, int mode) override;


  /*
   * Returns the editors in activation order (oldest first).
   */
  QList<IEditorReference::Pointer> GetSortedEditors();

  /**
   * @see IWorkbenchPage#getOpenPerspectives()
   */
  QList<IPerspectiveDescriptor::Pointer> GetOpenPerspectives() override;

protected:

  /**
   * Do not call this method.  Use <code>busyOpenEditor</code>.
   *
   * @see IWorkbenchPage#openEditor(IEditorInput, String, boolean)
   */
  IEditorPart::Pointer BusyOpenEditorBatched(IEditorInput::Pointer input,
      const QString& editorID, bool activate, int matchFlags,
      IMemento::Pointer editorState);

  void ShowEditor(bool activate, IEditorPart::Pointer editor);

  /*
   * Saves the workbench part.
   */
  bool SavePart(ISaveablePart::Pointer saveable, IWorkbenchPart::Pointer part,
      bool confirm);

  /**
   * Restore the toolbar layout for the active perspective.
   */
  void ResetToolBarLayout();

  /**
   * Return all open Perspective objects.
   *
   * @return all open Perspective objects
   */
  /*package*/
  QList<SmartPointer<Perspective> > GetOpenInternalPerspectives();

  /**
   * Checks perspectives in the order they were activiated
   * for the specfied part.  The first sorted perspective
   * that contains the specified part is returned.
   *
   * @param part specified part to search for
   * @return the first sorted perspespective containing the part
   */
  /*package*/
  SmartPointer<Perspective> GetFirstPerspectiveWithView(IViewPart::Pointer part);

  // for dynamic UI
  void AddPerspective(SmartPointer<Perspective> persp);

public:

  /**
   * Returns the perspectives in activation order (oldest first).
   */
  QList<IPerspectiveDescriptor::Pointer> GetSortedPerspectives() override;

  /*
   * Returns the parts in activation order (oldest first).
   */
  QList<IWorkbenchPartReference::Pointer> GetSortedParts();

  /**
   * Returns the reference to the given part, or <code>null</code> if it has no reference
   * (i.e. it is not a top-level part in this workbench page).
   *
   * @param part the part
   * @return the part's reference or <code>null</code> if the given part does not belong
   * to this workbench page
   */
  IWorkbenchPartReference::Pointer GetReference(IWorkbenchPart::Pointer part) override;

  /*
   * (non-Javadoc)
   *
   * @see org.blueberry.ui.IWorkbenchPage#getViewStack(org.blueberry.ui.IViewPart)
   */
  QList<IViewPart::Pointer> GetViewStack(IViewPart::Pointer part);

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
  void ResizeView(IViewPart::Pointer part, int width, int height);

  /**
   * Sanity-checks the objects in this page. Throws an Assertation exception
   * if an object's internal state is invalid. ONLY INTENDED FOR USE IN THE
   * UI TEST SUITES.
   */
  void TestInvariants();

  IExtensionTracker* GetExtensionTracker() const override;

  /*
   * (non-Javadoc)
   *
   * @see org.blueberry.ui.IWorkbenchPage#getPerspectiveShortcuts()
   */
  QList<QString> GetPerspectiveShortcuts() override;

  /*
   * (non-Javadoc)
   *
   * @see org.blueberry.ui.IWorkbenchPage#getShowViewShortcuts()
   */
  QList<QString> GetShowViewShortcuts() override;

  bool IsPartVisible(IWorkbenchPartReference::Pointer reference);

private:

  QString GetId(IWorkbenchPart::Pointer part);

  QString GetId(IWorkbenchPartReference::Pointer ref);

  /**
   * Sets the active part.
   */
  void SetActivePart(IWorkbenchPart::Pointer newPart);

  /**
   * Sets the layout of the page. Assumes the new perspective is not null.
   * Keeps the active part if possible. Updates the window menubar and
   * toolbar if necessary.
   */
  void SetPerspective(SmartPointer<Perspective> newPersp);

  /*
   * Update visibility state of all views.
   */
  void UpdateVisibility(SmartPointer<Perspective> oldPersp,
      SmartPointer<Perspective> newPersp);

  /**
   * @param mode the mode to test
   * @return whether the mode is recognized
   */
  bool CertifyMode(int mode);

  /**
   * Find the stack of view references stacked with this view part.
   *
   * @param part
   *            the part
   * @return the stack of references
   */
  QList<IViewReference::Pointer> GetViewReferenceStack(
      IViewPart::Pointer part);

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

protected:

  /**
   * Returns all parts that are owned by this page
   *
   * @return
   */
  QList<IWorkbenchPartReference::Pointer> GetAllParts();

  /**
   * Returns all open parts that are owned by this page (that is, all parts
   * for which a part opened event would have been sent -- these would be
   * activated parts whose controls have already been created.
   */
  QList<IWorkbenchPartReference::Pointer> GetOpenParts();

private:

  void SuggestReset();

};

}
#endif /*BERRYWORKBENCHPAGE_H_*/
