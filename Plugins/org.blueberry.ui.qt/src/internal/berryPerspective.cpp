/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "tweaklets/berryGuiWidgetsTweaklet.h"

#include "berryPerspective.h"

#include "berryPerspectiveHelper.h"
#include "berryWorkbenchPlugin.h"
#include "berryWorkbenchConstants.h"
#include "berryPerspectiveExtensionReader.h"
#include "berryEditorSashContainer.h"
#include "berryPartSite.h"
#include "berryViewSite.h"
#include "berryEditorAreaHelper.h"
#include "intro/berryIntroConstants.h"
#include "berryWorkbenchWindow.h"
#include "berryStatusUtil.h"
#include "berryMultiStatus.h"
#include "berryXMLMemento.h"

#include "presentations/berryIStackPresentationSite.h"
#include "berryIContextService.h"

#include <QMessageBox>

namespace berry
{

const QString Perspective::VERSION_STRING = "0.016";

Perspective::Perspective(PerspectiveDescriptor::Pointer desc,
    WorkbenchPage::Pointer page)
 : descriptor(desc)
 {
  this->Register();
  this->Init(page);
  if (desc.IsNotNull())
  {
    this->CreatePresentation(desc);
  }
  this->UnRegister(false);
}

Perspective::Perspective(WorkbenchPage::Pointer page)
{
  this->Init(page);
}

void Perspective::Init(WorkbenchPage::Pointer page)
{
  editorHidden = false;
  editorAreaState = IStackPresentationSite::STATE_RESTORED;
  fixed = false;
  presentation = nullptr;
  shouldHideEditorsOnActivate = false;
  this->page = page.GetPointer();
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

bool Perspective::ContainsView(const QString& viewId) const
{
  return mapIDtoViewLayoutRec.contains(viewId);
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
  if (presentation == nullptr)
  {
    DisposeViewRefs();
    return;
  }

  presentation->Deactivate();

  // Release each view.
  QList<IViewReference::Pointer> refs(this->GetViewReferences());
  for (auto & ref : refs)
  {
    this->GetViewFactory()->ReleaseView(ref);
  }

  mapIDtoViewLayoutRec.clear();
  delete presentation;
}

void Perspective::DisposeViewRefs()
{
  if (!memento)
  {
    return;
  }
  QList<IMemento::Pointer> views(memento->GetChildren(WorkbenchConstants::TAG_VIEW));
  for (int x = 0; x < views.size(); x++)
  {
    // Get the view details.
    IMemento::Pointer childMem = views[x];
    QString id; childMem->GetString(WorkbenchConstants::TAG_ID, id);
    // skip creation of the intro reference - it's handled elsewhere.
    if (id == IntroConstants::INTRO_VIEW_ID)
    {
      continue;
    }

    QString secondaryId = ViewFactory::ExtractSecondaryId(id);
    if (!secondaryId.isEmpty())
    {
      id = ViewFactory::ExtractPrimaryId(id);
    }

    QString removed;
    childMem->GetString(WorkbenchConstants::TAG_REMOVED, removed);
    if (removed != "true")
    {
      IViewReference::Pointer ref = viewFactory->GetView(id, secondaryId);
      if (ref)
      {
        viewFactory->ReleaseView(ref);
      }
    }
  }
}

IViewReference::Pointer Perspective::FindView(const QString& viewId)
{
  return this->FindView(viewId, "");
}

IViewReference::Pointer Perspective::FindView(const QString& id, const QString& secondaryId)
{
  QList<IViewReference::Pointer> refs(this->GetViewReferences());
  for (int i = 0; i < refs.size(); i++)
  {
    IViewReference::Pointer ref = refs[i];
    if (id == ref->GetId()
        && (secondaryId  == ref->GetSecondaryId()))
    {
      return ref;
    }
  }
  return IViewReference::Pointer(nullptr);
}

QWidget* Perspective::GetClientComposite()
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

QList<QString> Perspective::GetPerspectiveShortcuts()
{
  return perspectiveShortcuts;
}

PerspectiveHelper* Perspective::GetPresentation() const
{
  return presentation;
}

QList<QString> Perspective::GetShowViewShortcuts()
{
  return showViewShortcuts;
}

ViewFactory* Perspective::GetViewFactory()
{
  return viewFactory;
}

QList<IViewReference::Pointer> Perspective::GetViewReferences()
{
  // Get normal views.
  if (presentation == nullptr)
  {
    return QList<IViewReference::Pointer>();
  }

  QList<PartPane::Pointer> panes;
  presentation->CollectViewPanes(panes);

  QList<IViewReference::Pointer> result;
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
  for (QList<PartPane::Pointer>::iterator iter = panes.begin();
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
  if (this->UseNewMinMax(Perspective::Pointer(this)))
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
  editorHolder = new PartPlaceholder(editorArea->GetID());
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

ViewLayoutRec::Pointer Perspective::GetViewLayoutRec(const QString& viewId, bool create)
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
  catch (const WorkbenchException& e)
  {
    this->UnableToOpenPerspective(persp, e.what());
  }
}

void Perspective::UnableToOpenPerspective(PerspectiveDescriptor::Pointer persp,
    const QString& status)
{
  PerspectiveRegistry* perspRegistry = dynamic_cast<PerspectiveRegistry*>(WorkbenchPlugin
  ::GetDefault()->GetPerspectiveRegistry());
  perspRegistry->DeletePerspective(persp);
  // If this is a predefined perspective, we will not be able to delete
  // the perspective (we wouldn't want to).  But make sure to delete the
  // customized portion.
  persp->DeleteCustomDefinition();
  QString title = "Restoring problems";
  QString msg = "Unable to read workbench state.";
  if (status == "")
  {
    QMessageBox::critical(nullptr, title, msg);
  }
  else
  {
    //TODO error dialog
    //ErrorDialog.openError((Shell) 0, title, msg, status);
    QMessageBox::critical(nullptr, title, msg + "\n" + status);
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
  catch (CoreException& /*e*/)
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
  ViewSashContainer::Pointer container(new ViewSashContainer(page, this->GetClientComposite()));
  layout = new PageLayout(container, this->GetViewFactory(),
      editorArea, descriptor);
  layout->SetFixed(descriptor->GetFixed());

//  // add the placeholders for the sticky folders and their contents
  IPlaceholderFolderLayout::Pointer stickyFolderRight, stickyFolderLeft, stickyFolderTop, stickyFolderBottom;

  QList<IStickyViewDescriptor::Pointer> descs(WorkbenchPlugin::GetDefault()
  ->GetViewRegistry()->GetStickyViews());
  for (int i = 0; i < descs.size(); i++)
  {
    IStickyViewDescriptor::Pointer stickyViewDescriptor = descs[i];
    QString id = stickyViewDescriptor->GetId();
    int location = stickyViewDescriptor->GetLocation();
    if (location == IPageLayout::RIGHT)
    {
      if (stickyFolderRight == 0)
      {
        stickyFolderRight = layout
        ->CreatePlaceholderFolder(
            StickyViewDescriptor::STICKY_FOLDER_RIGHT,
            IPageLayout::RIGHT, .75f,
            IPageLayout::ID_EDITOR_AREA);
      }
      stickyFolderRight->AddPlaceholder(id);
    }
    else if (location == IPageLayout::LEFT)
    {
      if (stickyFolderLeft == 0)
      {
        stickyFolderLeft = layout->CreatePlaceholderFolder(
            StickyViewDescriptor::STICKY_FOLDER_LEFT,
            IPageLayout::LEFT, .25f, IPageLayout::ID_EDITOR_AREA);
      }
      stickyFolderLeft->AddPlaceholder(id);
    }
    else if (location == IPageLayout::TOP)
    {
      if (stickyFolderTop == 0)
      {
        stickyFolderTop = layout->CreatePlaceholderFolder(
            StickyViewDescriptor::STICKY_FOLDER_TOP,
            IPageLayout::TOP, .25f, IPageLayout::ID_EDITOR_AREA);
      }
      stickyFolderTop->AddPlaceholder(id);
    }
    else if (location == IPageLayout::BOTTOM)
    {
      if (stickyFolderBottom == 0)
      {
        stickyFolderBottom = layout->CreatePlaceholderFolder(
            StickyViewDescriptor::STICKY_FOLDER_BOTTOM,
            IPageLayout::BOTTOM, .75f,
            IPageLayout::ID_EDITOR_AREA);
      }
      stickyFolderBottom->AddPlaceholder(id);
    }

    //should never be 0 as we've just added the view above
    IViewLayout::Pointer viewLayout = layout->GetViewLayout(id);
    viewLayout->SetCloseable(stickyViewDescriptor->IsCloseable());
    viewLayout->SetMoveable(stickyViewDescriptor->IsMoveable());
  }

  // Run layout engine.
  factory->CreateInitialLayout(layout);
  PerspectiveExtensionReader extender;
  extender.ExtendLayout(page->GetExtensionTracker(), descriptor->GetId(), layout);

  // Retrieve view layout info stored in the page layout.
  QHash<QString, ViewLayoutRec::Pointer> layoutInfo = layout->GetIDtoViewLayoutRecMap();
  mapIDtoViewLayoutRec.unite(layoutInfo);

  //TODO Perspective action sets
  // Create action sets.
  //List temp = new ArrayList();
  //this->CreateInitialActionSets(temp, layout.getActionSets());

//  IContextService* service = 0;
//  if (page != 0)
//  {
//    service = page->GetWorkbenchWindow()->GetService<IContextService>();
//  }
//  try
//  {
//    if (service != 0)
//    {
//      service->DeferUpdates(true);
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
    showViewShortcuts = layout->GetShowViewShortcuts();
    perspectiveShortcuts = layout->GetPerspectiveShortcuts();
    showInPartIds = layout->GetShowInPartIds();

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
  showViewShortcuts = layout->GetShowViewShortcuts();

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
  if (editorArea->GetControl() != nullptr)
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
  bool useNewMinMax = Perspective::UseNewMinMax(Perspective::Pointer(this));
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

  // Fix perspectives whose contributing bundle has gone away
  FixOrphan();
}

void Perspective::FixOrphan()
{
  PerspectiveRegistry* reg = static_cast<PerspectiveRegistry*>(PlatformUI::GetWorkbench()->GetPerspectiveRegistry());
  IPerspectiveDescriptor::Pointer regDesc = reg->FindPerspectiveWithId(descriptor->GetId());
  if (regDesc.IsNull())
  {
    QString msg = "Perspective " + descriptor->GetLabel() + " has been made into a local copy";
    IStatus::Pointer status = StatusUtil::NewStatus(IStatus::WARNING_TYPE, msg, BERRY_STATUS_LOC);
    //StatusManager.getManager().handle(status, StatusManager.LOG);
    WorkbenchPlugin::Log(status);

    QString localCopyLabel("<%1>");
    QString newDescId = localCopyLabel.arg(descriptor->GetLabel());
    while (reg->FindPerspectiveWithId(newDescId).IsNotNull())
    {
      newDescId = localCopyLabel.arg(newDescId);
    }
    IPerspectiveDescriptor::Pointer newDesc = reg->CreatePerspective(newDescId, descriptor);
    page->SavePerspectiveAs(newDesc);
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

void Perspective::PartActivated(IWorkbenchPart::Pointer  /*activePart*/)
{
//  // If a fastview is open close it.
//  if (activeFastView != 0
//      && activeFastView.getPart(false) != activePart)
//  {
//    setActiveFastView(0);
//  }
}

void Perspective::PerformedShowIn(const QString&  /*partId*/)
{
  //showInTimes.insert(std::make_pair(partId, new Long(System.currentTimeMillis())));
}

bool Perspective::RestoreState(IMemento::Pointer memento)
{

//  MultiStatus result = new MultiStatus(
//      PlatformUI.PLUGIN_ID,
//      IStatus.OK,
//      WorkbenchMessages.Perspective_problemsRestoringPerspective, 0);
  bool result = true;

  // Create persp descriptor.
  descriptor = new PerspectiveDescriptor("", "", PerspectiveDescriptor::Pointer(nullptr));
  //result.add(descriptor.restoreState(memento));
  result &= descriptor->RestoreState(memento);
  PerspectiveDescriptor::Pointer desc = WorkbenchPlugin::GetDefault()->
      GetPerspectiveRegistry()->FindPerspectiveWithId(descriptor->GetId()).Cast<PerspectiveDescriptor>();
  if (desc)
  {
    descriptor = desc;
  }

  this->memento = memento;
  // Add the visible views.
  QList<IMemento::Pointer> views(memento->GetChildren(WorkbenchConstants::TAG_VIEW));
  //result.merge(createReferences(views));
  result &= this->CreateReferences(views);

  memento = memento->GetChild(WorkbenchConstants::TAG_FAST_VIEWS);
  if (memento)
  {
    views = memento->GetChildren(WorkbenchConstants::TAG_VIEW);
    //result.merge(createReferences(views));
    result &= this->CreateReferences(views);
  }
  return result;
}

bool Perspective::CreateReferences(const QList<IMemento::Pointer>& views)
{
//  MultiStatus result = new MultiStatus(PlatformUI.PLUGIN_ID, IStatus.OK,
//      WorkbenchMessages.Perspective_problemsRestoringViews, 0);
  bool result = true;

  for (int x = 0; x < views.size(); x++)
  {
    // Get the view details.
    IMemento::Pointer childMem = views[x];
    QString id; childMem->GetString(WorkbenchConstants::TAG_ID, id);
    // skip creation of the intro reference -  it's handled elsewhere.
    if (id == IntroConstants::INTRO_VIEW_ID)
    {
      continue;
    }

    QString secondaryId(ViewFactory::ExtractSecondaryId(id));
    if (!secondaryId.isEmpty())
    {
      id = ViewFactory::ExtractPrimaryId(id);
    }
    // Create and open the view.
    try
    {
      QString rm; childMem->GetString(WorkbenchConstants::TAG_REMOVED, rm);
      if (rm != "true")
      {
        viewFactory->CreateView(id, secondaryId);
      }
    }
    catch (const PartInitException& e)
    {
      childMem->PutString(WorkbenchConstants::TAG_REMOVED, "true");
//      result.add(StatusUtil.newStatus(IStatus.ERR,
//              e.getMessage() == 0 ? "" : e.getMessage(), //$NON-NLS-1$
//              e));
      WorkbenchPlugin::Log(e.what(), e);
      result &= true;
    }
  }
  return result;
}

bool Perspective::RestoreState()
{
  if (this->memento == 0)
  {
    //return new Status(IStatus.OK, PlatformUI.PLUGIN_ID, 0, "", 0); //$NON-NLS-1$
    return true;
  }

//  MultiStatus result = new MultiStatus(
//      PlatformUI.PLUGIN_ID,
//      IStatus.OK,
//      WorkbenchMessages.Perspective_problemsRestoringPerspective, 0);
  bool result = true;

  IMemento::Pointer memento = this->memento;
  this->memento = nullptr;

  const IMemento::Pointer boundsMem(memento->GetChild(WorkbenchConstants::TAG_WINDOW));
  if (boundsMem)
  {
    int x, y, w, h;
    boundsMem->GetInteger(WorkbenchConstants::TAG_X, x);
    boundsMem->GetInteger(WorkbenchConstants::TAG_Y, y);
    boundsMem->GetInteger(WorkbenchConstants::TAG_HEIGHT, h);
    boundsMem->GetInteger(WorkbenchConstants::TAG_WIDTH, w);
    QRect r(x, y, w, h);
    //StartupThreading.runWithoutExceptions(new StartupRunnable()
    //    {

    //      void runWithException() throws Throwable
    //      {
            if (page->GetWorkbenchWindow()->GetActivePage() == 0)
            {
              page->GetWorkbenchWindow()->GetShell()->SetBounds(r);
            }
    //      }
    //    });

  }

  // Create an empty presentation..
  PerspectiveHelper* pres;
  //StartupThreading.runWithoutExceptions(new StartupRunnable()
  //    {

  //      void runWithException() throws Throwable
  //      {
          ViewSashContainer::Pointer mainLayout(new ViewSashContainer(page, this->GetClientComposite()));
          pres = new PerspectiveHelper(page, mainLayout, this);
  //      }});


  // Read the layout.
//  result.merge(pres.restoreState(memento
//          .getChild(IWorkbenchConstants.TAG_LAYOUT)));
  result &= pres->RestoreState(memento->GetChild(WorkbenchConstants::TAG_LAYOUT));

  //StartupThreading.runWithoutExceptions(new StartupRunnable()
  //    {

  //      void runWithException() throws Throwable
  //      {
          // Add the editor workbook. Do not hide it now.
          pres->ReplacePlaceholderWithPart(editorArea);
  //      }});

  // Add the visible views.
  QList<IMemento::Pointer> views(memento->GetChildren(WorkbenchConstants::TAG_VIEW));

  for (int x = 0; x < views.size(); x++)
  {
    // Get the view details.
    IMemento::Pointer childMem = views[x];
    QString id; childMem->GetString(WorkbenchConstants::TAG_ID, id);
    QString secondaryId(ViewFactory::ExtractSecondaryId(id));
    if (!secondaryId.isEmpty())
    {
      id = ViewFactory::ExtractPrimaryId(id);
    }

    // skip the intro as it is restored higher up in workbench.
    if (id == IntroConstants::INTRO_VIEW_ID)
    {
      continue;
    }

    // Create and open the view.
    IViewReference::Pointer viewRef = viewFactory->GetView(id, secondaryId);
    WorkbenchPartReference::Pointer ref = viewRef.Cast<WorkbenchPartReference>();

    // report error
    if (ref == 0)
    {
      QString key = ViewFactory::GetKey(id, secondaryId);
//      result.add(new Status(IStatus.ERR, PlatformUI.PLUGIN_ID, 0,
//              NLS.bind(WorkbenchMessages.Perspective_couldNotFind, key ), 0));
      WorkbenchPlugin::Log("Could not find view: " + key);
      continue;
    }
    bool willPartBeVisible = pres->WillPartBeVisible(ref->GetId(), secondaryId);
    if (willPartBeVisible)
    {
      IViewPart::Pointer view = ref->GetPart(true).Cast<IViewPart>();
      if (view)
      {
        ViewSite::Pointer site = view->GetSite().Cast<ViewSite>();
        pres->ReplacePlaceholderWithPart(site->GetPane().Cast<LayoutPart>());
      }
    }
    else
    {
      pres->ReplacePlaceholderWithPart(ref->GetPane());
    }
  }

//  // Load the fast views
//  if (fastViewManager != 0)
//  fastViewManager.restoreState(memento, result);

  // Load the view layout recs
  QList<IMemento::Pointer> recMementos(memento
  ->GetChildren(WorkbenchConstants::TAG_VIEW_LAYOUT_REC));
  for (int i = 0; i < recMementos.size(); i++)
  {
    IMemento::Pointer recMemento = recMementos[i];
    QString compoundId;
    if (recMemento->GetString(WorkbenchConstants::TAG_ID, compoundId))
    {
      ViewLayoutRec::Pointer rec = GetViewLayoutRec(compoundId, true);
      QString closeablestr; recMemento->GetString(WorkbenchConstants::TAG_CLOSEABLE, closeablestr);
      if (WorkbenchConstants::FALSE_VAL == closeablestr)
      {
        rec->isCloseable = false;
      }
      QString moveablestr; recMemento->GetString(WorkbenchConstants::TAG_MOVEABLE, moveablestr);
      if (WorkbenchConstants::FALSE_VAL == moveablestr)
      {
        rec->isMoveable = false;
      }
      QString standalonestr; recMemento->GetString(WorkbenchConstants::TAG_STANDALONE, standalonestr);
      if (WorkbenchConstants::TRUE_VAL == standalonestr)
      {
        rec->isStandalone = true;
        QString showstr; recMemento->GetString(WorkbenchConstants::TAG_SHOW_TITLE, showstr);
        rec->showTitle = WorkbenchConstants::FALSE_VAL != showstr;
      }
    }
  }

  //final IContextService service = (IContextService)page.getWorkbenchWindow().getService(IContextService.class);
  try
  { // one big try block, don't kill me here
//    // defer context events
//    if (service != 0)
//    {
//      service.activateContext(ContextAuthority.DEFER_EVENTS);
//    }
//
//    HashSet knownActionSetIds = new HashSet();
//
//    // Load the always on action sets.
    QList<IMemento::Pointer> actions; // = memento
//    .getChildren(IWorkbenchConstants.TAG_ALWAYS_ON_ACTION_SET);
//    for (int x = 0; x < actions.length; x++)
//    {
//      String actionSetID = actions[x]
//      .getString(IWorkbenchConstants.TAG_ID);
//      final IActionSetDescriptor d = WorkbenchPlugin.getDefault()
//      .getActionSetRegistry().findActionSet(actionSetID);
//      if (d != 0)
//      {
//        StartupThreading
//        .runWithoutExceptions(new StartupRunnable()
//            {
//              void runWithException() throws Throwable
//              {
//                addAlwaysOn(d);
//              }
//            });
//
//        knownActionSetIds.add(actionSetID);
//      }
//    }
//
//    // Load the always off action sets.
//    actions = memento
//    .getChildren(IWorkbenchConstants.TAG_ALWAYS_OFF_ACTION_SET);
//    for (int x = 0; x < actions.length; x++)
//    {
//      String actionSetID = actions[x]
//      .getString(IWorkbenchConstants.TAG_ID);
//      final IActionSetDescriptor d = WorkbenchPlugin.getDefault()
//      .getActionSetRegistry().findActionSet(actionSetID);
//      if (d != 0)
//      {
//        StartupThreading
//        .runWithoutExceptions(new StartupRunnable()
//            {
//              void runWithException() throws Throwable
//              {
//                addAlwaysOff(d);
//              }
//            });
//        knownActionSetIds.add(actionSetID);
//      }
//    }

    // Load "show view actions".
    actions = memento->GetChildren(WorkbenchConstants::TAG_SHOW_VIEW_ACTION);
    for (int x = 0; x < actions.size(); x++)
    {
      QString id; actions[x]->GetString(WorkbenchConstants::TAG_ID, id);
      showViewShortcuts.push_back(id);
    }

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

    // Load "show in parts" from registry, not memento
    showInPartIds = this->GetShowInIdsFromRegistry();

//    // Load "new wizard actions".
//    actions = memento
//    .getChildren(IWorkbenchConstants.TAG_NEW_WIZARD_ACTION);
//    newWizardShortcuts = new ArrayList(actions.length);
//    for (int x = 0; x < actions.length; x++)
//    {
//      String id = actions[x].getString(IWorkbenchConstants.TAG_ID);
//      newWizardShortcuts.add(id);
//    }

    // Load "perspective actions".
    actions = memento->GetChildren(WorkbenchConstants::TAG_PERSPECTIVE_ACTION);
    for (int x = 0; x < actions.size(); x++)
    {
      QString id; actions[x]->GetString(WorkbenchConstants::TAG_ID, id);
      perspectiveShortcuts.push_back(id);
    }

//    ArrayList extActionSets = getPerspectiveExtensionActionSets();
//    for (int i = 0; i < extActionSets.size(); i++)
//    {
//      String actionSetID = (String) extActionSets.get(i);
//      if (knownActionSetIds.contains(actionSetID))
//      {
//        continue;
//      }
//      final IActionSetDescriptor d = WorkbenchPlugin.getDefault()
//      .getActionSetRegistry().findActionSet(actionSetID);
//      if (d != 0)
//      {
//        StartupThreading
//        .runWithoutExceptions(new StartupRunnable()
//            {
//              void runWithException() throws Throwable
//              {
//                addAlwaysOn(d);
//              }
//            });
//        knownActionSetIds.add(d.getId());
//      }
//    }

//    // Add the visible set of action sets to our knownActionSetIds
//    // Now go through the registry to ensure we pick up any new action
//    // sets
//    // that have been added but not yet considered by this perspective.
//    ActionSetRegistry reg = WorkbenchPlugin.getDefault()
//    .getActionSetRegistry();
//    IActionSetDescriptor[] array = reg.getActionSets();
//    int count = array.length;
//    for (int i = 0; i < count; i++)
//    {
//      IActionSetDescriptor desc = array[i];
//      if ((!knownActionSetIds.contains(desc.getId()))
//          && (desc.isInitiallyVisible()))
//      {
//        addActionSet(desc);
//      }
//    }
  }
  catch (...)
  {
//    // restart context changes
//    if (service != 0)
//    {
//      StartupThreading.runWithoutExceptions(new StartupRunnable()
//          {
//            void runWithException() throws Throwable
//            {
//              service.activateContext(ContextAuthority.SEND_EVENTS);
//            }
//          });
//    }
  }

  // Save presentation.
  presentation = pres;

  // Hide the editor area if needed. Need to wait for the
  // presentation to be fully setup first.
  int areaVisible = 0;
  bool areaVisibleExists = memento->GetInteger(WorkbenchConstants::TAG_AREA_VISIBLE, areaVisible);
  // Rather than hiding the editors now we must wait until after their
  // controls
  // are created. This ensures that if an editor is instantiated,
  // createPartControl
  // is also called. See bug 20166.
  shouldHideEditorsOnActivate = (areaVisibleExists && areaVisible == 0);

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

  // restore the fixed state
  int isFixed = 0;
  fixed = (memento->GetInteger(WorkbenchConstants::TAG_FIXED, isFixed) && isFixed == 1);

  return true;
}

QList<QString> Perspective::GetShowInIdsFromRegistry()
{
  PerspectiveExtensionReader reader;
  QList<QString> tags;
  tags.push_back(WorkbenchRegistryConstants::TAG_SHOW_IN_PART);
  reader.SetIncludeOnlyTags(tags);
  PageLayout::Pointer layout(new PageLayout());
  reader.ExtendLayout(nullptr, descriptor->GetOriginalId(), layout);
  return layout->GetShowInPartIds();
}

void Perspective::SaveDesc()
{
  this->SaveDescAs(descriptor);
}

void Perspective::SaveDescAs(IPerspectiveDescriptor::Pointer desc)
{
  PerspectiveDescriptor::Pointer realDesc = desc.Cast<PerspectiveDescriptor>();
  //get the layout from the registry
  PerspectiveRegistry* perspRegistry = dynamic_cast<PerspectiveRegistry*>(
                                         WorkbenchPlugin::GetDefault()->GetPerspectiveRegistry());
  // Capture the layout state.
  XMLMemento::Pointer memento = XMLMemento::CreateWriteRoot("perspective");
  //IStatus::Pointer status = SaveState(memento, realDesc, false);
//  if (status->GetSeverity() == IStatus::ERROR_TYPE)
//  {
//    ErrorDialog.openError((Shell) 0, "Saving Problems",
//        "Unable to store layout state.",
//        status);
//    return;
//  }
  bool status = SaveState(memento, realDesc, false);
  if (!status)
  {
    QMessageBox::critical(nullptr, "Saving Problems", "Unable to store layout state.");
    return;
  }

  //save it to the preference store
  try
  {
    perspRegistry->SaveCustomPersp(realDesc, memento.GetPointer());
    descriptor = realDesc;
  }
  catch (const std::exception& /*e*/)
  {
    perspRegistry->DeletePerspective(realDesc);
    QMessageBox::critical(nullptr, "Saving Problems", "Unable to store layout state.");
  }
}

bool Perspective::SaveState(IMemento::Pointer memento)
{
//  MultiStatus::Pointer result(new MultiStatus(
//                                PlatformUI::PLUGIN_ID(),
//                                IStatus::OK_TYPE,
//                                "Problems occurred saving perspective.",
//                                BERRY_STATUS_LOC));
//  result->Merge(SaveState(memento, descriptor, true));

  bool result = true;
  result &= this->SaveState(memento, descriptor, true);

  return result;
}

bool Perspective::SaveState(IMemento::Pointer memento, PerspectiveDescriptor::Pointer p,
    bool saveInnerViewState)
{

//  MultiStatus::Pointer result(new MultiStatus(
//                                PlatformUI::PLUGIN_ID(),
//                                IStatus::OK_TYPE,
//                                "Problems occurred saving perspective.",
//                                BERRY_STATUS_LOC));
  bool result = true;

  if (this->memento)
  {
    memento->PutMemento(this->memento);
    return result;
  }

  // Save the version number.
  memento->PutString(WorkbenchConstants::TAG_VERSION, VERSION_STRING);
  //result.add(p.saveState(memento));
  result &= p->SaveState(memento);
  if (!saveInnerViewState)
  {
    QRect bounds(page->GetWorkbenchWindow()->GetShell()->GetBounds());
    IMemento::Pointer boundsMem = memento
    ->CreateChild(WorkbenchConstants::TAG_WINDOW);
    boundsMem->PutInteger(WorkbenchConstants::TAG_X, bounds.x());
    boundsMem->PutInteger(WorkbenchConstants::TAG_Y, bounds.y());
    boundsMem->PutInteger(WorkbenchConstants::TAG_HEIGHT, bounds.height());
    boundsMem->PutInteger(WorkbenchConstants::TAG_WIDTH, bounds.width());
  }

//  // Save the "always on" action sets.
//  Iterator itr = alwaysOnActionSets.iterator();
//  while (itr.hasNext())
//  {
//    IActionSetDescriptor desc = (IActionSetDescriptor) itr.next();
//    IMemento child = memento
//    .createChild(IWorkbenchConstants.TAG_ALWAYS_ON_ACTION_SET);
//    child.putString(IWorkbenchConstants.TAG_ID, desc.getId());
//  }

//  // Save the "always off" action sets.
//  itr = alwaysOffActionSets.iterator();
//  while (itr.hasNext())
//  {
//    IActionSetDescriptor desc = (IActionSetDescriptor) itr.next();
//    IMemento child = memento
//    .createChild(IWorkbenchConstants.TAG_ALWAYS_OFF_ACTION_SET);
//    child.putString(IWorkbenchConstants.TAG_ID, desc.getId());
//  }

  // Save "show view actions"
  for (QList<QString>::iterator itr = showViewShortcuts.begin();
      itr != showViewShortcuts.end(); ++itr)
  {
    IMemento::Pointer child = memento
    ->CreateChild(WorkbenchConstants::TAG_SHOW_VIEW_ACTION);
    child->PutString(WorkbenchConstants::TAG_ID, *itr);
  }

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

//  // Save "new wizard actions".
//  itr = newWizardShortcuts.iterator();
//  while (itr.hasNext())
//  {
//    String str = (String) itr.next();
//    IMemento child = memento
//    .createChild(IWorkbenchConstants.TAG_NEW_WIZARD_ACTION);
//    child.putString(IWorkbenchConstants.TAG_ID, str);
//  }

  // Save "perspective actions".
  for (QList<QString>::iterator itr = perspectiveShortcuts.begin();
      itr != perspectiveShortcuts.end(); ++itr)
  {
    IMemento::Pointer child = memento
    ->CreateChild(WorkbenchConstants::TAG_PERSPECTIVE_ACTION);
    child->PutString(WorkbenchConstants::TAG_ID, *itr);
  }

  // Get visible views.
  QList<PartPane::Pointer> viewPanes;
  presentation->CollectViewPanes(viewPanes);

  // Save the views.
  for (QList<PartPane::Pointer>::iterator itr = viewPanes.begin();
      itr != viewPanes.end(); ++itr)
  {
    IWorkbenchPartReference::Pointer ref((*itr)->GetPartReference());
    IViewDescriptor::Pointer desc = page->GetViewFactory()->GetViewRegistry()
          ->Find(ref->GetId());
    if(desc && desc->IsRestorable())
    {
      IMemento::Pointer viewMemento = memento
      ->CreateChild(WorkbenchConstants::TAG_VIEW);
      viewMemento->PutString(WorkbenchConstants::TAG_ID, ViewFactory::GetKey(ref.Cast<IViewReference>()));
    }
  }

//  // save all fastview state
//  if (fastViewManager != 0)
//  fastViewManager.saveState(memento);

  // Save the view layout recs.
  for (QHash<QString, ViewLayoutRec::Pointer>::iterator i = mapIDtoViewLayoutRec.begin();
      i != mapIDtoViewLayoutRec.end(); ++i)
  {
    QString compoundId(i.key());
    ViewLayoutRec::Pointer rec(i.value());
    if (rec && (!rec->isCloseable || !rec->isMoveable || rec->isStandalone))
    {
      IMemento::Pointer layoutMemento(memento
      ->CreateChild(WorkbenchConstants::TAG_VIEW_LAYOUT_REC));
      layoutMemento->PutString(WorkbenchConstants::TAG_ID, compoundId);
      if (!rec->isCloseable)
      {
        layoutMemento->PutString(WorkbenchConstants::TAG_CLOSEABLE,
            WorkbenchConstants::FALSE_VAL);
      }
      if (!rec->isMoveable)
      {
        layoutMemento->PutString(WorkbenchConstants::TAG_MOVEABLE,
            WorkbenchConstants::FALSE_VAL);
      }
      if (rec->isStandalone)
      {
        layoutMemento->PutString(WorkbenchConstants::TAG_STANDALONE,
            WorkbenchConstants::TRUE_VAL);
        layoutMemento->PutString(WorkbenchConstants::TAG_SHOW_TITLE,
            rec->showTitle ? WorkbenchConstants::TRUE_VAL : WorkbenchConstants::FALSE_VAL);
      }
    }
  }

  // Save the layout.
  IMemento::Pointer childMem(memento->CreateChild(WorkbenchConstants::TAG_LAYOUT));
  //result->Add(presentation->SaveState(childMem));
  result &= presentation->SaveState(childMem);

  // Save the editor visibility state
  if (this->IsEditorAreaVisible())
  {
    memento->PutInteger(WorkbenchConstants::TAG_AREA_VISIBLE, 1);
  }
  else
  {
    memento->PutInteger(WorkbenchConstants::TAG_AREA_VISIBLE, 0);
  }

//  // Save the trim state of the editor area if using the new min/max
//  IPreferenceStore preferenceStore = PrefUtil.getAPIPreferenceStore();
//  bool useNewMinMax = preferenceStore.getbool(IWorkbenchPreferenceConstants.ENABLE_NEW_MIN_MAX);
//  if (useNewMinMax)
//  {
//    int trimState = editorAreaState;
//    trimState |= editorAreaRestoreOnUnzoom ? 4 : 0;
//    memento.putInteger(IWorkbenchConstants.TAG_AREA_TRIM_STATE, trimState);
//  }

  // Save the fixed state
  if (fixed)
  {
    memento->PutInteger(WorkbenchConstants::TAG_FIXED, 1);
  }
  else
  {
    memento->PutInteger(WorkbenchConstants::TAG_FIXED, 0);
  }

  return result;
}

void Perspective::SetPerspectiveActionIds(const QList<QString>& list)
{
  perspectiveShortcuts = list;
}

void Perspective::SetShowInPartIds(const QList<QString>& list)
{
  showInPartIds = list;
}

void Perspective::SetShowViewActionIds(const QList<QString>& list)
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
  if (this->UseNewMinMax(Perspective::Pointer(this)))
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
  editorHolder = nullptr;
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

IViewReference::Pointer Perspective::GetViewReference(const QString& viewId, const QString& secondaryId)
{
  IViewReference::Pointer ref = page->FindViewReference(viewId, secondaryId);
  if (ref == 0)
  {
    ViewFactory* factory = this->GetViewFactory();
    try
    {
      ref = factory->CreateView(viewId, secondaryId);
    }
    catch (PartInitException& /*e*/)
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

IViewPart::Pointer Perspective::ShowView(const QString& viewId, const QString& secondaryId)
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
    if (this->UseNewMinMax(Perspective::Pointer(this)))
    {
      // Is this view going to show in the trim?
 //     LayoutPart vPart = presentation.findPart(viewId, secondaryId);

      // Determine if there is a trim stack that should get the view
      QString trimId;

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
  if (pane != 0 && pane->GetControl() != nullptr)
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

void Perspective::DescribeLayout(QString& buf) const
{
//  QList<IViewReference::Pointer> fastViews = getFastViews();
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
  if (factory == nullptr)
  return false;

  // Ok, we should be good to go, return the pref
  //IPreferenceStore preferenceStore = PrefUtil.getAPIPreferenceStore();
  //bool useNewMinMax = preferenceStore.getbool(IWorkbenchPreferenceConstants.ENABLE_NEW_MIN_MAX);
  return true;
}

}
