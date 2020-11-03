/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryLog.h"

#include "tweaklets/berryGuiWidgetsTweaklet.h"
#include "tweaklets/berryWorkbenchPageTweaklet.h"

#include "berryWorkbenchPage.h"
#include "berryPartSite.h"
#include "berryWorkbenchRegistryConstants.h"
#include "berryPerspective.h"
#include "berryLayoutPartSash.h"
#include "berryWorkbenchPlugin.h"
#include "berryEditorAreaHelper.h"
#include "berrySaveablesList.h"
#include "berryPerspectiveHelper.h"
#include "berryLayoutTreeNode.h"
#include "berryWorkbench.h"
#include "berryWorkbenchConstants.h"
#include "berryPartService.h"
#include "berryStickyViewManager.h"
#include "berryUIExtensionTracker.h"
#include "intro/berryIntroConstants.h"
#include "intro/berryViewIntroAdapterPart.h"

#include "berryWorkbenchWindow.h"
#include "berryUIException.h"
#include "berryPlatformUI.h"
#include "berryPartPane.h"

#include <berryPlatform.h>

#include <QMessageBox>

namespace berry
{

WorkbenchPage::ActivationOrderPred::ActivationOrderPred(
    WorkbenchPage::ActivationList* al) :
  activationList(al)
{

}

bool WorkbenchPage::ActivationOrderPred::operator()(
    const IViewReference::Pointer o1, const IViewReference::Pointer o2) const
{
  auto pos1 = activationList->IndexOf(
      o1.Cast<IWorkbenchPartReference> ());
  auto pos2 = activationList->IndexOf(
      o2.Cast<IWorkbenchPartReference> ());
  return pos1 < pos2;
}

void WorkbenchPage::PerspectiveList::UpdateActionSets(
    Perspective::Pointer /*oldPersp*/, Perspective::Pointer /*newPersp*/)
{
  //TODO WorkbenchPage action sets
  //        // Update action sets
  //
  //        IContextService service = (IContextService) window
  //            .getService(IContextService.class);
  //        try {
  //          service.activateContext(ContextAuthority.DEFER_EVENTS);
  //          if (newPersp != 0) {
  //            IActionSetDescriptor[] newAlwaysOn = newPersp
  //                .getAlwaysOnActionSets();
  //            for (int i = 0; i < newAlwaysOn.length; i++) {
  //              IActionSetDescriptor descriptor = newAlwaysOn[i];
  //
  //              actionSets.showAction(descriptor);
  //            }
  //
  //            IActionSetDescriptor[] newAlwaysOff = newPersp
  //                .getAlwaysOffActionSets();
  //            for (int i = 0; i < newAlwaysOff.length; i++) {
  //              IActionSetDescriptor descriptor = newAlwaysOff[i];
  //
  //              actionSets.maskAction(descriptor);
  //            }
  //          }
  //
  //          if (oldPersp != 0) {
  //            IActionSetDescriptor[] newAlwaysOn = oldPersp
  //                .getAlwaysOnActionSets();
  //            for (int i = 0; i < newAlwaysOn.length; i++) {
  //              IActionSetDescriptor descriptor = newAlwaysOn[i];
  //
  //              actionSets.hideAction(descriptor);
  //            }
  //
  //            IActionSetDescriptor[] newAlwaysOff = oldPersp
  //                .getAlwaysOffActionSets();
  //            for (int i = 0; i < newAlwaysOff.length; i++) {
  //              IActionSetDescriptor descriptor = newAlwaysOff[i];
  //
  //              actionSets.unmaskAction(descriptor);
  //            }
  //          }
  //        } finally {
  //          service.activateContext(ContextAuthority.SEND_EVENTS);
  //        }
}

WorkbenchPage::PerspectiveList::PerspectiveList()
{

}

void WorkbenchPage::PerspectiveList::Reorder(
    IPerspectiveDescriptor::Pointer perspective, int newLoc)
{
  PerspectiveListType::iterator oldLocation = openedList.end();
  Perspective::Pointer movedPerspective;
  for (PerspectiveListType::iterator iterator = openedList.begin(); iterator
      != openedList.end(); ++iterator)
  {
    Perspective::Pointer openPerspective = *iterator;
    if (openPerspective->GetDesc() == perspective)
    {
      oldLocation = std::find(openedList.begin(), openedList.end(),
          openPerspective);
      movedPerspective = openPerspective;
    }
  }

  PerspectiveListType::iterator newLocation = openedList.begin();
  for (int i = 0; i < newLoc; ++i, ++newLocation)
    ;
  if (oldLocation == newLocation)
  {
    return;
  }

  openedList.erase(oldLocation);
  openedList.insert(newLocation, movedPerspective);

}

WorkbenchPage::PerspectiveList::PerspectiveListType WorkbenchPage::PerspectiveList::GetSortedPerspectives()
{
  return usedList;
}

bool WorkbenchPage::PerspectiveList::Add(Perspective::Pointer perspective)
{
  openedList.push_back(perspective);
  usedList.push_front(perspective);
  //It will be moved to top only when activated.
  return true;
}

WorkbenchPage::PerspectiveList::PerspectiveListType::iterator WorkbenchPage::PerspectiveList::Begin()
{
  return openedList.begin();
}

WorkbenchPage::PerspectiveList::PerspectiveListType::iterator WorkbenchPage::PerspectiveList::End()
{
  return openedList.end();
}

WorkbenchPage::PerspectiveList::PerspectiveListType WorkbenchPage::PerspectiveList::GetOpenedPerspectives()
{
  return openedList;
}

bool WorkbenchPage::PerspectiveList::Remove(Perspective::Pointer perspective)
{
  if (active == perspective)
  {
    this->UpdateActionSets(active, Perspective::Pointer(nullptr));
    active = nullptr;
  }
  usedList.removeAll(perspective);
  PerspectiveListType::size_type origSize = openedList.size();
  openedList.removeAll(perspective);
  return openedList.size() != origSize;
}

void WorkbenchPage::PerspectiveList::Swap(Perspective::Pointer oldPerspective,
    Perspective::Pointer newPerspective)
{
  PerspectiveListType::iterator oldIter = std::find(openedList.begin(),
      openedList.end(), oldPerspective);
  PerspectiveListType::iterator newIter = std::find(openedList.begin(),
      openedList.end(), newPerspective);

  if (oldIter == openedList.end() || newIter == openedList.end())
  {
    return;
  }

  std::iter_swap(oldIter, newIter);
}

bool WorkbenchPage::PerspectiveList::IsEmpty()
{
  return openedList.empty();
}

Perspective::Pointer WorkbenchPage::PerspectiveList::GetActive()
{
  return active;
}

Perspective::Pointer WorkbenchPage::PerspectiveList::GetNextActive()
{
  if (active == 0)
  {
    if (usedList.empty())
    {
      return Perspective::Pointer(nullptr);
    }
    else
    {
      return usedList.back();
    }
  }
  else
  {
    if (usedList.size() < 2)
    {
      return Perspective::Pointer(nullptr);
    }
    else
    {
      return *(usedList.end() - 2);
    }
  }
}

WorkbenchPage::PerspectiveList::PerspectiveListType::size_type WorkbenchPage::PerspectiveList::Size()
{
  return openedList.size();
}

void WorkbenchPage::PerspectiveList::SetActive(Perspective::Pointer perspective)
{
  if (perspective == active)
  {
    return;
  }

  this->UpdateActionSets(active, perspective);
  active = perspective;

  if (perspective != 0)
  {
    usedList.removeAll(perspective);
    usedList.push_back(perspective);
  }
}

WorkbenchPage::ActivationList::ActivationList(WorkbenchPage* page) :
  page(page)
{

}

void WorkbenchPage::ActivationList::SetActive(SmartPointer<IWorkbenchPart> part)
{
  if (parts.empty())
  {
    return;
  }
  IWorkbenchPartReference::Pointer ref(page->GetReference(part));
  if (ref)
  {
    if (ref == parts.back())
    {
      return;
    }
    parts.erase(std::find(parts.begin(), parts.end(), ref));
    parts.push_back(ref);
  }
}

void WorkbenchPage::ActivationList::BringToTop(SmartPointer<
    IWorkbenchPartReference> ref)
{
  ILayoutContainer::Pointer targetContainer(page->GetContainer(ref));

  auto newIndex = this->LastIndexOfContainer(targetContainer);

  if (newIndex != parts.end() && ref == *newIndex)
  {
    return;
  }

  if (newIndex == parts.end())
  {
    parts.push_back(ref);
  }
  else
  {
    PartListType::size_type index = newIndex - parts.begin();
    parts.erase(std::find(parts.begin(), parts.end(), ref));
    auto insertIndex = parts.begin() + index;
    parts.insert(insertIndex, ref);
  }
}

WorkbenchPage::ActivationList::PartListIter WorkbenchPage::ActivationList::LastIndexOfContainer(
    SmartPointer<ILayoutContainer> container)
{
  auto i = parts.rbegin();
  while (i != parts.rend())
  {
    IWorkbenchPartReference::Pointer ref(*i);
    ILayoutContainer::Pointer cnt(page->GetContainer(ref));
    if (cnt == container)
    {
      return --i.base();
    }
    ++i;
  }

  return parts.end();
}

void WorkbenchPage::ActivationList::SetActive(SmartPointer<
    IWorkbenchPartReference> ref)
{
  this->SetActive(ref->GetPart(true));
}

void WorkbenchPage::ActivationList::Add(
    SmartPointer<IWorkbenchPartReference> ref)
{
  if (std::find(parts.begin(), parts.end(), ref) != parts.end())
  {
    return;
  }

  ref->GetPart(false);
  parts.push_front(ref);
}

SmartPointer<IWorkbenchPart> WorkbenchPage::ActivationList::GetActive()
{
  if (parts.empty())
  {
    return IWorkbenchPart::Pointer(nullptr);
  }
  return this->GetActive(parts.end());
}

SmartPointer<IWorkbenchPart> WorkbenchPage::ActivationList::GetPreviouslyActive()
{
  if (parts.size() < 2)
  {
    return IWorkbenchPart::Pointer(nullptr);
  }
  return this->GetActive(--parts.end());
}

SmartPointer<IWorkbenchPartReference> WorkbenchPage::ActivationList::GetActiveReference(
    bool editorsOnly)
{
  return this->GetActiveReference(parts.end(), editorsOnly);
}

WorkbenchPage::ActivationList::PartListIter WorkbenchPage::ActivationList::IndexOf(
    SmartPointer<IWorkbenchPart> part)
{
  IWorkbenchPartReference::Pointer ref(page->GetReference(part));
  if (ref == 0)
  {
    return parts.end();
  }
  return std::find(parts.begin(), parts.end(), ref);
}

WorkbenchPage::ActivationList::PartListIter WorkbenchPage::ActivationList::IndexOf(
    SmartPointer<IWorkbenchPartReference> ref)
{
  return std::find(parts.begin(), parts.end(), ref);
}

bool WorkbenchPage::ActivationList::Remove(
    SmartPointer<IWorkbenchPartReference> ref)
{
  bool contains = std::find(parts.begin(), parts.end(), ref) != parts.end();
  parts.erase(std::find(parts.begin(), parts.end(), ref));
  return contains;
}

SmartPointer<IEditorPart> WorkbenchPage::ActivationList::GetTopEditor()
{
  IEditorReference::Pointer editor =
      this->GetActiveReference(parts.end(), true).Cast<IEditorReference> ();

  if (editor == 0)
  {
    return IEditorPart::Pointer(nullptr);
  }

  return editor->GetEditor(true);
}

SmartPointer<IWorkbenchPart> WorkbenchPage::ActivationList::GetActive(
    PartListIter start)
{
  IWorkbenchPartReference::Pointer ref(this->GetActiveReference(start, false));

  if (!ref)
  {
    return IWorkbenchPart::Pointer(nullptr);
  }

  return ref->GetPart(true);
}

SmartPointer<IWorkbenchPartReference> WorkbenchPage::ActivationList::GetActiveReference(
    PartListIter start, bool editorsOnly)
{
  // First look for parts that aren't obscured by the current zoom state
  IWorkbenchPartReference::Pointer nonObscured = this->GetActiveReference(
      start, editorsOnly, true);

  if (nonObscured)
  {
    return nonObscured;
  }

  // Now try all the rest of the parts
  return this->GetActiveReference(start, editorsOnly, false);
}

SmartPointer<IWorkbenchPartReference> WorkbenchPage::ActivationList::GetActiveReference(
    PartListIter start, bool editorsOnly, bool /*skipPartsObscuredByZoom*/)
{
  QList<IViewReference::Pointer> views = page->GetViewReferences();
  PartListReverseIter i(start);
  while (i != parts.rend())
  {
    WorkbenchPartReference::Pointer ref(i->Cast<WorkbenchPartReference> ());

    if (editorsOnly && (ref.Cast<IEditorReference> () == 0))
    {
      ++i;
      continue;
    }

    // Skip parts whose containers have disabled auto-focus
    PartPane::Pointer pane(ref->GetPane());

    if (pane)
    {
      if (!pane->AllowsAutoFocus())
      {
        ++i;
        continue;
      }

      //                    if (skipPartsObscuredByZoom) {
      //                        if (pane.isObscuredByZoom()) {
      //                            continue;
      //                        }
      //                    }
    }

    // Skip fastviews (unless overridden)
    if (IViewReference::Pointer viewRef = ref.Cast<IViewReference>())
    {
      //if (ref == getActiveFastView() || !((IViewReference) ref).isFastView()) {
      for (int j = 0; j < views.size(); j++)
      {
        if (views[j] == viewRef)
        {
          return viewRef.Cast<IWorkbenchPartReference> ();
        }
      }
      //}
    }
    else
    {
      return ref.Cast<IWorkbenchPartReference> ();
    }
    ++i;
  }
  return IWorkbenchPartReference::Pointer(nullptr);
}

QList<SmartPointer<IEditorReference> > WorkbenchPage::ActivationList::GetEditors()
{
  QList<IEditorReference::Pointer> editors;
  for (auto i = parts.begin(); i != parts.end(); ++i)
  {
    if (IEditorReference::Pointer part = i->Cast<IEditorReference>())
    {
      editors.push_back(part);
    }
  }
  return editors;
}

QList<SmartPointer<IWorkbenchPartReference> > WorkbenchPage::ActivationList::GetParts()
{
  QList<IViewReference::Pointer> views(page->GetViewReferences());
  QList<IWorkbenchPartReference::Pointer> resultList;
  for (auto iterator = parts.begin(); iterator != parts.end(); ++iterator)
  {

    if (IViewReference::Pointer ref = iterator->Cast<IViewReference>())
    {
      //Filter views from other perspectives
      for (int i = 0; i < views.size(); i++)
      {
        if (ref == views[i])
        {
          resultList.push_back(ref);
          break;
        }
      }
    }
    else
    {
      resultList.push_back(*iterator);
    }
  }
  return resultList;
}

WorkbenchPage::ActionSwitcher::ActionSwitcher(WorkbenchPage* page)
  : page(page)
{
}

void WorkbenchPage::ActionSwitcher::UpdateActivePart(
    IWorkbenchPart::Pointer newPart)
{
  IWorkbenchPart::Pointer _activePart = this->activePart.Lock();
  IEditorPart::Pointer _topEditor = this->topEditor.Lock();

  if (_activePart == newPart)
  {
    return;
  }

  bool isNewPartAnEditor = newPart.Cast<IEditorPart> ().IsNotNull();
  if (isNewPartAnEditor)
  {
    QString oldId;
    if (_topEditor)
    {
      oldId = _topEditor->GetSite()->GetId();
    }
    QString newId = newPart->GetSite()->GetId();

    // if the active part is an editor and the new editor
    // is the same kind of editor, then we don't have to do
    // anything
    if (activePart == topEditor && newId == oldId)
    {
      activePart = newPart;
      topEditor = newPart.Cast<IEditorPart> ();
      return;
    }

    // remove the contributions of the old editor
    // if it is a different kind of editor
    if (oldId != newId)
    {
      this->DeactivateContributions(_topEditor, true);
    }

    // if a view was the active part, disable its contributions
    if (_activePart && _activePart != _topEditor)
    {
      this->DeactivateContributions(_activePart, true);
    }

    // show (and enable) the contributions of the new editor
    // if it is a different kind of editor or if the
    // old active part was a view
    if (newId != oldId || _activePart != _topEditor)
    {
      this->ActivateContributions(newPart, true);
    }

  }
  else if (newPart.IsNull())
  {
    if (_activePart)
    {
      // remove all contributions
      this->DeactivateContributions(_activePart, true);
    }
  }
  else
  {
    // new part is a view

    // if old active part is a view, remove all contributions,
    // but if old part is an editor only disable
    if (_activePart)
    {
      this->DeactivateContributions(_activePart,
          _activePart.Cast<IViewPart> ().IsNotNull());
    }

    this->ActivateContributions(newPart, true);
  }

  //TODO WorkbenchPage action sets
  //  ArrayList newActionSets = 0;
  //  if (isNewPartAnEditor || (activePart == topEditor && newPart == 0))
  //  {
  //    newActionSets = calculateActionSets(newPart, 0);
  //  }
  //  else
  //  {
  //    newActionSets = calculateActionSets(newPart, topEditor);
  //  }
  //
  //  if (!updateActionSets(newActionSets))
  //  {
  page->UpdateActionBars();
  //  }

  if (isNewPartAnEditor)
  {
    topEditor = newPart.Cast<IEditorPart> ();
  }
  else if (activePart == topEditor && newPart.IsNull())
  {
    // since we removed all the contributions, we clear the top
    // editor
    topEditor.Reset();
  }

  activePart = newPart;
}

void WorkbenchPage::ActionSwitcher::UpdateTopEditor(
    IEditorPart::Pointer newEditor)
{
  if (topEditor.Lock() == newEditor)
  {
    return;
  }

  if (activePart == topEditor)
  {
    this->UpdateActivePart(newEditor);
    return;
  }

  QString oldId;
  if (!topEditor.Expired())
  {
    oldId = topEditor.Lock()->GetSite()->GetId();
  }
  QString newId;
  if (newEditor.IsNotNull())
  {
    newId = newEditor->GetSite()->GetId();
  }
  if (oldId == newId)
  {
    // we don't have to change anything
    topEditor = newEditor;
    return;
  }

  // Remove the contributions of the old editor
  if (!topEditor.Expired())
  {
    this->DeactivateContributions(topEditor.Lock(), true);
  }

  // Show (disabled) the contributions of the new editor
  if (newEditor.IsNotNull())
  {
    this->ActivateContributions(newEditor, false);
  }

  //  ArrayList newActionSets = calculateActionSets(activePart, newEditor);
  //  if (!updateActionSets(newActionSets))
  //  {
  page->UpdateActionBars();
  //  }

  topEditor = newEditor;
}

void WorkbenchPage::ActionSwitcher::ActivateContributions(
    IWorkbenchPart::Pointer /*part*/, bool /*enable*/)
{
  //PartSite::Pointer site = part->GetSite().Cast<PartSite> ();
  //site->ActivateActionBars(enable);
}

void WorkbenchPage::ActionSwitcher::DeactivateContributions(
    IWorkbenchPart::Pointer /*part*/, bool /*remove*/)
{
  //PartSite::Pointer site = part->GetSite().Cast<PartSite> ();
  //site->DeactivateActionBars(remove);
}

IExtensionPoint::Pointer WorkbenchPage::GetPerspectiveExtensionPoint()
{
  return Platform::GetExtensionRegistry()->GetExtensionPoint(
        PlatformUI::PLUGIN_ID(), WorkbenchRegistryConstants::PL_PERSPECTIVE_EXTENSIONS);
}

WorkbenchPage::WorkbenchPage(WorkbenchWindow* w, const QString& layoutID,
                             IAdaptable* input)
  : actionSwitcher(this)
{
  if (layoutID == "")
  {
    throw WorkbenchException("Perspective ID is undefined");
  }

  this->Register();
  this->Init(w, layoutID, input, true);
  this->UnRegister(false);
}

WorkbenchPage::WorkbenchPage(WorkbenchWindow* w, IAdaptable* input)
  : actionSwitcher(this)
{
  this->Register();
  this->Init(w, "", input, false);
  this->UnRegister(false);
}

void WorkbenchPage::Activate(IWorkbenchPart::Pointer part)
{
  // Sanity check.
  if (!this->CertifyPart(part))
  {
    return;
  }

  if (window->IsClosing())
  {
    return;
  }

  //  if (composite!=0 && composite.isVisible() && !((GrabFocus)Tweaklets.get(GrabFocus.KEY)).grabFocusAllowed(part))
  //  {
  //    return;
  //  }

  // Activate part.
  //if (window.getActivePage() == this) {
  IWorkbenchPartReference::Pointer ref = this->GetReference(part);
  this->InternalBringToTop(ref);
  this->SetActivePart(part);
}

void WorkbenchPage::ActivatePart(const IWorkbenchPart::Pointer part)
{
  //  Platform.run(new SafeRunnable(WorkbenchMessages.WorkbenchPage_ErrorActivatingView)
  //      {
  //      public void WorkbenchPage::run()
  //        {
  if (part.IsNotNull())
  {
    //part.setFocus();
    PartPane::Pointer pane = this->GetPane(part);
    pane->SetFocus();
    PartSite::Pointer site = part->GetSite().Cast<PartSite> ();
    pane->ShowFocus(true);
    //this->UpdateTabList(part);
    //SubActionBars bars = (SubActionBars) site.getActionBars();
    //bars.partChanged(part);
  }
  //        }
  //      }
  //      );
}

void WorkbenchPage::AddPartListener(IPartListener* l)
{
  partList->GetPartService()->AddPartListener(l);
}

void WorkbenchPage::AddSelectionListener(ISelectionListener* listener)
{
  selectionService->AddSelectionListener(listener);
}

void WorkbenchPage::AddSelectionListener(const QString& partId,
    ISelectionListener* listener)
{
  selectionService->AddSelectionListener(partId, listener);
}

void WorkbenchPage::AddPostSelectionListener(
    ISelectionListener* listener)
{
  selectionService->AddPostSelectionListener(listener);
}

void WorkbenchPage::AddPostSelectionListener(const QString& partId,
    ISelectionListener* listener)
{
  selectionService->AddPostSelectionListener(partId, listener);
}

ILayoutContainer::Pointer WorkbenchPage::GetContainer(
    IWorkbenchPart::Pointer part)
{
  PartPane::Pointer pane = this->GetPane(part);
  if (pane == 0)
  {
    return ILayoutContainer::Pointer(nullptr);
  }

  return pane->GetContainer();
}

ILayoutContainer::Pointer WorkbenchPage::GetContainer(
    IWorkbenchPartReference::Pointer part)
{
  PartPane::Pointer pane = this->GetPane(part);
  if (pane == 0)
  {
    return ILayoutContainer::Pointer(nullptr);
  }

  return pane->GetContainer();
}

PartPane::Pointer WorkbenchPage::GetPane(IWorkbenchPart::Pointer part)
{
  if (part.IsNull())
  {
    return PartPane::Pointer(nullptr);
  }
  return this->GetPane(this->GetReference(part));
}

PartPane::Pointer WorkbenchPage::GetPane(IWorkbenchPartReference::Pointer part)
{
  if (part.IsNull())
  {
    return PartPane::Pointer(nullptr);
  }

  return part.Cast<WorkbenchPartReference> ()->GetPane();
}

bool WorkbenchPage::InternalBringToTop(IWorkbenchPartReference::Pointer part)
{

  bool broughtToTop = false;

  // Move part.
  if (part.Cast<IEditorReference> ().IsNotNull())
  {
    ILayoutContainer::Pointer container = this->GetContainer(part);
    if (container.Cast<PartStack> () != 0)
    {
      PartStack::Pointer stack = container.Cast<PartStack> ();
      PartPane::Pointer newPart = this->GetPane(part);
      if (stack->GetSelection() != newPart)
      {
        stack->SetSelection(newPart);
      }
      broughtToTop = true;
    }
  }
  else if (part.Cast<IViewReference> ().IsNotNull())
  {
    Perspective::Pointer persp = this->GetActivePerspective();
    if (persp != 0)
    {
      broughtToTop = persp->BringToTop(part.Cast<IViewReference> ());
    }
  }

  // Ensure that this part is considered the most recently activated part
  // in this stack
  activationList->BringToTop(part);

  return broughtToTop;
}

void WorkbenchPage::BringToTop(IWorkbenchPart::Pointer part)
{
  // Sanity check.
  Perspective::Pointer persp = this->GetActivePerspective();
  if (persp == 0 || !this->CertifyPart(part))
  {
    return;
  }

  //  if (!((GrabFocus)Tweaklets.get(GrabFocus.KEY)).grabFocusAllowed(part))
  //  {
  //    return;
  //  }

  //  QString label; // debugging only
  //  if (UIStats.isDebugging(UIStats.BRING_PART_TO_TOP))
  //  {
  //    label = part != 0 ? part.getTitle() : "none"; //$NON-NLS-1$
  //  }

  IWorkbenchPartReference::Pointer ref = this->GetReference(part);
  ILayoutContainer::Pointer activeEditorContainer = this->GetContainer(
      this->GetActiveEditor().Cast<IWorkbenchPart> ());
  ILayoutContainer::Pointer activePartContainer = this->GetContainer(
      this->GetActivePart());
  ILayoutContainer::Pointer newPartContainer = this->GetContainer(part);

  if (newPartContainer == activePartContainer)
  {
    this->MakeActive(ref);
  }
  else if (newPartContainer == activeEditorContainer)
  {
    if (ref.Cast<IEditorReference> () != 0)
    {
      if (part != 0)
      {
        IWorkbenchPartSite::Pointer site = part->GetSite();
        if (site.Cast<PartSite> () != 0)
        {
          ref = site.Cast<PartSite> ()->GetPane()->GetPartReference();
        }
      }
      this->MakeActiveEditor(ref.Cast<IEditorReference> ());
    }
    else
    {
      this->MakeActiveEditor(IEditorReference::Pointer(nullptr));
    }
  }
  else
  {
    this->InternalBringToTop(ref);
    if (ref != 0)
    {
      partList->FirePartBroughtToTop(ref);
    }
  }

}

void WorkbenchPage::BusyResetPerspective()
{

  ViewIntroAdapterPart::Pointer
      introViewAdapter =
          dynamic_cast<WorkbenchIntroManager*> (GetWorkbenchWindow() ->GetWorkbench()->GetIntroManager())->GetIntroAdapterPart().Cast<
              ViewIntroAdapterPart> ();
  //  PartPane introPane = 0;
  //  boolean introFullScreen = false;
  //  if (introViewAdapter != 0)
  //  {
  //    introPane = ((PartSite) introViewAdapter.getSite()).getPane();
  //    introViewAdapter.setHandleZoomEvents(false);
  //    introFullScreen = introPane.isZoomed();
  //  }

  //  //try to prevent intro flicker.
  //  if (introFullScreen)
  //  {
  //    window.getShell().setRedraw(false);
  //  }

  //  try
  //  {

  //    // Always unzoom
  //    if (isZoomed())
  //    {
  //      zoomOut();
  //    }

  // Get the current perspective.
  // This describes the working layout of the page and differs from
  // the original template.
  Perspective::Pointer oldPersp = this->GetActivePerspective();

  // Map the current perspective to the original template.
  // If the original template cannot be found then it has been deleted.
  // In that case just return. (PR#1GDSABU).
  IPerspectiveRegistry* reg =
      WorkbenchPlugin::GetDefault() ->GetPerspectiveRegistry();
  PerspectiveDescriptor::Pointer desc = reg->FindPerspectiveWithId(
      oldPersp->GetDesc()->GetId()).Cast<PerspectiveDescriptor> ();
  if (desc == 0)
  {
    desc
        = reg->FindPerspectiveWithId(oldPersp ->GetDesc().Cast<
            PerspectiveDescriptor> ()->GetOriginalId()).Cast<
            PerspectiveDescriptor> ();
  }
  if (desc == 0)
  {
    return;
  }

  // Notify listeners that we are doing a reset.
  window->FirePerspectiveChanged(IWorkbenchPage::Pointer(this), desc,
      CHANGE_RESET);

  // Create new persp from original template.
  // Suppress the perspectiveOpened and perspectiveClosed events otherwise it looks like two
  // instances of the same perspective are open temporarily (see bug 127470).
  Perspective::Pointer newPersp = this->CreatePerspective(desc, false);
  if (newPersp == 0)
  {
    // We're not going through with the reset, so it is complete.
    window->FirePerspectiveChanged(IWorkbenchPage::Pointer(this), desc,
        CHANGE_RESET_COMPLETE);
    return;
  }

  // Update the perspective list and shortcut
  perspList.Swap(oldPersp, newPersp);

  // Install new persp.
  this->SetPerspective(newPersp);

  // Destroy old persp.
  this->DisposePerspective(oldPersp, false);

  // Update the Coolbar layout.
  this->ResetToolBarLayout();

  // restore the maximized intro
  if (introViewAdapter)
  {
    try
    {
      // ensure that the intro is visible in the new perspective
      ShowView(IntroConstants::INTRO_VIEW_ID);
      //      if (introFullScreen)
      //      {
      //        toggleZoom(introPane.getPartReference());
      //      }
    } catch (PartInitException& e)
    {
      //TODO IStatus
      WorkbenchPlugin::Log("Could not restore intro", e);
      // WorkbenchPlugin.getStatus(e));
    }
    //      finally
    //      {
    //        // we want the intro back to a normal state before we fire the event
    //        introViewAdapter.setHandleZoomEvents(true);
    //      }
  }
  // Notify listeners that we have completed our reset.
  window->FirePerspectiveChanged(IWorkbenchPage::Pointer(this), desc,
      CHANGE_RESET_COMPLETE);
  //  }
  //  finally
  //  {
  //    // reset the handling of zoom events (possibly for the second time) in case there was
  //    // an exception thrown
  //    if (introViewAdapter != 0)
  //    {
  //      introViewAdapter.setHandleZoomEvents(true);
  //    }
  //
  //    if (introFullScreen)
  //    {
  //      window.getShell().setRedraw(true);
  //    }
  //  }
}

void WorkbenchPage::RemovePerspective(IPerspectiveDescriptor::Pointer desc)
{
  Perspective::Pointer newPersp;
  PerspectiveDescriptor::Pointer realDesc = desc.Cast<PerspectiveDescriptor> ();
  newPersp = this->FindPerspective(desc);
  perspList.Remove(newPersp);
}

void WorkbenchPage::BusySetPerspective(IPerspectiveDescriptor::Pointer desc)
{
  // Create new layout.
  QString label = desc->GetId(); // debugging only
  Perspective::Pointer newPersp;
  //try
  //{
  //UIStats.start(UIStats.SWITCH_PERSPECTIVE, label);
  PerspectiveDescriptor::Pointer realDesc = desc.Cast<PerspectiveDescriptor> ();
  newPersp = this->FindPerspective(realDesc);
  if (newPersp == 0)
  {
    newPersp = this->CreatePerspective(realDesc, true);
    if (newPersp == 0)
    {
      return;
    }
  }

  // Change layout.
  this->SetPerspective(newPersp);
  //  }
  //  catch (std::exception& e)
  //  {
  //    UIStats.end(UIStats.SWITCH_PERSPECTIVE, desc.getId(), label);
  //    throw e;
  //  }
}

IViewPart::Pointer WorkbenchPage::BusyShowView(const QString& viewID,
    const QString& secondaryID, int mode)
{
  Perspective::Pointer persp = this->GetActivePerspective();
  if (persp == 0)
  {
    return IViewPart::Pointer(nullptr);
  }

  // If this view is already visible just return.
  IViewReference::Pointer ref = persp->FindView(viewID, secondaryID);
  IViewPart::Pointer view;
  if (ref != 0)
  {
    view = ref->GetView(true);
  }
  if (view != 0)
  {
    this->BusyShowView(view, mode);
    return view;
  }

  // Show the view.
  view = persp->ShowView(viewID, secondaryID);
  if (view != 0)
  {
    this->BusyShowView(view, mode);

    IWorkbenchPartReference::Pointer partReference = this->GetReference(view);
    PartPane::Pointer partPane = this->GetPane(partReference);
    partPane->SetInLayout(true);

    IWorkbenchPage::Pointer thisPage(this);
    window->FirePerspectiveChanged(thisPage, GetPerspective(), partReference,
        CHANGE_VIEW_SHOW);
    window->FirePerspectiveChanged(thisPage, GetPerspective(), CHANGE_VIEW_SHOW);
  }
  return view;
}

void WorkbenchPage::UpdateActionBars()
{
  window->UpdateActionBars();
}

void WorkbenchPage::BusyShowView(IViewPart::Pointer part, int mode)
{
  //  if (!((GrabFocus) Tweaklets.get(GrabFocus.KEY)).grabFocusAllowed(part))
  //  {
  //    return;
  //  }
  if (mode == VIEW_ACTIVATE)
  {
    this->Activate(part);
  }
  else if (mode == VIEW_VISIBLE)
  {
    IWorkbenchPartReference::Pointer ref = this->GetActivePartReference();
    // if there is no active part or it's not a view, bring to top
    if (ref == 0 || ref.Cast<IViewReference> () == 0)
    {
      this->BringToTop(part);
    }
    else
    {
      // otherwise check to see if the we're in the same stack as the active view
      IViewReference::Pointer activeView = ref.Cast<IViewReference> ();
      QList<IViewReference::Pointer> viewStack =
          this->GetViewReferenceStack(part);
      for (int i = 0; i < viewStack.size(); i++)
      {
        if (viewStack[i] == activeView)
        {
          return;
        }
      }
      this->BringToTop(part);
    }
  }
}

bool WorkbenchPage::CertifyPart(IWorkbenchPart::Pointer part)
{
  //Workaround for bug 22325
  if (part != 0 && part->GetSite().Cast<PartSite> () == 0)
  {
    return false;
  }

  if (part.Cast<IEditorPart> () != 0)
  {
    IEditorReference::Pointer ref = this->GetReference(part).Cast<
        IEditorReference> ();
    return ref != 0 && this->GetEditorManager()->ContainsEditor(ref);
  }
  if (part.Cast<IViewPart> () != 0)
  {
    Perspective::Pointer persp = this->GetActivePerspective();
    return persp != 0 && persp->ContainsView(part.Cast<IViewPart> ());
  }
  return false;
}

bool WorkbenchPage::Close()
{
  bool ret;
  //BusyIndicator.showWhile(0, new Runnable()
  //    {
  //    public void WorkbenchPage::run()
  //      {
  ret = window->ClosePage(IWorkbenchPage::Pointer(this), true);
  //      }
  //    });
  return ret;
}

bool WorkbenchPage::CloseAllSavedEditors()
{
  // get the Saved editors
  QList<IEditorReference::Pointer> editors = this->GetEditorReferences();
  QList<IEditorReference::Pointer> savedEditors;
  for (QList<IEditorReference::Pointer>::iterator iter = editors.begin(); iter
      != editors.end(); ++iter)
  {
    IEditorReference::Pointer editor = *iter;
    if (!editor->IsDirty())
    {
      savedEditors.push_back(editor);
    }
  }
  //there are no unsaved editors
  if (savedEditors.empty())
  {
    return true;
  }

  return this->CloseEditors(savedEditors, false);
}

bool WorkbenchPage::CloseAllEditors(bool save)
{
  return this->CloseEditors(this->GetEditorReferences(), save);
}

void WorkbenchPage::UpdateActivePart()
{

  if (this->IsDeferred())
  {
    return;
  }

  IWorkbenchPartReference::Pointer oldActivePart =
      partList->GetActivePartReference();
  IWorkbenchPartReference::Pointer oldActiveEditor =
      partList->GetActiveEditorReference();
  IWorkbenchPartReference::Pointer newActivePart;
  IEditorReference::Pointer newActiveEditor;

  if (!window->IsClosing())
  {
    // If an editor is active, try to keep an editor active
    if (oldActiveEditor && oldActivePart == oldActiveEditor)
    {
      newActiveEditor = activationList->GetActiveReference(true).Cast<
          IEditorReference> ();
      newActivePart = newActiveEditor;
      if (newActivePart == 0)
      {
        // Only activate a non-editor if there's no editors left
        newActivePart = activationList->GetActiveReference(false);
      }
    }
    else
    {
      // If a non-editor is active, activate whatever was activated most recently
      newActivePart = activationList->GetActiveReference(false);

      if (newActivePart.Cast<IEditorReference> () != 0)
      {
        // If that happens to be an editor, make it the active editor as well
        newActiveEditor = newActivePart.Cast<IEditorReference> ();
      }
      else
      {
        // Otherwise, select whatever editor was most recently active
        newActiveEditor = activationList->GetActiveReference(true).Cast<
            IEditorReference> ();
      }
    }
  }

  if (oldActiveEditor != newActiveEditor)
  {
    this->MakeActiveEditor(newActiveEditor);
  }

  if (newActivePart != oldActivePart)
  {
    this->MakeActive(newActivePart);
  }
}

void WorkbenchPage::MakeActive(IWorkbenchPartReference::Pointer ref)
{
  if (ref == 0)
  {
    this->SetActivePart(IWorkbenchPart::Pointer(nullptr));
  }
  else
  {
    IWorkbenchPart::Pointer newActive = ref->GetPart(true);
    if (newActive == 0)
    {
      this->SetActivePart(IWorkbenchPart::Pointer(nullptr));
    }
    else
    {
      this->Activate(newActive);
    }
  }
}

void WorkbenchPage::MakeActiveEditor(IEditorReference::Pointer ref)
{
  if (ref == this->GetActiveEditorReference())
  {
    return;
  }

  IEditorPart::Pointer part = (ref == 0) ? IEditorPart::Pointer(nullptr)
      : ref->GetEditor(true);

  if (part)
  {
    editorMgr->SetVisibleEditor(ref, false);
    //navigationHistory.MarkEditor(part);
  }

  actionSwitcher.UpdateTopEditor(part);

  if (ref)
  {
    activationList->BringToTop(this->GetReference(part));
  }

  partList->SetActiveEditor(ref);
}

bool WorkbenchPage::CloseEditors(
    const QList<IEditorReference::Pointer>& refArray, bool save)
{
  if (refArray.empty())
  {
    return true;
  }

  IWorkbenchPage::Pointer thisPage(this);

  // Check if we're being asked to close any parts that are already closed or cannot
  // be closed at this time
  QList<IEditorReference::Pointer> editorRefs;
  for (QList<IEditorReference::Pointer>::const_iterator iter =
      refArray.begin(); iter != refArray.end(); ++iter)
  {
    IEditorReference::Pointer reference = *iter;

    // If we're in the middle of creating this part, this is a programming error. Abort the entire
    // close operation. This usually occurs if someone tries to open a dialog in a method that
    // isn't allowed to do so, and a *syncExec tries to close the part. If this shows up in a log
    // file with a dialog's event loop on the stack, then the code that opened the dialog is usually
    // at fault.
    if (partBeingActivated == reference)
    {
      ctkRuntimeException re(
          "WARNING: Blocked recursive attempt to close part "
              + partBeingActivated->GetId()
              + " while still in the middle of activating it");
      WorkbenchPlugin::Log(re);
      return false;
    }

    //    if (reference.Cast<WorkbenchPartReference> () != 0)
    //    {
    //      WorkbenchPartReference::Pointer ref = reference.Cast<WorkbenchPartReference>();
    //
    //      // If we're being asked to close a part that is disposed (ie: already closed),
    //      // skip it and proceed with closing the remaining parts.
    //      if (ref.isDisposed())
    //      {
    //        continue;
    //      }
    //    }

    editorRefs.push_back(reference);
  }

  // notify the model manager before the close
  QList<IWorkbenchPart::Pointer> partsToClose;
  for (int i = 0; i < editorRefs.size(); i++)
  {
    IWorkbenchPart::Pointer refPart = editorRefs[i]->GetPart(false);
    if (refPart != 0)
    {
      partsToClose.push_back(refPart);
    }
  }
  SaveablesList::Pointer modelManager;
  SaveablesList::PostCloseInfo::Pointer postCloseInfo;
  if (partsToClose.size() > 0)
  {
    modelManager = dynamic_cast<SaveablesList*>(
          this->GetWorkbenchWindow()->GetService<ISaveablesLifecycleListener>());
    // this may prompt for saving and return 0 if the user canceled:
    postCloseInfo = modelManager->PreCloseParts(partsToClose, save,
        this->GetWorkbenchWindow());
    if (postCloseInfo == 0)
    {
      return false;
    }
  }

  // Fire pre-removal changes
  for (int i = 0; i < editorRefs.size(); i++)
  {
    IEditorReference::Pointer ref = editorRefs[i];

    // Notify interested listeners before the close
    window->FirePerspectiveChanged(thisPage, this->GetPerspective(), ref,
        CHANGE_EDITOR_CLOSE);

  }

  this->DeferUpdates(true);
  try
  {
    if (modelManager != 0)
    {
      modelManager->PostClose(postCloseInfo);
    }

    // Close all editors.
    for (int i = 0; i < editorRefs.size(); i++)
    {
      IEditorReference::Pointer ref = editorRefs[i];

      // Remove editor from the presentation
      editorPresentation->CloseEditor(ref);

      this->PartRemoved(ref.Cast<WorkbenchPartReference> ());
    }
  } catch (...)
  {
  }
  this->DeferUpdates(false);

  // Notify interested listeners after the close
  window->FirePerspectiveChanged(thisPage, this->GetPerspective(),
      CHANGE_EDITOR_CLOSE);

  // Return true on success.
  return true;
}

void WorkbenchPage::DeferUpdates(bool shouldDefer)
{
  if (shouldDefer)
  {
    if (deferCount == 0)
    {
      this->StartDeferring();
    }
    deferCount++;
  }
  else
  {
    deferCount--;
    if (deferCount == 0)
    {
      this->HandleDeferredEvents();
    }
  }
}

void WorkbenchPage::StartDeferring()
{
  //editorPresentation.getLayoutPart().deferUpdates(true);
}

void WorkbenchPage::HandleDeferredEvents()
{
  editorPresentation->GetLayoutPart()->DeferUpdates(false);
  this->UpdateActivePart();
  QList<WorkbenchPartReference::Pointer> disposals = pendingDisposals;
  pendingDisposals.clear();
  for (int i = 0; i < disposals.size(); i++)
  {
    this->DisposePart(disposals[i]);
  }

}

bool WorkbenchPage::IsDeferred()
{
  return deferCount > 0;
}

bool WorkbenchPage::CloseEditor(IEditorReference::Pointer editorRef, bool save)
{
  QList<IEditorReference::Pointer> list;
  list.push_back(editorRef);
  return this->CloseEditors(list, save);
}

bool WorkbenchPage::CloseEditor(IEditorPart::Pointer editor, bool save)
{
  IWorkbenchPartReference::Pointer ref = this->GetReference(editor);
  if (ref.Cast<IEditorReference> ().IsNotNull())
  {
    QList<IEditorReference::Pointer> list;
    list.push_back(ref.Cast<IEditorReference> ());
    return this->CloseEditors(list, save);
  }
  return false;
}

void WorkbenchPage::CloseCurrentPerspective(bool saveParts, bool closePage)
{
    Perspective::Pointer persp = this->GetActivePerspective();
    if (persp != 0)
    {
      this->ClosePerspective(persp, saveParts, closePage);
    }
}

void WorkbenchPage::ClosePerspective(IPerspectiveDescriptor::Pointer desc,
    bool saveParts, bool closePage)
{
  Perspective::Pointer persp = this->FindPerspective(desc);
  if (persp != 0)
  {
    this->ClosePerspective(persp, saveParts, closePage);
  }
}

void WorkbenchPage::ClosePerspective(Perspective::Pointer persp,
    bool saveParts, bool closePage)
{

  //  // Always unzoom
  //  if (isZoomed())
  //  {
  //    zoomOut();
  //  }

  QList<IWorkbenchPart::Pointer> partsToSave;
  QList<IWorkbenchPart::Pointer> viewsToClose;
  // collect views that will go away and views that are dirty
  QList<IViewReference::Pointer> viewReferences =
      persp->GetViewReferences();
  for (int i = 0; i < viewReferences.size(); i++)
  {
    IViewReference::Pointer reference = viewReferences[i];
    if (this->GetViewFactory()->GetReferenceCount(reference) == 1)
    {
      IViewPart::Pointer viewPart = reference->GetView(false);
      if (viewPart != 0)
      {
        viewsToClose.push_back(viewPart);
        if (saveParts && reference->IsDirty())
        {
          partsToSave.push_back(viewPart);
        }
      }
    }
  }
  if (saveParts && perspList.Size() == 1)
  {
    // collect editors that are dirty
    QList<IEditorReference::Pointer> editorReferences =
        this->GetEditorReferences();
    for (QList<IEditorReference::Pointer>::iterator refIter =
        editorReferences.begin(); refIter != editorReferences.end(); ++refIter)
    {
      IEditorReference::Pointer reference = *refIter;
      if (reference->IsDirty())
      {
        IEditorPart::Pointer editorPart = reference->GetEditor(false);
        if (editorPart != 0)
        {
          partsToSave.push_back(editorPart);
        }
      }
    }
  }
  if (saveParts && !partsToSave.empty())
  {
    if (!EditorManager::SaveAll(partsToSave, true, true, false,
        IWorkbenchWindow::Pointer(window)))
    {
      // user canceled
      return;
    }
  }
  // Close all editors on last perspective close
  if (perspList.Size() == 1 && this->GetEditorManager()->GetEditorCount() > 0)
  {
    // Close all editors
    if (!this->CloseAllEditors(false))
    {
      return;
    }
  }

  // closeAllEditors already notified the saveables list about the editors.
  SaveablesList::Pointer saveablesList(
        dynamic_cast<SaveablesList*>(
          this->GetWorkbenchWindow()->GetWorkbench()->GetService<ISaveablesLifecycleListener>()));
  // we took care of the saving already, so pass in false (postCloseInfo will be non-0)
  SaveablesList::PostCloseInfo::Pointer postCloseInfo =
      saveablesList->PreCloseParts(viewsToClose, false,
          this->GetWorkbenchWindow());
  saveablesList->PostClose(postCloseInfo);

  // Dispose of the perspective
  bool isActive = (perspList.GetActive() == persp);
  if (isActive)
  {
    this->SetPerspective(perspList.GetNextActive());
  }
  this->DisposePerspective(persp, true);
  if (closePage && perspList.Size() == 0)
  {
    this->Close();
  }
}

void WorkbenchPage::CloseAllPerspectives(bool saveEditors, bool closePage)
{

  if (perspList.IsEmpty())
  {
    return;
  }

  //  // Always unzoom
  //  if (isZoomed())
  //  {
  //    zoomOut();
  //  }

  if (saveEditors)
  {
    if (!this->SaveAllEditors(true))
    {
      return;
    }
  }
  // Close all editors
  if (!this->CloseAllEditors(false))
  {
    return;
  }

  // Deactivate the active perspective and part
  this->SetPerspective(Perspective::Pointer(nullptr));

  // Close each perspective in turn
  PerspectiveList oldList = perspList;
  perspList = PerspectiveList();
  for (PerspectiveList::iterator itr = oldList.Begin(); itr != oldList.End(); ++itr)
  {
    this->ClosePerspective(*itr, false, false);
  }
  if (closePage)
  {
    this->Close();
  }
}

void WorkbenchPage::CreateClientComposite()
{
  QWidget* parent = window->GetPageComposite();
  //  StartupThreading.runWithoutExceptions(new StartupRunnable()
  //      {
  //
  //      public void WorkbenchPage::runWithException()
  //        {
  composite
      = Tweaklets::Get(WorkbenchPageTweaklet::KEY)->CreateClientComposite(
          parent);
  Tweaklets::Get(GuiWidgetsTweaklet::KEY)->SetVisible(composite, false); // Make visible on activate.
  // force the client composite to be layed out
  //          parent.layout();
  //        }
  //      });

}

Perspective::Pointer WorkbenchPage::CreatePerspective(
    PerspectiveDescriptor::Pointer desc, bool notify)
{
  QString label = desc->GetId(); // debugging only
  try
  {
    //UIStats.start(UIStats.CREATE_PERSPECTIVE, label);
    WorkbenchPage::Pointer thisPage(this);
    Perspective::Pointer persp(new Perspective(desc, thisPage));
    perspList.Add(persp);
    if (notify)
    {
      window->FirePerspectiveOpened(thisPage, desc);
    }
    //if the perspective is fresh and uncustomzied then it is not dirty
    //no reset will be prompted for
    if (!desc->HasCustomDefinition())
    {
      dirtyPerspectives.erase(desc->GetId());
    }
    return persp;
  } catch (WorkbenchException& /*e*/)
  {
    if (!window->GetWorkbenchImpl()->IsStarting())
    {
      QMessageBox::critical(reinterpret_cast<QWidget*>(window->GetShell()->GetControl()),
                            "Error",
                            "Problems opening perspective \"" + desc->GetId() + "\"");
    }
    return Perspective::Pointer(nullptr);
  }
  //    finally
  //    {
  //      UIStats.end(UIStats.CREATE_PERSPECTIVE, desc.getId(), label);
  //    }
}

void WorkbenchPage::PartAdded(WorkbenchPartReference::Pointer ref)
{
  activationList->Add(ref);
  partList->AddPart(ref);
  this->UpdateActivePart();
}

void WorkbenchPage::PartRemoved(WorkbenchPartReference::Pointer ref)
{
  activationList->Remove(ref);
  this->DisposePart(ref);
}

void WorkbenchPage::DisposePart(WorkbenchPartReference::Pointer ref)
{
  if (this->IsDeferred())
  {
    pendingDisposals.push_back(ref);
  }
  else
  {
    partList->RemovePart(ref);
    ref->Dispose();
  }
}

void WorkbenchPage::DeactivatePart(IWorkbenchPart::Pointer part)
{
  if (part.IsNotNull())
  {
    PartSite::Pointer site = part->GetSite().Cast<PartSite> ();
    site->GetPane()->ShowFocus(false);
  }
}

void WorkbenchPage::DetachView(IViewReference::Pointer ref)
{
  Perspective::Pointer persp = this->GetActivePerspective();
  if (persp == 0)
  {
    return;
  }

  PerspectiveHelper* presentation = persp->GetPresentation();
  presentation->DetachPart(ref);
}

void WorkbenchPage::AttachView(IViewReference::Pointer ref)
{
  PerspectiveHelper* presentation = this->GetPerspectivePresentation();
  presentation->AttachPart(ref);
}

WorkbenchPage::~WorkbenchPage()
{
  // increment reference count to prevent recursive deletes
  this->Register();

  {
    {

      this->MakeActiveEditor(IEditorReference::Pointer(nullptr));
      this->MakeActive(IWorkbenchPartReference::Pointer(nullptr));

      // Close and dispose the editors.
      this->CloseAllEditors(false);

      // Need to make sure model data is cleaned up when the page is
      // disposed. Collect all the views on the page and notify the
      // saveable list of a pre/post close. This will free model data.
      QList<IWorkbenchPartReference::Pointer> partsToClose =
          this->GetOpenParts();
      QList<IWorkbenchPart::Pointer> dirtyParts;
      for (int i = 0; i < partsToClose.size(); i++)
      {
        IWorkbenchPart::Pointer part = partsToClose[i]->GetPart(false);
        if (part != 0 && part.Cast<IViewPart> () != 0)
        {
          dirtyParts.push_back(part);
        }
      }
      SaveablesList::Pointer saveablesList(dynamic_cast<SaveablesList*>(
          this->GetWorkbenchWindow()->GetWorkbench()->GetService<ISaveablesLifecycleListener>()));
      SaveablesList::PostCloseInfo::Pointer postCloseInfo =
          saveablesList->PreCloseParts(dirtyParts, false,
              this->GetWorkbenchWindow());
      saveablesList->PostClose(postCloseInfo);

      IWorkbenchPage::Pointer thisPage(this);
      // Get rid of perspectives. This will close the views
      for (PerspectiveList::iterator itr = perspList.Begin(); itr
          != perspList.End(); ++itr)
      {
        Perspective::Pointer perspective = *itr;
        window->FirePerspectiveClosed(thisPage, perspective->GetDesc());
        //perspective->Dispose();
      }
      perspList = PerspectiveList();

      // Get rid of editor presentation.
      //editorPresentation->Dispose();

      // Get rid of composite.
      //composite.dispose();

      //navigationHistory.dispose();

      //stickyViewMan.clear();

      //  if (tracker != 0)
      //  {
      //    tracker.close();
      //  }

      //  // if we're destroying a window in a non-shutdown situation then we should
      //  // clean up the working set we made.
      //  if (!window->GetWorkbench()->IsClosing())
      //  {
      //    if (aggregateWorkingSet != 0)
      //    {
      //      PlatformUI.getWorkbench().getWorkingSetManager().removeWorkingSet(
      //          aggregateWorkingSet);
      //    }
      //  }
    }

    partBeingActivated = nullptr;
    pendingDisposals.clear();

    stickyViewMan = nullptr;
    delete viewFactory;
    delete editorPresentation;
    delete editorMgr;
    delete activationList;

    deferredActivePersp = nullptr;

    dirtyPerspectives.clear();

    delete selectionService;
    partList.reset();

  }

  // decrement reference count again, without explicit deletion
  this->UnRegister(false);
}

void WorkbenchPage::DisposePerspective(Perspective::Pointer persp, bool notify)
{
  // Get rid of perspective.
  perspList.Remove(persp);
  if (notify)
  {
    IWorkbenchPage::Pointer thisPage(this);
    window->FirePerspectiveClosed(thisPage, persp->GetDesc());
  }
  //persp->Dispose();

  stickyViewMan->Remove(persp->GetDesc()->GetId());
}

Perspective::Pointer WorkbenchPage::FindPerspective(
    IPerspectiveDescriptor::Pointer desc)
{
  for (PerspectiveList::iterator itr = perspList.Begin(); itr
      != perspList.End(); ++itr)
  {
    Perspective::Pointer mgr = *itr;
    if (desc->GetId() == mgr->GetDesc()->GetId())
    {
      return mgr;
    }
  }
  return Perspective::Pointer(nullptr);
}

IViewPart::Pointer WorkbenchPage::FindView(const QString& id)
{
  IViewReference::Pointer ref = this->FindViewReference(id);
  if (ref == 0)
  {
    return IViewPart::Pointer(nullptr);
  }
  return ref->GetView(true);
}

IViewReference::Pointer WorkbenchPage::FindViewReference(
    const QString& viewId)
{
  return this->FindViewReference(viewId, "");
}

IViewReference::Pointer WorkbenchPage::FindViewReference(
    const QString& viewId, const QString& secondaryId)
{
  Perspective::Pointer persp = this->GetActivePerspective();
  if (persp == 0)
  {
    return IViewReference::Pointer(nullptr);
  }
  return persp->FindView(viewId, secondaryId);
}

IEditorPart::Pointer WorkbenchPage::GetActiveEditor()
{
  return partList->GetActiveEditor();
}

IEditorReference::Pointer WorkbenchPage::GetActiveEditorReference()
{
  return partList->GetActiveEditorReference();
}

IWorkbenchPart::Pointer WorkbenchPage::GetActivePart()
{
  return partList->GetActivePart();
}

IWorkbenchPartReference::Pointer WorkbenchPage::GetActivePartReference()
{
  return partList->GetActivePartReference();
}

Perspective::Pointer WorkbenchPage::GetActivePerspective()
{
  return perspList.GetActive();
}

QWidget* WorkbenchPage::GetClientComposite()
{
  return composite;
}

EditorManager* WorkbenchPage::GetEditorManager()
{
  return editorMgr;
}

PerspectiveHelper* WorkbenchPage::GetPerspectivePresentation()
{
  if (this->GetActivePerspective() != 0)
  {
    return this->GetActivePerspective()->GetPresentation();
  }
  return nullptr;
}

bool WorkbenchPage::HasView(const QString& perspectiveId, const QString& viewId)
{
  PerspectiveList::PerspectiveListType list = perspList.GetSortedPerspectives();
  for ( PerspectiveList::PerspectiveListType::iterator it = list.begin(); it!=list.end(); it++)
  {
    SmartPointer<Perspective> p = *it;
    if (p->GetDesc()->GetId() == perspectiveId)
    {
      if (p->ContainsView(viewId))
      {
        return true;
      }
    }
  }
  return false;
}

/**
 * Answer the editor presentation.
 */
EditorAreaHelper* WorkbenchPage::GetEditorPresentation()
{
  return editorPresentation;
}

QList<IEditorPart::Pointer> WorkbenchPage::GetEditors()
{
  QList<IEditorReference::Pointer> refs = this->GetEditorReferences();
  QList<IEditorPart::Pointer> result;
  //Display d = getWorkbenchWindow().getShell().getDisplay();
  //Must be backward compatible.
  //    d.syncExec(new Runnable()
  //        {
  //        public void WorkbenchPage::run()
  //          {
  for (QList<IEditorReference::Pointer>::iterator iter = refs.begin(); iter
      != refs.end(); ++iter)
  {
    IEditorPart::Pointer part = (*iter)->GetEditor(true);
    if (part != 0)
    {
      result.push_back(part);
    }
  }
  //          }
  //        });
  return result;
}

QList<IEditorPart::Pointer> WorkbenchPage::GetDirtyEditors()
{
  return this->GetEditorManager()->GetDirtyEditors();
}

QList<ISaveablePart::Pointer> WorkbenchPage::GetDirtyParts()
{
  QList<ISaveablePart::Pointer> result;
  QList<IWorkbenchPartReference::Pointer> allParts = this->GetAllParts();
  for (int i = 0; i < allParts.size(); i++)
  {
    IWorkbenchPartReference::Pointer reference = allParts[i];

    IWorkbenchPart::Pointer part = reference->GetPart(false);
    if (part != 0 && part.Cast<ISaveablePart> () != 0)
    {
      ISaveablePart::Pointer saveable = part.Cast<ISaveablePart> ();
      if (saveable->IsDirty())
      {
        result.push_back(saveable);
      }
    }
  }

  return result;
}

IEditorPart::Pointer WorkbenchPage::FindEditor(IEditorInput::Pointer input)
{
  return this->GetEditorManager()->FindEditor(input);
}

QList<IEditorReference::Pointer> WorkbenchPage::FindEditors(
    IEditorInput::Pointer input, const QString& editorId, int matchFlags)
{
  return this->GetEditorManager()->FindEditors(input, editorId, matchFlags);
}

QList<IEditorReference::Pointer> WorkbenchPage::GetEditorReferences()
{
  return editorPresentation->GetEditors();
}

IAdaptable* WorkbenchPage::GetInput()
{
  return input;
}

QString WorkbenchPage::GetLabel()
{
  QString label = "<Unknown label>";
  //    IWorkbenchAdapter adapter = (IWorkbenchAdapter) Util.getAdapter(input,
  //        IWorkbenchAdapter.class);
  //    if (adapter != 0)
  //    {
  //      label = adapter.getLabel(input);
  //    }
  Perspective::Pointer persp = this->GetActivePerspective();
  if (persp != 0)
  {
    label = label + " - " + persp->GetDesc()->GetLabel();
  }
  else if (deferredActivePersp != 0)
  {
    label = label + " - " + deferredActivePersp->GetLabel();
  }
  return label;
}

IPerspectiveDescriptor::Pointer WorkbenchPage::GetPerspective()
{
  if (deferredActivePersp != 0)
  {
    return deferredActivePersp;
  }
  Perspective::Pointer persp = this->GetActivePerspective();
  if (persp != 0)
  {
    return persp->GetDesc();
  }
  else
  {
    return IPerspectiveDescriptor::Pointer(nullptr);
  }
}

ISelection::ConstPointer WorkbenchPage::GetSelection() const
{
  return selectionService->GetSelection();
}

ISelection::ConstPointer WorkbenchPage::GetSelection(const QString& partId)
{
  return selectionService->GetSelection(partId);
}

//ISelectionService::SelectionEvents& WorkbenchPage::GetSelectionEvents(const QString& partId)
//{
//  return selectionService->GetSelectionEvents(partId);
//}

ViewFactory* WorkbenchPage::GetViewFactory()
{
  if (viewFactory == nullptr)
  {
    viewFactory = new ViewFactory(this,
        WorkbenchPlugin::GetDefault()->GetViewRegistry());
  }
  return viewFactory;
}

QList<IViewReference::Pointer> WorkbenchPage::GetViewReferences()
{
  Perspective::Pointer persp = this->GetActivePerspective();
  if (persp != 0)
  {
    return persp->GetViewReferences();
  }
  else
  {
    return QList<IViewReference::Pointer>();
  }
}

QList<IViewPart::Pointer> WorkbenchPage::GetViews()
{
  return this->GetViews(Perspective::Pointer(nullptr), true);
}

QList<IViewPart::Pointer> WorkbenchPage::GetViews(
    Perspective::Pointer persp, bool restore)
{
  if (persp == 0)
  {
    persp = this->GetActivePerspective();
  }

  QList<IViewPart::Pointer> parts;
  if (persp != 0)
  {
    QList<IViewReference::Pointer> refs = persp->GetViewReferences();
    for (int i = 0; i < refs.size(); i++)
    {
      IViewPart::Pointer part = refs[i]->GetPart(restore).Cast<IViewPart> ();
      if (part != 0)
      {
        parts.push_back(part);
      }
    }
  }
  return parts;
}

IWorkbenchWindow::Pointer WorkbenchPage::GetWorkbenchWindow() const
{
  return IWorkbenchWindow::Pointer(window);
}

void WorkbenchPage::HideView(IViewReference::Pointer ref)
{

  // Sanity check.
  if (ref == 0)
  {
    return;
  }

  Perspective::Pointer persp = this->GetActivePerspective();
  if (persp == 0)
  {
    return;
  }

  bool promptedForSave = false;
  IViewPart::Pointer view = ref->GetView(false);
  if (view != 0)
  {

    if (!this->CertifyPart(view))
    {
      return;
    }

    // Confirm.
    if (view.Cast<ISaveablePart> () != 0)
    {
      ISaveablePart::Pointer saveable = view.Cast<ISaveablePart> ();
      if (saveable->IsSaveOnCloseNeeded())
      {
        IWorkbenchWindow::Pointer window =
            view->GetSite()->GetWorkbenchWindow();
        QList<IWorkbenchPart::Pointer> partsToSave;
        partsToSave.push_back(view);
        bool success = EditorManager::SaveAll(partsToSave, true, true, false,
            window);
        if (!success)
        {
          // the user cancelled.
          return;
        }
        promptedForSave = true;
      }
    }
  }

  int refCount = this->GetViewFactory()->GetReferenceCount(ref);
  SaveablesList* saveablesList = nullptr;
  SaveablesList::PostCloseInfo::Pointer postCloseInfo;
  if (refCount == 1)
  {
    IWorkbenchPart::Pointer actualPart = ref->GetPart(false);
    if (actualPart != 0)
    {
      saveablesList = dynamic_cast<SaveablesList*>(
            actualPart->GetSite()->GetService<ISaveablesLifecycleListener>());
      QList<IWorkbenchPart::Pointer> partsToClose;
      partsToClose.push_back(actualPart);
      postCloseInfo = saveablesList->PreCloseParts(partsToClose,
          !promptedForSave, this->GetWorkbenchWindow());
      if (postCloseInfo == 0)
      {
        // cancel
        return;
      }
    }
  }

  IWorkbenchPage::Pointer thisPage(this);
  // Notify interested listeners before the hide
  window->FirePerspectiveChanged(thisPage, persp->GetDesc(), ref,
      CHANGE_VIEW_HIDE);

  PartPane::Pointer pane = this->GetPane(ref.Cast<IWorkbenchPartReference> ());
  pane->SetInLayout(false);

  this->UpdateActivePart();

  if (saveablesList != nullptr)
  {
    saveablesList->PostClose(postCloseInfo);
  }

  // Hide the part.
  persp->HideView(ref);

  // Notify interested listeners after the hide
  window->FirePerspectiveChanged(thisPage, this->GetPerspective(),
      CHANGE_VIEW_HIDE);
}

void WorkbenchPage::RefreshActiveView()
{
  this->UpdateActivePart();
}

void WorkbenchPage::HideView(IViewPart::Pointer view)
{
  this->HideView(this->GetReference(view).Cast<IViewReference> ());
}

void WorkbenchPage::Init(WorkbenchWindow* w, const QString& layoutID,
    IAdaptable* input, bool openExtras)
{
  // Save args.
  this->window = w;
  this->input = input;
  this->composite = nullptr;
  this->viewFactory = nullptr;

  this->activationList = new ActivationList(this);
  this->selectionService = new PageSelectionService(this);
  this->partList.reset(new WorkbenchPagePartList(this->selectionService));
  this->stickyViewMan = new StickyViewManager(this);

  //actionSets = new ActionSetManager(w);

  deferCount = 0;

  // Create presentation.
  this->CreateClientComposite();
  editorPresentation = new EditorAreaHelper(this);
  editorMgr = new EditorManager(WorkbenchWindow::Pointer(window),
      WorkbenchPage::Pointer(this), editorPresentation);

  //TODO WorkbenchPage perspective reorder listener?
  //    // add this page as a client to be notified when the UI has re-ordered perspectives
  //    // so that the order can be properly maintained in the receiver.
  //    // E.g. a UI might support drag-and-drop and will need to make this known to ensure
  //    // #saveState and #restoreState do not lose this re-ordering
  //    w.addPerspectiveReorderListener(new IReorderListener()
  //        {
  //        public void WorkbenchPage::reorder(Object perspective, int newLoc)
  //          {
  //            perspList.reorder((IPerspectiveDescriptor)perspective, newLoc);
  //          }
  //        });

  if (openExtras)
  {
    this->OpenPerspectiveExtras();
  }

  // Get perspective descriptor.
  if (layoutID != "")
  {
    PerspectiveDescriptor::Pointer
        desc =
            WorkbenchPlugin::GetDefault()->GetPerspectiveRegistry()->FindPerspectiveWithId(
                layoutID).Cast<PerspectiveDescriptor> ();
    if (desc == 0)
    {
      throw WorkbenchException("Unable to create Perspective " + layoutID
          + ". There is no corresponding perspective extension.");
    }
    Perspective::Pointer persp = this->FindPerspective(desc);
    if (persp == 0)
    {
      persp = this->CreatePerspective(desc, true);
    }
    perspList.SetActive(persp);
    window->FirePerspectiveActivated(IWorkbenchPage::Pointer(this), desc);
  }

  //this->GetExtensionTracker()->RegisterHandler(perspectiveChangeHandler,
  //                                             ExtensionTracker::CreateExtensionPointFilter(
  //                                              GetPerspectiveExtensionPoint()));
}

void WorkbenchPage::OpenPerspectiveExtras()
{
  //TODO WorkbenchPage perspectice extras
  QString extras = ""; //PrefUtil.getAPIPreferenceStore().getString(
  //    IWorkbenchPreferenceConstants.PERSPECTIVE_BAR_EXTRAS);
  QList<IPerspectiveDescriptor::Pointer> descs;
  foreach (QString id, extras.split(", ", QString::SkipEmptyParts))
  {
    if (id.trimmed().isEmpty()) continue;
    IPerspectiveDescriptor::Pointer desc =
        WorkbenchPlugin::GetDefault()->GetPerspectiveRegistry()->FindPerspectiveWithId(id);
    if (desc != 0)
    {
      descs.push_back(desc);
    }
  }
  // HACK: The perspective switcher currently adds the button for a new perspective to the beginning of the list.
  // So, we process the extra perspectives in reverse order here to have their buttons appear in the order declared.
  for (int i = (int) descs.size(); --i >= 0;)
  {
    PerspectiveDescriptor::Pointer desc =
        descs[i].Cast<PerspectiveDescriptor> ();
    if (this->FindPerspective(desc) == 0)
    {
      this->CreatePerspective(desc, true);
    }
  }
}

bool WorkbenchPage::IsPartVisible(IWorkbenchPart::Pointer part)
{
  PartPane::Pointer pane = this->GetPane(part);
  return pane != 0 && pane->GetVisible();
}

bool WorkbenchPage::IsEditorAreaVisible()
{
  Perspective::Pointer persp = this->GetActivePerspective();
  if (persp == 0)
  {
    return false;
  }
  return persp->IsEditorAreaVisible();
}

bool WorkbenchPage::IsFastView(IViewReference::Pointer /*ref*/)
{
  Perspective::Pointer persp = this->GetActivePerspective();
  if (persp != 0)
  {
    //return persp->IsFastView(ref);
    return false;
  }
  else
  {
    return false;
  }
}

bool WorkbenchPage::IsCloseable(IViewReference::Pointer ref)
{
  Perspective::Pointer persp = this->GetActivePerspective();
  if (persp != 0)
  {
    return persp->IsCloseable(ref);
  }
  return false;
}

bool WorkbenchPage::IsMoveable(IViewReference::Pointer ref)
{
  Perspective::Pointer persp = this->GetActivePerspective();
  if (persp != 0)
  {
    return persp->IsMoveable(ref);
  }
  return false;
}

bool WorkbenchPage::IsFixedLayout()
{
  Perspective::Pointer persp = this->GetActivePerspective();
  if (persp != 0)
  {
    return persp->IsFixedLayout();
  }
  else
  {
    return false;
  }
}

bool WorkbenchPage::IsSaveNeeded()
{
  return this->GetEditorManager()->IsSaveAllNeeded();
}

void WorkbenchPage::OnActivate()
{
  Tweaklets::Get(GuiWidgetsTweaklet::KEY)->SetVisible(composite, true);
  Perspective::Pointer persp = this->GetActivePerspective();

  if (persp != 0)
  {
    persp->OnActivate();
    this->UpdateVisibility(Perspective::Pointer(nullptr), persp);
  }
}

void WorkbenchPage::OnDeactivate()
{
  this->MakeActiveEditor(IEditorReference::Pointer(nullptr));
  this->MakeActive(IWorkbenchPartReference::Pointer(nullptr));
  if (this->GetActivePerspective() != 0)
  {
    this->GetActivePerspective()->OnDeactivate();
  }
  Tweaklets::Get(GuiWidgetsTweaklet::KEY)->SetVisible(composite, false);
}

void WorkbenchPage::ReuseEditor(IReusableEditor::Pointer editor,
    IEditorInput::Pointer input)
{

  // Rather than calling editor.setInput on the editor directly, we do it through the part reference.
  // This case lets us detect badly behaved editors that are not firing a PROP_INPUT event in response
  // to the input change... but if all editors obeyed their API contract, the "else" branch would be
  // sufficient.
  IWorkbenchPartReference::Pointer ref = this->GetReference(editor);
  if (ref.Cast<EditorReference> () != 0)
  {
    EditorReference::Pointer editorRef = ref.Cast<EditorReference> ();

    editorRef->SetInput(input);
  }
  else
  {
    editor->SetInput(input);
  }
  //navigationHistory.markEditor(editor);
}

IEditorPart::Pointer WorkbenchPage::OpenEditor(IEditorInput::Pointer input,
    const QString& editorID)
{
  return this->OpenEditor(input, editorID, true, MATCH_INPUT);
}

IEditorPart::Pointer WorkbenchPage::OpenEditor(IEditorInput::Pointer input,
    const QString& editorID, bool activate)
{
  return this->OpenEditor(input, editorID, activate, MATCH_INPUT);
}

IEditorPart::Pointer WorkbenchPage::OpenEditor(
    const IEditorInput::Pointer input, const QString& editorID,
    bool activate, int matchFlags)
{
  return this->OpenEditor(input, editorID, activate, matchFlags,
      IMemento::Pointer(nullptr));
}

IEditorPart::Pointer WorkbenchPage::OpenEditor(
    const IEditorInput::Pointer input, const QString& editorID,
    bool activate, int matchFlags, IMemento::Pointer editorState)
{
  if (input == 0 || editorID == "")
  {
    throw Poco::InvalidArgumentException();
  }

  //    BusyIndicator.showWhile(window.getWorkbench().getDisplay(),
  //        new Runnable()
  //        {
  //        public void WorkbenchPage::run()
  //          {
  return this->BusyOpenEditor(input, editorID, activate, matchFlags,
      editorState);
}

IEditorPart::Pointer WorkbenchPage::OpenEditorFromDescriptor(
    IEditorInput::Pointer input, IEditorDescriptor::Pointer editorDescriptor,
    bool activate, IMemento::Pointer editorState)
{
  if (input == 0 || !(editorDescriptor.Cast<EditorDescriptor> () != 0))
  {
    throw Poco::InvalidArgumentException();
  }

  //    BusyIndicator.showWhile(window.getWorkbench().getDisplay(),
  //        new Runnable()
  //        {
  //        public void WorkbenchPage::run()
  //          {
  return this->BusyOpenEditorFromDescriptor(input, editorDescriptor.Cast<
      EditorDescriptor> (), activate, editorState);

  //          }
  //        });

}

IEditorPart::Pointer WorkbenchPage::BusyOpenEditor(IEditorInput::Pointer input,
    const QString& editorID, bool activate, int matchFlags,
    IMemento::Pointer editorState)
{

  Workbench* workbench =
      this->GetWorkbenchWindow().Cast<WorkbenchWindow> ()->GetWorkbenchImpl();
  workbench->LargeUpdateStart();

  IEditorPart::Pointer result;
  try
  {
    result = this->BusyOpenEditorBatched(input, editorID, activate, matchFlags,
        editorState);

  } catch (std::exception& e)
  {
    workbench->LargeUpdateEnd();
    throw e;
  }

  workbench->LargeUpdateEnd();
  return result;
}

IEditorPart::Pointer WorkbenchPage::BusyOpenEditorFromDescriptor(
    IEditorInput::Pointer input, EditorDescriptor::Pointer editorDescriptor,
    bool activate, IMemento::Pointer editorState)
{

  Workbench* workbench =
      this->GetWorkbenchWindow().Cast<WorkbenchWindow> ()->GetWorkbenchImpl();
  workbench->LargeUpdateStart();

  IEditorPart::Pointer result;
  try
  {
    result = this->BusyOpenEditorFromDescriptorBatched(input, editorDescriptor,
        activate, editorState);

  } catch (std::exception& e)
  {
    workbench->LargeUpdateEnd();
    throw e;
  }

  workbench->LargeUpdateEnd();
  return result;
}

IEditorPart::Pointer WorkbenchPage::BusyOpenEditorBatched(
    IEditorInput::Pointer input, const QString& editorID, bool activate,
    int matchFlags, IMemento::Pointer editorState)
{

  // If an editor already exists for the input, use it.
  IEditorPart::Pointer editor;
  // Reuse an existing open editor, unless we are in "new editor tab management" mode
  editor = this->GetEditorManager()->FindEditor(editorID, input, matchFlags);
  if (editor != 0)
  {
    if (IEditorRegistry::SYSTEM_EXTERNAL_EDITOR_ID == editorID)
    {
      if (editor->IsDirty())
      {
        QMessageBox::StandardButton saveFile = QMessageBox::question(
                                                 this->GetWorkbenchWindow()->GetShell()->GetControl(),
                                                 "Save", "\"" + input->GetName() + "\" is opened and has unsaved changes. Do you want to save it?",
                                                 QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Cancel);
        if (saveFile == QMessageBox::Yes)
        {
          //          try
          //          {
          IEditorPart::Pointer editorToSave = editor;
          //              getWorkbenchWindow().run(false, false,
          //                  new IRunnableWithProgress()
          //                  {
          //                  public void WorkbenchPage::run(IProgressMonitor monitor)
          //                    throws InvocationTargetException,
          //                    InterruptedException
          //                    {
          //TODO progress monitor
          editorToSave->DoSave();//monitor);
          //                    }
          //                  });
          // }
          //              catch (InvocationTargetException& e)
          //              {
          //                throw(RuntimeException) e->GetTargetException();
          //              }
          //          catch (InterruptedException& e)
          //          {
          //            return 0;
          //          }
        }
        else if (saveFile == QMessageBox::Cancel)
        {
          return IEditorPart::Pointer(nullptr);
        }
      }
    }
    else
    {
      //          // do the IShowEditorInput notification before showing the editor
      //          // to reduce flicker
      //          if (editor.Cast<IShowEditorInput> () != 0)
      //          {
      //            ((IShowEditorInput) editor).showEditorInput(input);
      //          }
      this->ShowEditor(activate, editor);
      return editor;
    }
  }

  // Otherwise, create a new one. This may cause the new editor to
  // become the visible (i.e top) editor.
  IEditorReference::Pointer ref = this->GetEditorManager()->OpenEditor(
      editorID, input, true, editorState);
  if (ref != 0)
  {
    editor = ref->GetEditor(true);
  }

  if (editor != 0)
  {
    this->SetEditorAreaVisible(true);
    if (activate)
    {
      this->Activate(editor);
    }
    else
    {
      this->BringToTop(editor);
    }
    IWorkbenchPage::Pointer thisPage(this);
    window->FirePerspectiveChanged(thisPage, this->GetPerspective(), ref,
        CHANGE_EDITOR_OPEN);
    window->FirePerspectiveChanged(thisPage, this->GetPerspective(),
        CHANGE_EDITOR_OPEN);
  }

  return editor;
}

/*
 * Added to fix Bug 178235 [EditorMgmt] DBCS 3.3 - Cannot open file with external program.
 * See openEditorFromDescriptor().
 */
IEditorPart::Pointer WorkbenchPage::BusyOpenEditorFromDescriptorBatched(
    IEditorInput::Pointer input, EditorDescriptor::Pointer editorDescriptor,
    bool activate, IMemento::Pointer editorState)
{

  IEditorPart::Pointer editor;
  // Create a new one. This may cause the new editor to
  // become the visible (i.e top) editor.
  IEditorReference::Pointer ref;
  ref = this->GetEditorManager()->OpenEditorFromDescriptor(editorDescriptor,
      input, editorState);
  if (ref != 0)
  {
    editor = ref->GetEditor(true);
  }

  if (editor != 0)
  {
    this->SetEditorAreaVisible(true);
    if (activate)
    {

      this->Activate(editor);

    }
    else
    {
      this->BringToTop(editor);
    }
    IWorkbenchPage::Pointer thisPage(this);
    window->FirePerspectiveChanged(thisPage, this->GetPerspective(), ref,
        CHANGE_EDITOR_OPEN);
    window->FirePerspectiveChanged(thisPage, this->GetPerspective(),
        CHANGE_EDITOR_OPEN);
  }

  return editor;
}

void WorkbenchPage::OpenEmptyTab()
{
  IEditorPart::Pointer editor;
  EditorReference::Pointer ref;
  ref = this->GetEditorManager()->OpenEmptyTab().Cast<EditorReference> ();
  if (ref != 0)
  {
    editor
        = ref->GetEmptyEditor(
            dynamic_cast<EditorRegistry*> (WorkbenchPlugin::GetDefault()->GetEditorRegistry())->FindEditor(
                EditorRegistry::EMPTY_EDITOR_ID).Cast<EditorDescriptor> ());
  }

  if (editor != 0)
  {
    this->SetEditorAreaVisible(true);
    this->Activate(editor);
    IWorkbenchPage::Pointer thisPage(this);
    window->FirePerspectiveChanged(thisPage, this->GetPerspective(), ref,
        CHANGE_EDITOR_OPEN);
    window->FirePerspectiveChanged(thisPage, this->GetPerspective(),
        CHANGE_EDITOR_OPEN);
  }
}

void WorkbenchPage::ShowEditor(bool activate, IEditorPart::Pointer editor)
{
  this->SetEditorAreaVisible(true);
  if (activate)
  {
    //zoomOutIfNecessary(editor);
    this->Activate(editor);
  }
  else
  {
    this->BringToTop(editor);
  }
}

bool WorkbenchPage::IsEditorPinned(IEditorPart::Pointer editor)
{
  WorkbenchPartReference::Pointer ref = this->GetReference(editor).Cast<
      WorkbenchPartReference> ();
  return ref != 0 && ref->IsPinned();
}

/**
 * Removes an IPartListener from the part service.
 */
void WorkbenchPage::RemovePartListener(IPartListener* l)
{
  partList->GetPartService()->RemovePartListener(l);
}

/**
 * Implements IWorkbenchPage
 *
 * @see org.blueberry.ui.IWorkbenchPage#removePropertyChangeListener(IPropertyChangeListener)
 * @since 2.0
 * @deprecated individual views should store a working set if needed and
 *             register a property change listener directly with the
 *             working set manager to receive notification when the view
 *             working set is removed.
 */
//    void WorkbenchPage::RemovePropertyChangeListener(IPropertyChangeListener listener) {
//        propertyChangeListeners.remove(listener);
//    }


void WorkbenchPage::RemoveSelectionListener(ISelectionListener* listener)
{
  selectionService->RemoveSelectionListener(listener);
}

void WorkbenchPage::RemoveSelectionListener(const QString& partId,
                                            ISelectionListener* listener)
{
  selectionService->RemoveSelectionListener(partId, listener);
}

void WorkbenchPage::RemovePostSelectionListener(ISelectionListener* listener)
{
  selectionService->RemovePostSelectionListener(listener);
}

void WorkbenchPage::RemovePostSelectionListener(const QString& partId,
                                                ISelectionListener* listener)
{
  selectionService->RemovePostSelectionListener(partId, listener);
}

void WorkbenchPage::RequestActivation(IWorkbenchPart::Pointer part)
{
  // Sanity check.
  if (!this->CertifyPart(part))
  {
    return;
  }

  // Real work.
  this->SetActivePart(part);
}

/**
 * Resets the layout for the perspective. The active part in the old layout
 * is activated in the new layout for consistent user context.
 */
void WorkbenchPage::ResetPerspective()
{
  // Run op in busy cursor.
  // Use set redraw to eliminate the "flash" that can occur in the
  // coolbar as the perspective is reset.
  //    ICoolBarManager2 mgr = (ICoolBarManager2) window.getCoolBarManager2();
  //    try
  //    {
  //      mgr.getControl2().setRedraw(false);
  //      BusyIndicator.showWhile(0, new Runnable()
  //          {
  //          public void WorkbenchPage::run()
  //            {
  this->BusyResetPerspective();
  //            }
  //          });
  //    }finally
  //    {
  //      mgr.getControl2().setRedraw(true);
  //    }
}

bool WorkbenchPage::RestoreState(IMemento::Pointer memento,
    const IPerspectiveDescriptor::Pointer activeDescriptor)
{
  //    StartupThreading.runWithoutExceptions(new StartupRunnable()
  //        {
  //
  //        public void WorkbenchPage::runWithException() throws Throwable
  //          {
  this->DeferUpdates(true);
  //        }});

  try
  {
    // Restore working set
    QString pageName;
    memento->GetString(WorkbenchConstants::TAG_LABEL, pageName);

    //    String label = 0; // debugging only
    //    if (UIStats.isDebugging(UIStats.RESTORE_WORKBENCH))
    //    {
    //      label = pageName == 0 ? "" : "::" + pageName; //$NON-NLS-1$ //$NON-NLS-2$
    //    }

    try
    {
      //UIStats.start(UIStats.RESTORE_WORKBENCH, "WorkbenchPage" + label); //$NON-NLS-1$
      //      MultiStatus result =
      //          new MultiStatus(PlatformUI.PLUGIN_ID, IStatus.OK, NLS.bind(
      //              WorkbenchMessages.WorkbenchPage_unableToRestorePerspective,
      //              pageName), 0);
      bool result = true;

      //      String workingSetName = memento .getString(
      //          IWorkbenchConstants.TAG_WORKING_SET);
      //      if (workingSetName != 0)
      //      {
      //        AbstractWorkingSetManager
      //            workingSetManager =
      //                (AbstractWorkingSetManager) getWorkbenchWindow() .getWorkbench().getWorkingSetManager();
      //        setWorkingSet(workingSetManager.getWorkingSet(workingSetName));
      //      }
      //
      //      IMemento workingSetMem = memento .getChild(
      //          IWorkbenchConstants.TAG_WORKING_SETS);
      //      if (workingSetMem != 0)
      //      {
      //        QList<IMemento::Pointer> workingSetChildren =
      //            workingSetMem .getChildren(IWorkbenchConstants.TAG_WORKING_SET);
      //        List workingSetList = new ArrayList(workingSetChildren.length);
      //        for (int i = 0; i < workingSetChildren.length; i++)
      //        {
      //          IWorkingSet
      //              set =
      //                  getWorkbenchWindow().getWorkbench() .getWorkingSetManager().getWorkingSet(
      //                      workingSetChildren[i].getID());
      //          if (set != 0)
      //          {
      //            workingSetList.add(set);
      //          }
      //        }
      //
      //        workingSets = (IWorkingSet[]) workingSetList .toArray(
      //            new IWorkingSet[workingSetList.size()]);
      //      }
      //
      //      aggregateWorkingSetId = memento.getString(ATT_AGGREGATE_WORKING_SET_ID);
      //
      //      IWorkingSet setWithId =
      //          window.getWorkbench().getWorkingSetManager().getWorkingSet(
      //              aggregateWorkingSetId);
      //
      //      // check to see if the set has already been made and assign it if it has
      //      if (setWithId.Cast<AggregateWorkingSet> () != 0)
      //      {
      //        aggregateWorkingSet = (AggregateWorkingSet) setWithId;
      //      }

      // Restore editor manager.
      IMemento::Pointer childMem = memento->GetChild(
          WorkbenchConstants::TAG_EDITORS);
      //result.merge(getEditorManager().restoreState(childMem));
      result &= this->GetEditorManager()->RestoreState(childMem);

      childMem = memento->GetChild(WorkbenchConstants::TAG_VIEWS);
      if (childMem)
      {
        //result.merge(getViewFactory().restoreState(childMem));
        result &= this->GetViewFactory()->RestoreState(childMem);
      }

      // Get persp block.
      childMem = memento->GetChild(WorkbenchConstants::TAG_PERSPECTIVES);
      QString activePartID;
      childMem->GetString(WorkbenchConstants::TAG_ACTIVE_PART, activePartID);
      QString activePartSecondaryID;
      if (!activePartID.isEmpty())
      {
        activePartSecondaryID = ViewFactory::ExtractSecondaryId(activePartID);
        if (!activePartSecondaryID.isEmpty())
        {
          activePartID = ViewFactory::ExtractPrimaryId(activePartID);
        }
      }
      QString activePerspectiveID;
      childMem->GetString(WorkbenchConstants::TAG_ACTIVE_PERSPECTIVE,
          activePerspectiveID);

      // Restore perspectives.
      QList<IMemento::Pointer> perspMems(childMem->GetChildren(
          WorkbenchConstants::TAG_PERSPECTIVE));
      Perspective::Pointer activePerspective;

      for (int i = 0; i < perspMems.size(); i++)
      {

        IMemento::Pointer current = perspMems[i];
        //          StartupThreading
        //          .runWithoutExceptions(new StartupRunnable()
        //              {
        //
        //              public void WorkbenchPage::runWithException() throws Throwable
        //                {
        Perspective::Pointer persp(new Perspective(
            PerspectiveDescriptor::Pointer(nullptr), WorkbenchPage::Pointer(this)));
        //result.merge(persp.restoreState(current));
        result &= persp->RestoreState(current);
        IPerspectiveDescriptor::Pointer desc = persp->GetDesc();
        if (desc == activeDescriptor)
        {
          activePerspective = persp;
        }
        else if ((activePerspective == 0) && desc->GetId()
            == activePerspectiveID)
        {
          activePerspective = persp;
        }
        perspList.Add(persp);
        window->FirePerspectiveOpened(WorkbenchPage::Pointer(this), desc);
        //                }
        //              });
      }
      bool restoreActivePerspective = false;
      if (!activeDescriptor)
      {
        restoreActivePerspective = true;

      }
      else if (activePerspective && activePerspective->GetDesc()
          == activeDescriptor)
      {
        restoreActivePerspective = true;
      }
      else
      {
        restoreActivePerspective = false;
        activePerspective = this->CreatePerspective(activeDescriptor.Cast<
            PerspectiveDescriptor> (), true);
        if (activePerspective == 0)
        {
          //          result .merge(
          //              new Status(IStatus.ERR, PlatformUI.PLUGIN_ID, 0, NLS.bind(
          //                  WorkbenchMessages.Workbench_showPerspectiveError,
          //                  activeDescriptor.getId()), 0));
          result &= false;
        }
      }

      perspList.SetActive(activePerspective);

      // Make sure we have a valid perspective to work with,
      // otherwise return.
      activePerspective = perspList.GetActive();
      if (activePerspective == 0)
      {
        activePerspective = perspList.GetNextActive();
        perspList.SetActive(activePerspective);
      }
      if (activePerspective && restoreActivePerspective)
      {
        //result.merge(activePerspective.restoreState());
        result &= activePerspective->RestoreState();
      }

      if (activePerspective)
      {
        Perspective::Pointer myPerspective = activePerspective;
        QString myActivePartId = activePartID;
        QString mySecondaryId = activePartSecondaryID;
        //          StartupThreading.runWithoutExceptions(new StartupRunnable()
        //              {
        //
        //              public void WorkbenchPage::runWithException() throws Throwable
        //                {
        window->FirePerspectiveActivated(WorkbenchPage::Pointer(this),
            myPerspective->GetDesc());

        // Restore active part.
        if (!myActivePartId.isEmpty())
        {
          IWorkbenchPartReference::Pointer ref = myPerspective->FindView(
              myActivePartId, mySecondaryId);

          if (ref)
          {
            activationList->SetActive(ref);
          }
        }
        // }});

      }

      //      childMem = memento->GetChild(WorkbenchConstants::TAG_NAVIGATION_HISTORY);
      //      if (childMem)
      //      {
      //        navigationHistory.restoreState(childMem);
      //      }
      //      else if (GetActiveEditor())
      //      {
      //        navigationHistory.markEditor(getActiveEditor());
      //      }
      //
      // restore sticky view state
      stickyViewMan->Restore(memento);

      //      QString blame = activeDescriptor == 0 ? pageName
      //          : activeDescriptor.getId();
      //      UIStats.end(UIStats.RESTORE_WORKBENCH, blame, "WorkbenchPage" + label); //$NON-NLS-1$

      //      StartupThreading.runWithoutExceptions(new StartupRunnable()
      //                {
      //                public void WorkbenchPage::runWithException() throws Throwable
      //                  {
      DeferUpdates(false);
      //            }
      //          });

      return result;
    } catch (...)
    {
      //      QString blame = activeDescriptor == 0 ? pageName
      //          : activeDescriptor.getId();
      //      UIStats.end(UIStats.RESTORE_WORKBENCH, blame, "WorkbenchPage" + label); //$NON-NLS-1$
      throw ;
    }
  }
  catch (...)
  {
    //      StartupThreading.runWithoutExceptions(new StartupRunnable()
    //                {
    //                public void WorkbenchPage::runWithException() throws Throwable
    //                  {
    DeferUpdates(false);
    //            }
    //          });

    throw;
  }

}

bool WorkbenchPage::SaveAllEditors(bool confirm)
{
  return this->SaveAllEditors(confirm, false);
}

bool WorkbenchPage::SaveAllEditors(bool confirm, bool addNonPartSources)
{
  return this->GetEditorManager()->SaveAll(confirm, false, addNonPartSources);
}

bool WorkbenchPage::SavePart(ISaveablePart::Pointer saveable,
    IWorkbenchPart::Pointer part, bool confirm)
{
  // Do not certify part do allow editors inside a multipageeditor to
  // call this.
  return this->GetEditorManager()->SavePart(saveable, part, confirm);
}

bool WorkbenchPage::SaveEditor(IEditorPart::Pointer editor, bool confirm)
{
  return this->SavePart(editor, editor, confirm);
}

/**
 * Saves the current perspective.
 */
void WorkbenchPage::SavePerspective()
{
  Perspective::Pointer persp = this->GetActivePerspective();
  if (persp == 0)
  {
    return;
  }

  //    // Always unzoom.
  //    if (isZoomed())
  //    {
  //      zoomOut();
  //    }

  persp->SaveDesc();
}

/**
 * Saves the perspective.
 */
void WorkbenchPage::SavePerspectiveAs(IPerspectiveDescriptor::Pointer newDesc)
{
  Perspective::Pointer persp = this->GetActivePerspective();
  if (persp == 0)
  {
    return;
  }
  IPerspectiveDescriptor::Pointer oldDesc = persp->GetDesc();

  //    // Always unzoom.
  //    if (isZoomed())
  //    {
  //      zoomOut();
  //    }

  persp->SaveDescAs(newDesc);
  window->FirePerspectiveSavedAs(IWorkbenchPage::Pointer(this), oldDesc, newDesc);
}

/**
 * Save the state of the page.
 */
bool WorkbenchPage::SaveState(IMemento::Pointer memento)
{
  //  // We must unzoom to get correct layout.
  //  if (isZoomed())
  //  {
  //    zoomOut();
  //  }

  //  MultiStatus
  //      result =
  //          new MultiStatus(PlatformUI.PLUGIN_ID, IStatus.OK, NLS.bind(
  //              WorkbenchMessages.WorkbenchPage_unableToSavePerspective,
  //              getLabel()), 0);
  bool result = true;

  // Save editor manager.
  IMemento::Pointer childMem = memento->CreateChild(WorkbenchConstants::TAG_EDITORS);
  //result.merge(editorMgr.saveState(childMem));
  result &= editorMgr->SaveState(childMem);

  childMem = memento->CreateChild(WorkbenchConstants::TAG_VIEWS);
  //result.merge(getViewFactory().saveState(childMem));
  result &= this->GetViewFactory()->SaveState(childMem);

  // Create persp block.
  childMem = memento->CreateChild(WorkbenchConstants::TAG_PERSPECTIVES);
  if (this->GetPerspective())
  {
    childMem->PutString(WorkbenchConstants::TAG_ACTIVE_PERSPECTIVE,
        this->GetPerspective()->GetId());
  }
  if (this->GetActivePart() != 0)
  {
    if (this->GetActivePart().Cast<IViewPart> ())
    {
      IViewReference::Pointer ref = this->GetReference(this->GetActivePart()).Cast<IViewReference>();
      if (ref)
      {
        childMem->PutString(WorkbenchConstants::TAG_ACTIVE_PART,
            ViewFactory::GetKey(ref));
      }
    }
    else
    {
      childMem->PutString(WorkbenchConstants::TAG_ACTIVE_PART,
          this->GetActivePart()->GetSite()->GetId());
    }
  }

  // Save each perspective in opened order
  for (PerspectiveList::PerspectiveListType::iterator itr = perspList.Begin();
      itr != perspList.End(); ++itr)
  {
    IMemento::Pointer gChildMem = childMem->CreateChild(
        WorkbenchConstants::TAG_PERSPECTIVE);
    //result.merge(persp.saveState(gChildMem));
    result &= (*itr)->SaveState(gChildMem);
  }
  //  // Save working set if set
  //  if (workingSet != 0)
  //  {
  //    memento.putString(IWorkbenchConstants.TAG_WORKING_SET,
  //        workingSet .getName());
  //  }
  //
  //  IMemento workingSetMem = memento .createChild(
  //      IWorkbenchConstants.TAG_WORKING_SETS);
  //  for (int i = 0; i < workingSets.length; i++)
  //  {
  //    workingSetMem.createChild(IWorkbenchConstants.TAG_WORKING_SET,
  //        workingSets[i].getName());
  //  }
  //
  //  if (aggregateWorkingSetId != 0)
  //  {
  //    memento.putString(ATT_AGGREGATE_WORKING_SET_ID, aggregateWorkingSetId);
  //  }
  //
  //  navigationHistory.saveState(memento .createChild(
  //      IWorkbenchConstants.TAG_NAVIGATION_HISTORY));
  //
  // save the sticky activation state
  stickyViewMan->Save(memento);

  return result;
}

QString WorkbenchPage::GetId(IWorkbenchPart::Pointer part)
{
  return this->GetId(this->GetReference(part));
}

QString WorkbenchPage::GetId(IWorkbenchPartReference::Pointer ref)
{
  if (ref == 0)
  {
    return "0"; //$NON-NLS-1$
  }
  return ref->GetId();
}

void WorkbenchPage::SetActivePart(IWorkbenchPart::Pointer newPart)
{
  // Optimize it.
  if (this->GetActivePart() == newPart)
  {
    return;
  }

  if (partBeingActivated != 0)
  {
    if (partBeingActivated->GetPart(false) != newPart)
    {
      WorkbenchPlugin::Log(ctkRuntimeException(
                             QString("WARNING: Prevented recursive attempt to activate part ")
                             + this->GetId(newPart)
                             + " while still in the middle of activating part " + this->GetId(
                               partBeingActivated)));
    }
    return;
  }

  //No need to change the history if the active editor is becoming the
  // active part
  //  String label = 0; // debugging only
  //  if (UIStats.isDebugging(UIStats.ACTIVATE_PART))
  //  {
  //    label = newPart != 0 ? newPart.getTitle() : "none"; //$NON-NLS-1$
  //  }
  try
  {
    IWorkbenchPartReference::Pointer partref = this->GetReference(newPart);
    IWorkbenchPartReference::Pointer realPartRef;
    if (newPart != 0)
    {
      IWorkbenchPartSite::Pointer site = newPart->GetSite();
      if (site.Cast<PartSite> () != 0)
      {
        realPartRef = site.Cast<PartSite> ()->GetPane()->GetPartReference();
      }
    }

    partBeingActivated = realPartRef;

    //UIStats.start(UIStats.ACTIVATE_PART, label);
    // Notify perspective. It may deactivate fast view.
    Perspective::Pointer persp = this->GetActivePerspective();
    if (persp != 0)
    {
      persp->PartActivated(newPart);
    }

    // Deactivate old part
    IWorkbenchPart::Pointer oldPart = this->GetActivePart();
    if (oldPart != 0)
    {
      this->DeactivatePart(oldPart);
    }

    // Set active part.
    if (newPart != 0)
    {
      activationList->SetActive(newPart);
      if (newPart.Cast<IEditorPart> () != 0)
      {
        this->MakeActiveEditor(realPartRef.Cast<IEditorReference> ());
      }
    }
    this->ActivatePart(newPart);

    actionSwitcher.UpdateActivePart(newPart);

    partList->SetActivePart(partref);
  }
  catch (std::exception& e)
  {
    partBeingActivated = nullptr;
    //    Object blame = newPart == 0 ? (Object) this : newPart;
    //    UIStats.end(UIStats.ACTIVATE_PART, blame, label);
    throw e;
  }

  partBeingActivated = nullptr;
}

void WorkbenchPage::SetEditorAreaVisible(bool showEditorArea)
{
  Perspective::Pointer persp = this->GetActivePerspective();
  if (persp == 0)
  {
    return;
  }
  if (showEditorArea == persp->IsEditorAreaVisible())
  {
    return;
  }
  //  // If parts change always update zoom.
  //  if (isZoomed())
  //  {
  //    zoomOut();
  //  }
  // Update editor area visibility.
  IWorkbenchPage::Pointer thisPage(this);
  if (showEditorArea)
  {
    persp->ShowEditorArea();
    window->FirePerspectiveChanged(thisPage, this->GetPerspective(),
        CHANGE_EDITOR_AREA_SHOW);
  }
  else
  {
    persp->HideEditorArea();
    this->UpdateActivePart();
    window->FirePerspectiveChanged(thisPage, this->GetPerspective(),
        CHANGE_EDITOR_AREA_HIDE);
  }
}

/**
 * Sets the layout of the page. Assumes the new perspective is not 0.
 * Keeps the active part if possible. Updates the window menubar and
 * toolbar if necessary.
 */
void WorkbenchPage::SetPerspective(Perspective::Pointer newPersp)
{
  // Don't do anything if already active layout
  Perspective::Pointer oldPersp = this->GetActivePerspective();
  if (oldPersp == newPersp)
  {
    return;
  }

  window->LargeUpdateStart();
  std::exception exc;
  bool exceptionOccured = false;
  try
  {
    IWorkbenchPage::Pointer thisPage(this);
    if (oldPersp != 0)
    {
      // fire the pre-deactivate
      window->FirePerspectivePreDeactivate(thisPage, oldPersp->GetDesc());
    }

    if (newPersp != 0)
    {
      bool status = newPersp->RestoreState();
      if (!status)
      {
        QString title = "Restoring problems";
        QString msg = "Unable to read workbench state.";
        QMessageBox::critical(reinterpret_cast<QWidget*>(this->GetWorkbenchWindow()->GetShell()->GetControl()),
                                                         title, msg);
      }
    }

    // Deactivate the old layout
    if (oldPersp != 0)
    {
      oldPersp->OnDeactivate();

      // Notify listeners of deactivation
      window->FirePerspectiveDeactivated(thisPage, oldPersp->GetDesc());
    }

    // Activate the new layout
    perspList.SetActive(newPersp);
    if (newPersp != 0)
    {
      newPersp->OnActivate();

      // Notify listeners of activation
      window->FirePerspectiveActivated(thisPage, newPersp->GetDesc());
    }

    this->UpdateVisibility(oldPersp, newPersp);

    // Update the window
    //TODO action sets
    //window->UpdateActionSets();

    // Update sticky views
    stickyViewMan->Update(oldPersp, newPersp);

  }
  catch (std::exception& e)
  {
    exc = e;
    exceptionOccured = true;
  }

  window->LargeUpdateEnd();
  if (newPersp == 0)
  {
    return;
  }
  IPerspectiveDescriptor::Pointer desc = newPersp->GetDesc();
  if (desc == 0)
  {
    return;
  }
  if (dirtyPerspectives.erase(desc->GetId()))
  {
    this->SuggestReset();
  }

  if (exceptionOccured)
  throw exc;
}

void WorkbenchPage::UpdateVisibility(Perspective::Pointer oldPersp,
    Perspective::Pointer newPersp)
{

  // Flag all parts in the old perspective
  QList<IViewReference::Pointer> oldRefs;
  if (oldPersp != 0)
  {
    oldRefs = oldPersp->GetViewReferences();
    for (int i = 0; i < oldRefs.size(); i++)
    {
      PartPane::Pointer pane =
      oldRefs[i].Cast<WorkbenchPartReference> ()->GetPane();
      pane->SetInLayout(false);
    }
  }

  PerspectiveHelper* pres = nullptr;
  // Make parts in the new perspective visible
  if (newPersp != 0)
  {
    pres = newPersp->GetPresentation();
    QList<IViewReference::Pointer> newRefs =
    newPersp->GetViewReferences();
    for (int i = 0; i < newRefs.size(); i++)
    {
      WorkbenchPartReference::Pointer ref = newRefs[i].Cast<
      WorkbenchPartReference> ();
      PartPane::Pointer pane = ref->GetPane();
      if (pres->IsPartVisible(ref))
      {
        activationList->BringToTop(ref);
      }

      pane->SetInLayout(true);
    }
  }

  this->UpdateActivePart();

  // Hide any parts in the old perspective that are no longer visible
  for (int i = 0; i < oldRefs.size(); i++)
  {
    WorkbenchPartReference::Pointer ref = oldRefs[i].Cast<
    WorkbenchPartReference> ();

    PartPane::Pointer pane = ref->GetPane();
    if (pres == nullptr || !pres->IsPartVisible(ref))
    {
      pane->SetVisible(false);
    }
  }
}

/**
 * Sets the perspective.
 *
 * @param desc
 *            identifies the new perspective.
 */
void WorkbenchPage::SetPerspective(IPerspectiveDescriptor::Pointer desc)
{
  if (this->GetPerspective() == desc)
  {
    return;
  }
  //  // Going from multiple to single rows can make the coolbar
  //  // and its adjacent views appear jumpy as perspectives are
  //  // switched. Turn off redraw to help with this.
  //  ICoolBarManager2 mgr = (ICoolBarManager2) window.getCoolBarManager2();
  std::exception exc;
  bool exceptionOccured = false;
  try
  {
    //mgr.getControl2().setRedraw(false);
    //getClientComposite().setRedraw(false);

    // Run op in busy cursor.
    //    BusyIndicator.showWhile(0, new Runnable()
    //        {
    //        public void WorkbenchPage::run()
    //          {
    this->BusySetPerspective(desc);
    //          }
    //        });
  }
  catch (std::exception& e)
  {
    exc = e;
    exceptionOccured = true;
  }

  //  getClientComposite().setRedraw(true);
  //  mgr.getControl2().setRedraw(true);
  IWorkbenchPart::Pointer part = this->GetActivePart();
  if (part != 0)
  {
    part->SetFocus();
  }

  if (exceptionOccured)
  throw exc;
}

PartService* WorkbenchPage::GetPartService()
{
  return dynamic_cast<PartService*> (partList->GetPartService());
}

void WorkbenchPage::ResetToolBarLayout()
{
  //  ICoolBarManager2 mgr = (ICoolBarManager2) window.getCoolBarManager2();
  //  mgr.resetItemOrder();
}

IViewPart::Pointer WorkbenchPage::ShowView(const QString& viewID)
{
  return this->ShowView(viewID, "", VIEW_ACTIVATE);
}

IViewPart::Pointer WorkbenchPage::ShowView(const QString& viewID,
    const QString& secondaryID, int mode)
{

  if (secondaryID != "")
  {
    if (secondaryID.size() == 0 || secondaryID.indexOf(ViewFactory::ID_SEP) != -1)
    {
      throw ctkInvalidArgumentException(
          "Illegal secondary id (cannot be empty or contain a colon)");
    }
  }
  if (!this->CertifyMode(mode))
  {
    throw ctkInvalidArgumentException("Illegal view mode");
  }

  // Run op in busy cursor.

  //    BusyIndicator.showWhile(0, new Runnable()
  //        {
  //        public void WorkbenchPage::run()
  //          {
  //  try
  //  {
  return this->BusyShowView(viewID, secondaryID, mode);
  //  } catch (PartInitException& e)
  //  {
  //    result = e;
  //  }
  //          }
  //        });

}

bool WorkbenchPage::CertifyMode(int mode)
{
  if (mode == VIEW_ACTIVATE || mode == VIEW_VISIBLE || mode == VIEW_CREATE)
  return true;

  return false;
}

QList<IEditorReference::Pointer> WorkbenchPage::GetSortedEditors()
{
  return activationList->GetEditors();
}

QList<IPerspectiveDescriptor::Pointer> WorkbenchPage::GetOpenPerspectives()
{
  QList<Perspective::Pointer> opened = perspList.GetOpenedPerspectives();
  QList<IPerspectiveDescriptor::Pointer> result;
  for (QList<Perspective::Pointer>::iterator iter = opened.begin(); iter
      != opened.end(); ++iter)
  {
    result.push_back((*iter)->GetDesc());
  }
  return result;
}

QList<Perspective::Pointer> WorkbenchPage::GetOpenInternalPerspectives()
{
  return perspList.GetOpenedPerspectives();
}

Perspective::Pointer WorkbenchPage::GetFirstPerspectiveWithView(
    IViewPart::Pointer part)
{
  QListIterator<Perspective::Pointer> iter(perspList.GetSortedPerspectives());
  iter.toBack();
  while(iter.hasPrevious())
  {
    Perspective::Pointer p = iter.previous();
    if (p->ContainsView(part))
    {
      return p;
    }
  };
  // we should never get here
  return Perspective::Pointer(nullptr);
}

QList<IPerspectiveDescriptor::Pointer> WorkbenchPage::GetSortedPerspectives()
{
  QList<Perspective::Pointer> sortedArray =
  perspList.GetSortedPerspectives();
  QList<IPerspectiveDescriptor::Pointer> result;
  for (QList<Perspective::Pointer>::iterator iter = sortedArray.begin();
      iter != sortedArray.end(); ++iter)
  {
    result.push_back((*iter)->GetDesc());
  }
  return result;
}

QList<IWorkbenchPartReference::Pointer> WorkbenchPage::GetSortedParts()
{
  //return partList->GetParts(this->GetViewReferences());
  return activationList->GetParts();
}

IWorkbenchPartReference::Pointer WorkbenchPage::GetReference(
    IWorkbenchPart::Pointer part)
{
  if (part == 0)
  {
    return IWorkbenchPartReference::Pointer(nullptr);
  }
  IWorkbenchPartSite::Pointer site = part->GetSite();
  if (site.Cast<PartSite> () == 0)
  {
    return IWorkbenchPartReference::Pointer(nullptr);
  }
  PartSite::Pointer partSite = site.Cast<PartSite> ();
  PartPane::Pointer pane = partSite->GetPane();

  return partSite->GetPartReference();
}

// for dynamic UI
void WorkbenchPage::AddPerspective(Perspective::Pointer persp)
{
  perspList.Add(persp);
  IWorkbenchPage::Pointer thisPage(this);
  window->FirePerspectiveOpened(thisPage, persp->GetDesc());
}

QList<IViewReference::Pointer> WorkbenchPage::GetViewReferenceStack(
    IViewPart::Pointer part)
{
  // Sanity check.
  Perspective::Pointer persp = this->GetActivePerspective();
  if (persp == 0 || !this->CertifyPart(part))
  {
    return QList<IViewReference::Pointer>();
  }

  ILayoutContainer::Pointer container =
  part->GetSite().Cast<PartSite> ()->GetPane()->GetContainer();
  if (container.Cast<PartStack> () != 0)
  {
    PartStack::Pointer folder = container.Cast<PartStack> ();
    QList<IViewReference::Pointer> list;
    ILayoutContainer::ChildrenType children = folder->GetChildren();
    for (ILayoutContainer::ChildrenType::iterator childIter =
        children.begin(); childIter != children.end(); ++childIter)
    {
      LayoutPart::Pointer stackablePart = *childIter;
      if (stackablePart.Cast<PartPane> () != 0)
      {
        IViewReference::Pointer view =
        stackablePart.Cast<PartPane> ()->GetPartReference().Cast<
        IViewReference> ();
        if (view != 0)
        {
          list.push_back(view);
        }
      }
    }

    // sort the list by activation order (most recently activated first)
    std::sort(list.begin(), list.end(), ActivationOrderPred(activationList));

    return list;
  }

  QList<IViewReference::Pointer> result;
  result.push_back(this->GetReference(part).Cast<IViewReference> ());
  return result;
}

QList<IViewPart::Pointer> WorkbenchPage::GetViewStack(
    IViewPart::Pointer part)
{
  QList<IViewReference::Pointer> refStack = this->GetViewReferenceStack(
      part);

  QList<IViewPart::Pointer> result;

  for (int i = 0; i < refStack.size(); i++)
  {
    IViewPart::Pointer next = refStack[i]->GetView(false);
    if (next != 0)
    {
      result.push_back(next);
    }
  }

  return result;
}

void WorkbenchPage::ResizeView(IViewPart::Pointer part, int width, int height)
{
  SashInfo sashInfo;
  PartPane::Pointer pane = part->GetSite().Cast<PartSite> ()->GetPane();
  ILayoutContainer::Pointer container = pane->GetContainer();
  LayoutTree::Pointer tree =
  this->GetPerspectivePresentation()->GetLayout()->GetLayoutTree()->Find(
      container.Cast<PartStack> ());

  // retrieve our layout sashes from the layout tree
  this->FindSashParts(tree, pane->FindSashes(), sashInfo);

  // first set the width
  int deltaWidth = width - pane->GetBounds().width();
  if (sashInfo.right != 0)
  {
    QRect rightBounds = sashInfo.rightNode->GetBounds();
    // set the new ratio
    sashInfo.right->SetRatio(static_cast<float>((deltaWidth + sashInfo.right->GetBounds().x())
            - rightBounds.x()) / rightBounds.width());
    // complete the resize
    sashInfo.rightNode->SetBounds(rightBounds);
  }
  else if (sashInfo.left != 0)
  {
    QRect leftBounds = sashInfo.leftNode->GetBounds();
    // set the ratio
    sashInfo.left->SetRatio(static_cast<float>((sashInfo.left->GetBounds().x() - deltaWidth)
            - leftBounds.x()) / leftBounds.width());
    // complete the resize
    sashInfo.leftNode->SetBounds(sashInfo.leftNode->GetBounds());
  }

  // next set the height
  int deltaHeight = height - pane->GetBounds().height();
  if (sashInfo.bottom != 0)
  {
    QRect bottomBounds = sashInfo.bottomNode->GetBounds();
    // set the new ratio
    sashInfo.bottom->SetRatio(static_cast<float>((deltaHeight + sashInfo.bottom->GetBounds().y())
            - bottomBounds.y()) / bottomBounds.height());
    // complete the resize
    sashInfo.bottomNode->SetBounds(bottomBounds);
  }
  else if (sashInfo.top != 0)
  {
    QRect topBounds = sashInfo.topNode->GetBounds();
    // set the ratio
    sashInfo.top->SetRatio(static_cast<float>((sashInfo.top->GetBounds().y() - deltaHeight)
            - topBounds.y()) / topBounds.height());
    // complete the resize
    sashInfo.topNode->SetBounds(topBounds);
  }

}

void WorkbenchPage::FindSashParts(LayoutTree::Pointer tree,
    const PartPane::Sashes& sashes, SashInfo& info)
{
  LayoutTree::Pointer parent(tree->GetParent());
  if (parent == 0)
  {
    return;
  }

  if (parent->part.Cast<LayoutPartSash> () != 0)
  {
    // get the layout part sash from this tree node
    LayoutPartSash::Pointer sash = parent->part.Cast<LayoutPartSash> ();
    // make sure it has a sash control
    QWidget* control = sash->GetControl();
    if (control != nullptr)
    {
      // check for a vertical sash
      if (sash->IsVertical())
      {
        if (sashes.left == control)
        {
          info.left = sash;
          info.leftNode = parent->FindSash(sash);
        }
        else if (sashes.right == control)
        {
          info.right = sash;
          info.rightNode = parent->FindSash(sash);
        }
      }
      // check for a horizontal sash

      else
      {
        if (sashes.top == control)
        {
          info.top = sash;
          info.topNode = parent->FindSash(sash);
        }
        else if (sashes.bottom == control)
        {
          info.bottom = sash;
          info.bottomNode = parent->FindSash(sash);
        }
      }
    }
  }
  // recursive call to continue up the tree
  this->FindSashParts(parent, sashes, info);
}

QList<IWorkbenchPartReference::Pointer> WorkbenchPage::GetAllParts()
{
  QList<IViewReference::Pointer> views = this->GetViewFactory()->GetViews();
  QList<IEditorReference::Pointer> editors = this->GetEditorReferences();

  QList<IWorkbenchPartReference::Pointer> result;

  for (int i = 0; i < views.size(); i++)
  {
    result.push_back(views[i]);
  }

  for (QList<IEditorReference::Pointer>::iterator iter = editors.begin(); iter
      != editors.end(); ++iter)
  {
    result.push_back(*iter);
  }

  return result;
}

QList<IWorkbenchPartReference::Pointer> WorkbenchPage::GetOpenParts()
{
  QList<IWorkbenchPartReference::Pointer> refs = this->GetAllParts();
  QList<IWorkbenchPartReference::Pointer> result;

  for (int i = 0; i < refs.size(); i++)
  {
    IWorkbenchPartReference::Pointer reference = refs[i];

    IWorkbenchPart::Pointer part = reference->GetPart(false);
    if (part != 0)
    {
      result.push_back(reference);
    }
  }

  return result;
}

void WorkbenchPage::TestInvariants()
{
  Perspective::Pointer persp = this->GetActivePerspective();

  if (persp != 0)
  {

    persp->TestInvariants();

    // When we have widgets, ensure that there is no situation where the editor area is visible
    // and the perspective doesn't want an editor area.
    if (this->GetClientComposite()
        && editorPresentation->GetLayoutPart()->IsVisible())
    {
      poco_assert(persp->IsEditorAreaVisible());
    }
  }

}

IExtensionTracker* WorkbenchPage::GetExtensionTracker() const
{
  if (tracker.isNull())
  {
    tracker.reset(new UIExtensionTracker(GetWorkbenchWindow()->GetWorkbench()->GetDisplay()));
  }
  return tracker.data();
}

/*
 * (non-Javadoc)
 *
 * @see org.blueberry.ui.IWorkbenchPage#getPerspectiveShortcuts()
 */
QList<QString> WorkbenchPage::GetPerspectiveShortcuts()
{
  Perspective::Pointer persp = this->GetActivePerspective();
  if (persp == 0)
  {
    return QList<QString>();
  }
  return persp->GetPerspectiveShortcuts();
}

QList<QString> WorkbenchPage::GetShowViewShortcuts()
{
  Perspective::Pointer persp = this->GetActivePerspective();
  if (persp == 0)
  {
    return QList<QString>();
  }
  return persp->GetShowViewShortcuts();
}

void WorkbenchPage::SuggestReset()
{
  IWorkbench* workbench = this->GetWorkbenchWindow()->GetWorkbench();
  //    workbench.getDisplay().asyncExec(new Runnable()
  //        {
  //        public void WorkbenchPage::run()
  //          {
  Shell::Pointer parentShell;

  IWorkbenchWindow::Pointer window = workbench->GetActiveWorkbenchWindow();
  if (window == 0)
  {
    if (workbench->GetWorkbenchWindowCount() == 0)
    {
      return;
    }
    window = workbench->GetWorkbenchWindows()[0];
  }

  parentShell = window->GetShell();

  if (QMessageBox::question(parentShell.IsNull() ? nullptr : reinterpret_cast<QWidget*>(parentShell->GetControl()),
                            "Reset Perspective?",
                            "Changes to installed plug-ins have affected this perspective. Would you like to reset this perspective to accept these changes?") ==
      QMessageBox::Yes)
  {
    IWorkbenchPage::Pointer page = window->GetActivePage();
    if (page == 0)
    {
      return;
    }
    page->ResetPerspective();
  }
  //          }
  //        });

}

bool WorkbenchPage::IsPartVisible(
    IWorkbenchPartReference::Pointer reference)
{
  IWorkbenchPart::Pointer part = reference->GetPart(false);
  // Can't be visible if it isn't created yet
  if (part == 0)
  {
    return false;
  }

  return this->IsPartVisible(part);
}

}
