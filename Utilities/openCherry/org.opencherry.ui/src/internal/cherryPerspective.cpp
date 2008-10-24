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

#include "cherryPerspectiveHelper.h"
#include "cherryWorkbenchPlugin.h"
#include "cherryPerspectiveExtensionReader.h"
#include "cherryEditorSashContainer.h"
#include "cherryPartSite.h"
#include "cherryEditorAreaHelper.h"
#include "../cherryWorkbenchWindow.h"

#include "../tweaklets/cherryGuiWidgetsTweaklet.h"

#include "../presentations/cherryIStackPresentationSite.h"

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
  editorAreaState = IStackPresentationSite::STATE_RESTORED;
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
  return (view.Cast<IWorkbenchPart>() == ref->GetPart(false));
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

void* Perspective::GetClientComposite()
{
  return page->GetClientComposite();
}

IPerspectiveDescriptor::Pointer Perspective::GetDesc()
{
  return descriptor;
}

PartPane::Pointer Perspective::GetPane(IViewReference::Pointer ref)
{
  return ref.Cast<WorkbenchPartReference>()->GetPane();
}

std::vector<std::string> Perspective::GetPerspectiveShortcuts()
{
  return perspectiveShortcuts;
}

PerspectiveHelper* Perspective::GetPresentation() const
{
  return presentation;
}

std::vector<std::string> Perspective::GetShowViewShortcuts()
{
  return showViewShortcuts;
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

  std::vector<PartPane::Pointer> panes;
  presentation->CollectViewPanes(panes);

  std::vector<IViewReference::Pointer> result;
//  List fastViews = (fastViewManager != 0) ?
//  fastViewManager.getFastViews(0)
//  : new ArrayList();
//  IViewReference[] resultArray = new IViewReference[panes.size()
//  + fastViews.size()];
//
//  // Copy fast views.
//  int nView = 0;
//  for (int i = 0; i < fastViews.size(); i++)
//  {
//    resultArray[nView] = (IViewReference) fastViews.get(i);
//    ++nView;
//  }

  // Copy normal views.
  for (std::vector<PartPane::Pointer>::iterator iter = panes.begin();
       iter != panes.end(); ++iter)
  {
    PartPane::Pointer pane = *iter;
    result.push_back(pane->GetPartReference().Cast<IViewReference>());
  }

  return result;
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
//    if (this->GetPresentation().getMaximizedStack().Cast<EditorStack>() != 0)
//    {
//      getPresentation().getMaximizedStack().setState(IStackPresentationSite.STATE_RESTORED);
//    }

    bool isMinimized = editorAreaState == IStackPresentationSite::STATE_MINIMIZED;
    if (!isMinimized)
      this->HideEditorAreaLocal();
    //else
    //  this->SetEditorAreaTrimVisibility(false);
  }
  else
  {
    this->HideEditorAreaLocal();
  }

  editorHidden = true;
}

void Perspective::HideEditorAreaLocal()
{
  if (editorHolder != 0)
  {
    return;
  }

  // Replace the editor area with a placeholder so we
  // know where to put it back on show editor area request.
  editorHolder = new ContainerPlaceholder(editorArea->GetID());
  presentation->GetLayout()->Replace(editorArea, editorHolder);
}

bool Perspective::HideView(IViewReference::Pointer ref)
{
  // If the view is locked just return.
  PartPane::Pointer pane = this->GetPane(ref);

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
  PerspectiveRegistry* perspRegistry = dynamic_cast<PerspectiveRegistry*>(WorkbenchPlugin::GetDefault()->GetPerspectiveRegistry());
  try
  {
    IMemento::Pointer memento = perspRegistry->GetCustomPersp(persp->GetId());
    // Restore the layout state.
//    MultiStatus status = new MultiStatus(
//        PlatformUI.PLUGIN_ID,
//        IStatus.OK,
//        NLS.bind(WorkbenchMessages.Perspective_unableToRestorePerspective, persp.getLabel()),
//        0);
//    status.merge(restoreState(memento));
//    status.merge(restoreState());
    bool okay = true;
    okay &= this->RestoreState(memento);
    okay &= this->RestoreState();
    if (!okay)
    {
      this->UnableToOpenPerspective(persp, "Unable to open perspective: " + persp->GetLabel());
    }
  }
  //catch (IOException e)
  //{
  //  unableToOpenPerspective(persp, 0);
  //}
  catch (WorkbenchException& e)
  {
    this->UnableToOpenPerspective(persp, e.displayText());
  }
}

void Perspective::UnableToOpenPerspective(PerspectiveDescriptor::Pointer persp,
    const std::string& status)
{
  PerspectiveRegistry* perspRegistry = dynamic_cast<PerspectiveRegistry*>(WorkbenchPlugin
  ::GetDefault()->GetPerspectiveRegistry());
  perspRegistry->DeletePerspective(persp);
  // If this is a predefined perspective, we will not be able to delete
  // the perspective (we wouldn't want to).  But make sure to delete the
  // customized portion.
  persp->DeleteCustomDefinition();
  std::string title = "Restoring problems";
  std::string msg = "Unable to read workbench state";
  if (status == "")
  {
    //TODO message dialog
    //MessageDialog.openError((Shell) 0, title, msg);
    WorkbenchPlugin::Log(title + ": " + msg);
  }
  else
  {
    //TODO message dialog
    //ErrorDialog.openError((Shell) 0, title, msg, status);
    WorkbenchPlugin::Log(status);
  }
}

void Perspective::LoadPredefinedPersp(PerspectiveDescriptor::Pointer persp)
{
  // Create layout engine.
  IPerspectiveFactory::Pointer factory;
  try
  {
    factory = persp->CreateFactory();
  }
  catch (CoreException& e)
  {
    throw WorkbenchException("Unable to load perspective: " + persp->GetId());
  }

  /*
   * IPerspectiveFactory#createFactory() can return 0
   */
  if (factory == 0)
  {
    throw WorkbenchException("Unable to load perspective: " + persp->GetId());
  }

  // Create layout factory.
  ViewSashContainer::Pointer container = new ViewSashContainer(page, this->GetClientComposite());
  layout = new PageLayout(container, this->GetViewFactory(),
      editorArea, descriptor);
  layout->SetFixed(descriptor->GetFixed());

//  // add the placeholders for the sticky folders and their contents
//  IPlaceholderFolderLayout::Pointer stickyFolderRight, stickyFolderLeft, stickyFolderTop, stickyFolderBottom;
//
//  std::vector<IStickyViewDescriptor::Pointer> descs = WorkbenchPlugin.getDefault()
//  .getViewRegistry().getStickyViews();
//  for (int i = 0; i < descs.length; i++)
//  {
//    IStickyViewDescriptor stickyViewDescriptor = descs[i];
//    String id = stickyViewDescriptor.getId();
//    switch (stickyViewDescriptor.getLocation())
//    {
//      case IPageLayout.RIGHT:
//      if (stickyFolderRight == 0)
//      {
//        stickyFolderRight = layout
//        .createPlaceholderFolder(
//            StickyViewDescriptor.STICKY_FOLDER_RIGHT,
//            IPageLayout.RIGHT, .75f,
//            IPageLayout.ID_EDITOR_AREA);
//      }
//      stickyFolderRight.addPlaceholder(id);
//      break;
//      case IPageLayout.LEFT:
//      if (stickyFolderLeft == 0)
//      {
//        stickyFolderLeft = layout.createPlaceholderFolder(
//            StickyViewDescriptor.STICKY_FOLDER_LEFT,
//            IPageLayout.LEFT, .25f, IPageLayout.ID_EDITOR_AREA);
//      }
//      stickyFolderLeft.addPlaceholder(id);
//      break;
//      case IPageLayout.TOP:
//      if (stickyFolderTop == 0)
//      {
//        stickyFolderTop = layout.createPlaceholderFolder(
//            StickyViewDescriptor.STICKY_FOLDER_TOP,
//            IPageLayout.TOP, .25f, IPageLayout.ID_EDITOR_AREA);
//      }
//      stickyFolderTop.addPlaceholder(id);
//      break;
//      case IPageLayout.BOTTOM:
//      if (stickyFolderBottom == 0)
//      {
//        stickyFolderBottom = layout.createPlaceholderFolder(
//            StickyViewDescriptor.STICKY_FOLDER_BOTTOM,
//            IPageLayout.BOTTOM, .75f,
//            IPageLayout.ID_EDITOR_AREA);
//      }
//      stickyFolderBottom.addPlaceholder(id);
//      break;
//    }
//
//    //should never be 0 as we've just added the view above
//    IViewLayout viewLayout = layout.getViewLayout(id);
//    viewLayout.setCloseable(stickyViewDescriptor.isCloseable());
//    viewLayout.setMoveable(stickyViewDescriptor.isMoveable());
//  }

  // Run layout engine.
  factory->CreateInitialLayout(layout);
  PerspectiveExtensionReader extender;
  extender.ExtendLayout(descriptor->GetId(), layout);

  // Retrieve view layout info stored in the page layout.
  std::map<std::string, ViewLayoutRec::Pointer> layoutInfo = layout->GetIDtoViewLayoutRecMap();
  mapIDtoViewLayoutRec.insert(layoutInfo.begin(), layoutInfo.end());

  //TODO Perspective action sets
  // Create action sets.
  //List temp = new ArrayList();
  //this->CreateInitialActionSets(temp, layout.getActionSets());

//  IContextService service = 0;
//  if (page != 0)
//  {
//    service = (IContextService) page.getWorkbenchWindow().getService(
//        IContextService.class);
//  }
//  try
//  {
//    if (service!=0)
//    {
//      service.activateContext(ContextAuthority.DEFER_EVENTS);
//    }
//    for (Iterator iter = temp.iterator(); iter.hasNext();)
//    {
//      IActionSetDescriptor descriptor = (IActionSetDescriptor) iter
//      .next();
//      addAlwaysOn(descriptor);
//    }
//  }finally
//  {
//    if (service!=0)
//    {
//      service.activateContext(ContextAuthority.SEND_EVENTS);
//    }
//  }

//  newWizardShortcuts = layout.getNewWizardShortcuts();
//  showViewShortcuts = layout.getShowViewShortcuts();
//  perspectiveShortcuts = layout.getPerspectiveShortcuts();
//  showInPartIds = layout.getShowInPartIds();
//
//  // Retrieve fast views
//  if (fastViewManager != 0)
//  {
//    ArrayList fastViews = layout.getFastViews();
//    for (Iterator fvIter = fastViews.iterator(); fvIter.hasNext();)
//    {
//      IViewReference ref = (IViewReference) fvIter.next();
//      fastViewManager.addViewReference(FastViewBar.FASTVIEWBAR_ID, -1, ref,
//          !fvIter.hasNext());
//    }
//  }

  // Is the layout fixed
  fixed = layout->IsFixed();

  // Create presentation.
  presentation = new PerspectiveHelper(page, container, this);

  // Hide editor area if requested by factory
  if (!layout->IsEditorAreaVisible())
  {
    this->HideEditorArea();
  }

}

void Perspective::OnActivate()
{
  // Update editor area state.
  if (editorArea->GetControl() != 0)
  {
    bool visible = this->IsEditorAreaVisible();
    bool inTrim = editorAreaState == IStackPresentationSite::STATE_MINIMIZED;

    editorArea->SetVisible(visible && !inTrim);
  }

//  // Update fast views.
//  // Make sure the control for the fastviews are created so they can
//  // be activated.
//  if (fastViewManager != 0)
//  {
//    List fastViews = fastViewManager.getFastViews(0);
//    for (int i = 0; i < fastViews.size(); i++)
//    {
//      ViewPane pane = getPane((IViewReference) fastViews.get(i));
//      if (pane != 0)
//      {
//        Control ctrl = pane.getControl();
//        if (ctrl == 0)
//        {
//          pane.createControl(getClientComposite());
//          ctrl = pane.getControl();
//        }
//        ctrl.setEnabled(false); // Remove focus support.
//      }
//    }
//  }

//  // Set the visibility of all fast view pins
//  setAllPinsVisible(true);

  // Trim Stack Support
  bool useNewMinMax = Perspective::UseNewMinMax(this);
  bool hideEditorArea = shouldHideEditorsOnActivate || (editorHidden && editorHolder == 0);

  // We have to set the editor area's stack state -before-
  // activating the presentation since it's used there to determine
  // size of the resulting stack
  if (useNewMinMax && !hideEditorArea)
  {
    this->RefreshEditorAreaVisibility();
  }

  // Show the layout
  presentation->Activate(this->GetClientComposite());

//  if (useNewMinMax)
//  {
//    fastViewManager.activate();
//
//    // Move any minimized extension stacks to the trim
//    if (layout != 0)
//    {
//      // Turn aimations off
//      IPreferenceStore preferenceStore = PrefUtil.getAPIPreferenceStore();
//      bool useAnimations = preferenceStore
//      .getbool(IWorkbenchPreferenceConstants.ENABLE_ANIMATIONS);
//      preferenceStore.setValue(IWorkbenchPreferenceConstants.ENABLE_ANIMATIONS, false);
//
//      List minStacks = layout.getMinimizedStacks();
//      for (Iterator msIter = minStacks.iterator(); msIter.hasNext();)
//      {
//        ViewStack vs = (ViewStack) msIter.next();
//        vs.setMinimized(true);
//      }
//
//      // Restore the animation pref
//      preferenceStore.setValue(IWorkbenchPreferenceConstants.ENABLE_ANIMATIONS, useAnimations);
//
//      // this is a one-off deal...set during the extension reading
//      minStacks.clear();
//      layout = 0;
//    }
//  }
//  else
//  {
//    // Update the FVB only if not using the new min/max
//
//    //    WorkbenchWindow wbw = (WorkbenchWindow) page.getWorkbenchWindow();
////    if (wbw != 0)
////    {
////      ITrimManager tbm = wbw.getTrimManager();
////      if (tbm != 0)
////      {
////        IWindowTrim fvb = tbm.getTrim(FastViewBar.FASTVIEWBAR_ID);
////        if (fvb instanceof FastViewBar)
////        {
////          ((FastViewBar)fvb).update(true);
////        }
////      }
////    }
//  }

//  // If we are -not- using the new min/max then ensure that there
//  // are no stacks in the trim. This can happen when a user switches
//  // back to the 3.0 presentation...
//  if (!Perspective.useNewMinMax(this) && fastViewManager != 0)
//  {
//    bool stacksWereRestored = fastViewManager.restoreAllTrimStacks();
//    setEditorAreaTrimVisibility(false);
//
//    // Restore any 'maximized' view stack since we've restored
//    // the minimized stacks
//    if (stacksWereRestored && presentation.getMaximizedStack().Cast<ViewStack>() != 0)
//    {
//      ViewStack vs = (ViewStack) presentation.getMaximizedStack();
//      vs.setPresentationState(IStackPresentationSite.STATE_RESTORED);
//      presentation.setMaximizedStack(0);
//    }
//  }

  // We hide the editor area -after- the presentation activates
  if (hideEditorArea)
  {
    // We do this here to ensure that createPartControl is called on the
    // top editor
    // before it is hidden. See bug 20166.
    this->HideEditorArea();
    shouldHideEditorsOnActivate = false;

//    // this is an override so it should handle both states
//    if (useNewMinMax)
//    setEditorAreaTrimVisibility(editorAreaState == IStackPresentationSite.STATE_MINIMIZED);
  }
}

void Perspective::OnDeactivate()
{
  presentation->Deactivate();
  //setActiveFastView(0);
  //setAllPinsVisible(false);

//  // Update fast views.
//  if (fastViewManager != 0)
//  {
//    List fastViews = fastViewManager.getFastViews(0);
//    for (int i = 0; i < fastViews.size(); i++)
//    {
//      ViewPane pane = getPane((IViewReference) fastViews.get(i));
//      if (pane != 0)
//      {
//        Control ctrl = pane.getControl();
//        if (ctrl != 0)
//        {
//          ctrl.setEnabled(true); // Add focus support.
//        }
//      }
//    }
//
//    fastViewManager.deActivate();
//  }
//
//  // Ensure that the editor area trim is hidden as well
//  setEditorAreaTrimVisibility(false);
}

void Perspective::PartActivated(IWorkbenchPart::Pointer activePart)
{
//  // If a fastview is open close it.
//  if (activeFastView != 0
//      && activeFastView.getPart(false) != activePart)
//  {
//    setActiveFastView(0);
//  }
}

void Perspective::PerformedShowIn(const std::string& partId)
{
  //showInTimes.insert(std::make_pair(partId, new Long(System.currentTimeMillis())));
}

bool Perspective::RestoreState(IMemento::Pointer memento)
{
  //TODO Perspective restore state
//  MultiStatus result = new MultiStatus(
//      PlatformUI.PLUGIN_ID,
//      IStatus.OK,
//      WorkbenchMessages.Perspective_problemsRestoringPerspective, 0);
//
//  // Create persp descriptor.
//  descriptor = new PerspectiveDescriptor(0, 0, 0);
//  result.add(descriptor.restoreState(memento));
//  PerspectiveDescriptor desc = (PerspectiveDescriptor) WorkbenchPlugin
//  .getDefault().getPerspectiveRegistry().findPerspectiveWithId(
//      descriptor.getId());
//  if (desc != 0)
//  {
//    descriptor = desc;
//  }
//
//  this.memento = memento;
//  // Add the visible views.
//  IMemento views[] = memento.getChildren(IWorkbenchConstants.TAG_VIEW);
//  result.merge(createReferences(views));
//
//  memento = memento.getChild(IWorkbenchConstants.TAG_FAST_VIEWS);
//  if (memento != 0)
//  {
//    views = memento.getChildren(IWorkbenchConstants.TAG_VIEW);
//    result.merge(createReferences(views));
//  }
//  return result;
  return true;
}

bool Perspective::CreateReferences(std::vector<IMemento::Pointer> views)
{
  //TODO Perspective create references
//  MultiStatus result = new MultiStatus(PlatformUI.PLUGIN_ID, IStatus.OK,
//      WorkbenchMessages.Perspective_problemsRestoringViews, 0);
//
//  for (int x = 0; x < views.length; x++)
//  {
//    // Get the view details.
//    IMemento childMem = views[x];
//    String id = childMem.getString(IWorkbenchConstants.TAG_ID);
//    // skip creation of the intro reference -  it's handled elsewhere.
//    if (id.equals(IIntroConstants.INTRO_VIEW_ID))
//    {
//      continue;
//    }
//
//    String secondaryId = ViewFactory.extractSecondaryId(id);
//    if (secondaryId != 0)
//    {
//      id = ViewFactory.extractPrimaryId(id);
//    }
//    // Create and open the view.
//    try
//    {
//      if (!"true".equals(childMem.getString(IWorkbenchConstants.TAG_REMOVED)))
//      { //$NON-NLS-1$
//        viewFactory.createView(id, secondaryId);
//      }
//    }
//    catch (PartInitException e)
//    {
//      childMem.putString(IWorkbenchConstants.TAG_REMOVED, "true"); //$NON-NLS-1$
//      result.add(StatusUtil.newStatus(IStatus.ERR,
//              e.getMessage() == 0 ? "" : e.getMessage(), //$NON-NLS-1$
//              e));
//    }
//  }
//  return result;
  return true;
}

bool Perspective::RestoreState()
{
  //TODO Perspective restore state
//  if (this.memento == 0)
//  {
//    return new Status(IStatus.OK, PlatformUI.PLUGIN_ID, 0, "", 0); //$NON-NLS-1$
//  }
//
//  MultiStatus result = new MultiStatus(
//      PlatformUI.PLUGIN_ID,
//      IStatus.OK,
//      WorkbenchMessages.Perspective_problemsRestoringPerspective, 0);
//
//  IMemento memento = this.memento;
//  this.memento = 0;
//
//  const IMemento boundsMem = memento.getChild(IWorkbenchConstants.TAG_WINDOW);
//  if (boundsMem != 0)
//  {
//    Rectangle r = new Rectangle(0, 0, 0, 0);
//    r.x = boundsMem.getInteger(IWorkbenchConstants.TAG_X).intValue();
//    r.y = boundsMem.getInteger(IWorkbenchConstants.TAG_Y).intValue();
//    r.height = boundsMem.getInteger(IWorkbenchConstants.TAG_HEIGHT)
//    .intValue();
//    r.width = boundsMem.getInteger(IWorkbenchConstants.TAG_WIDTH)
//    .intValue();
//    //StartupThreading.runWithoutExceptions(new StartupRunnable()
//    //    {
//
//    //      void runWithException() throws Throwable
//    //      {
//            if (page.getWorkbenchWindow().getPages().length == 0)
//            {
//              page.getWorkbenchWindow().getShell().setBounds(r);
//            }
//    //      }
//    //    });
//
//  }
//
//  // Create an empty presentation..
//  PerspectiveHelper* pres;
//  //StartupThreading.runWithoutExceptions(new StartupRunnable()
//  //    {
//
//  //      void runWithException() throws Throwable
//  //      {
//          ViewSashContainer mainLayout = new ViewSashContainer(page, getClientComposite());
//          pres = new PerspectiveHelper(page, mainLayout, this);
//  //      }});
//
//
//  // Read the layout.
//  result.merge(pres.restoreState(memento
//          .getChild(IWorkbenchConstants.TAG_LAYOUT)));
//
//  //StartupThreading.runWithoutExceptions(new StartupRunnable()
//  //    {
//
//  //      void runWithException() throws Throwable
//  //      {
//          // Add the editor workbook. Do not hide it now.
//          pres.replacePlaceholderWithPart(editorArea);
//  //      }});
//
//  // Add the visible views.
//  std::vector<IMemento::Pointer> views = memento.getChildren(IWorkbenchConstants.TAG_VIEW);
//
//  for (int x = 0; x < views.length; x++)
//  {
//    // Get the view details.
//    IMemento childMem = views[x];
//    String id = childMem.getString(IWorkbenchConstants.TAG_ID);
//    String secondaryId = ViewFactory.extractSecondaryId(id);
//    if (secondaryId != 0)
//    {
//      id = ViewFactory.extractPrimaryId(id);
//    }
//
//    // skip the intro as it is restored higher up in workbench.
//    if (id.equals(IIntroConstants.INTRO_VIEW_ID))
//    {
//      continue;
//    }
//
//    // Create and open the view.
//    IViewReference viewRef = viewFactory.getView(id, secondaryId);
//    WorkbenchPartReference ref = (WorkbenchPartReference) viewRef;
//
//    // report error
//    if (ref == 0)
//    {
//      String key = ViewFactory.getKey(id, secondaryId);
//      result.add(new Status(IStatus.ERR, PlatformUI.PLUGIN_ID, 0,
//              NLS.bind(WorkbenchMessages.Perspective_couldNotFind, key ), 0));
//      continue;
//    }
//    bool willPartBeVisible = pres.willPartBeVisible(ref.getId(),
//        secondaryId);
//    if (willPartBeVisible)
//    {
//      IViewPart view = (IViewPart) ref.getPart(true);
//      if (view != 0)
//      {
//        ViewSite site = (ViewSite) view.getSite();
//        ViewPane pane = (ViewPane) site.getPane();
//        pres.replacePlaceholderWithPart(pane);
//      }
//    }
//    else
//    {
//      pres.replacePlaceholderWithPart(ref.getPane());
//    }
//  }
//
//  // Load the fast views
//  if (fastViewManager != 0)
//  fastViewManager.restoreState(memento, result);
//
//  // Load the view layout recs
//  std::vector<IMemento::Pointer> recMementos = memento
//  .getChildren(IWorkbenchConstants.TAG_VIEW_LAYOUT_REC);
//  for (int i = 0; i < recMementos.length; i++)
//  {
//    IMemento recMemento = recMementos[i];
//    String compoundId = recMemento
//    .getString(IWorkbenchConstants.TAG_ID);
//    if (compoundId != 0)
//    {
//      ViewLayoutRec rec = getViewLayoutRec(compoundId, true);
//      if (IWorkbenchConstants.FALSE.equals(recMemento
//              .getString(IWorkbenchConstants.TAG_CLOSEABLE)))
//      {
//        rec.isCloseable = false;
//      }
//      if (IWorkbenchConstants.FALSE.equals(recMemento
//              .getString(IWorkbenchConstants.TAG_MOVEABLE)))
//      {
//        rec.isMoveable = false;
//      }
//      if (IWorkbenchConstants.TRUE.equals(recMemento
//              .getString(IWorkbenchConstants.TAG_STANDALONE)))
//      {
//        rec.isStandalone = true;
//        rec.showTitle = !IWorkbenchConstants.FALSE
//        .equals(recMemento
//            .getString(IWorkbenchConstants.TAG_SHOW_TITLE));
//      }
//    }
//  }
//
//  //final IContextService service = (IContextService)page.getWorkbenchWindow().getService(IContextService.class);
//  try
//  { // one big try block, don't kill me here
////    // defer context events
////    if (service != 0)
////    {
////      service.activateContext(ContextAuthority.DEFER_EVENTS);
////    }
////
////    HashSet knownActionSetIds = new HashSet();
////
////    // Load the always on action sets.
////    IMemento[] actions = memento
////    .getChildren(IWorkbenchConstants.TAG_ALWAYS_ON_ACTION_SET);
////    for (int x = 0; x < actions.length; x++)
////    {
////      String actionSetID = actions[x]
////      .getString(IWorkbenchConstants.TAG_ID);
////      final IActionSetDescriptor d = WorkbenchPlugin.getDefault()
////      .getActionSetRegistry().findActionSet(actionSetID);
////      if (d != 0)
////      {
////        StartupThreading
////        .runWithoutExceptions(new StartupRunnable()
////            {
////              void runWithException() throws Throwable
////              {
////                addAlwaysOn(d);
////              }
////            });
////
////        knownActionSetIds.add(actionSetID);
////      }
////    }
////
////    // Load the always off action sets.
////    actions = memento
////    .getChildren(IWorkbenchConstants.TAG_ALWAYS_OFF_ACTION_SET);
////    for (int x = 0; x < actions.length; x++)
////    {
////      String actionSetID = actions[x]
////      .getString(IWorkbenchConstants.TAG_ID);
////      final IActionSetDescriptor d = WorkbenchPlugin.getDefault()
////      .getActionSetRegistry().findActionSet(actionSetID);
////      if (d != 0)
////      {
////        StartupThreading
////        .runWithoutExceptions(new StartupRunnable()
////            {
////              void runWithException() throws Throwable
////              {
////                addAlwaysOff(d);
////              }
////            });
////        knownActionSetIds.add(actionSetID);
////      }
////    }
//
//    // Load "show view actions".
//    actions = memento
//    .getChildren(IWorkbenchConstants.TAG_SHOW_VIEW_ACTION);
//    showViewShortcuts = new ArrayList(actions.length);
//    for (int x = 0; x < actions.length; x++)
//    {
//      String id = actions[x].getString(IWorkbenchConstants.TAG_ID);
//      showViewShortcuts.add(id);
//    }
//
//    // Load "show in times".
//    actions = memento.getChildren(IWorkbenchConstants.TAG_SHOW_IN_TIME);
//    for (int x = 0; x < actions.length; x++)
//    {
//      String id = actions[x].getString(IWorkbenchConstants.TAG_ID);
//      String timeStr = actions[x]
//      .getString(IWorkbenchConstants.TAG_TIME);
//      if (id != 0 && timeStr != 0)
//      {
//        try
//        {
//          long time = Long.parseLong(timeStr);
//          showInTimes.put(id, new Long(time));
//        }
//        catch (NumberFormatException e)
//        {
//          // skip this one
//        }
//      }
//    }
//
//    // Load "show in parts" from registry, not memento
//    showInPartIds = getShowInIdsFromRegistry();
//
//    // Load "new wizard actions".
//    actions = memento
//    .getChildren(IWorkbenchConstants.TAG_NEW_WIZARD_ACTION);
//    newWizardShortcuts = new ArrayList(actions.length);
//    for (int x = 0; x < actions.length; x++)
//    {
//      String id = actions[x].getString(IWorkbenchConstants.TAG_ID);
//      newWizardShortcuts.add(id);
//    }
//
//    // Load "perspective actions".
//    actions = memento
//    .getChildren(IWorkbenchConstants.TAG_PERSPECTIVE_ACTION);
//    perspectiveShortcuts = new ArrayList(actions.length);
//    for (int x = 0; x < actions.length; x++)
//    {
//      String id = actions[x].getString(IWorkbenchConstants.TAG_ID);
//      perspectiveShortcuts.add(id);
//    }
//
////    ArrayList extActionSets = getPerspectiveExtensionActionSets();
////    for (int i = 0; i < extActionSets.size(); i++)
////    {
////      String actionSetID = (String) extActionSets.get(i);
////      if (knownActionSetIds.contains(actionSetID))
////      {
////        continue;
////      }
////      final IActionSetDescriptor d = WorkbenchPlugin.getDefault()
////      .getActionSetRegistry().findActionSet(actionSetID);
////      if (d != 0)
////      {
////        StartupThreading
////        .runWithoutExceptions(new StartupRunnable()
////            {
////              void runWithException() throws Throwable
////              {
////                addAlwaysOn(d);
////              }
////            });
////        knownActionSetIds.add(d.getId());
////      }
////    }
//
////    // Add the visible set of action sets to our knownActionSetIds
////    // Now go through the registry to ensure we pick up any new action
////    // sets
////    // that have been added but not yet considered by this perspective.
////    ActionSetRegistry reg = WorkbenchPlugin.getDefault()
////    .getActionSetRegistry();
////    IActionSetDescriptor[] array = reg.getActionSets();
////    int count = array.length;
////    for (int i = 0; i < count; i++)
////    {
////      IActionSetDescriptor desc = array[i];
////      if ((!knownActionSetIds.contains(desc.getId()))
////          && (desc.isInitiallyVisible()))
////      {
////        addActionSet(desc);
////      }
////    }
//  }
//  catch (...)
//  {
////    // restart context changes
////    if (service != 0)
////    {
////      StartupThreading.runWithoutExceptions(new StartupRunnable()
////          {
////            void runWithException() throws Throwable
////            {
////              service.activateContext(ContextAuthority.SEND_EVENTS);
////            }
////          });
////    }
//  }
//
//  // Save presentation.
//  presentation = pres;
//
//  // Hide the editor area if needed. Need to wait for the
//  // presentation to be fully setup first.
//  Integer areaVisible = memento
//  .getInteger(IWorkbenchConstants.TAG_AREA_VISIBLE);
//  // Rather than hiding the editors now we must wait until after their
//  // controls
//  // are created. This ensures that if an editor is instantiated,
//  // createPartControl
//  // is also called. See bug 20166.
//  shouldHideEditorsOnActivate = (areaVisible != 0 && areaVisible
//      .intValue() == 0);
//
//  // Restore the trim state of the editor area
//  IPreferenceStore preferenceStore = PrefUtil.getAPIPreferenceStore();
//  bool useNewMinMax = preferenceStore.getbool(IWorkbenchPreferenceConstants.ENABLE_NEW_MIN_MAX);
//  if (useNewMinMax)
//  {
//    Integer trimStateInt = memento.getInteger(IWorkbenchConstants.TAG_AREA_TRIM_STATE);
//    if (trimStateInt != 0)
//    {
//      editorAreaState = trimStateInt.intValue() & 0x3; // low order two bits contain the state
//      editorAreaRestoreOnUnzoom = (trimStateInt.intValue() & 4) != 0;
//    }
//  }
//
//  // restore the fixed state
//  Integer isFixed = memento.getInteger(IWorkbenchConstants.TAG_FIXED);
//  fixed = (isFixed != 0 && isFixed.intValue() == 1);
//
//  return result;
  return true;
}

std::vector<std::string> Perspective::GetShowInIdsFromRegistry()
{
  PerspectiveExtensionReader reader;
  //reader.setIncludeOnlyTags(new String[]
  //    { IWorkbenchRegistryConstants.TAG_SHOW_IN_PART});
  PageLayout::Pointer layout = new PageLayout();
  reader.ExtendLayout(descriptor->GetOriginalId(), layout);
  return layout->GetShowInPartIds();
}

void Perspective::SaveDesc()
{
  this->SaveDescAs(descriptor);
}

void Perspective::SaveDescAs(IPerspectiveDescriptor::Pointer desc)
{
  //TODO Perspective SaveDescAs
//  PerspectiveDescriptor::Pointer realDesc = desc.Cast<PerspectiveDescriptor>();
//  //get the layout from the registry
//  PerspectiveRegistry* perspRegistry = dynamic_cast<PerspectiveRegistry*>(WorkbenchPlugin
//  ::GetDefault()->GetPerspectiveRegistry());
//  // Capture the layout state.
//  XMLMemento memento = XMLMemento.createWriteRoot("perspective");//$NON-NLS-1$
//  IStatus status = saveState(memento, realDesc, false);
//  if (status.getSeverity() == IStatus.ERR)
//  {
//    ErrorDialog.openError((Shell) 0, WorkbenchMessages.Perspective_problemSavingTitle,
//        WorkbenchMessages.Perspective_problemSavingMessage,
//        status);
//    return;
//  }
//  //save it to the preference store
//  try
//  {
//    perspRegistry.saveCustomPersp(realDesc, memento);
//    descriptor = realDesc;
//  }
//  catch (IOException e)
//  {
//    perspRegistry.deletePerspective(realDesc);
//    MessageDialog.openError((Shell) 0, WorkbenchMessages.Perspective_problemSavingTitle,
//        WorkbenchMessages.Perspective_problemSavingMessage);
//  }
}

bool Perspective::SaveState(IMemento::Pointer memento)
{
//  MultiStatus result = new MultiStatus(
//      PlatformUI.PLUGIN_ID,
//      IStatus.OK,
//      WorkbenchMessages.Perspective_problemsSavingPerspective, 0);
//
//  result.merge(saveState(memento, descriptor, true));

  bool result = true;
  result &= this->SaveState(memento, descriptor, true);

  return result;
}

bool Perspective::SaveState(IMemento::Pointer memento, PerspectiveDescriptor::Pointer p,
    bool saveInnerViewState)
{
  //TODO Perspective save state
//  MultiStatus result = new MultiStatus(
//      PlatformUI.PLUGIN_ID,
//      IStatus.OK,
//      WorkbenchMessages.Perspective_problemsSavingPerspective, 0);
//
//  if (this.memento != 0)
//  {
//    memento.putMemento(this.memento);
//    return result;
//  }
//
//  // Save the version number.
//  memento.putString(IWorkbenchConstants.TAG_VERSION, VERSION_STRING);
//  result.add(p.saveState(memento));
//  if (!saveInnerViewState)
//  {
//    Rectangle bounds = page.getWorkbenchWindow().getShell().getBounds();
//    IMemento boundsMem = memento
//    .createChild(IWorkbenchConstants.TAG_WINDOW);
//    boundsMem.putInteger(IWorkbenchConstants.TAG_X, bounds.x);
//    boundsMem.putInteger(IWorkbenchConstants.TAG_Y, bounds.y);
//    boundsMem.putInteger(IWorkbenchConstants.TAG_HEIGHT, bounds.height);
//    boundsMem.putInteger(IWorkbenchConstants.TAG_WIDTH, bounds.width);
//  }
//
//  // Save the "always on" action sets.
//  Iterator itr = alwaysOnActionSets.iterator();
//  while (itr.hasNext())
//  {
//    IActionSetDescriptor desc = (IActionSetDescriptor) itr.next();
//    IMemento child = memento
//    .createChild(IWorkbenchConstants.TAG_ALWAYS_ON_ACTION_SET);
//    child.putString(IWorkbenchConstants.TAG_ID, desc.getId());
//  }
//
//  // Save the "always off" action sets.
//  itr = alwaysOffActionSets.iterator();
//  while (itr.hasNext())
//  {
//    IActionSetDescriptor desc = (IActionSetDescriptor) itr.next();
//    IMemento child = memento
//    .createChild(IWorkbenchConstants.TAG_ALWAYS_OFF_ACTION_SET);
//    child.putString(IWorkbenchConstants.TAG_ID, desc.getId());
//  }
//
//  // Save "show view actions"
//  itr = showViewShortcuts.iterator();
//  while (itr.hasNext())
//  {
//    String str = (String) itr.next();
//    IMemento child = memento
//    .createChild(IWorkbenchConstants.TAG_SHOW_VIEW_ACTION);
//    child.putString(IWorkbenchConstants.TAG_ID, str);
//  }
//
//  // Save "show in times"
//  itr = showInTimes.keySet().iterator();
//  while (itr.hasNext())
//  {
//    String id = (String) itr.next();
//    Long time = (Long) showInTimes.get(id);
//    IMemento child = memento
//    .createChild(IWorkbenchConstants.TAG_SHOW_IN_TIME);
//    child.putString(IWorkbenchConstants.TAG_ID, id);
//    child.putString(IWorkbenchConstants.TAG_TIME, time.toString());
//  }
//
//  // Save "new wizard actions".
//  itr = newWizardShortcuts.iterator();
//  while (itr.hasNext())
//  {
//    String str = (String) itr.next();
//    IMemento child = memento
//    .createChild(IWorkbenchConstants.TAG_NEW_WIZARD_ACTION);
//    child.putString(IWorkbenchConstants.TAG_ID, str);
//  }
//
//  // Save "perspective actions".
//  itr = perspectiveShortcuts.iterator();
//  while (itr.hasNext())
//  {
//    String str = (String) itr.next();
//    IMemento child = memento
//    .createChild(IWorkbenchConstants.TAG_PERSPECTIVE_ACTION);
//    child.putString(IWorkbenchConstants.TAG_ID, str);
//  }
//
//  // Get visible views.
//  List viewPanes = new ArrayList(5);
//  presentation.collectViewPanes(viewPanes);
//
//  // Save the views.
//  itr = viewPanes.iterator();
//  int errors = 0;
//  while (itr.hasNext())
//  {
//    ViewPane pane = (ViewPane) itr.next();
//    IViewReference ref = pane.getViewReference();
//    bool restorable = page.getViewFactory().getViewRegistry().find(
//        ref.getId()).isRestorable();
//    if(restorable)
//    {
//      IMemento viewMemento = memento
//      .createChild(IWorkbenchConstants.TAG_VIEW);
//      viewMemento.putString(IWorkbenchConstants.TAG_ID, ViewFactory
//          .getKey(ref));
//    }
//  }
//
//  // save all fastview state
//  if (fastViewManager != 0)
//  fastViewManager.saveState(memento);
//
//  // Save the view layout recs.
//  for (Iterator i = mapIDtoViewLayoutRec.keySet().iterator(); i.hasNext();)
//  {
//    String compoundId = (String) i.next();
//    ViewLayoutRec rec = (ViewLayoutRec) mapIDtoViewLayoutRec
//    .get(compoundId);
//    if (rec != 0
//        && (!rec.isCloseable || !rec.isMoveable || rec.isStandalone))
//    {
//      IMemento layoutMemento = memento
//      .createChild(IWorkbenchConstants.TAG_VIEW_LAYOUT_REC);
//      layoutMemento.putString(IWorkbenchConstants.TAG_ID, compoundId);
//      if (!rec.isCloseable)
//      {
//        layoutMemento.putString(IWorkbenchConstants.TAG_CLOSEABLE,
//            IWorkbenchConstants.FALSE);
//      }
//      if (!rec.isMoveable)
//      {
//        layoutMemento.putString(IWorkbenchConstants.TAG_MOVEABLE,
//            IWorkbenchConstants.FALSE);
//      }
//      if (rec.isStandalone)
//      {
//        layoutMemento.putString(IWorkbenchConstants.TAG_STANDALONE,
//            IWorkbenchConstants.TRUE);
//        layoutMemento.putString(IWorkbenchConstants.TAG_SHOW_TITLE,
//            String.valueOf(rec.showTitle));
//      }
//    }
//  }
//
//  if (errors> 0)
//  {
//    String message = WorkbenchMessages.Perspective_multipleErrors;
//    if (errors == 1)
//    {
//      message = WorkbenchMessages.Perspective_oneError;
//    }
//    MessageDialog.openError(0,
//        WorkbenchMessages.Error, message);
//  }
//
//  // Save the layout.
//  IMemento childMem = memento.createChild(IWorkbenchConstants.TAG_LAYOUT);
//  result.add(presentation.saveState(childMem));
//
//  // Save the editor visibility state
//  if (isEditorAreaVisible())
//  {
//    memento.putInteger(IWorkbenchConstants.TAG_AREA_VISIBLE, 1);
//  }
//  else
//  {
//    memento.putInteger(IWorkbenchConstants.TAG_AREA_VISIBLE, 0);
//  }
//
//  // Save the trim state of the editor area if using the new min/max
//  IPreferenceStore preferenceStore = PrefUtil.getAPIPreferenceStore();
//  bool useNewMinMax = preferenceStore.getbool(IWorkbenchPreferenceConstants.ENABLE_NEW_MIN_MAX);
//  if (useNewMinMax)
//  {
//    int trimState = editorAreaState;
//    trimState |= editorAreaRestoreOnUnzoom ? 4 : 0;
//    memento.putInteger(IWorkbenchConstants.TAG_AREA_TRIM_STATE, trimState);
//  }
//
//  // Save the fixed state
//  if (fixed)
//  {
//    memento.putInteger(IWorkbenchConstants.TAG_FIXED, 1);
//  }
//  else
//  {
//    memento.putInteger(IWorkbenchConstants.TAG_FIXED, 0);
//  }
//
//  return result;
  return true;
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
  if (this->IsEditorAreaVisible())
  {
    return;
  }

  editorHidden = false;

  // Show the editor in the appropriate location
  if (this->UseNewMinMax(this))
  {
    bool isMinimized = editorAreaState == IStackPresentationSite::STATE_MINIMIZED;
    if (!isMinimized)
    {
      // If the editor area is going to show then we have to restore
//      if (getPresentation().getMaximizedStack() != 0)
//      getPresentation().getMaximizedStack().setState(IStackPresentationSite.STATE_RESTORED);

      this->ShowEditorAreaLocal();
    }
//    else
//    setEditorAreaTrimVisibility(true);
  }
  else
  {
    this->ShowEditorAreaLocal();
  }
}

void Perspective::ShowEditorAreaLocal()
{
  if (editorHolder == 0 || editorHidden)
  {
    return;
  }

  // Replace the part holder with the editor area.
  presentation->GetLayout()->Replace(editorHolder, editorArea);
  editorHolder = 0;
}

void Perspective::SetEditorAreaState(int newState)
{
  if (newState == editorAreaState)
  return;

  editorAreaState = newState;

//  // reset the restore flag if we're not minimized
//  if (newState != IStackPresentationSite::STATE_MINIMIZED)
//  editorAreaRestoreOnUnzoom = false;

  this->RefreshEditorAreaVisibility();
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
    this->HideEditorAreaLocal();
    //setEditorAreaTrimVisibility(false);
    return;
  }

  PartStack::Pointer editorStack = editorArea.Cast<EditorSashContainer>()->GetUpperRightEditorStack();
  if (editorStack == 0)
  return;

  // Whatever we're doing, make the current editor stack match it
  //editorStack->SetStateLocal(editorAreaState);

  // If it's minimized then it's in the trim
  if (editorAreaState == IStackPresentationSite::STATE_MINIMIZED)
  {
    // Hide the editor area and show its trim
    this->HideEditorAreaLocal();
    //setEditorAreaTrimVisibility(true);
  }
  else
  {
    // Show the editor area and hide its trim
    //setEditorAreaTrimVisibility(false);
    this->ShowEditorAreaLocal();

//    if (editorAreaState == IStackPresentationSite::STATE_MAXIMIZED)
//    getPresentation().setMaximizedStack(editorStack);
  }
}

IViewReference::Pointer Perspective::GetViewReference(const std::string& viewId, const std::string& secondaryId)
{
  IViewReference::Pointer ref = page->FindViewReference(viewId, secondaryId);
  if (ref == 0)
  {
    ViewFactory* factory = this->GetViewFactory();
    try
    {
      ref = factory->CreateView(viewId, secondaryId);
    }
    catch (PartInitException& e)
    {
//      IStatus status = StatusUtil.newStatus(IStatus.ERR,
//          e.getMessage() == 0 ? "" : e.getMessage(), //$NON-NLS-1$
//          e);
//      StatusUtil.handleStatus(status, "Failed to create view: id=" + viewId, //$NON-NLS-1$
//          StatusManager.LOG);
      //TODO Perspective status message
      WorkbenchPlugin::Log("Failed to create view: id=" + viewId);
    }
  }
  return ref;
}

IViewPart::Pointer Perspective::ShowView(const std::string& viewId, const std::string& secondaryId)
{
  ViewFactory* factory = this->GetViewFactory();
  IViewReference::Pointer ref = factory->CreateView(viewId, secondaryId);
  IViewPart::Pointer part = ref->GetPart(true).Cast<IViewPart>();
  if (part == 0)
  {
    throw PartInitException("Could not create view: " + ref->GetId());
  }
  PartSite::Pointer site = part->GetSite().Cast<PartSite>();
  PartPane::Pointer pane = site->GetPane();

  //TODO Perspective preference store
//  IPreferenceStore store = WorkbenchPlugin.getDefault()
//  .getPreferenceStore();
//  int openViewMode = store.getInt(IPreferenceConstants.OPEN_VIEW_MODE);
//
//  if (openViewMode == IPreferenceConstants.OVM_FAST &&
//      fastViewManager != 0)
//  {
//    fastViewManager.addViewReference(FastViewBar.FASTVIEWBAR_ID, -1, ref, true);
//    setActiveFastView(ref);
//  }
//  else if (openViewMode == IPreferenceConstants.OVM_FLOAT
//      && presentation.canDetach())
//  {
//    presentation.addDetachedPart(pane);
//  }
//  else
//  {
    if (this->UseNewMinMax(this))
    {
      // Is this view going to show in the trim?
 //     LayoutPart vPart = presentation.findPart(viewId, secondaryId);

      // Determine if there is a trim stack that should get the view
      std::string trimId;

//      // If we can locate the correct trim stack then do so
//      if (vPart != 0)
//      {
//        String id = 0;
//        ILayoutContainer container = vPart.getContainer();
//        if (container.Cast<ContainerPlaceholder>() != 0)
//        id = ((ContainerPlaceholder)container).getID();
//        else if (container.Cast<ViewStack>() != 0)
//        id = ((ViewStack)container).getID();
//
//        // Is this place-holder in the trim?
//        if (id != 0 && fastViewManager.getFastViews(id).size()> 0)
//        {
//          trimId = id;
//        }
//      }
//
//      // No explicit trim found; If we're maximized then we either have to find an
//      // arbitrary stack...
//      if (trimId == 0 && presentation.getMaximizedStack() != 0)
//      {
//        if (vPart == 0)
//        {
//          ViewStackTrimToolBar blTrimStack = fastViewManager.getBottomRightTrimStack();
//          if (blTrimStack != 0)
//          {
//            // OK, we've found a trim stack to add it to...
//            trimId = blTrimStack.getId();
//
//            // Since there was no placeholder we have to add one
//            LayoutPart blPart = presentation.findPart(trimId, 0);
//            if (blPart.Cast<ContainerPlaceholder>() != 0)
//            {
//              ContainerPlaceholder cph = (ContainerPlaceholder) blPart;
//              if (cph.getRealContainer().Cast<ViewStack>() != 0)
//              {
//                ViewStack vs = (ViewStack) cph.getRealContainer();
//
//                // Create a 'compound' id if this is a multi-instance part
//                String compoundId = ref.getId();
//                if (ref.getSecondaryId() != 0)
//                compoundId = compoundId + ':' + ref.getSecondaryId();
//
//                // Add the new placeholder
//                vs.add(new PartPlaceholder(compoundId));
//              }
//            }
//          }
//        }
//      }
//
//      // If we have a trim stack located then add the view to it
//      if (trimId != "")
//      {
//        fastViewManager.addViewReference(trimId, -1, ref, true);
//      }
//      else
//      {
//        bool inMaximizedStack = vPart != 0 && vPart.getContainer() == presentation.getMaximizedStack();

        // Do the default behavior
        presentation->AddPart(pane);

//        // Now, if we're maximized then we have to minimize the new stack
//        if (presentation.getMaximizedStack() != 0 && !inMaximizedStack)
//        {
//          vPart = presentation.findPart(viewId, secondaryId);
//          if (vPart != 0 && vPart.getContainer().Cast<ViewStack>() != 0)
//          {
//            ViewStack vs = (ViewStack)vPart.getContainer();
//            vs.setState(IStackPresentationSite.STATE_MINIMIZED);
//
//            // setting the state to minimized will create the trim toolbar
//            // so we don't need a 0 pointer check here...
//            fastViewManager.getViewStackTrimToolbar(vs.getID()).setRestoreOnUnzoom(true);
//          }
//        }
  //    }
    }
    else
    {
      presentation->AddPart(pane);
    }
  //}

  // Ensure that the newly showing part is enabled
  if (pane != 0 && pane->GetControl() != 0)
  Tweaklets::Get(GuiWidgetsTweaklet::KEY)->SetEnabled(pane->GetControl(), true);

  return part;
}

IWorkbenchPartReference::Pointer Perspective::GetOldPartRef()
{
  return oldPartRef;
}

void Perspective::SetOldPartRef(IWorkbenchPartReference::Pointer oldPartRef)
{
  this->oldPartRef = oldPartRef;
}

bool Perspective::IsCloseable(IViewReference::Pointer reference)
{
  ViewLayoutRec::Pointer rec = this->GetViewLayoutRec(reference, false);
  if (rec != 0)
  {
    return rec->isCloseable;
  }
  return true;
}

bool Perspective::IsMoveable(IViewReference::Pointer reference)
{
  ViewLayoutRec::Pointer rec = this->GetViewLayoutRec(reference, false);
  if (rec != 0)
  {
    return rec->isMoveable;
  }
  return true;
}

void Perspective::DescribeLayout(std::string& buf) const
{
//  std::vector<IViewReference::Pointer> fastViews = getFastViews();
//
//  if (fastViews.length != 0)
//  {
//    buf.append("fastviews ("); //$NON-NLS-1$
//    for (int idx = 0; idx < fastViews.length; idx++)
//    {
//      IViewReference ref = fastViews[idx];
//
//      if (idx> 0)
//      {
//        buf.append(", "); //$NON-NLS-1$
//      }
//
//      buf.append(ref.getPartName());
//    }
//    buf.append("), "); //$NON-NLS-1$
//  }

  this->GetPresentation()->DescribeLayout(buf);
}

void Perspective::TestInvariants()
{
  this->GetPresentation()->GetLayout()->TestInvariants();
}

bool Perspective::UseNewMinMax(Perspective::Pointer activePerspective)
{
  // We need to have an active perspective
  if (activePerspective == 0)
  return false;

  // We need to have a trim manager (if we don't then we
  // don't create a FastViewManager because it'd be useless)
 // if (activePerspective->GetFastViewManager() == 0)
 // return false;

  // Make sure we don't NPE anyplace
  WorkbenchWindow::Pointer wbw = activePerspective->page->GetWorkbenchWindow().Cast<WorkbenchWindow>();
  if (wbw == 0)
  return false;

//  WorkbenchWindowConfigurer* configurer = wbw->GetWindowConfigurer();
//  if (configurer == 0)
//  return false;

  IPresentationFactory* factory = WorkbenchPlugin::GetDefault()->GetPresentationFactory();
  if (factory == 0)
  return false;

  // Ok, we should be good to go, return the pref
  //IPreferenceStore preferenceStore = PrefUtil.getAPIPreferenceStore();
  //bool useNewMinMax = preferenceStore.getbool(IWorkbenchPreferenceConstants.ENABLE_NEW_MIN_MAX);
  return true;
}

}
