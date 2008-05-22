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

#include "cherryPerspective.h"

namespace cherry
{

const std::string Perspective::VERSION_STRING = "0.016";

Perspective::Perspective(PerspectiveDescriptor::Pointer desc,
    WorkbenchPage::Pointer page)
 : descriptor(desc)
 {
  this->Init(page);
  if (desc.IsNotNull())
  {
    this->CreatePresentation(desc);
  }
}

Perspective::Perspective(WorkbenchPage::Pointer page)
{
  this->Init(page);
}

void Perspective::Init(WorkbenchPage::Pointer page)
{
  editorHidden = false;
  editorAreaState = IStackPresentationSite.STATE_RESTORED;
  shouldHideEditorsOnActivate = false;
  this->page = page;
  this->editorArea = page->GetEditorPresentation()->GetLayoutPart();
  this->viewFactory = page->GetViewFactory();
}

bool Perspective::BringToTop(IViewReference::Pointer ref)
{
  return presentation->BringPartToTop(this->GetPane(ref));
}

bool Perspective::ContainsView(IViewPart::Pointer view)
{
  IViewSite::Pointer site = view->GetViewSite();
  IViewReference::Pointer ref = this->FindView(site->GetId(), site->GetSecondaryId());
  if (ref.IsNull())
  {
    return false;
  }
  return (view == ref->GetPart(false));
}

void Perspective::CreatePresentation(PerspectiveDescriptor::Pointer persp)
{
  if (persp->HasCustomDefinition())
  {
    this->LoadCustomPersp(persp);
  }
  else
  {
    this->LoadPredefinedPersp(persp);
  }
}

Perspective::~Perspective()
{
  // Get rid of presentation.
  if (presentation == 0)
  {
    return;
  }

  presentation->Deactivate();

  // Release each view.
  std::vector<IViewReference::Pointer> refs(this->GetViewReferences());
  for (unsigned int i = 0, length = refs.size(); i < length; i++)
  {
    this->GetViewFactory()->ReleaseView(refs[i]);
  }

  mapIDtoViewLayoutRec.clear();
}

IViewReference::Pointer Perspective::FindView(const std::string& viewId)
{
  return this->FindView(viewId, "");
}

IViewReference::Pointer Perspective::FindView(const std::string& id, const std::string& secondaryId)
{
  std::vector<IViewReference::Pointer> refs(this->GetViewReferences());
  for (unsigned int i = 0; i < refs.size(); i++)
  {
    IViewReference::Pointer ref = refs[i];
    if (id == ref->GetId()
        && (secondaryId  == ref->GetSecondaryId()))
    {
      return ref;
    }
  }
  return 0;
}

IPerspectiveDescriptor::Pointer Perspective::GetDesc()
{
  return descriptor;
}

IViewPane::Pointer Perspective::GetPane(IViewReference::Pointer ref)
{
  return ref.Cast<WorkbenchPartReference>()->GetPane().Cast<IViewPane>();
}

std::vector<std::string> Perspective::GetPerspectiveShortcuts()
{
  return (String[]) perspectiveShortcuts.toArray(new String[perspectiveShortcuts.size()]);
}

PerspectiveHelper* Perspective::GetPresentation()
{
  return presentation;
}

std::vector<std::string> Perspective::GetShowViewShortcuts()
{
  return (String[]) showViewShortcuts.toArray(new String[showViewShortcuts.size()]);
}

ViewFactory* Perspective::GetViewFactory()
{
  return viewFactory;
}

std::vector<IViewReference::Pointer> Perspective::GetViewReferences()
{
  // Get normal views.
  if (presentation == 0)
  {
    return std::vector<IViewReference::Pointer>();
  }

  List panes = new ArrayList(5);
  presentation->CollectViewPanes(panes);

  List fastViews = (fastViewManager != null) ?
  fastViewManager.getFastViews(null)
  : new ArrayList();
  IViewReference[] resultArray = new IViewReference[panes.size()
  + fastViews.size()];

  // Copy fast views.
  int nView = 0;
  for (int i = 0; i < fastViews.size(); i++)
  {
    resultArray[nView] = (IViewReference) fastViews.get(i);
    ++nView;
  }

  // Copy normal views.
  for (int i = 0; i < panes.size(); i++)
  {
    ViewPane pane = (ViewPane) panes.get(i);
    resultArray[nView] = pane.getViewReference();
    ++nView;
  }

  return resultArray;
}

void Perspective::HideEditorArea()
{
  if (!this->IsEditorAreaVisible())
  {
    return;
  }

  // Show the editor in the appropriate location
  if (this->UseNewMinMax(this))
  {
    // If it's the currently maximized part we have to restore first
    if (getPresentation().getMaximizedStack() instanceof EditorStack)
    {
      getPresentation().getMaximizedStack().setState(IStackPresentationSite.STATE_RESTORED);
    }

    bool isMinimized = editorAreaState == IStackPresentationSite.STATE_MINIMIZED;
    if (!isMinimized)
    hideEditorAreaLocal();
    else
    setEditorAreaTrimVisibility(false);
  }
  else
  {
    hideEditorAreaLocal();
  }

  editorHidden = true;
}

void Perspective::HideEditorAreaLocal()
{
  if (editorHolder != null)
  {
    return;
  }

  // Replace the editor area with a placeholder so we
  // know where to put it back on show editor area request.
  editorHolder = new PartPlaceholder(editorArea.getID());
  presentation.getLayout().replace(editorArea, editorHolder);
}

bool Perspective::HideView(IViewReference::Pointer ref)
{
  // If the view is locked just return.
  IViewPane::Pointer pane = this->GetPane(ref);

  presentation->RemovePart(pane);
  
  // Dispose view if ref count == 0.
  this->GetViewFactory()->ReleaseView(ref);
  return true;
}

bool Perspective::IsEditorAreaVisible()
{
  return !editorHidden;
}

ViewLayoutRec::Pointer Perspective::GetViewLayoutRec(IViewReference::Pointer ref, bool create)
{
  ViewLayoutRec::Pointer result = this->GetViewLayoutRec(ViewFactory::GetKey(ref), create);
  if (result.IsNull() && create==false)
  {
    result = this->GetViewLayoutRec(ref->GetId(), false);
  }
  return result;
}

ViewLayoutRec::Pointer Perspective::GetViewLayoutRec(const std::string& viewId, bool create)
{
  ViewLayoutRec::Pointer rec = mapIDtoViewLayoutRec[viewId];
  if (rec.IsNull() && create)
  {
    rec = new ViewLayoutRec();
    mapIDtoViewLayoutRec[viewId] = rec;
  }
  return rec;
}

bool Perspective::IsFixedLayout()
{
  //@issue is there a difference between a fixed
  //layout and a fixed perspective?? If not the API
  //may need some polish, WorkbenchPage, PageLayout
  //and Perspective all have isFixed methods.  
  //PageLayout and Perspective have their own fixed
  //attribute, we are assuming they are always in sync.
  //WorkbenchPage delegates to the perspective.
  return fixed;
}

bool Perspective::IsStandaloneView(IViewReference::Pointer ref)
{
  ViewLayoutRec::Pointer rec = this->GetViewLayoutRec(ref, false);
  return rec.IsNotNull() && rec->isStandalone;
}

bool Perspective::GetShowTitleView(IViewReference::Pointer ref)
{
  ViewLayoutRec::Pointer rec = this->GetViewLayoutRec(ref, false);
  return rec.IsNotNull() && rec->showTitle;
}

void Perspective::LoadCustomPersp(PerspectiveDescriptor::Pointer persp)
{
  //get the layout from the registry  
  PerspectiveRegistry* perspRegistry = WorkbenchPlugin::GetDefault()->GetPerspectiveRegistry();
  try
  {
    IMemento::Pointer memento = perspRegistry.getCustomPersp(persp.getId());
    // Restore the layout state.
    MultiStatus status = new MultiStatus(
        PlatformUI.PLUGIN_ID,
        IStatus.OK,
        NLS.bind(WorkbenchMessages.Perspective_unableToRestorePerspective, persp.getLabel()),
        null);
    status.merge(restoreState(memento));
    status.merge(restoreState());
    if (status.getSeverity() != IStatus.OK)
    {
      unableToOpenPerspective(persp, status);
    }
  }
  catch (IOException e)
  {
    unableToOpenPerspective(persp, null);
  }
  catch (WorkbenchException e)
  {
    unableToOpenPerspective(persp, e.getStatus());
  }
}

void Perspective::UnableToOpenPerspective(PerspectiveDescriptor::Pointer persp,
    IStatus status)
{
  PerspectiveRegistry perspRegistry = (PerspectiveRegistry) WorkbenchPlugin
  .getDefault().getPerspectiveRegistry();
  perspRegistry.deletePerspective(persp);
  // If this is a predefined perspective, we will not be able to delete
  // the perspective (we wouldn't want to).  But make sure to delete the
  // customized portion.
  persp.deleteCustomDefinition();
  String title = WorkbenchMessages.Perspective_problemRestoringTitle;
  String msg = WorkbenchMessages.Perspective_errorReadingState;
  if (status == null)
  {
    MessageDialog.openError((Shell) null, title, msg);
  }
  else
  {
    ErrorDialog.openError((Shell) null, title, msg, status);
  }
}

void Perspective::LoadPredefinedPersp(PerspectiveDescriptor::Pointer persp)
{
  // Create layout engine.
  IPerspectiveFactory factory = null;
  try
  {
    factory = persp.createFactory();
  }
  catch (CoreException e)
  {
    throw new WorkbenchException(NLS.bind(WorkbenchMessages.Perspective_unableToLoad, persp.getId() ));
  }

  /*
   * IPerspectiveFactory#createFactory() can return null
   */
  if (factory == null)
  {
    throw new WorkbenchException(NLS.bind(WorkbenchMessages.Perspective_unableToLoad, persp.getId() ));
  }

  // Create layout factory.
  ViewSashContainer container = new ViewSashContainer(page, getClientComposite());
  layout = new PageLayout(container, getViewFactory(),
      editorArea, descriptor);
  layout.setFixed(descriptor.getFixed());

  // add the placeholders for the sticky folders and their contents 
  IPlaceholderFolderLayout stickyFolderRight = null, stickyFolderLeft = null, stickyFolderTop = null, stickyFolderBottom = null;

  IStickyViewDescriptor[] descs = WorkbenchPlugin.getDefault()
  .getViewRegistry().getStickyViews();
  for (int i = 0; i < descs.length; i++)
  {
    IStickyViewDescriptor stickyViewDescriptor = descs[i];
    String id = stickyViewDescriptor.getId();
    switch (stickyViewDescriptor.getLocation())
    {
      case IPageLayout.RIGHT:
      if (stickyFolderRight == null)
      {
        stickyFolderRight = layout
        .createPlaceholderFolder(
            StickyViewDescriptor.STICKY_FOLDER_RIGHT,
            IPageLayout.RIGHT, .75f,
            IPageLayout.ID_EDITOR_AREA);
      }
      stickyFolderRight.addPlaceholder(id);
      break;
      case IPageLayout.LEFT:
      if (stickyFolderLeft == null)
      {
        stickyFolderLeft = layout.createPlaceholderFolder(
            StickyViewDescriptor.STICKY_FOLDER_LEFT,
            IPageLayout.LEFT, .25f, IPageLayout.ID_EDITOR_AREA);
      }
      stickyFolderLeft.addPlaceholder(id);
      break;
      case IPageLayout.TOP:
      if (stickyFolderTop == null)
      {
        stickyFolderTop = layout.createPlaceholderFolder(
            StickyViewDescriptor.STICKY_FOLDER_TOP,
            IPageLayout.TOP, .25f, IPageLayout.ID_EDITOR_AREA);
      }
      stickyFolderTop.addPlaceholder(id);
      break;
      case IPageLayout.BOTTOM:
      if (stickyFolderBottom == null)
      {
        stickyFolderBottom = layout.createPlaceholderFolder(
            StickyViewDescriptor.STICKY_FOLDER_BOTTOM,
            IPageLayout.BOTTOM, .75f,
            IPageLayout.ID_EDITOR_AREA);
      }
      stickyFolderBottom.addPlaceholder(id);
      break;
    }

    //should never be null as we've just added the view above
    IViewLayout viewLayout = layout.getViewLayout(id);
    viewLayout.setCloseable(stickyViewDescriptor.isCloseable());
    viewLayout.setMoveable(stickyViewDescriptor.isMoveable());
  }

  // Run layout engine.
  factory.createInitialLayout(layout);
  PerspectiveExtensionReader extender = new PerspectiveExtensionReader();
  extender.extendLayout(page.getExtensionTracker(), descriptor.getId(), layout);

  // Retrieve view layout info stored in the page layout.
  mapIDtoViewLayoutRec.putAll(layout.getIDtoViewLayoutRecMap());

  // Create action sets.
  List temp = new ArrayList();
  createInitialActionSets(temp, layout.getActionSets());

  IContextService service = null;
  if (page != null)
  {
    service = (IContextService) page.getWorkbenchWindow().getService(
        IContextService.class);
  }
  try
  {
    if (service!=null)
    {
      service.activateContext(ContextAuthority.DEFER_EVENTS);
    }
    for (Iterator iter = temp.iterator(); iter.hasNext();)
    {
      IActionSetDescriptor descriptor = (IActionSetDescriptor) iter
      .next();
      addAlwaysOn(descriptor);
    }
  }finally
  {
    if (service!=null)
    {
      service.activateContext(ContextAuthority.SEND_EVENTS);
    }
  }
  newWizardShortcuts = layout.getNewWizardShortcuts();
  showViewShortcuts = layout.getShowViewShortcuts();
  perspectiveShortcuts = layout.getPerspectiveShortcuts();
  showInPartIds = layout.getShowInPartIds();

  // Retrieve fast views
  if (fastViewManager != null)
  {
    ArrayList fastViews = layout.getFastViews();
    for (Iterator fvIter = fastViews.iterator(); fvIter.hasNext();)
    {
      IViewReference ref = (IViewReference) fvIter.next();
      fastViewManager.addViewReference(FastViewBar.FASTVIEWBAR_ID, -1, ref,
          !fvIter.hasNext());
    }
  }

  // Is the layout fixed
  fixed = layout.isFixed();

  // Create presentation. 
  presentation = new PerspectiveHelper(page, container, this);

  // Hide editor area if requested by factory
  if (!layout.isEditorAreaVisible())
  {
    hideEditorArea();
  }

}

void Perspective::OnActivate()
{
  // Update editor area state.
  if (editorArea.getControl() != null)
  {
    bool visible = isEditorAreaVisible();
    bool inTrim = editorAreaState == IStackPresentationSite.STATE_MINIMIZED;

    // Funky check: Intro uses the old zoom behaviour when maximized. Make sure we don't show the
    // editor if it's supposed to be hidden because the intro is maximized. Note that
    // 'childObscuredByZoom' will only respond 'true' when using the old behaviour.
    bool introMaxed = getPresentation().getLayout().childObscuredByZoom(editorArea);

    editorArea.setVisible(visible && !inTrim && !introMaxed);
  }

  // Update fast views.
  // Make sure the control for the fastviews are created so they can
  // be activated.
  if (fastViewManager != null)
  {
    List fastViews = fastViewManager.getFastViews(null);
    for (int i = 0; i < fastViews.size(); i++)
    {
      ViewPane pane = getPane((IViewReference) fastViews.get(i));
      if (pane != null)
      {
        Control ctrl = pane.getControl();
        if (ctrl == null)
        {
          pane.createControl(getClientComposite());
          ctrl = pane.getControl();
        }
        ctrl.setEnabled(false); // Remove focus support.
      }
    }
  }

  // Set the visibility of all fast view pins
  setAllPinsVisible(true);

  // Trim Stack Support
  bool useNewMinMax = Perspective.useNewMinMax(this);
  bool hideEditorArea = shouldHideEditorsOnActivate || (editorHidden && editorHolder == null);

  // We have to set the editor area's stack state -before-
  // activating the presentation since it's used there to determine
  // size of the resulting stack
  if (useNewMinMax && !hideEditorArea)
  {
    refreshEditorAreaVisibility();
  }

  // Show the layout
  presentation.activate(getClientComposite());

  if (useNewMinMax)
  {
    fastViewManager.activate();

    // Move any minimized extension stacks to the trim      
    if (layout != null)
    {
      // Turn aimations off
      IPreferenceStore preferenceStore = PrefUtil.getAPIPreferenceStore();
      bool useAnimations = preferenceStore
      .getbool(IWorkbenchPreferenceConstants.ENABLE_ANIMATIONS);
      preferenceStore.setValue(IWorkbenchPreferenceConstants.ENABLE_ANIMATIONS, false);

      List minStacks = layout.getMinimizedStacks();
      for (Iterator msIter = minStacks.iterator(); msIter.hasNext();)
      {
        ViewStack vs = (ViewStack) msIter.next();
        vs.setMinimized(true);
      }

      // Restore the animation pref
      preferenceStore.setValue(IWorkbenchPreferenceConstants.ENABLE_ANIMATIONS, useAnimations);

      // this is a one-off deal...set during the extension reading
      minStacks.clear();
      layout = null;
    }
  }
  else
  {
    // Update the FVB only if not using the new min/max
    WorkbenchWindow wbw = (WorkbenchWindow) page.getWorkbenchWindow();
    if (wbw != null)
    {
      ITrimManager tbm = wbw.getTrimManager();
      if (tbm != null)
      {
        IWindowTrim fvb = tbm.getTrim(FastViewBar.FASTVIEWBAR_ID);
        if (fvb instanceof FastViewBar)
        {
          ((FastViewBar)fvb).update(true);
        }
      }
    }
  }

  // If we are -not- using the new min/max then ensure that there
  // are no stacks in the trim. This can happen when a user switches
  // back to the 3.0 presentation... 
  if (!Perspective.useNewMinMax(this) && fastViewManager != null)
  {
    bool stacksWereRestored = fastViewManager.restoreAllTrimStacks();
    setEditorAreaTrimVisibility(false);

    // Restore any 'maximized' view stack since we've restored
    // the minimized stacks
    if (stacksWereRestored && presentation.getMaximizedStack() instanceof ViewStack)
    {
      ViewStack vs = (ViewStack) presentation.getMaximizedStack();
      vs.setPresentationState(IStackPresentationSite.STATE_RESTORED);
      presentation.setMaximizedStack(null);
    }
  }

  // We hide the editor area -after- the presentation activates
  if (hideEditorArea)
  {
    // We do this here to ensure that createPartControl is called on the
    // top editor
    // before it is hidden. See bug 20166.
    hideEditorArea();
    shouldHideEditorsOnActivate = false;

    // this is an override so it should handle both states
    if (useNewMinMax)
    setEditorAreaTrimVisibility(editorAreaState == IStackPresentationSite.STATE_MINIMIZED);
  }
}

void Perspective::OnDeactivate()
{
  presentation.deactivate();
  setActiveFastView(null);
  setAllPinsVisible(false);

  // Update fast views.
  if (fastViewManager != null)
  {
    List fastViews = fastViewManager.getFastViews(null);
    for (int i = 0; i < fastViews.size(); i++)
    {
      ViewPane pane = getPane((IViewReference) fastViews.get(i));
      if (pane != null)
      {
        Control ctrl = pane.getControl();
        if (ctrl != null)
        {
          ctrl.setEnabled(true); // Add focus support.
        }
      }
    }

    fastViewManager.deActivate();
  }

  // Ensure that the editor area trim is hidden as well
  setEditorAreaTrimVisibility(false);
}

void Perspective::PartActivated(IWorkbenchPart::Pointer activePart)
{
  // If a fastview is open close it.
  if (activeFastView != null
      && activeFastView.getPart(false) != activePart)
  {
    setActiveFastView(null);
  }
}

void Perspective::PerformedShowIn(const std::string& partId)
{
  showInTimes.put(partId, new Long(System.currentTimeMillis()));
}

bool Perspective::RestoreState(IMemento::Pointer memento)
{
  MultiStatus result = new MultiStatus(
      PlatformUI.PLUGIN_ID,
      IStatus.OK,
      WorkbenchMessages.Perspective_problemsRestoringPerspective, null);

  // Create persp descriptor.
  descriptor = new PerspectiveDescriptor(null, null, null);
  result.add(descriptor.restoreState(memento));
  PerspectiveDescriptor desc = (PerspectiveDescriptor) WorkbenchPlugin
  .getDefault().getPerspectiveRegistry().findPerspectiveWithId(
      descriptor.getId());
  if (desc != null)
  {
    descriptor = desc;
  }

  this.memento = memento;
  // Add the visible views.
  IMemento views[] = memento.getChildren(IWorkbenchConstants.TAG_VIEW);
  result.merge(createReferences(views));

  memento = memento.getChild(IWorkbenchConstants.TAG_FAST_VIEWS);
  if (memento != null)
  {
    views = memento.getChildren(IWorkbenchConstants.TAG_VIEW);
    result.merge(createReferences(views));
  }
  return result;
}

bool Perspective::CreateReferences(std::vector<IMemento::Pointer> views)
{
  MultiStatus result = new MultiStatus(PlatformUI.PLUGIN_ID, IStatus.OK,
      WorkbenchMessages.Perspective_problemsRestoringViews, null);

  for (int x = 0; x < views.length; x++)
  {
    // Get the view details.
    IMemento childMem = views[x];
    String id = childMem.getString(IWorkbenchConstants.TAG_ID);
    // skip creation of the intro reference -  it's handled elsewhere.
    if (id.equals(IIntroConstants.INTRO_VIEW_ID))
    {
      continue;
    }

    String secondaryId = ViewFactory.extractSecondaryId(id);
    if (secondaryId != null)
    {
      id = ViewFactory.extractPrimaryId(id);
    }
    // Create and open the view.
    try
    {
      if (!"true".equals(childMem.getString(IWorkbenchConstants.TAG_REMOVED)))
      { //$NON-NLS-1$
        viewFactory.createView(id, secondaryId);
      }
    }
    catch (PartInitException e)
    {
      childMem.putString(IWorkbenchConstants.TAG_REMOVED, "true"); //$NON-NLS-1$
      result.add(StatusUtil.newStatus(IStatus.ERROR,
              e.getMessage() == null ? "" : e.getMessage(), //$NON-NLS-1$
              e));
    }
  }
  return result;
}

bool Perspective::RestoreState()
{
  if (this.memento == null)
  {
    return new Status(IStatus.OK, PlatformUI.PLUGIN_ID, 0, "", null); //$NON-NLS-1$
  }

  MultiStatus result = new MultiStatus(
      PlatformUI.PLUGIN_ID,
      IStatus.OK,
      WorkbenchMessages.Perspective_problemsRestoringPerspective, null);

  IMemento memento = this.memento;
  this.memento = null;

  final IMemento boundsMem = memento.getChild(IWorkbenchConstants.TAG_WINDOW);
  if (boundsMem != null)
  {
    final Rectangle r = new Rectangle(0, 0, 0, 0);
    r.x = boundsMem.getInteger(IWorkbenchConstants.TAG_X).intValue();
    r.y = boundsMem.getInteger(IWorkbenchConstants.TAG_Y).intValue();
    r.height = boundsMem.getInteger(IWorkbenchConstants.TAG_HEIGHT)
    .intValue();
    r.width = boundsMem.getInteger(IWorkbenchConstants.TAG_WIDTH)
    .intValue();
    StartupThreading.runWithoutExceptions(new StartupRunnable()
        {

          void runWithException() throws Throwable
          {
            if (page.getWorkbenchWindow().getPages().length == 0)
            {
              page.getWorkbenchWindow().getShell().setBounds(r);
            }
          }
        });

  }

  // Create an empty presentation..
  final PerspectiveHelper [] presArray = new PerspectiveHelper[1];
  StartupThreading.runWithoutExceptions(new StartupRunnable()
      {

        void runWithException() throws Throwable
        {
          ViewSashContainer mainLayout = new ViewSashContainer(page, getClientComposite());
          presArray[0] = new PerspectiveHelper(page, mainLayout, Perspective.this);
        }});
  final PerspectiveHelper pres = presArray[0];

  // Read the layout.
  result.merge(pres.restoreState(memento
          .getChild(IWorkbenchConstants.TAG_LAYOUT)));

  StartupThreading.runWithoutExceptions(new StartupRunnable()
      {

        void runWithException() throws Throwable
        {
          // Add the editor workbook. Do not hide it now.
          pres.replacePlaceholderWithPart(editorArea);
        }});

  // Add the visible views.
  IMemento[] views = memento.getChildren(IWorkbenchConstants.TAG_VIEW);

  for (int x = 0; x < views.length; x++)
  {
    // Get the view details.
    IMemento childMem = views[x];
    String id = childMem.getString(IWorkbenchConstants.TAG_ID);
    String secondaryId = ViewFactory.extractSecondaryId(id);
    if (secondaryId != null)
    {
      id = ViewFactory.extractPrimaryId(id);
    }

    // skip the intro as it is restored higher up in workbench.
    if (id.equals(IIntroConstants.INTRO_VIEW_ID))
    {
      continue;
    }

    // Create and open the view.
    IViewReference viewRef = viewFactory.getView(id, secondaryId);
    WorkbenchPartReference ref = (WorkbenchPartReference) viewRef;

    // report error
    if (ref == null)
    {
      String key = ViewFactory.getKey(id, secondaryId);
      result.add(new Status(IStatus.ERROR, PlatformUI.PLUGIN_ID, 0,
              NLS.bind(WorkbenchMessages.Perspective_couldNotFind, key ), null));
      continue;
    }
    bool willPartBeVisible = pres.willPartBeVisible(ref.getId(),
        secondaryId);
    if (willPartBeVisible)
    {
      IViewPart view = (IViewPart) ref.getPart(true);
      if (view != null)
      {
        ViewSite site = (ViewSite) view.getSite();
        ViewPane pane = (ViewPane) site.getPane();
        pres.replacePlaceholderWithPart(pane);
      }
    }
    else
    {
      pres.replacePlaceholderWithPart(ref.getPane());
    }
  }

  // Load the fast views
  if (fastViewManager != null)
  fastViewManager.restoreState(memento, result);

  // Load the view layout recs
  IMemento[] recMementos = memento
  .getChildren(IWorkbenchConstants.TAG_VIEW_LAYOUT_REC);
  for (int i = 0; i < recMementos.length; i++)
  {
    IMemento recMemento = recMementos[i];
    String compoundId = recMemento
    .getString(IWorkbenchConstants.TAG_ID);
    if (compoundId != null)
    {
      ViewLayoutRec rec = getViewLayoutRec(compoundId, true);
      if (IWorkbenchConstants.FALSE.equals(recMemento
              .getString(IWorkbenchConstants.TAG_CLOSEABLE)))
      {
        rec.isCloseable = false;
      }
      if (IWorkbenchConstants.FALSE.equals(recMemento
              .getString(IWorkbenchConstants.TAG_MOVEABLE)))
      {
        rec.isMoveable = false;
      }
      if (IWorkbenchConstants.TRUE.equals(recMemento
              .getString(IWorkbenchConstants.TAG_STANDALONE)))
      {
        rec.isStandalone = true;
        rec.showTitle = !IWorkbenchConstants.FALSE
        .equals(recMemento
            .getString(IWorkbenchConstants.TAG_SHOW_TITLE));
      }
    }
  }

  final IContextService service = (IContextService)page.getWorkbenchWindow().getService(IContextService.class);
  try
  { // one big try block, don't kill me here
    // defer context events
    if (service != null)
    {
      service.activateContext(ContextAuthority.DEFER_EVENTS);
    }

    HashSet knownActionSetIds = new HashSet();

    // Load the always on action sets.
    IMemento[] actions = memento
    .getChildren(IWorkbenchConstants.TAG_ALWAYS_ON_ACTION_SET);
    for (int x = 0; x < actions.length; x++)
    {
      String actionSetID = actions[x]
      .getString(IWorkbenchConstants.TAG_ID);
      final IActionSetDescriptor d = WorkbenchPlugin.getDefault()
      .getActionSetRegistry().findActionSet(actionSetID);
      if (d != null)
      {
        StartupThreading
        .runWithoutExceptions(new StartupRunnable()
            {
              void runWithException() throws Throwable
              {
                addAlwaysOn(d);
              }
            });

        knownActionSetIds.add(actionSetID);
      }
    }

    // Load the always off action sets.
    actions = memento
    .getChildren(IWorkbenchConstants.TAG_ALWAYS_OFF_ACTION_SET);
    for (int x = 0; x < actions.length; x++)
    {
      String actionSetID = actions[x]
      .getString(IWorkbenchConstants.TAG_ID);
      final IActionSetDescriptor d = WorkbenchPlugin.getDefault()
      .getActionSetRegistry().findActionSet(actionSetID);
      if (d != null)
      {
        StartupThreading
        .runWithoutExceptions(new StartupRunnable()
            {
              void runWithException() throws Throwable
              {
                addAlwaysOff(d);
              }
            });
        knownActionSetIds.add(actionSetID);
      }
    }

    // Load "show view actions".
    actions = memento
    .getChildren(IWorkbenchConstants.TAG_SHOW_VIEW_ACTION);
    showViewShortcuts = new ArrayList(actions.length);
    for (int x = 0; x < actions.length; x++)
    {
      String id = actions[x].getString(IWorkbenchConstants.TAG_ID);
      showViewShortcuts.add(id);
    }

    // Load "show in times".
    actions = memento.getChildren(IWorkbenchConstants.TAG_SHOW_IN_TIME);
    for (int x = 0; x < actions.length; x++)
    {
      String id = actions[x].getString(IWorkbenchConstants.TAG_ID);
      String timeStr = actions[x]
      .getString(IWorkbenchConstants.TAG_TIME);
      if (id != null && timeStr != null)
      {
        try
        {
          long time = Long.parseLong(timeStr);
          showInTimes.put(id, new Long(time));
        }
        catch (NumberFormatException e)
        {
          // skip this one
        }
      }
    }

    // Load "show in parts" from registry, not memento
    showInPartIds = getShowInIdsFromRegistry();

    // Load "new wizard actions".
    actions = memento
    .getChildren(IWorkbenchConstants.TAG_NEW_WIZARD_ACTION);
    newWizardShortcuts = new ArrayList(actions.length);
    for (int x = 0; x < actions.length; x++)
    {
      String id = actions[x].getString(IWorkbenchConstants.TAG_ID);
      newWizardShortcuts.add(id);
    }

    // Load "perspective actions".
    actions = memento
    .getChildren(IWorkbenchConstants.TAG_PERSPECTIVE_ACTION);
    perspectiveShortcuts = new ArrayList(actions.length);
    for (int x = 0; x < actions.length; x++)
    {
      String id = actions[x].getString(IWorkbenchConstants.TAG_ID);
      perspectiveShortcuts.add(id);
    }

    ArrayList extActionSets = getPerspectiveExtensionActionSets();
    for (int i = 0; i < extActionSets.size(); i++)
    {
      String actionSetID = (String) extActionSets.get(i);
      if (knownActionSetIds.contains(actionSetID))
      {
        continue;
      }
      final IActionSetDescriptor d = WorkbenchPlugin.getDefault()
      .getActionSetRegistry().findActionSet(actionSetID);
      if (d != null)
      {
        StartupThreading
        .runWithoutExceptions(new StartupRunnable()
            {
              void runWithException() throws Throwable
              {
                addAlwaysOn(d);
              }
            });
        knownActionSetIds.add(d.getId());
      }
    }

    // Add the visible set of action sets to our knownActionSetIds
    // Now go through the registry to ensure we pick up any new action
    // sets
    // that have been added but not yet considered by this perspective.
    ActionSetRegistry reg = WorkbenchPlugin.getDefault()
    .getActionSetRegistry();
    IActionSetDescriptor[] array = reg.getActionSets();
    int count = array.length;
    for (int i = 0; i < count; i++)
    {
      IActionSetDescriptor desc = array[i];
      if ((!knownActionSetIds.contains(desc.getId()))
          && (desc.isInitiallyVisible()))
      {
        addActionSet(desc);
      }
    }
  }finally
  {
    // restart context changes
    if (service != null)
    {
      StartupThreading.runWithoutExceptions(new StartupRunnable()
          {
            void runWithException() throws Throwable
            {
              service.activateContext(ContextAuthority.SEND_EVENTS);
            }
          });
    }
  }

  // Save presentation.
  presentation = pres;

  // Hide the editor area if needed. Need to wait for the
  // presentation to be fully setup first.
  Integer areaVisible = memento
  .getInteger(IWorkbenchConstants.TAG_AREA_VISIBLE);
  // Rather than hiding the editors now we must wait until after their
  // controls
  // are created. This ensures that if an editor is instantiated,
  // createPartControl
  // is also called. See bug 20166.
  shouldHideEditorsOnActivate = (areaVisible != null && areaVisible
      .intValue() == 0);

  // Restore the trim state of the editor area
  IPreferenceStore preferenceStore = PrefUtil.getAPIPreferenceStore();
  bool useNewMinMax = preferenceStore.getbool(IWorkbenchPreferenceConstants.ENABLE_NEW_MIN_MAX);
  if (useNewMinMax)
  {
    Integer trimStateInt = memento.getInteger(IWorkbenchConstants.TAG_AREA_TRIM_STATE);
    if (trimStateInt != null)
    {
      editorAreaState = trimStateInt.intValue() & 0x3; // low order two bits contain the state
      editorAreaRestoreOnUnzoom = (trimStateInt.intValue() & 4) != 0;
    }
  }

  // restore the fixed state
  Integer isFixed = memento.getInteger(IWorkbenchConstants.TAG_FIXED);
  fixed = (isFixed != null && isFixed.intValue() == 1);

  return result;
}

std::vector<std::string> Perspective::GetShowInIdsFromRegistry()
{
  PerspectiveExtensionReader reader = new PerspectiveExtensionReader();
  reader
  .setIncludeOnlyTags(new String[]
      { IWorkbenchRegistryConstants.TAG_SHOW_IN_PART});
  PageLayout layout = new PageLayout();
  reader.extendLayout(null, descriptor.getOriginalId(), layout);
  return layout.getShowInPartIds();
}

void Perspective::SaveDesc()
{
  saveDescAs(descriptor);
}

void Perspective::SaveDescAs(IPerspectiveDescriptor::Pointer desc)
{
  PerspectiveDescriptor realDesc = (PerspectiveDescriptor) desc;
  //get the layout from the registry  
  PerspectiveRegistry perspRegistry = (PerspectiveRegistry) WorkbenchPlugin
  .getDefault().getPerspectiveRegistry();
  // Capture the layout state.  
  XMLMemento memento = XMLMemento.createWriteRoot("perspective");//$NON-NLS-1$
  IStatus status = saveState(memento, realDesc, false);
  if (status.getSeverity() == IStatus.ERROR)
  {
    ErrorDialog.openError((Shell) null, WorkbenchMessages.Perspective_problemSavingTitle,
        WorkbenchMessages.Perspective_problemSavingMessage,
        status);
    return;
  }
  //save it to the preference store
  try
  {
    perspRegistry.saveCustomPersp(realDesc, memento);
    descriptor = realDesc;
  }
  catch (IOException e)
  {
    perspRegistry.deletePerspective(realDesc);
    MessageDialog.openError((Shell) null, WorkbenchMessages.Perspective_problemSavingTitle,
        WorkbenchMessages.Perspective_problemSavingMessage);
  }
}

bool Perspective::SaveState(IMemento::Pointer memento)
{
  MultiStatus result = new MultiStatus(
      PlatformUI.PLUGIN_ID,
      IStatus.OK,
      WorkbenchMessages.Perspective_problemsSavingPerspective, null);

  result.merge(saveState(memento, descriptor, true));

  return result;
}

bool Perspective::SaveState(IMemento::Pointer memento, PerspectiveDescriptor::Pointer p,
    bool saveInnerViewState)
{
  MultiStatus result = new MultiStatus(
      PlatformUI.PLUGIN_ID,
      IStatus.OK,
      WorkbenchMessages.Perspective_problemsSavingPerspective, null);

  if (this.memento != null)
  {
    memento.putMemento(this.memento);
    return result;
  }

  // Save the version number.
  memento.putString(IWorkbenchConstants.TAG_VERSION, VERSION_STRING);
  result.add(p.saveState(memento));
  if (!saveInnerViewState)
  {
    Rectangle bounds = page.getWorkbenchWindow().getShell().getBounds();
    IMemento boundsMem = memento
    .createChild(IWorkbenchConstants.TAG_WINDOW);
    boundsMem.putInteger(IWorkbenchConstants.TAG_X, bounds.x);
    boundsMem.putInteger(IWorkbenchConstants.TAG_Y, bounds.y);
    boundsMem.putInteger(IWorkbenchConstants.TAG_HEIGHT, bounds.height);
    boundsMem.putInteger(IWorkbenchConstants.TAG_WIDTH, bounds.width);
  }

  // Save the "always on" action sets.
  Iterator itr = alwaysOnActionSets.iterator();
  while (itr.hasNext())
  {
    IActionSetDescriptor desc = (IActionSetDescriptor) itr.next();
    IMemento child = memento
    .createChild(IWorkbenchConstants.TAG_ALWAYS_ON_ACTION_SET);
    child.putString(IWorkbenchConstants.TAG_ID, desc.getId());
  }

  // Save the "always off" action sets.
  itr = alwaysOffActionSets.iterator();
  while (itr.hasNext())
  {
    IActionSetDescriptor desc = (IActionSetDescriptor) itr.next();
    IMemento child = memento
    .createChild(IWorkbenchConstants.TAG_ALWAYS_OFF_ACTION_SET);
    child.putString(IWorkbenchConstants.TAG_ID, desc.getId());
  }

  // Save "show view actions"
  itr = showViewShortcuts.iterator();
  while (itr.hasNext())
  {
    String str = (String) itr.next();
    IMemento child = memento
    .createChild(IWorkbenchConstants.TAG_SHOW_VIEW_ACTION);
    child.putString(IWorkbenchConstants.TAG_ID, str);
  }

  // Save "show in times"
  itr = showInTimes.keySet().iterator();
  while (itr.hasNext())
  {
    String id = (String) itr.next();
    Long time = (Long) showInTimes.get(id);
    IMemento child = memento
    .createChild(IWorkbenchConstants.TAG_SHOW_IN_TIME);
    child.putString(IWorkbenchConstants.TAG_ID, id);
    child.putString(IWorkbenchConstants.TAG_TIME, time.toString());
  }

  // Save "new wizard actions".
  itr = newWizardShortcuts.iterator();
  while (itr.hasNext())
  {
    String str = (String) itr.next();
    IMemento child = memento
    .createChild(IWorkbenchConstants.TAG_NEW_WIZARD_ACTION);
    child.putString(IWorkbenchConstants.TAG_ID, str);
  }

  // Save "perspective actions".
  itr = perspectiveShortcuts.iterator();
  while (itr.hasNext())
  {
    String str = (String) itr.next();
    IMemento child = memento
    .createChild(IWorkbenchConstants.TAG_PERSPECTIVE_ACTION);
    child.putString(IWorkbenchConstants.TAG_ID, str);
  }

  // Get visible views.
  List viewPanes = new ArrayList(5);
  presentation.collectViewPanes(viewPanes);

  // Save the views.
  itr = viewPanes.iterator();
  int errors = 0;
  while (itr.hasNext())
  {
    ViewPane pane = (ViewPane) itr.next();
    IViewReference ref = pane.getViewReference();
    bool restorable = page.getViewFactory().getViewRegistry().find(
        ref.getId()).isRestorable();
    if(restorable)
    {
      IMemento viewMemento = memento
      .createChild(IWorkbenchConstants.TAG_VIEW);
      viewMemento.putString(IWorkbenchConstants.TAG_ID, ViewFactory
          .getKey(ref));
    }
  }

  // save all fastview state
  if (fastViewManager != null)
  fastViewManager.saveState(memento);

  // Save the view layout recs.
  for (Iterator i = mapIDtoViewLayoutRec.keySet().iterator(); i.hasNext();)
  {
    String compoundId = (String) i.next();
    ViewLayoutRec rec = (ViewLayoutRec) mapIDtoViewLayoutRec
    .get(compoundId);
    if (rec != null
        && (!rec.isCloseable || !rec.isMoveable || rec.isStandalone))
    {
      IMemento layoutMemento = memento
      .createChild(IWorkbenchConstants.TAG_VIEW_LAYOUT_REC);
      layoutMemento.putString(IWorkbenchConstants.TAG_ID, compoundId);
      if (!rec.isCloseable)
      {
        layoutMemento.putString(IWorkbenchConstants.TAG_CLOSEABLE,
            IWorkbenchConstants.FALSE);
      }
      if (!rec.isMoveable)
      {
        layoutMemento.putString(IWorkbenchConstants.TAG_MOVEABLE,
            IWorkbenchConstants.FALSE);
      }
      if (rec.isStandalone)
      {
        layoutMemento.putString(IWorkbenchConstants.TAG_STANDALONE,
            IWorkbenchConstants.TRUE);
        layoutMemento.putString(IWorkbenchConstants.TAG_SHOW_TITLE,
            String.valueOf(rec.showTitle));
      }
    }
  }

  if (errors> 0)
  {
    String message = WorkbenchMessages.Perspective_multipleErrors;
    if (errors == 1)
    {
      message = WorkbenchMessages.Perspective_oneError;
    }
    MessageDialog.openError(null,
        WorkbenchMessages.Error, message);
  }

  // Save the layout.
  IMemento childMem = memento.createChild(IWorkbenchConstants.TAG_LAYOUT);
  result.add(presentation.saveState(childMem));

  // Save the editor visibility state
  if (isEditorAreaVisible())
  {
    memento.putInteger(IWorkbenchConstants.TAG_AREA_VISIBLE, 1);
  }
  else
  {
    memento.putInteger(IWorkbenchConstants.TAG_AREA_VISIBLE, 0);
  }

  // Save the trim state of the editor area if using the new min/max
  IPreferenceStore preferenceStore = PrefUtil.getAPIPreferenceStore();
  bool useNewMinMax = preferenceStore.getbool(IWorkbenchPreferenceConstants.ENABLE_NEW_MIN_MAX);
  if (useNewMinMax)
  {
    int trimState = editorAreaState;
    trimState |= editorAreaRestoreOnUnzoom ? 4 : 0;
    memento.putInteger(IWorkbenchConstants.TAG_AREA_TRIM_STATE, trimState);
  }

  // Save the fixed state
  if (fixed)
  {
    memento.putInteger(IWorkbenchConstants.TAG_FIXED, 1);
  }
  else
  {
    memento.putInteger(IWorkbenchConstants.TAG_FIXED, 0);
  }

  return result;
}

void Perspective::SetPerspectiveActionIds(const std::vector<std::string>& list)
{
  perspectiveShortcuts = list;
}

void Perspective::SetShowInPartIds(const std::vector<std::string>& list)
{
  showInPartIds = list;
}

void Perspective::SetShowViewActionIds(const std::vector<std::string>& list)
{
  showViewShortcuts = list;
}

void Perspective::ShowEditorArea()
{
  if (isEditorAreaVisible())
  {
    return;
  }

  editorHidden = false;

  // Show the editor in the appropriate location
  if (useNewMinMax(this))
  {
    bool isMinimized = editorAreaState == IStackPresentationSite.STATE_MINIMIZED;
    if (!isMinimized)
    {
      // If the editor area is going to show then we have to restore
      if (getPresentation().getMaximizedStack() != null)
      getPresentation().getMaximizedStack().setState(IStackPresentationSite.STATE_RESTORED);

      showEditorAreaLocal();
    }
    else
    setEditorAreaTrimVisibility(true);
  }
  else
  {
    showEditorAreaLocal();
  }
}

void Perspective::ShowEditorAreaLocal()
{
  if (editorHolder == null || editorHidden)
  {
    return;
  }

  // Replace the part holder with the editor area.
  presentation.getLayout().replace(editorHolder, editorArea);
  editorHolder = null;
}

void Perspective::SetEditorAreaState(int newState)
{
  if (newState == editorAreaState)
  return;

  editorAreaState = newState;

  // reset the restore flag if we're not minimized
  if (newState != IStackPresentationSite.STATE_MINIMIZED)
  editorAreaRestoreOnUnzoom = false;

  refreshEditorAreaVisibility();
}

int Perspective::GetEditorAreaState()
{
  return editorAreaState;
}

void Perspective::RefreshEditorAreaVisibility()
{
  // Nothing shows up if the editor area isn't visible at all
  if (editorHidden)
  {
    hideEditorAreaLocal();
    setEditorAreaTrimVisibility(false);
    return;
  }

  EditorStack editorStack = ((EditorSashContainer) editorArea).getUpperRightEditorStack(null);
  if (editorStack == null)
  return;

  // Whatever we're doing, make the current editor stack match it
  editorStack.setStateLocal(editorAreaState);

  // If it's minimized then it's in the trim
  if (editorAreaState == IStackPresentationSite.STATE_MINIMIZED)
  {
    // Hide the editor area and show its trim 
    hideEditorAreaLocal();
    setEditorAreaTrimVisibility(true);
  }
  else
  {
    // Show the editor area and hide its trim 
    setEditorAreaTrimVisibility(false);
    showEditorAreaLocal();

    if (editorAreaState == IStackPresentationSite.STATE_MAXIMIZED)
    getPresentation().setMaximizedStack(editorStack);
  }
}

IViewReference::Pointer Perspective::GetViewReference(const std::string& viewId, const std::string& secondaryId)
{
  IViewReference ref = page.findViewReference(viewId, secondaryId);
  if (ref == null)
  {
    ViewFactory factory = getViewFactory();
    try
    {
      ref = factory.createView(viewId, secondaryId);
    }
    catch (PartInitException e)
    {
      IStatus status = StatusUtil.newStatus(IStatus.ERROR,
          e.getMessage() == null ? "" : e.getMessage(), //$NON-NLS-1$
          e);
      StatusUtil.handleStatus(status, "Failed to create view: id=" + viewId, //$NON-NLS-1$
          StatusManager.LOG);
    }
  }
  return ref;
}

IViewPart::Pointer Perspective::ShowView(const std::string& viewId, const std::string& secondaryId)
{
  ViewFactory factory = getViewFactory();
  IViewReference ref = factory.createView(viewId, secondaryId);
  IViewPart part = (IViewPart) ref.getPart(true);
  if (part == null)
  {
    throw new PartInitException(NLS.bind(WorkbenchMessages.ViewFactory_couldNotCreate, ref.getId()));
  }
  ViewSite site = (ViewSite) part.getSite();
  ViewPane pane = (ViewPane) site.getPane();

  IPreferenceStore store = WorkbenchPlugin.getDefault()
  .getPreferenceStore();
  int openViewMode = store.getInt(IPreferenceConstants.OPEN_VIEW_MODE);

  if (openViewMode == IPreferenceConstants.OVM_FAST &&
      fastViewManager != null)
  {
    fastViewManager.addViewReference(FastViewBar.FASTVIEWBAR_ID, -1, ref, true);
    setActiveFastView(ref);
  }
  else if (openViewMode == IPreferenceConstants.OVM_FLOAT
      && presentation.canDetach())
  {
    presentation.addDetachedPart(pane);
  }
  else
  {
    if (useNewMinMax(this))
    {
      // Is this view going to show in the trim?
      LayoutPart vPart = presentation.findPart(viewId, secondaryId);

      // Determine if there is a trim stack that should get the view
      String trimId = null;

      // If we can locate the correct trim stack then do so
      if (vPart != null)
      {
        String id = null;
        ILayoutContainer container = vPart.getContainer();
        if (container instanceof ContainerPlaceholder)
        id = ((ContainerPlaceholder)container).getID();
        else if (container instanceof ViewStack)
        id = ((ViewStack)container).getID();

        // Is this place-holder in the trim?
        if (id != null && fastViewManager.getFastViews(id).size()> 0)
        {
          trimId = id;
        }
      }

      // No explicit trim found; If we're maximized then we either have to find an
      // arbitrary stack...
      if (trimId == null && presentation.getMaximizedStack() != null)
      {
        if (vPart == null)
        {
          ViewStackTrimToolBar blTrimStack = fastViewManager.getBottomRightTrimStack();
          if (blTrimStack != null)
          {
            // OK, we've found a trim stack to add it to...
            trimId = blTrimStack.getId();

            // Since there was no placeholder we have to add one
            LayoutPart blPart = presentation.findPart(trimId, null);
            if (blPart instanceof ContainerPlaceholder)
            {
              ContainerPlaceholder cph = (ContainerPlaceholder) blPart;
              if (cph.getRealContainer() instanceof ViewStack)
              {
                ViewStack vs = (ViewStack) cph.getRealContainer();

                // Create a 'compound' id if this is a multi-instance part
                String compoundId = ref.getId();
                if (ref.getSecondaryId() != null)
                compoundId = compoundId + ':' + ref.getSecondaryId();

                // Add the new placeholder
                vs.add(new PartPlaceholder(compoundId));
              }
            }
          }
        }
      }

      // If we have a trim stack located then add the view to it
      if (trimId != null)
      {
        fastViewManager.addViewReference(trimId, -1, ref, true);
      }
      else
      {
        bool inMaximizedStack = vPart != null && vPart.getContainer() == presentation.getMaximizedStack();

        // Do the default behavior
        presentation.addPart(pane);

        // Now, if we're maximized then we have to minimize the new stack
        if (presentation.getMaximizedStack() != null && !inMaximizedStack)
        {
          vPart = presentation.findPart(viewId, secondaryId);
          if (vPart != null && vPart.getContainer() instanceof ViewStack)
          {
            ViewStack vs = (ViewStack)vPart.getContainer();
            vs.setState(IStackPresentationSite.STATE_MINIMIZED);

            // setting the state to minimized will create the trim toolbar
            // so we don't need a null pointer check here...
            fastViewManager.getViewStackTrimToolbar(vs.getID()).setRestoreOnUnzoom(true);
          }
        }
      }
    }
    else
    {
      presentation.addPart(pane);
    }
  }

  // Ensure that the newly showing part is enabled
  if (pane != null && pane.getControl() != null)
  pane.getControl().setEnabled(true);

  return part;
}

IWorkbenchPartReference::Pointer Perspective::GetOldPartRef()
{
  return oldPartRef;
}

void Perspective::SetOldPartRef(IWorkbenchPartReference::Pointer oldPartRef)
{
  this.oldPartRef = oldPartRef;
}

bool Perspective::IsCloseable(IViewReference::Pointer reference)
{
  ViewLayoutRec rec = getViewLayoutRec(reference, false);
  if (rec != null)
  {
    return rec.isCloseable;
  }
  return true;
}

bool Perspective::IsMoveable(IViewReference::Pointer reference)
{
  ViewLayoutRec rec = getViewLayoutRec(reference, false);
  if (rec != null)
  {
    return rec.isMoveable;
  }
  return true;
}

void Perspective::DescribeLayout(StringBuffer buf)
{
  IViewReference[] fastViews = getFastViews();

  if (fastViews.length != 0)
  {
    buf.append("fastviews ("); //$NON-NLS-1$
    for (int idx = 0; idx < fastViews.length; idx++)
    {
      IViewReference ref = fastViews[idx];

      if (idx> 0)
      {
        buf.append(", "); //$NON-NLS-1$
      }

      buf.append(ref.getPartName());
    }
    buf.append("), "); //$NON-NLS-1$
  }

  getPresentation().describeLayout(buf);
}

void Perspective::TestInvariants()
{
  getPresentation().getLayout().testInvariants();
}

static bool Perspective::UseNewMinMax(Perspective::Pointer activePerspective)
{
  // We need to have an active perspective
  if (activePerspective == null)
  return false;

  // We need to have a trim manager (if we don't then we
  // don't create a FastViewManager because it'd be useless)
  if (activePerspective.getFastViewManager() == null)
  return false;

  // Make sure we don't NPE anyplace
  WorkbenchWindow wbw = (WorkbenchWindow) activePerspective.page.getWorkbenchWindow();
  if (wbw == null)
  return false;

  WorkbenchWindowConfigurer configurer = wbw.getWindowConfigurer();
  if (configurer == null)
  return false;

  AbstractPresentationFactory factory = configurer.getPresentationFactory();
  if (factory == null)
  return false;

  // Ok, we should be good to go, return the pref
  IPreferenceStore preferenceStore = PrefUtil.getAPIPreferenceStore();
  bool useNewMinMax = preferenceStore.getbool(IWorkbenchPreferenceConstants.ENABLE_NEW_MIN_MAX);
  return useNewMinMax;
}

}
