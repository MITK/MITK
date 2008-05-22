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

#include "cherryWorkbenchPage.h"
#include "cherryPartSite.h"
#include "cherryWorkbenchRegistryConstants.h"

#include "../cherryUIException.h"
#include "../cherryPlatformUI.h"

#include <org.opencherry.osgi/cherryPlatform.h>

namespace cherry
{

void WorkbenchPage::ActionSwitcher::UpdateActivePart(
    IWorkbenchPart::Pointer newPart)
{
  if (activePart == newPart)
  {
    return;
  }

  bool isNewPartAnEditor = newPart.Cast<IEditorPart>().IsNotNull();
  if (isNewPartAnEditor)
  {
    std::string oldId;
    if (topEditor.IsNotNull())
    {
      oldId = topEditor->GetSite()->GetId();
    }
    std::string newId = newPart->GetSite()->GetId();

    // if the active part is an editor and the new editor
    // is the same kind of editor, then we don't have to do
    // anything
    if (activePart == topEditor && newId == oldId)
    {
      activePart = newPart;
      topEditor = newPart.Cast<IEditorPart>();
      return;
    }

    // remove the contributions of the old editor
    // if it is a different kind of editor
    if (oldId != newId)
    {
      this->DeactivateContributions(topEditor, true);
    }

    // if a view was the active part, disable its contributions
    if (activePart.IsNotNull() && activePart != topEditor)
    {
      this->DeactivateContributions(activePart, true);
    }

    // show (and enable) the contributions of the new editor
    // if it is a different kind of editor or if the
    // old active part was a view
    if (newId != oldId || activePart != topEditor)
    {
      this->ActivateContributions(newPart, true);
    }

  }
  else if (newPart.IsNull())
  {
    if (activePart.IsNotNull())
    {
      // remove all contributions
      this->DeactivateContributions(activePart, true);
    }
  }
  else
  {
    // new part is a view

    // if old active part is a view, remove all contributions,
    // but if old part is an editor only disable
    if (activePart.IsNotNull())
    {
      this->DeactivateContributions(activePart,
          activePart.Cast<IViewPart>().IsNotNull());
    }

    this->ActivateContributions(newPart, true);
  }

//  ArrayList newActionSets = null;
//  if (isNewPartAnEditor || (activePart == topEditor && newPart == null))
//  {
//    newActionSets = calculateActionSets(newPart, null);
//  }
//  else
//  {
//    newActionSets = calculateActionSets(newPart, topEditor);
//  }
//
//  if (!updateActionSets(newActionSets))
//  {
//    updateActionBars();
//  }

  if (isNewPartAnEditor)
  {
    topEditor = newPart.Cast<IEditorPart>();
  }
  else if (activePart == topEditor && newPart.IsNull())
  {
    // since we removed all the contributions, we clear the top
    // editor
    topEditor = 0;
  }

  activePart = newPart;
}

void WorkbenchPage::ActionSwitcher::UpdateTopEditor(
    IEditorPart::Pointer newEditor)
{
  if (topEditor == newEditor)
  {
    return;
  }

  if (activePart == topEditor)
  {
    this->UpdateActivePart(newEditor);
    return;
  }

  std::string oldId;
  if (topEditor.IsNotNull())
  {
    oldId = topEditor->GetSite()->GetId();
  }
  std::string newId;
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
  if (topEditor.IsNotNull())
  {
    this->DeactivateContributions(topEditor, true);
  }

  // Show (disabled) the contributions of the new editor
  if (newEditor.IsNotNull())
  {
    this->ActivateContributions(newEditor, false);
  }

//  ArrayList newActionSets = calculateActionSets(activePart, newEditor);
//  if (!updateActionSets(newActionSets))
//  {
//    updateActionBars();
//  }

  topEditor = newEditor;
}

void WorkbenchPage::ActionSwitcher::ActivateContributions(
    IWorkbenchPart::Pointer part, bool enable)
{
  PartSite::Pointer site = part->GetSite().Cast<PartSite>();
  //site->ActivateActionBars(enable);
}

void WorkbenchPage::ActionSwitcher::DeactivateContributions(
    IWorkbenchPart::Pointer part, bool remove)
{
  PartSite::Pointer site = part->GetSite().Cast<PartSite>();
  //site->DeactivateActionBars(remove);
}

IExtensionPoint* WorkbenchPage::GetPerspectiveExtensionPoint()
{
  return Platform::GetExtensionPointerService()->GetExtensionPoint(PlatformUI::PLUGIN_ID,
      WorkbenchRegistryConstants::PL_PERSPECTIVE_EXTENSIONS);
}

WorkbenchPage::WorkbenchPage(WorkbenchWindow::Pointer w,
    const std::string& layoutID, IAdaptable* input)
{
  if (layoutID == "")
  {
    throw WorkbenchException("Perspective ID is undefined");
  }
  this->Init(w, layoutID, input, true);
}

WorkbenchPage::WorkbenchPage(WorkbenchWindow::Pointer w, IAdaptable* input)
{
  this->Init(w, "", input, false);
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

//  if (composite!=null && composite.isVisible() && !((GrabFocus)Tweaklets.get(GrabFocus.KEY)).grabFocusAllowed(part))
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
            IPartPane::Pointer pane = this->GetPane(part);
            pane->SetFocus();
            PartSite::Pointer site = part->GetSite().Cast<PartSite>();
            pane->ShowFocus(true);
            //this->UpdateTabList(part);
            //SubActionBars bars = (SubActionBars) site.getActionBars();
            //bars.partChanged(part);
          }
//        }
//      }
//      );
    }

//void WorkbenchPage::AddSelectionListener(ISelectionListener listener)
//{
//  selectionService.addSelectionListener(listener);
//}

//void WorkbenchPage::AddSelectionListener(const std::string& partId,
//    ISelectionListener listener)
//{
//  selectionService.addSelectionListener(partId, listener);
//}

//void WorkbenchPage::AddPostSelectionListener(ISelectionListener listener)
//{
//  selectionService.addPostSelectionListener(listener);
//}

//void WorkbenchPage::AddPostSelectionListener(const std::string partId,
//    ISelectionListener listener)
//{
//  selectionService.addPostSelectionListener(partId, listener);
//}

//ILayoutContainer WorkbenchPage::GetContainer(IWorkbenchPart::Pointer part)
//{
//  PartPane pane = getPane(part);
//  if (pane == null)
//  {
//    return null;
//  }
//
//  return pane.getContainer();
//}

//ILayoutContainer WorkbenchPage::GetContainer(
//    IWorkbenchPartReference::Pointer part)
//{
//  PartPane pane = getPane(part);
//  if (pane == null)
//  {
//    return null;
//  }
//
//  return pane.getContainer();
//}

IPartPane::Pointer WorkbenchPage::GetPane(IWorkbenchPart::Pointer part)
{
  if (part.IsNull())
  {
    return 0;
  }
  return this->GetPane(this->GetReference(part));
}

IPartPane::Pointer WorkbenchPage::GetPane(IWorkbenchPartReference::Pointer part)
{
  if (part.IsNull())
  {
    return 0;
  }

  return part.Cast<WorkbenchPartReference>()->GetPane();
}

bool WorkbenchPage::InternalBringToTop(IWorkbenchPartReference::Pointer part)
{

  bool broughtToTop = false;

  // Move part.
  if (part.Cast<IEditorReference>().IsNotNull())
  {
    ILayoutContainer container = getContainer(part);
    if (container instanceof PartStack)
    {
      PartStack stack = (PartStack)container;
      PartPane newPart = getPane(part);
      if (stack.getSelection() != newPart)
      {
        stack.setSelection(newPart);
      }
      broughtToTop = true;
    }
  }
  else if (part.Cast<IViewReference>().IsNotNull())
  {
//    Perspective persp = getActivePerspective();
//    if (persp != null)
//    {
//      broughtToTop = persp.bringToTop((IViewReference)part);
//    }
  }

  // Ensure that this part is considered the most recently activated part
  // in this stack
  //activationList.bringToTop(part);

  return broughtToTop;
}

void WorkbenchPage::BringToTop(IWorkbenchPart::Pointer part)
{
  // Sanity check.
  Perspective persp = getActivePerspective();
  if (persp == null || !certifyPart(part))
  {
    return;
  }

//  if (!((GrabFocus)Tweaklets.get(GrabFocus.KEY)).grabFocusAllowed(part))
//  {
//    return;
//  }

  String label = null; // debugging only
  if (UIStats.isDebugging(UIStats.BRING_PART_TO_TOP))
  {
    label = part != null ? part.getTitle() : "none"; //$NON-NLS-1$
  }

  
    IWorkbenchPartReference::Pointer ref = this->GetReference(part);
    ILayoutContainer activeEditorContainer = getContainer(getActiveEditor());
    ILayoutContainer activePartContainer = getContainer(getActivePart());
    ILayoutContainer newPartContainer = getContainer(part);

    if (newPartContainer == activePartContainer)
    {
      makeActive(ref);
    }
    else if (newPartContainer == activeEditorContainer)
    {
      if (ref instanceof IEditorReference)
      {
        if (part!=null)
        {
          IWorkbenchPartSite site = part.getSite();
          if (site instanceof PartSite)
          {
            ref = ((PartSite) site).getPane()
            .getPartReference();
          }
        }
        makeActiveEditor((IEditorReference)ref);
      }
      else
      {
        makeActiveEditor(null);
      }
    }
    else
    {
      internalBringToTop(ref);
      if (ref != null)
      {
        partList.firePartBroughtToTop(ref);
      }
    }
  
}

//void WorkbenchPage::BusyResetPerspective()
//{
//
//  ViewIntroAdapterPart introViewAdapter = ((WorkbenchIntroManager) getWorkbenchWindow()
//      .getWorkbench().getIntroManager()).getViewIntroAdapterPart();
//  PartPane introPane = null;
//  boolean introFullScreen = false;
//  if (introViewAdapter != null)
//  {
//    introPane = ((PartSite) introViewAdapter.getSite()).getPane();
//    introViewAdapter.setHandleZoomEvents(false);
//    introFullScreen = introPane.isZoomed();
//  }
//
//  //try to prevent intro flicker.
//  if (introFullScreen)
//  {
//    window.getShell().setRedraw(false);
//  }
//
//  try
//  {
//
//    // Always unzoom
//    if (isZoomed())
//    {
//      zoomOut();
//    }
//
//    // Get the current perspective.
//    // This describes the working layout of the page and differs from
//    // the original template.
//    Perspective oldPersp = getActivePerspective();
//
//    // Map the current perspective to the original template.
//    // If the original template cannot be found then it has been deleted.
//    // In that case just return. (PR#1GDSABU).
//    IPerspectiveRegistry reg = WorkbenchPlugin.getDefault()
//    .getPerspectiveRegistry();
//    PerspectiveDescriptor desc = (PerspectiveDescriptor) reg
//    .findPerspectiveWithId(oldPersp.getDesc().getId());
//    if (desc == null)
//    {
//      desc = (PerspectiveDescriptor) reg
//      .findPerspectiveWithId(((PerspectiveDescriptor) oldPersp
//              .getDesc()).getOriginalId());
//    }
//    if (desc == null)
//    {
//      return;
//    }
//
//    // Notify listeners that we are doing a reset.
//    window.firePerspectiveChanged(this, desc, CHANGE_RESET);
//
//    // Create new persp from original template.
//    // Suppress the perspectiveOpened and perspectiveClosed events otherwise it looks like two
//    // instances of the same perspective are open temporarily (see bug 127470).
//    Perspective newPersp = createPerspective(desc, false);
//    if (newPersp == null)
//    {
//      // We're not going through with the reset, so it is complete.
//      window
//      .firePerspectiveChanged(this, desc,
//          CHANGE_RESET_COMPLETE);
//      return;
//    }
//
//    // Update the perspective list and shortcut
//    perspList.swap(oldPersp, newPersp);
//
//    // Install new persp.
//    setPerspective(newPersp);
//
//    // Destroy old persp.
//    disposePerspective(oldPersp, false);
//
//    // Update the Coolbar layout.
//    resetToolBarLayout();
//
//    // restore the maximized intro
//    if (introViewAdapter != null)
//    {
//      try
//      {
//        // ensure that the intro is visible in the new perspective
//        showView(IIntroConstants.INTRO_VIEW_ID);
//        if (introFullScreen)
//        {
//          toggleZoom(introPane.getPartReference());
//        }
//      }
//      catch (PartInitException e)
//      {
//        WorkbenchPlugin.log("Could not restore intro", //$NON-NLS-1$
//            WorkbenchPlugin.getStatus(e));
//      }finally
//      {
//        // we want the intro back to a normal state before we fire the event
//        introViewAdapter.setHandleZoomEvents(true);
//      }
//    }
//    // Notify listeners that we have completed our reset.
//    window.firePerspectiveChanged(this, desc, CHANGE_RESET_COMPLETE);
//  }
//  finally
//  {
//    // reset the handling of zoom events (possibly for the second time) in case there was 
//    // an exception thrown
//    if (introViewAdapter != null)
//    {
//      introViewAdapter.setHandleZoomEvents(true);
//    }
//
//    if (introFullScreen)
//    {
//      window.getShell().setRedraw(true);
//    }
//  }
//
//}

//void WorkbenchPage::BusySetPerspective(IPerspectiveDescriptor::Pointer desc)
//{
//  // Create new layout.
//  String label = desc.getId(); // debugging only
//  Perspective newPersp = null;
//  try
//  {
//    UIStats.start(UIStats.SWITCH_PERSPECTIVE, label);
//    PerspectiveDescriptor realDesc = (PerspectiveDescriptor) desc;
//    newPersp = findPerspective(realDesc);
//    if (newPersp == null)
//    {
//      newPersp = createPerspective(realDesc, true);
//      if (newPersp == null)
//      {
//        return;
//      }
//    }
//
//    // Change layout.
//    setPerspective(newPersp);
//  }
//  finally
//  {
//    UIStats.end(UIStats.SWITCH_PERSPECTIVE, desc.getId(), label);
//  }
//}

IViewPart::Pointer WorkbenchPage::BusyShowView(const std::string& viewID,
    const std::string& secondaryID, int mode)
{
  Perspective persp = getActivePerspective();
  if (persp == null)
  {
    return null;
  }

  // If this view is already visible just return.
  IViewReference ref = persp.findView(viewID, secondaryID);
  IViewPart view = null;
  if (ref != null)
  {
    view = ref.getView(true);
  }
  if (view != null)
  {
    busyShowView(view, mode);
    return view;
  }

  // Show the view.
  view = persp.showView(viewID, secondaryID);
  if (view != null)
  {
    busyShowView(view, mode);

    IWorkbenchPartReference partReference = getReference(view);
    PartPane partPane = getPane(partReference);
    partPane.setInLayout(true);

    window.firePerspectiveChanged(this, getPerspective(), partReference,
        CHANGE_VIEW_SHOW);
    window.firePerspectiveChanged(this, getPerspective(), CHANGE_VIEW_SHOW);
  }
  return view;
}

void WorkbenchPage::BusyShowView(IViewPart::Pointer part, int mode)
{
  if (!((GrabFocus)Tweaklets.get(GrabFocus.KEY)).grabFocusAllowed(part))
  {
    return;
  }
  if (mode == VIEW_ACTIVATE)
  {
    activate(part);
  }
  else if (mode == VIEW_VISIBLE)
  {
    IWorkbenchPartReference ref = getActivePartReference();
    // if there is no active part or it's not a view, bring to top
    if (ref == null || !(ref instanceof IViewReference))
    {
      bringToTop(part);
    }
    else
    {
      // otherwise check to see if the we're in the same stack as the active view
      IViewReference activeView = (IViewReference) ref;
      IViewReference[] viewStack = getViewReferenceStack(part);
      for (int i = 0; i < viewStack.length; i++)
      {
        if (viewStack[i].equals(activeView))
        {
          return;
        }
      }
      bringToTop(part);
    }
  }
}

bool WorkbenchPage::CertifyPart(IWorkbenchPart::Pointer part)
{
  //Workaround for bug 22325
  if (part != null && !(part.getSite() instanceof PartSite)
)    {
      return false;
    }

    if (part instanceof IEditorPart)
    {
      IEditorReference ref = (IEditorReference) getReference(part);
      return ref != null && getEditorManager().containsEditor(ref);
    }
    if (part instanceof IViewPart)
    {
      Perspective persp = getActivePerspective();
      return persp != null && persp.containsView((IViewPart) part);
    }
    return false;
  }

  bool WorkbenchPage::Close()
  {
    final boolean[] ret = new boolean[1];
    BusyIndicator.showWhile(null, new Runnable()
        {
        public void WorkbenchPage::run()
          {
            ret[0] = window.closePage(WorkbenchPage.this, true);
          }
        });
    return ret[0];
  }

  bool WorkbenchPage::CloseAllSavedEditors()
  {
    // get the Saved editors
    IEditorReference editors[] = getEditorReferences();
    IEditorReference savedEditors[] = new IEditorReference[editors.length];
    int j = 0;
    for (int i = 0; i < editors.length; i++)
    {
      IEditorReference editor = editors[i];
      if (!editor.isDirty())
      {
        savedEditors[j++] = editor;
      }
    }
    //there are no unsaved editors
    if (j == 0)
    {
      return true;
    }
    IEditorReference[] newSaved = new IEditorReference[j];
    System.arraycopy(savedEditors, 0, newSaved, 0, j);
    return closeEditors(newSaved, false);
  }

  bool WorkbenchPage::CloseAllEditors(bool save)
  {
    return closeEditors(getEditorReferences(), save);
  }

  void WorkbenchPage::UpdateActivePart()
  {

    if (isDeferred())
    {
      return;
    }

    IWorkbenchPartReference oldActivePart = partList.getActivePartReference();
    IWorkbenchPartReference oldActiveEditor = partList.getActiveEditorReference();
    IWorkbenchPartReference newActivePart = null;
    IEditorReference newActiveEditor = null;

    if (!window.isClosing())
    {
      // If an editor is active, try to keep an editor active
      if (oldActivePart == oldActiveEditor)
      {
        newActiveEditor = (IEditorReference)activationList.getActiveReference(true);
        newActivePart = newActiveEditor;
        if (newActivePart == null)
        {
          // Only activate a non-editor if there's no editors left
          newActivePart = activationList.getActiveReference(false);
        }
      }
      else
      {
        // If a non-editor is active, activate whatever was activated most recently
        newActivePart = activationList.getActiveReference(false);

        if (newActivePart instanceof IEditorReference)
        {
          // If that happens to be an editor, make it the active editor as well
          newActiveEditor = (IEditorReference)newActivePart;
        }
        else
        {
          // Otherwise, select whatever editor was most recently active
          newActiveEditor = (IEditorReference)activationList.getActiveReference(true);
        }
      }
    }

    if (newActiveEditor != oldActiveEditor)
    {
      makeActiveEditor(newActiveEditor);
    }

    if (newActivePart != oldActivePart)
    {
      makeActive(newActivePart);
    }
  }

  void WorkbenchPage::MakeActive(IWorkbenchPartReference::Pointer ref)
  {
    if (ref == null)
    {
      setActivePart(null);
    }
    else
    {
      IWorkbenchPart newActive = ref.getPart(true);
      if (newActive == null)
      {
        setActivePart(null);
      }
      else
      {
        activate(newActive);
      }
    }
  }

  void WorkbenchPage::MakeActiveEditor(IEditorReference::Pointer ref)
  {
    if (ref == getActiveEditorReference())
    {
      return;
    }

    IEditorPart part = (ref == null) ? null : ref.getEditor(true);

    if (part != null)
    {
      editorMgr.setVisibleEditor(ref, false);
      navigationHistory.markEditor(part);
    }

    actionSwitcher.updateTopEditor(part);

    if (ref != null)
    {
      activationList.bringToTop(getReference(part));
    }

    partList.setActiveEditor(ref);
  }

  bool WorkbenchPage::CloseEditors(const std::vector<IEditorReference::Pointer>& refArray, bool save)
  {
    if (refArray.length == 0)
    {
      return true;
    }

    // Check if we're being asked to close any parts that are already closed or cannot
    // be closed at this time
    ArrayList toClose = new ArrayList();
    for (int i = 0; i < refArray.length; i++)
    {
      IEditorReference reference = refArray[i];

      // If we're in the middle of creating this part, this is a programming error. Abort the entire
      // close operation. This usually occurs if someone tries to open a dialog in a method that
      // isn't allowed to do so, and a *syncExec tries to close the part. If this shows up in a log
      // file with a dialog's event loop on the stack, then the code that opened the dialog is usually
      // at fault.
      if (reference == partBeingActivated)
      {
        WorkbenchPlugin.log(new RuntimeException("WARNING: Blocked recursive attempt to close part " //$NON-NLS-1$
                + partBeingActivated.getId() + " while still in the middle of activating it")); //$NON-NLS-1$
        return false;
      }

      if(reference instanceof WorkbenchPartReference)
      {
        WorkbenchPartReference ref = (WorkbenchPartReference) reference;

        // If we're being asked to close a part that is disposed (ie: already closed),
        // skip it and proceed with closing the remaining parts.
        if (ref.isDisposed())
        {
          continue;
        }
      }

      toClose.add(reference);
    }

    IEditorReference[] editorRefs = (IEditorReference[]) toClose.toArray(new IEditorReference[toClose.size()]);

    // notify the model manager before the close
    List partsToClose = new ArrayList();
    for (int i = 0; i < editorRefs.length; i++)
    {
      IEditorPart refPart = editorRefs[i].getEditor(false);
      if (refPart != null)
      {
        partsToClose.add(refPart);
      }
    }
    SaveablesList modelManager = null;
    Object postCloseInfo = null;
    if(partsToClose.size()>0)
    {
      modelManager = (SaveablesList) getWorkbenchWindow().getService(ISaveablesLifecycleListener.class);
      // this may prompt for saving and return null if the user canceled:
      postCloseInfo = modelManager.preCloseParts(partsToClose, save, getWorkbenchWindow());
      if (postCloseInfo==null)
      {
        return false;
      }
    }

    // Fire pre-removal changes 
    for (int i = 0; i < editorRefs.length; i++)
    {
      IEditorReference ref = editorRefs[i];

      // Notify interested listeners before the close
      window.firePerspectiveChanged(this, getPerspective(), ref,
          CHANGE_EDITOR_CLOSE);

    }

    deferUpdates(true);
    try
    {
      if(modelManager!=null)
      {
        modelManager.postClose(postCloseInfo);
      }

      // Close all editors.
      for (int i = 0; i < editorRefs.length; i++)
      {
        IEditorReference ref = editorRefs[i];

        // Remove editor from the presentation
        editorPresentation.closeEditor(ref);

        partRemoved((WorkbenchPartReference)ref);
      }
    }finally
    {
      deferUpdates(false);
    }

    // Notify interested listeners after the close
//    window.firePerspectiveChanged(this, getPerspective(),
//        CHANGE_EDITOR_CLOSE);

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
    editorPresentation.getLayoutPart().deferUpdates(false);
    updateActivePart();
    WorkbenchPartReference[] disposals = (WorkbenchPartReference[]) pendingDisposals.toArray(new WorkbenchPartReference[pendingDisposals.size()]);
    pendingDisposals.clear();
    for (int i = 0; i < disposals.length; i++)
    {
      WorkbenchPartReference reference = disposals[i];
      disposePart(reference);
    }

  }

  bool WorkbenchPage::IsDeferred()
  {
    return deferCount> 0;
  }

  bool WorkbenchPage::CloseEditor(IEditorReference::Pointer editorRef, bool save)
  {
    std::vector<IEditorReference::Pointer> list;
    list.push_back(editorRef);
    return closeEditors(list, save);
  }

  bool WorkbenchPage::CloseEditor(IEditorPart::Pointer editor, bool save)
  {
    IWorkbenchPartReference::Pointer ref = this->GetReference(editor);
    if (ref.Cast<IEditorReference>().IsNotNull())
    {
      std::vector<IEditorReference::Pointer> list;
      list.push_back(ref.Cast<IEditorRefernce>());
      return this->CloseEditors(list, save);
    }
    return false;
  }

//  void WorkbenchPage::ClosePerspective(IPerspectiveDescriptor::Pointer desc, bool saveParts, bool closePage)
//  {
//    Perspective persp = findPerspective(desc);
//    if (persp != null)
//    {
//      closePerspective(persp, saveParts, closePage);
//    }
//  }

//  void WorkbenchPage::ClosePerspective(Perspective::Pointer persp, bool saveParts, bool closePage)
//  {
//
//    // Always unzoom
//    if (isZoomed())
//    {
//      zoomOut();
//    }
//
//    List partsToSave = new ArrayList();
//    List viewsToClose = new ArrayList();
//    // collect views that will go away and views that are dirty
//    IViewReference[] viewReferences = persp.getViewReferences();
//    for (int i = 0; i < viewReferences.length; i++)
//    {
//      IViewReference reference = viewReferences[i];
//      if (getViewFactory().getReferenceCount(reference) == 1)
//      {
//        IViewPart viewPart = reference.getView(false);
//        if (viewPart != null)
//        {
//          viewsToClose.add(viewPart);
//          if (saveParts && reference.isDirty())
//          {
//            partsToSave.add(viewPart);
//          }
//        }
//      }
//    }
//    if (saveParts && perspList.size() == 1)
//    {
//      // collect editors that are dirty
//      IEditorReference[] editorReferences = getEditorReferences();
//      for (int i = 0; i < editorReferences.length; i++)
//      {
//        IEditorReference reference = editorReferences[i];
//        if (reference.isDirty())
//        {
//          IEditorPart editorPart = reference.getEditor(false);
//          if (editorPart != null)
//          {
//            partsToSave.add(editorPart);
//          }
//        }
//      }
//    }
//    if (saveParts && !partsToSave.isEmpty())
//    {
//      if (!EditorManager.saveAll(partsToSave, true, true, false, window))
//      {
//        // user canceled
//        return;
//      }
//    }
//    // Close all editors on last perspective close
//    if (perspList.size() == 1 && getEditorManager().getEditorCount()> 0)
//    {
//      // Close all editors
//      if (!closeAllEditors(false))
//      {
//        return;
//      }
//    }
//
//    // closeAllEditors already notified the saveables list about the editors.
//    SaveablesList saveablesList = (SaveablesList) getWorkbenchWindow().getWorkbench().getService(ISaveablesLifecycleListener.class);
//    // we took care of the saving already, so pass in false (postCloseInfo will be non-null)
//    Object postCloseInfo = saveablesList.preCloseParts(viewsToClose, false, getWorkbenchWindow());
//    saveablesList.postClose(postCloseInfo);
//
//    // Dispose of the perspective
//    boolean isActive = (perspList.getActive() == persp);
//    if (isActive)
//    {
//      setPerspective(perspList.getNextActive());
//    }
//    disposePerspective(persp, true);
//    if (closePage && perspList.size() == 0)
//    {
//      close();
//    }
//  }

//  void WorkbenchPage::CloseAllPerspectives(bool saveEditors, bool closePage)
//  {
//
//    if (perspList.isEmpty())
//    {
//      return;
//    }
//
//    // Always unzoom
//    if (isZoomed())
//    {
//      zoomOut();
//    }
//
//    if(saveEditors)
//    {
//      if (!saveAllEditors(true))
//      {
//        return;
//      }
//    }
//    // Close all editors
//    if (!closeAllEditors(false))
//    {
//      return;
//    }
//
//    // Deactivate the active perspective and part
//    setPerspective((Perspective) null);
//
//    // Close each perspective in turn
//    PerspectiveList oldList = perspList;
//    perspList = new PerspectiveList();
//    Iterator itr = oldList.iterator();
//    while (itr.hasNext())
//    {
//      closePerspective((Perspective) itr.next(), false, false);
//    }
//    if (closePage)
//    {
//      close();
//    }
//  }

//  void WorkbenchPage::CreateClientComposite()
//  {
//    final Composite parent = window.getPageComposite();
//    StartupThreading.runWithoutExceptions(new StartupRunnable()
//        {
//
//        public void WorkbenchPage::runWithException()
//          {
//            composite = new Composite(parent, SWT.NONE);
//            composite.setVisible(false); // Make visible on activate.
//            // force the client composite to be layed out
//            parent.layout();
//          }
//        });
//
//  }

//  Perspective::Pointer WorkbenchPage::CreatePerspective(PerspectiveDescriptor::Pointer desc, bool notify)
//  {
//    String label = desc.getId(); // debugging only
//    try
//    {
//      UIStats.start(UIStats.CREATE_PERSPECTIVE, label);
//      Perspective persp = ((WorkbenchImplementation) Tweaklets
//          .get(WorkbenchImplementation.KEY)).createPerspective(desc, this);
//      perspList.add(persp);
//      if (notify)
//      {
//        window.firePerspectiveOpened(this, desc);
//      }
//      //if the perspective is fresh and uncustomzied then it is not dirty
//      //no reset will be prompted for
//      if (!desc.hasCustomDefinition())
//      {
//        dirtyPerspectives.remove(desc.getId());
//      }
//      return persp;
//    }
//    catch (WorkbenchException e)
//    {
//      if (!((Workbench) window.getWorkbench()).isStarting())
//      {
//        MessageDialog
//        .openError(
//            window.getShell(),
//            WorkbenchMessages.Error,
//            NLS.bind(WorkbenchMessages.Workbench_showPerspectiveError,desc.getId() ));
//      }
//      return null;
//    }finally
//    {
//      UIStats.end(UIStats.CREATE_PERSPECTIVE, desc.getId(), label);
//    }
//  }

  void WorkbenchPage::PartAdded(WorkbenchPartReference::Pointer ref)
  {
    activationList.add(ref);
    partList.addPart(ref);
    this->UpdateActivePart();
  }

  void WorkbenchPage::PartRemoved(WorkbenchPartReference::Pointer ref)
  {
    activationList.remove(ref);
    this->DisposePart(ref);
  }

  void WorkbenchPage::DisposePart(WorkbenchPartReference::Pointer ref)
  {
    if (this->IsDeferred())
    {
      pendingDisposals.add(ref);
    }
    else
    {
      partList.removePart(ref);
      ref.dispose();
    }
  }

  void WorkbenchPage::DeactivatePart(IWorkbenchPart::Pointer part)
  {
    if (part.IsNotNull())
    {
      PartSite::Pointer site = part->GetSite().Cast<PartSite>();
      site->GetPane()->ShowFocus(false);
    }
  }

  void WorkbenchPage::DetachView(IViewReference::Pointer ref)
  {
//    Perspective persp = getActivePerspective();
//    if(persp == null)
//    {
//      return;
//    }
//
//    PerspectiveHelper presentation = persp.getPresentation();
//    presentation.detachPart(ref);
  }

  void WorkbenchPage::AttachView(IViewReference::Pointer ref)
  {
//    PerspectiveHelper presentation = getPerspectivePresentation();
//    presentation.attachPart(ref);
  }

  WorkbenchPage::~WorkbenchPage()
  {

    this->MakeActiveEditor(0);
    this->MakeActive(0);

    // Close and dispose the editors.
    this->CloseAllEditors(false);

    // Need to make sure model data is cleaned up when the page is
    // disposed. Collect all the views on the page and notify the
    // saveable list of a pre/post close. This will free model data.
    IWorkbenchPartReference[] partsToClose = getOpenParts();
    List dirtyParts = new ArrayList(partsToClose.length);
    for (int i = 0; i < partsToClose.length; i++)
    {
      IWorkbenchPart part = partsToClose[i].getPart(false);
      if (part != null && part instanceof IViewPart)
      {
        dirtyParts.add(part);
      }
    }
    SaveablesList saveablesList = (SaveablesList) getWorkbenchWindow().getWorkbench().getService(ISaveablesLifecycleListener.class);
    Object postCloseInfo = saveablesList.preCloseParts(dirtyParts, false,getWorkbenchWindow());
    saveablesList.postClose(postCloseInfo);

    // Get rid of perspectives. This will close the views.
//    Iterator itr = perspList.iterator();
//    while (itr.hasNext())
//    {
//      Perspective perspective = (Perspective) itr.next();
//      window.firePerspectiveClosed(this, perspective.getDesc());
//      perspective.dispose();
//    }
  

//    if (tracker != null)
//    {
//      tracker.close();
//    }

    
  }

//  void WorkbenchPage::DisposePerspective(Perspective::Pointer persp, bool notify)
//  {
//    // Get rid of perspective.
//    perspList.remove(persp);
//    if (notify)
//    {
//      window.firePerspectiveClosed(this, persp.getDesc());
//    }
//    persp.dispose();
//
//    stickyViewMan.remove(persp.getDesc().getId());
//  }

//  Perspective::Pointer WorkbenchPage::FindPerspective(IPerspectiveDescriptor::Pointer desc)
//  {
//    Iterator itr = perspList.iterator();
//    while (itr.hasNext())
//    {
//      Perspective mgr = (Perspective) itr.next();
//      if (desc.getId().equals(mgr.getDesc().getId()))
//      {
//        return mgr;
//      }
//    }
//    return null;
//  }

  IViewPart::Pointer WorkbenchPage::FindView(const stds::string& id)
  {
    IViewReference ref = findViewReference(id);
    if (ref == null)
    {
      return null;
    }
    return ref.getView(true);
  }

  IViewReference::Pointer WorkbenchPage::FindViewReference(const std::string& viewId)
  {
    return findViewReference(viewId, null);
  }

  IViewReference::Pointer WorkbenchPage::FindViewReference(const std::string& viewId, const std::string& secondaryId)
  {
    Perspective persp = getActivePerspective();
    if (persp == null)
    {
      return null;
    }
    return persp.findView(viewId, secondaryId);
  }

  IEditorPart::Pointer WorkbenchPage::GetActiveEditor()
  {
    return partList.getActiveEditor();
  }

  IEditorReference::Pointer WorkbenchPage::GetActiveEditorReference()
  {
    return partList.getActiveEditorReference();
  }

  IWorkbenchPart::Pointer WorkbenchPage::GetActivePart()
  {
    return partList.getActivePart();
  }

  IWorkbenchPartReference::Pointer WorkbenchPage::GetActivePartReference()
  {
    return partList.getActivePartReference();
  }

//  Perspective::Pointer WorkbenchPage::GetActivePerspective()
//  {
//    return perspList.getActive();
//  }

  Composite WorkbenchPage::GetClientComposite()
  {
    return composite;
  }

  EditorManager WorkbenchPage::GetEditorManager()
  {
    return editorMgr;
  }
  
 PerspectiveHelper* GetPerspectivePresentation() {
        if (getActivePerspective() != null) {
      return getActivePerspective().getPresentation();
    }
        return null;
    }

    /**
     * Answer the editor presentation.
     */
EditorAreaHelper* GetEditorPresentation() {
        return editorPresentation;
    }

  std::vector<IEditorPart::Pointer> WorkbenchPage::GetEditors()
  {
    final IEditorReference refs[] = getEditorReferences();
    final ArrayList result = new ArrayList(refs.length);
    Display d = getWorkbenchWindow().getShell().getDisplay();
    //Must be backward compatible.
    d.syncExec(new Runnable()
        {
        public void WorkbenchPage::run()
          {
            for (int i = 0; i < refs.length; i++)
            {
              IWorkbenchPart part = refs[i].getPart(true);
              if (part != null)
              {
                result.add(part);
              }
            }
          }
        });
    final IEditorPart editors[] = new IEditorPart[result.size()];
    return (IEditorPart[]) result.toArray(editors);
  }

  std::vector<IEditorPart::Pointer> WorkbenchPage::GetDirtyEditors()
  {
    return getEditorManager().getDirtyEditors();
  }

  std::vector<ISaveablePart> WorkbenchPage::GetDirtyParts()
  {
    List result = new ArrayList(3);
    IWorkbenchPartReference[] allParts = getAllParts();
    for (int i = 0; i < allParts.length; i++)
    {
      IWorkbenchPartReference reference = allParts[i];

      IWorkbenchPart part = reference.getPart(false);
      if (part != null && part instanceof ISaveablePart)
      {
        ISaveablePart saveable = (ISaveablePart)part;
        if (saveable.isDirty())
        {
          result.add(saveable);
        }
      }
    }

    return (ISaveablePart[]) result.toArray(new ISaveablePart[result.size()]);
  }

  /**
   * See IWorkbenchPage.
   */
  IEditorPart::Pointer WorkbenchPage::FindEditor(IEditorInput* input)
  {
    return getEditorManager().findEditor(input);
  }

  /**
   * See IWorkbenchPage.
   */
  std::vector<IEditorReference> WorkbenchPage::FindEditors(IEditorInput* input, const std::string& editorId, int matchFlags)
  {
    return getEditorManager().findEditors(input, editorId, matchFlags);
  }

  /**
   * See IWorkbenchPage.
   */
  std::vector<IEditorReference> WorkbenchPage::GetEditorReferences()
  {
    return editorPresentation.getEditors();
  }

  /**
   * @see IWorkbenchPage
   */
  IAdaptable* WorkbenchPage::GetInput()
  {
    return input;
  }

  /**
   * Returns the page label. This is a combination of the page input and
   * active perspective.
   */
  std::string WorkbenchPage::GetLabel()
  {
    String label = WorkbenchMessages.WorkbenchPage_UnknownLabel;
    IWorkbenchAdapter adapter = (IWorkbenchAdapter) Util.getAdapter(input,
        IWorkbenchAdapter.class);
    if (adapter != null)
    {
      label = adapter.getLabel(input);
    }
    Perspective persp = getActivePerspective();
    if (persp != null)
    {
      label = NLS.bind(WorkbenchMessages.WorkbenchPage_PerspectiveFormat, label, persp.getDesc().getLabel());
    }
    else if (deferredActivePersp != null)
    {
      label = NLS.bind(WorkbenchMessages.WorkbenchPage_PerspectiveFormat,label, deferredActivePersp.getLabel());
    }
    return label;
  }

  /**
   * Returns the perspective.
   */
//  IPerspectiveDescriptor::Pointer WorkbenchPage::GetPerspective()
//  {
//    if (deferredActivePersp != null)
//    {
//      return deferredActivePersp;
//    }
//    Perspective persp = getActivePerspective();
//    if (persp != null)
//    {
//      return persp.getDesc();
//    }
//    else
//    {
//      return null;
//    }
//  }

  /*
   * (non-Javadoc) Method declared on ISelectionService
   */
//  ISelection::Pointer WorkbenchPage::GetSelection()
//  {
//    return selectionService.getSelection();
//  }

  /*
   * (non-Javadoc) Method declared on ISelectionService
   */
//  ISelection::Pointer WorkbenchPage::GetSelection(const std::string& partId)
//  {
//    return selectionService.getSelection(partId);
//  }

  /*
   * Returns the view factory.
   */
  ViewFactory::Pointer WorkbenchPage::GetViewFactory()
  {
    if (viewFactory == null)
    {
      viewFactory = new ViewFactory(this, WorkbenchPlugin.getDefault()
          .getViewRegistry());
    }
    return viewFactory;
  }

  /**
   * See IWorkbenchPage.
   */
  std::vector<IViewReference::Pointer> WorkbenchPage::GetViewReferences()
  {
    Perspective persp = getActivePerspective();
    if (persp != null)
    {
      return persp.getViewReferences();
    }
    else
    {
      return new IViewReference[0];
    }
  }

  /**
   * See IWorkbenchPage.
   */
  std::vector<IViewPart::Pointer> WorkbenchPage::GetViews()
  {
    return getViews(null, true);
  }

  /**
   * Returns all view parts in the specified perspective
   * 
   * @param persp the perspective
   * @return an array of view parts
   * @since 3.1
   */
  /*package*/std::vector<IViewPart::Pointer> WorkbenchPage::GetViews(Perspective::Pointer persp, bool restore)
  {
    if (persp == null)
    {
      persp = getActivePerspective();
    }

    if (persp != null)
    {
      IViewReference refs[] = persp.getViewReferences();
      ArrayList parts = new ArrayList(refs.length);
      for (int i = 0; i < refs.length; i++)
      {
        IWorkbenchPart part = refs[i].getPart(restore);
        if (part != null)
        {
          parts.add(part);
        }
      }
      IViewPart[] result = new IViewPart[parts.size()];
      return (IViewPart[]) parts.toArray(result);
    }
    return new IViewPart[0];
  }

  /**
   * See IWorkbenchPage.
   */
  IWorkbenchWindow::Pointer WorkbenchPage::GetWorkbenchWindow()
  {
    return window;
  }
  

  /*
   * (non-Javadoc)
   * 
   * @see org.eclipse.ui.IWorkbenchPage#hideView(org.eclipse.ui.IViewReference)
   */
  void WorkbenchPage::HideView(IViewReference::Pointer ref)
  {

    // Sanity check.
    if (ref == null)
    {
      return;
    }

    Perspective persp = getActivePerspective();
    if (persp == null)
    {
      return;
    }

    boolean promptedForSave = false;
    IViewPart view = ref.getView(false);
    if (view != null)
    {

      if (!certifyPart(view))
      {
        return;
      }

      // Confirm.
      if (view instanceof ISaveablePart)
      {
        ISaveablePart saveable = (ISaveablePart)view;
        if (saveable.isSaveOnCloseNeeded())
        {
          IWorkbenchWindow window = view.getSite().getWorkbenchWindow();
          boolean success = EditorManager.saveAll(Collections.singletonList(view), true, true, false, window);
          if (!success)
          {
            // the user cancelled.
            return;
          }
          promptedForSave = true;
        }
      }
    }

    int refCount = getViewFactory().getReferenceCount(ref);
    SaveablesList saveablesList = null;
    Object postCloseInfo = null;
    if (refCount == 1)
    {
      IWorkbenchPart actualPart = ref.getPart(false);
      if (actualPart != null)
      {
        saveablesList = (SaveablesList) actualPart
        .getSite().getService(ISaveablesLifecycleListener.class);
        postCloseInfo = saveablesList.preCloseParts(Collections
            .singletonList(actualPart), !promptedForSave, this
            .getWorkbenchWindow());
        if (postCloseInfo==null)
        {
          // cancel
          return;
        }
      }
    }

    // Notify interested listeners before the hide
    window.firePerspectiveChanged(this, persp.getDesc(), ref,
        CHANGE_VIEW_HIDE);

    PartPane pane = getPane(ref);
    pane.setInLayout(false);

    updateActivePart();

    if (saveablesList != null)
    {
      saveablesList.postClose(postCloseInfo);
    }

    // Hide the part.
    persp.hideView(ref);

    // Notify interested listeners after the hide
    window.firePerspectiveChanged(this, getPerspective(), CHANGE_VIEW_HIDE);
  }

  /* package */void WorkbenchPage::refreshActiveView()
  {
    updateActivePart();
  }

  /**
   * See IPerspective
   */
  void WorkbenchPage::HideView(IViewPart::Pointer view)
  {
    hideView((IViewReference)getReference(view));
  }

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
  void WorkbenchPage::Init(WorkbenchWindow::Pointer w, const std::string& layoutID, IAdaptable* input, bool openExtras)
  {
    // Save args.
    this.window = w;
    this.input = input;
    actionSets = new ActionSetManager(w);

    deferCount = 0;
    
    // Create presentation.
    createClientComposite();
    editorPresentation = new EditorAreaHelper(this);
    editorMgr = new EditorManager(window, this, editorPresentation);

    // add this page as a client to be notified when the UI has re-orded perspectives 
    // so that the order can be properly maintained in the receiver.
    // E.g. a UI might support drag-and-drop and will need to make this known to ensure
    // #saveState and #restoreState do not lose this re-ordering
    w.addPerspectiveReorderListener(new IReorderListener()
        {
        public void WorkbenchPage::reorder(Object perspective, int newLoc)
          {
            perspList.reorder((IPerspectiveDescriptor)perspective, newLoc);
          }
        });

    if (openExtras)
    {
      openPerspectiveExtras();
    }

    // Get perspective descriptor.
    if (layoutID != null)
    {
      PerspectiveDescriptor desc = (PerspectiveDescriptor) WorkbenchPlugin
      .getDefault().getPerspectiveRegistry()
      .findPerspectiveWithId(layoutID);
      if (desc == null)
      {
        throw new WorkbenchException(
            NLS.bind(WorkbenchMessages.WorkbenchPage_ErrorCreatingPerspective,layoutID ));
      }
      Perspective persp = findPerspective(desc);
      if (persp == null)
      {
        persp = createPerspective(desc, true);
      }
      perspList.setActive(persp);
      window.firePerspectiveActivated(this, desc);
    }

    getExtensionTracker()
    .registerHandler(
        perspectiveChangeHandler,
        ExtensionTracker
        .createExtensionPointFilter(getPerspectiveExtensionPoint()));
  }

  /**
   * Opens the perspectives specified in the PERSPECTIVE_BAR_EXTRAS preference (see bug 84226).
   */
//  void WorkbenchPage::OpenPerspectiveExtras()
//  {
//    String extras = PrefUtil.getAPIPreferenceStore().getString(
//        IWorkbenchPreferenceConstants.PERSPECTIVE_BAR_EXTRAS);
//    StringTokenizer tok = new StringTokenizer(extras, ", "); //$NON-NLS-1$
//    ArrayList descs = new ArrayList();
//    while (tok.hasMoreTokens())
//    {
//      String id = tok.nextToken();
//      IPerspectiveDescriptor desc = WorkbenchPlugin.getDefault().getPerspectiveRegistry().findPerspectiveWithId(id);
//      if (desc != null)
//      {
//        descs.add(desc);
//      }
//    }
//    // HACK: The perspective switcher currently adds the button for a new perspective to the beginning of the list.
//    // So, we process the extra perspectives in reverse order here to have their buttons appear in the order declared. 
//    for (int i = descs.size(); --i >= 0;)
//    {
//      PerspectiveDescriptor desc = (PerspectiveDescriptor) descs.get(i);
//      if (findPerspective(desc) == null)
//      {
//        createPerspective(desc, true);
//      }
//    }
//  }

  /**
   * See IWorkbenchPage.
   */
  bool WorkbenchPage::IsPartVisible(IWorkbenchPart::Pointer part)
  {
    PartPane pane = getPane(part);
    return pane != null && pane.getVisible();
  }

  /**
   * See IWorkbenchPage.
   */
  bool WorkbenchPage::IsEditorAreaVisible()
  {
    Perspective persp = getActivePerspective();
    if (persp == null)
    {
      return false;
    }
    return persp.isEditorAreaVisible();
  }

  /**
   * Returns whether the view is fast.
   */
  bool WorkbenchPage::IsFastView(IViewReference::Pointer ref)
  {
    Perspective persp = getActivePerspective();
    if (persp != null)
    {
      return persp.isFastView(ref);
    }
    else
    {
      return false;
    }
  }

  /**
   * Return whether the view is closeable or not.
   * 
   * @param ref the view reference to check.  Must not be <code>null</code>.
   * @return true if the part is closeable.
   * @since 3.1.1
   */
  bool WorkbenchPage::IsCloseable(IViewReference::Pointer ref)
  {
    Perspective persp = getActivePerspective();
    if (persp != null)
    {
      return persp.isCloseable(ref);
    }
    return false;
  }

  /**
   * Return whether the view is moveable or not.
   * 
   * @param ref the view reference to check.  Must not be <code>null</code>.
   * @return true if the part is moveable.
   * @since 3.1.1
   */
  bool WorkbenchPage::IsMoveable(IViewReference::Pointer ref)
  {
    Perspective persp = getActivePerspective();
    if (persp != null)
    {
      return persp.isMoveable(ref);
    }
    return false;
  }

  /**
   * Returns whether the layout of the active
   * perspective is fixed.
   */
//  bool WorkbenchPage::IsFixedLayout()
//  {
//    Perspective persp = getActivePerspective();
//    if (persp != null)
//    {
//      return persp.isFixedLayout();
//    }
//    else
//    {
//      return false;
//    }
//  }

  /**
   * Return true if the perspective has a dirty editor.
   */
  bool WorkbenchPage::IsSaveNeeded()
  {
    return getEditorManager().isSaveAllNeeded();
  }

  /**
   * This method is called when the page is activated.
   */
  void WorkbenchPage::OnActivate()
  {
    composite.setVisible(true);
    Perspective persp = getActivePerspective();

    if (persp != null)
    {
      persp.onActivate();
      updateVisibility(null, persp);
    }
  }

  /**
   * This method is called when the page is deactivated.
   */
  void WorkbenchPage::OnDeactivate()
  {
    makeActiveEditor(null);
    makeActive(null);
    if (getActivePerspective() != null)
    {
      getActivePerspective().onDeactivate();
    }
    composite.setVisible(false);
  }

  /**
   * See IWorkbenchPage.
   */
  void WorkbenchPage::ReuseEditor(IReusableEditor::Pointer editor, IEditorInput* input)
  {

    // Rather than calling editor.setInput on the editor directly, we do it through the part reference.
    // This case lets us detect badly behaved editors that are not firing a PROP_INPUT event in response
    // to the input change... but if all editors obeyed their API contract, the "else" branch would be
    // sufficient.
    IWorkbenchPartReference ref = getReference(editor);
    if (ref instanceof EditorReference)
    {
      EditorReference editorRef = (EditorReference) ref;

      editorRef.setInput(input);
    }
    else
    {
      editor.setInput(input);
    }
    navigationHistory.markEditor(editor);
  }

  /**
   * See IWorkbenchPage.
   */
  IEditorPart::Pointer WorkbenchPage::OpenEditor(IEditorInput* input, const std::string& editorID)
  {
    return openEditor(input, editorID, true, MATCH_INPUT);
  }

  /**
   * See IWorkbenchPage.
   */
  IEditorPart::Pointer WorkbenchPage::OpenEditor(IEditorInput* input, const std::string& editorID,
      bool activate)
  {
    return openEditor(input, editorID, activate, MATCH_INPUT);
  }

  /**
   * See IWorkbenchPage.
   */
  IEditorPart::Pointer WorkbenchPage::OpenEditor(const IEditorInput* input,
      const std::string& editorID, bool activate, int matchFlags)
  {
    return openEditor(input, editorID, activate, matchFlags, null);
  }

  /**
   * This is not public API but for use internally.  editorState can be <code>null</code>.
   */
  IEditorPart::Pointer WorkbenchPage::OpenEditor(const IEditorInput* input,
      const std::string& editorID, bool activate, int matchFlags,
      IMemento::ConstPointer editorState)
  {
    if (input == null || editorID == null)
    {
      throw new IllegalArgumentException();
    }

    final IEditorPart result[] = new IEditorPart[1];
    final PartInitException ex[] = new PartInitException[1];
    BusyIndicator.showWhile(window.getWorkbench().getDisplay(),
        new Runnable()
        {
        public void WorkbenchPage::run()
          {
            try
            {
              result[0] = busyOpenEditor(input, editorID,
                  activate, matchFlags, editorState);
            }
            catch (PartInitException e)
            {
              ex[0] = e;
            }
          }
        });
    if (ex[0] != null)
    {
      throw ex[0];
    }
    return result[0];
  }

  /*
   * Added to fix Bug 178235 [EditorMgmt] DBCS 3.3 - Cannot open file with external program.
   * Opens a new editor using the given input and descriptor. (Normally, editors are opened using
   * an editor ID and an input.)
   */
  IEditorPart::Pointer WorkbenchPage::OpenEditorFromDescriptor(const IEditorInput* input,
      const IEditorDescriptor::ConstPointer editorDescriptor, bool activate,
      const IMemento* editorState)
  {
    if (input == null || !(editorDescriptor instanceof EditorDescriptor))
    {
      throw new IllegalArgumentException();
    }

    final IEditorPart result[] = new IEditorPart[1];
    final PartInitException ex[] = new PartInitException[1];
    BusyIndicator.showWhile(window.getWorkbench().getDisplay(),
        new Runnable()
        {
        public void WorkbenchPage::run()
          {
            try
            {
              result[0] = busyOpenEditorFromDescriptor(input, (EditorDescriptor)editorDescriptor,
                  activate, editorState);
            }
            catch (PartInitException e)
            {
              ex[0] = e;
            }
          }
        });
    if (ex[0] != null)
    {
      throw ex[0];
    }
    return result[0];
  }

  /**
   * @see #openEditor(IEditorInput, String, boolean, int)
   */
  IEditorPart::Pointer WorkbenchPage::BusyOpenEditor(IEditorInput* input, const std::string& editorID,
      bool activate, int matchFlags, IMemento* editorState)
  {

    final Workbench workbench = (Workbench) getWorkbenchWindow()
    .getWorkbench();
    workbench.largeUpdateStart();

    try
    {
      return busyOpenEditorBatched(input, editorID, activate, matchFlags, editorState);

    }finally
    {
      workbench.largeUpdateEnd();
    }
  }

  /*
   * Added to fix Bug 178235 [EditorMgmt] DBCS 3.3 - Cannot open file with external program.
   * See openEditorFromDescriptor().
   */
  IEditorPart::Pointer WorkbenchPage::BusyOpenEditorFromDescriptor(IEditorInput* input, EditorDescriptor::Pointer editorDescriptor,
      bool activate, IMemento* editorState)
  {

    final Workbench workbench = (Workbench) getWorkbenchWindow()
    .getWorkbench();
    workbench.largeUpdateStart();

    try
    {
      return busyOpenEditorFromDescriptorBatched(input, editorDescriptor, activate, editorState);

    }finally
    {
      workbench.largeUpdateEnd();
    }
  }

  /**
   * Do not call this method.  Use <code>busyOpenEditor</code>.
   * 
   * @see IWorkbenchPage#openEditor(IEditorInput, String, boolean)
   */
  IEditorPart::Pointer WorkbenchPage::BusyOpenEditorBatched(IEditorInput* input,
      const std::string& editorID, bool activate, int matchFlags, IMemento* editorState)
  {

    // If an editor already exists for the input, use it.
    IEditorPart editor = null;
    // Reuse an existing open editor, unless we are in "new editor tab management" mode
    editor = getEditorManager().findEditor(editorID, input, ((TabBehaviour)Tweaklets.get(TabBehaviour.KEY)).getReuseEditorMatchFlags(matchFlags));
    if (editor != null)
    {
      if (IEditorRegistry.SYSTEM_EXTERNAL_EDITOR_ID.equals(editorID))
      {
        if (editor.isDirty())
        {
          MessageDialog dialog = new MessageDialog(
              getWorkbenchWindow().getShell(),
              WorkbenchMessages.Save,
              null, // accept the default window icon
              NLS.bind(WorkbenchMessages.WorkbenchPage_editorAlreadyOpenedMsg, input.getName()),
              MessageDialog.QUESTION, new String[]
              {
                IDialogConstants.YES_LABEL,
                IDialogConstants.NO_LABEL,
                IDialogConstants.CANCEL_LABEL}, 0);
          int saveFile = dialog.open();
          if (saveFile == 0)
          {
            try
            {
              final IEditorPart editorToSave = editor;
              getWorkbenchWindow().run(false, false,
                  new IRunnableWithProgress()
                  {
                  public void WorkbenchPage::run(IProgressMonitor monitor)
                    throws InvocationTargetException,
                    InterruptedException
                    {
                      editorToSave.doSave(monitor);
                    }
                  });
            }
            catch (InvocationTargetException e)
            {
              throw (RuntimeException) e.getTargetException();
            }
            catch (InterruptedException e)
            {
              return null;
            }
          }
          else if (saveFile == 2)
          {
            return null;
          }
        }
      }
      else
      {
        // do the IShowEditorInput notification before showing the editor
        // to reduce flicker
        if (editor instanceof IShowEditorInput)
        {
          ((IShowEditorInput) editor).showEditorInput(input);
        }
        showEditor(activate, editor);
        return editor;
      }
    }

    // Otherwise, create a new one. This may cause the new editor to
    // become the visible (i.e top) editor.
    IEditorReference ref = null;
    ref = getEditorManager().openEditor(editorID, input, true, editorState);
    if (ref != null)
    {
      editor = ref.getEditor(true);
    }

    if (editor != null)
    {
      setEditorAreaVisible(true);
      if (activate)
      {
        if (editor instanceof MultiEditor)
        {
          activate(((MultiEditor) editor).getActiveEditor());
        }
        else
        {
          activate(editor);
        }
      }
      else
      {
        bringToTop(editor);
      }
      window.firePerspectiveChanged(this, getPerspective(), ref,
          CHANGE_EDITOR_OPEN);
      window.firePerspectiveChanged(this, getPerspective(),
          CHANGE_EDITOR_OPEN);
    }

    return editor;
  }

  /*
   * Added to fix Bug 178235 [EditorMgmt] DBCS 3.3 - Cannot open file with external program.
   * See openEditorFromDescriptor().
   */
  IEditorPart::Pointer WorkbenchPage::BusyOpenEditorFromDescriptorBatched(IEditorInput* input,
      EditorDescriptor::Pointer editorDescriptor, bool activate, IMemento* editorState)
  {

    IEditorPart editor = null;
    // Create a new one. This may cause the new editor to
    // become the visible (i.e top) editor.
    IEditorReference ref = null;
    ref = getEditorManager().openEditorFromDescriptor(editorDescriptor, input, editorState);
    if (ref != null)
    {
      editor = ref.getEditor(true);
    }

    if (editor != null)
    {
      setEditorAreaVisible(true);
      if (activate)
      {
        if (editor instanceof MultiEditor)
        {
          activate(((MultiEditor) editor).getActiveEditor());
        }
        else
        {
          activate(editor);
        }
      }
      else
      {
        bringToTop(editor);
      }
      window.firePerspectiveChanged(this, getPerspective(), ref,
          CHANGE_EDITOR_OPEN);
      window.firePerspectiveChanged(this, getPerspective(),
          CHANGE_EDITOR_OPEN);
    }

    return editor;
  }

  void WorkbenchPage::OpenEmptyTab()
  {
    IEditorPart editor = null;
    EditorReference ref = null;
    ref = (EditorReference) getEditorManager().openEmptyTab();
    if (ref != null)
    {
      editor = ref.getEmptyEditor((EditorDescriptor) ((EditorRegistry) WorkbenchPlugin
              .getDefault().getEditorRegistry())
          .findEditor(EditorRegistry.EMPTY_EDITOR_ID));
    }

    if (editor != null)
    {
      setEditorAreaVisible(true);
      activate(editor);
      window.firePerspectiveChanged(this, getPerspective(), ref,
          CHANGE_EDITOR_OPEN);
      window.firePerspectiveChanged(this, getPerspective(),
          CHANGE_EDITOR_OPEN);
    }
  }

  void WorkbenchPage::ShowEditor(bool activate, IEditorPart::Pointer editor)
  {
    setEditorAreaVisible(true);
    if (activate)
    {
      zoomOutIfNecessary(editor);
      activate(editor);
    }
    else
    {
      bringToTop(editor);
    }
  }

  /**
   * See IWorkbenchPage.
   */
  bool WorkbenchPage::IsEditorPinned(IEditorPart::Pointer editor)
  {
    WorkbenchPartReference ref = (WorkbenchPartReference)getReference(editor);
    return ref != null && ref.isPinned();
  }

  /**
   * Removes an IPartListener from the part service.
   */
  //    void WorkbenchPage::RemovePartListener(IPartListener l) {
  //        partList.getPartService().removePartListener(l);
  //    }

  /**
   * Removes an IPartListener from the part service.
   */
  //    void WorkbenchPage::RemovePartListener(IPartListener2 l) {
  //        partList.getPartService().removePartListener(l);
  //    }

  /**
   * Implements IWorkbenchPage
   * 
   * @see org.eclipse.ui.IWorkbenchPage#removePropertyChangeListener(IPropertyChangeListener)
   * @since 2.0
   * @deprecated individual views should store a working set if needed and
   *             register a property change listener directly with the
   *             working set manager to receive notification when the view
   *             working set is removed.
   */
  //    void WorkbenchPage::RemovePropertyChangeListener(IPropertyChangeListener listener) {
  //        propertyChangeListeners.remove(listener);
  //    }

  /*
   * (non-Javadoc) Method declared on ISelectionListener.
   */
  //    void WorkbenchPage::RemoveSelectionListener(ISelectionListener listener) {
  //        selectionService.removeSelectionListener(listener);
  //    }

  /*
   * (non-Javadoc) Method declared on ISelectionListener.
   */
  //    void WorkbenchPage::RemoveSelectionListener(String partId,
  //            ISelectionListener listener) {
  //        selectionService.removeSelectionListener(partId, listener);
  //    }

  /*
   * (non-Javadoc) Method declared on ISelectionListener.
   */
  //    void WorkbenchPage::RemovePostSelectionListener(ISelectionListener listener) {
  //        selectionService.removePostSelectionListener(listener);
  //    }

  /*
   * (non-Javadoc) Method declared on ISelectionListener.
   */
  //    void WorkbenchPage::RemovePostSelectionListener(String partId,
  //            ISelectionListener listener) {
  //        selectionService.removePostSelectionListener(partId, listener);
  //    }

  /**
   * This method is called when a part is activated by clicking within it. In
   * response, the part, the pane, and all of its actions will be activated.
   * 
   * In the current design this method is invoked by the part pane when the
   * pane, the part, or any children gain focus.
   */
  void WorkbenchPage::RequestActivation(IWorkbenchPart::Pointer part)
  {
    // Sanity check.
    if (!certifyPart(part))
    {
      return;
    }

    if (part instanceof MultiEditor)
    {
      part = ((MultiEditor) part).getActiveEditor();
    }

    // Real work.
    setActivePart(part);
  }

  /**
   * Resets the layout for the perspective. The active part in the old layout
   * is activated in the new layout for consistent user context.
   */
//  void WorkbenchPage::ResetPerspective()
//  {
//    // Run op in busy cursor.
//    // Use set redraw to eliminate the "flash" that can occur in the
//    // coolbar as the perspective is reset.
//    ICoolBarManager2 mgr = (ICoolBarManager2) window.getCoolBarManager2();
//    try
//    {
//      mgr.getControl2().setRedraw(false);
//      BusyIndicator.showWhile(null, new Runnable()
//          {
//          public void WorkbenchPage::run()
//            {
//              busyResetPerspective();
//            }
//          });
//    }finally
//    {
//      mgr.getControl2().setRedraw(true);
//    }
//  }

  /**
   * Restore this page from the memento and ensure that the active
   * perspective is equals the active descriptor otherwise create a new
   * perspective for that descriptor. If activeDescriptor is null active the
   * old perspective.
   */
  IStatus WorkbenchPage::RestoreState(IMemento* memento,
      const IPerspectiveDescriptor::ConstPointer activeDescriptor)
  {
    StartupThreading.runWithoutExceptions(new StartupRunnable()
        {

        public void WorkbenchPage::runWithException() throws Throwable
          {
            deferUpdates(true);
          }});

    try
    {
      // Restore working set
      String pageName = memento.getString(IWorkbenchConstants.TAG_LABEL);

      String label = null; // debugging only
      if (UIStats.isDebugging(UIStats.RESTORE_WORKBENCH))
      {
        label = pageName == null ? "" : "::" + pageName; //$NON-NLS-1$ //$NON-NLS-2$
      }

      try
      {
        UIStats.start(UIStats.RESTORE_WORKBENCH, "WorkbenchPage" + label); //$NON-NLS-1$
        if (pageName == null)
        {
          pageName = ""; //$NON-NLS-1$
        }
        final MultiStatus result = new MultiStatus(
            PlatformUI.PLUGIN_ID,
            IStatus.OK,
            NLS.bind(WorkbenchMessages.WorkbenchPage_unableToRestorePerspective, pageName ),
            null);

        String workingSetName = memento
        .getString(IWorkbenchConstants.TAG_WORKING_SET);
        if (workingSetName != null)
        {
          AbstractWorkingSetManager workingSetManager = (AbstractWorkingSetManager) getWorkbenchWindow()
          .getWorkbench().getWorkingSetManager();
          setWorkingSet(workingSetManager.getWorkingSet(workingSetName));
        }

        IMemento workingSetMem = memento
        .getChild(IWorkbenchConstants.TAG_WORKING_SETS);
        if (workingSetMem != null)
        {
          IMemento[] workingSetChildren = workingSetMem
          .getChildren(IWorkbenchConstants.TAG_WORKING_SET);
          List workingSetList = new ArrayList(
              workingSetChildren.length);
          for (int i = 0; i < workingSetChildren.length; i++)
          {
            IWorkingSet set = getWorkbenchWindow().getWorkbench()
            .getWorkingSetManager().getWorkingSet(
                workingSetChildren[i].getID());
            if (set != null)
            {
              workingSetList.add(set);
            }
          }

          workingSets = (IWorkingSet[]) workingSetList
          .toArray(new IWorkingSet[workingSetList.size()]);
        }

        aggregateWorkingSetId = memento.getString(ATT_AGGREGATE_WORKING_SET_ID);

        IWorkingSet setWithId = window.getWorkbench().getWorkingSetManager().getWorkingSet(aggregateWorkingSetId);

        // check to see if the set has already been made and assign it if it has
        if (setWithId instanceof AggregateWorkingSet)
        {
          aggregateWorkingSet = (AggregateWorkingSet) setWithId;
        }
        // Restore editor manager.
        IMemento childMem = memento
        .getChild(IWorkbenchConstants.TAG_EDITORS);
        result.merge(getEditorManager().restoreState(childMem));

        childMem = memento.getChild(IWorkbenchConstants.TAG_VIEWS);
        if (childMem != null)
        {
          result.merge(getViewFactory().restoreState(childMem));
        }

        // Get persp block.
        childMem = memento.getChild(IWorkbenchConstants.TAG_PERSPECTIVES);
        String activePartID = childMem
        .getString(IWorkbenchConstants.TAG_ACTIVE_PART);
        String activePartSecondaryID = null;
        if (activePartID != null)
        {
          activePartSecondaryID = ViewFactory
          .extractSecondaryId(activePartID);
          if (activePartSecondaryID != null)
          {
            activePartID = ViewFactory.extractPrimaryId(activePartID);
          }
        }
        final String activePerspectiveID = childMem
        .getString(IWorkbenchConstants.TAG_ACTIVE_PERSPECTIVE);

        // Restore perspectives.
        final IMemento perspMems[] = childMem
        .getChildren(IWorkbenchConstants.TAG_PERSPECTIVE);
        final Perspective activePerspectiveArray [] = new Perspective[1];

        for (int i = 0; i < perspMems.length; i++)
        {

          final IMemento current = perspMems[i];
          StartupThreading
          .runWithoutExceptions(new StartupRunnable()
              {

              public void WorkbenchPage::runWithException() throws Throwable
                {
                  Perspective persp = ((WorkbenchImplementation) Tweaklets
                      .get(WorkbenchImplementation.KEY)).createPerspective(null,
                      WorkbenchPage.this);
                  result.merge(persp.restoreState(current));
                  final IPerspectiveDescriptor desc = persp
                  .getDesc();
                  if (desc.equals(activeDescriptor))
                  {
                    activePerspectiveArray[0] = persp;
                  }
                  else if ((activePerspectiveArray[0] == null)
                      && desc.getId().equals(
                          activePerspectiveID))
                  {
                    activePerspectiveArray[0] = persp;
                  }
                  perspList.add(persp);
                  window.firePerspectiveOpened(
                      WorkbenchPage.this, desc);
                }
              });
        }
        Perspective activePerspective = activePerspectiveArray[0];
        boolean restoreActivePerspective = false;
        if (activeDescriptor == null)
        {
          restoreActivePerspective = true;

        }
        else if (activePerspective != null
            && activePerspective.getDesc().equals(activeDescriptor))
        {
          restoreActivePerspective = true;
        }
        else
        {
          restoreActivePerspective = false;
          activePerspective = createPerspective((PerspectiveDescriptor) activeDescriptor, true);
          if (activePerspective == null)
          {
            result
            .merge(new Status(
                    IStatus.ERROR,
                    PlatformUI.PLUGIN_ID,
                    0,
                    NLS.bind(WorkbenchMessages.Workbench_showPerspectiveError, activeDescriptor.getId() ),
                    null));
          }
        }

        perspList.setActive(activePerspective);

        // Make sure we have a valid perspective to work with,
        // otherwise return.
        activePerspective = perspList.getActive();
        if (activePerspective == null)
        {
          activePerspective = perspList.getNextActive();
          perspList.setActive(activePerspective);
        }
        if (activePerspective != null && restoreActivePerspective)
        {
          result.merge(activePerspective.restoreState());
        }

        if (activePerspective != null)
        {
          final Perspective myPerspective = activePerspective;
          final String myActivePartId = activePartID, mySecondaryId = activePartSecondaryID;
          StartupThreading.runWithoutExceptions(new StartupRunnable()
              {

              public void WorkbenchPage::runWithException() throws Throwable
                {
                  window.firePerspectiveActivated(WorkbenchPage.this, myPerspective
                      .getDesc());

                  // Restore active part.
                  if (myActivePartId != null)
                  {
                    IViewReference ref = myPerspective.findView(
                        myActivePartId, mySecondaryId);

                    if (ref != null)
                    {
                      activationList.setActive(ref);
                    }
                  }
                }});

        }

        childMem = memento
        .getChild(IWorkbenchConstants.TAG_NAVIGATION_HISTORY);
        if (childMem != null)
        {
          navigationHistory.restoreState(childMem);
        }
        else if (getActiveEditor() != null)
        {
          navigationHistory.markEditor(getActiveEditor());
        }

        // restore sticky view state
        stickyViewMan.restore(memento);

        return result;
      }finally
      {
        String blame = activeDescriptor == null ? pageName : activeDescriptor.getId();
        UIStats.end(UIStats.RESTORE_WORKBENCH, blame, "WorkbenchPage" + label); //$NON-NLS-1$
      }
    }finally
    {
      StartupThreading.runWithoutExceptions(new StartupRunnable()
          {

          public void WorkbenchPage::runWithException() throws Throwable
            {
              deferUpdates(false);
            }
          });
    }
  }

  /**
   * See IWorkbenchPage
   */
  bool WorkbenchPage::SaveAllEditors(bool confirm)
  {
    return saveAllEditors(confirm, false);
  }

  /**
   * @param confirm 
   * @param addNonPartSources true if saveables from non-part sources should be saved too
   * @return false if the user cancelled 
   * 
   */
  bool WorkbenchPage::SaveAllEditors(bool confirm, bool addNonPartSources)
  {
    return getEditorManager().saveAll(confirm, false, addNonPartSources);
  }

  /*
   * Saves the workbench part.
   */
  bool WorkbenchPage::SavePart(ISaveablePart* saveable, IWorkbenchPart::Pointer part,
      bool confirm)
  {
    // Do not certify part do allow editors inside a multipageeditor to
    // call this.
    return getEditorManager().savePart(saveable, part, confirm);
  }

  /**
   * Saves an editors in the workbench. If <code>confirm</code> is <code>true</code>
   * the user is prompted to confirm the command.
   * 
   * @param confirm
   *            if user confirmation should be sought
   * @return <code>true</code> if the command succeeded, or <code>false</code>
   *         if the user cancels the command
   */
  bool WorkbenchPage::SaveEditor(IEditorPart::Pointer editor, bool confirm)
  {
    return savePart(editor, editor, confirm);
  }

  /**
   * Saves the current perspective.
   */
//  void WorkbenchPage::SavePerspective()
//  {
//    Perspective persp = getActivePerspective();
//    if (persp == null)
//    {
//      return;
//    }
//
//    // Always unzoom.
//    if (isZoomed())
//    {
//      zoomOut();
//    }
//
//    persp.saveDesc();
//  }

  /**
   * Saves the perspective.
   */
//  void WorkbenchPage::SavePerspectiveAs(IPerspectiveDescriptor::Pointer newDesc)
//  {
//    Perspective persp = getActivePerspective();
//    if (persp == null)
//    {
//      return;
//    }
//    IPerspectiveDescriptor oldDesc = persp.getDesc();
//
//    // Always unzoom.
//    if (isZoomed())
//    {
//      zoomOut();
//    }
//
//    persp.saveDescAs(newDesc);
//    window.firePerspectiveSavedAs(this, oldDesc, newDesc);
//  }

  /**
   * Save the state of the page.
   */
  IStatus WorkbenchPage::SaveState(IMemento* memento)
  {
    // We must unzoom to get correct layout.
    if (isZoomed())
    {
      zoomOut();
    }

    MultiStatus result = new MultiStatus(
        PlatformUI.PLUGIN_ID,
        IStatus.OK,
        NLS.bind(WorkbenchMessages.WorkbenchPage_unableToSavePerspective, getLabel()),
        null);

    // Save editor manager.
    IMemento childMem = memento
    .createChild(IWorkbenchConstants.TAG_EDITORS);
    result.merge(editorMgr.saveState(childMem));

    childMem = memento.createChild(IWorkbenchConstants.TAG_VIEWS);
    result.merge(getViewFactory().saveState(childMem));

    // Create persp block.
    childMem = memento.createChild(IWorkbenchConstants.TAG_PERSPECTIVES);
    if (getPerspective() != null)
    {
      childMem.putString(IWorkbenchConstants.TAG_ACTIVE_PERSPECTIVE,
          getPerspective().getId());
    }
    if (getActivePart() != null)
    {
      if (getActivePart() instanceof IViewPart)
      {
        IViewReference ref = (IViewReference) getReference(getActivePart());
        if (ref != null)
        {
          childMem.putString(IWorkbenchConstants.TAG_ACTIVE_PART,
              ViewFactory.getKey(ref));
        }
      }
      else
      {
        childMem.putString(IWorkbenchConstants.TAG_ACTIVE_PART,
            getActivePart().getSite().getId());
      }
    }

    // Save each perspective in opened order
    Iterator itr = perspList.iterator();
    while (itr.hasNext())
    {
      Perspective persp = (Perspective) itr.next();
      IMemento gChildMem = childMem
      .createChild(IWorkbenchConstants.TAG_PERSPECTIVE);
      result.merge(persp.saveState(gChildMem));
    }
    // Save working set if set
    if (workingSet != null)
    {
      memento.putString(IWorkbenchConstants.TAG_WORKING_SET, workingSet
          .getName());
    }

    IMemento workingSetMem = memento
    .createChild(IWorkbenchConstants.TAG_WORKING_SETS);
    for (int i = 0; i < workingSets.length; i++)
    {
      workingSetMem.createChild(IWorkbenchConstants.TAG_WORKING_SET,
          workingSets[i].getName());
    }

    if (aggregateWorkingSetId != null)
    {
      memento.putString(ATT_AGGREGATE_WORKING_SET_ID, aggregateWorkingSetId);
    }

    navigationHistory.saveState(memento
        .createChild(IWorkbenchConstants.TAG_NAVIGATION_HISTORY));

    // save the sticky activation state
    stickyViewMan.save(memento);

    return result;
  }

  std::string WorkbenchPage::GetId(IWorkbenchPart::Pointer part)
  {
    return getId(getReference(part));
  }

  std::string WorkbenchPage::GetId(IWorkbenchPartReference::Pointer ref)
  {
    if (ref == null)
    {
      return "null"; //$NON-NLS-1$
    }return ref.getId();
  }

  /**
   * Sets the active part.
   */
  void WorkbenchPage::SetActivePart(IWorkbenchPart::Pointer newPart)
  {
    // Optimize it.
    if (getActivePart() == newPart)
    {
      return;
    }

    if (partBeingActivated != null)
    {
      if (partBeingActivated.getPart(false) != newPart)
      {
        WorkbenchPlugin.log(new RuntimeException(NLS.bind(
                    "WARNING: Prevented recursive attempt to activate part {0} while still in the middle of activating part {1}", //$NON-NLS-1$
                    getId(newPart), getId(partBeingActivated))));
      }
      return;
    }

    //No need to change the history if the active editor is becoming the
    // active part
    String label = null; // debugging only
    if (UIStats.isDebugging(UIStats.ACTIVATE_PART))
    {
      label = newPart != null ? newPart.getTitle() : "none"; //$NON-NLS-1$
    }
    try
    {
      IWorkbenchPartReference partref = getReference(newPart);
      IWorkbenchPartReference realPartRef = null;
      if (newPart != null)
      {
        IWorkbenchPartSite site = newPart.getSite();
        if (site instanceof PartSite)
        {
          realPartRef = ((PartSite) site).getPane()
          .getPartReference();
        }
      }

      partBeingActivated = realPartRef;

      UIStats.start(UIStats.ACTIVATE_PART, label);
      // Notify perspective. It may deactivate fast view.
      Perspective persp = getActivePerspective();
      if (persp != null)
      {
        persp.partActivated(newPart);
      }

      // Deactivate old part
      IWorkbenchPart oldPart = getActivePart();
      if (oldPart != null)
      {
        deactivatePart(oldPart);
      }

      // Set active part.
      if (newPart != null)
      {
        activationList.setActive(newPart);
        if (newPart instanceof IEditorPart)
        {
          makeActiveEditor((IEditorReference)realPartRef);
        }
      }
      activatePart(newPart);

      actionSwitcher.updateActivePart(newPart);

      partList.setActivePart(partref);
    }finally
    {
      partBeingActivated = null;
      Object blame = newPart == null ? (Object)this : newPart;
      UIStats.end(UIStats.ACTIVATE_PART, blame, label);
    }
  }

  /**
   * See IWorkbenchPage.
   */
  void WorkbenchPage::SetEditorAreaVisible(bool showEditorArea)
  {
    Perspective persp = getActivePerspective();
    if (persp == null)
    {
      return;
    }
    if (showEditorArea == persp.isEditorAreaVisible())
    {
      return;
    }
    // If parts change always update zoom.
    if (isZoomed())
    {
      zoomOut();
    }
    // Update editor area visibility.
    if (showEditorArea)
    {
      persp.showEditorArea();
      window.firePerspectiveChanged(this, getPerspective(),
          CHANGE_EDITOR_AREA_SHOW);
    }
    else
    {
      persp.hideEditorArea();
      updateActivePart();
      window.firePerspectiveChanged(this, getPerspective(),
          CHANGE_EDITOR_AREA_HIDE);
    }
  }

  /**
   * Sets the layout of the page. Assumes the new perspective is not null.
   * Keeps the active part if possible. Updates the window menubar and
   * toolbar if necessary.
   */
//  void WorkbenchPage::SetPerspective(Perspective::Pointer newPersp)
//  {
//    // Don't do anything if already active layout
//    Perspective oldPersp = getActivePerspective();
//    if (oldPersp == newPersp)
//    {
//      return;
//    }
//
//    window.largeUpdateStart();
//    try
//    {
//      if (oldPersp != null)
//      {
//        // fire the pre-deactivate
//        window.firePerspectivePreDeactivate(this, oldPersp.getDesc());
//      }
//
//      if (newPersp != null)
//      {
//        IStatus status = newPersp.restoreState();
//        if (status.getSeverity() != IStatus.OK)
//        {
//          String title = WorkbenchMessages.WorkbenchPage_problemRestoringTitle;
//          String msg = WorkbenchMessages.WorkbenchPage_errorReadingState;
//          ErrorDialog.openError(getWorkbenchWindow().getShell(), title,
//              msg, status);
//        }
//      }
//
//      // Deactivate the old layout
//      if (oldPersp != null)
//      {
//        oldPersp.onDeactivate();
//
//        // Notify listeners of deactivation
//        window.firePerspectiveDeactivated(this, oldPersp.getDesc());
//      }
//
//      // Activate the new layout
//      perspList.setActive(newPersp);
//      if (newPersp != null)
//      {
//        newPersp.onActivate();
//
//        // Notify listeners of activation
//        window.firePerspectiveActivated(this, newPersp.getDesc());
//      }
//
//      updateVisibility(oldPersp, newPersp);
//
//      // Update the window
//      window.updateActionSets();
//
//      // Update sticky views
//      stickyViewMan.update(oldPersp, newPersp);
//
//    }finally
//    {
//      window.largeUpdateEnd();
//      if (newPersp == null)
//      {
//        return;
//      }
//      IPerspectiveDescriptor desc = newPersp.getDesc();
//      if (desc == null)
//      {
//        return;
//      }
//      if (dirtyPerspectives.remove(desc.getId()))
//      {
//        suggestReset();
//      }
//    }
//  }

  /*
   * Update visibility state of all views.
   */
  void WorkbenchPage::UpdateVisibility(Perspective::Pointer oldPersp, Perspective::Pointer newPersp)
  {

    // Flag all parts in the old perspective
    IWorkbenchPartReference[] oldRefs = new IWorkbenchPartReference[0];
    if (oldPersp != null)
    {
      oldRefs = oldPersp.getViewReferences();
      for (int i = 0; i < oldRefs.length; i++)
      {
        PartPane pane = ((WorkbenchPartReference) oldRefs[i]).getPane();
        pane.setInLayout(false);
      }
    }

    PerspectiveHelper pres = null;
    // Make parts in the new perspective visible
    if (newPersp != null)
    {
      pres = newPersp.getPresentation();
      IWorkbenchPartReference[] newRefs = newPersp.getViewReferences();
      for (int i = 0; i < newRefs.length; i++)
      {
        WorkbenchPartReference ref = (WorkbenchPartReference)newRefs[i];
        PartPane pane = ref.getPane();
        if (pres.isPartVisible(ref))
        {
          activationList.bringToTop(ref);
        }

        pane.setInLayout(true);
      }
    }

    updateActivePart();

    // Hide any parts in the old perspective that are no longer visible
    for (int i = 0; i < oldRefs.length; i++)
    {
      WorkbenchPartReference ref = (WorkbenchPartReference)oldRefs[i];

      PartPane pane = ref.getPane();
      if (pres == null || !pres.isPartVisible(ref))
      {
        pane.setVisible(false);
      }
    }
  }

  /**
   * Sets the perspective.
   * 
   * @param desc
   *            identifies the new perspective.
   */
//  void WorkbenchPage::SetPerspective(const IPerspectiveDescriptor::ConstPointer desc)
//  {
//    if (Util.equals(getPerspective(), desc))
//    {
//      return;
//    }
//    // Going from multiple to single rows can make the coolbar
//    // and its adjacent views appear jumpy as perspectives are
//    // switched. Turn off redraw to help with this.
//    ICoolBarManager2 mgr = (ICoolBarManager2) window.getCoolBarManager2();
//    try
//    {
//      mgr.getControl2().setRedraw(false);
//      getClientComposite().setRedraw(false);
//      // Run op in busy cursor.
//      BusyIndicator.showWhile(null, new Runnable()
//          {
//          public void WorkbenchPage::run()
//            {
//              busySetPerspective(desc);
//            }
//          });
//    }finally
//    {
//      getClientComposite().setRedraw(true);
//      mgr.getControl2().setRedraw(true);
//      IWorkbenchPart part = getActivePart();
//      if (part != null)
//      {
//        part.setFocus();
//      }
//    }
//  }

  /**
   * Allow access to the part service for this page ... used internally to
   * propogate certain types of events to the page part listeners.
   * @return the part service for this page.
   */
  PartService* WorkbenchPage::GetPartService()
  {
    return (PartService)partList.getPartService();
  }

  /**
   * Restore the toolbar layout for the active perspective.
   */
  void WorkbenchPage::ResetToolBarLayout()
  {
    ICoolBarManager2 mgr = (ICoolBarManager2) window.getCoolBarManager2();
    mgr.resetItemOrder();
  }

  /**
   * See IWorkbenchPage.
   */
  IViewPart::Pointer WorkbenchPage::ShowView(const std::string& viewID)
  {
    return showView(viewID, null, VIEW_ACTIVATE);
  }

  /*
   * (non-Javadoc)
   * 
   * @see org.eclipse.ui.IWorkbenchPage#showView(java.lang.String,
   *      java.lang.String, int)
   */
  IViewPart::Pointer WorkbenchPage::ShowView(const std::string& viewID, const std::string& secondaryID,
      int mode)
  {

    if (secondaryID != null)
    {
      if (secondaryID.length() == 0
          || secondaryID.indexOf(ViewFactory.ID_SEP) != -1)
      {
        throw new IllegalArgumentException(WorkbenchMessages.WorkbenchPage_IllegalSecondaryId);
      }
    }
    if (!certifyMode(mode))
    {
      throw new IllegalArgumentException(WorkbenchMessages.WorkbenchPage_IllegalViewMode);
    }

    // Run op in busy cursor.
    final Object[] result = new Object[1];
    BusyIndicator.showWhile(null, new Runnable()
        {
        public void WorkbenchPage::run()
          {
            try
            {
              result[0] = busyShowView(viewID, secondaryID, mode);
            }
            catch (PartInitException e)
            {
              result[0] = e;
            }
          }
        });
    if (result[0] instanceof IViewPart)
    {
      return (IViewPart) result[0];
    }
    else if (result[0] instanceof PartInitException)
    {
      throw (PartInitException) result[0];
    }
    else
    {
      throw new PartInitException(WorkbenchMessages.WorkbenchPage_AbnormalWorkbenchCondition);
    }
  }

  /**
   * @param mode the mode to test
   * @return whether the mode is recognized
   * @since 3.0
   */
  bool WorkbenchPage::CertifyMode(int mode)
  {
    switch (mode)
    {
      case VIEW_ACTIVATE:
      case VIEW_VISIBLE:
      case VIEW_CREATE:
      return true;
      default:
      return false;
    }
  }

  /*
   * Returns the editors in activation order (oldest first).
   */
  std::vector<IEditorReference::Pointer> WorkbenchPage::GetSortedEditors()
  {
    return activationList.getEditors();
  }

  /**
   * @see IWorkbenchPage#getOpenPerspectives()
   */
//  std::vector<IPerspectiveDescriptor::Pointer> WorkbenchPage::GetOpenPerspectives()
//  {
//    Perspective opened[] = perspList.getOpenedPerspectives();
//    IPerspectiveDescriptor[] result = new IPerspectiveDescriptor[opened.length];
//    for (int i = 0; i < result.length; i++)
//    {
//      result[i] = opened[i].getDesc();
//    }
//    return result;
//  }

  /**
   * Return all open Perspective objects.
   * 
   * @return all open Perspective objects
   * @since 3.1
   */
//  /*package*/std::vector<Perspective::Pointer> WorkbenchPage::GetOpenInternalPerspectives()
//  {
//    return perspList.getOpenedPerspectives();
//  }

  /**
   * Checks perspectives in the order they were activiated
   * for the specfied part.  The first sorted perspective 
   * that contains the specified part is returned.
   * 
   * @param part specified part to search for
   * @return the first sorted perspespective containing the part
   * @since 3.1
   */
//  /*package*/Perspective::Pointer WorkbenchPage::GetFirstPerspectiveWithView(IViewPart::Pointer part)
//  {
//    Perspective [] perspectives = perspList.getSortedPerspectives();
//    for (int i=perspectives.length - 1; i >= 0; i--)
//    {
//      if (perspectives[i].containsView(part))
//      {
//        return perspectives[i];
//      }
//    }
//    // we should never get here
//    return null;
//  }

  /**
   * Returns the perspectives in activation order (oldest first).
   */
//  std::vector<IPerspectiveDescriptor::Pointer> WorkbenchPage::GetSortedPerspectives()
//  {
//    Perspective sortedArray[] = perspList.getSortedPerspectives();
//    IPerspectiveDescriptor[] result = new IPerspectiveDescriptor[sortedArray.length];
//    for (int i = 0; i < result.length; i++)
//    {
//      result[i] = sortedArray[i].getDesc();
//    }
//    return result;
//  }

  /*
   * Returns the parts in activation order (oldest first).
   */
  std::vector<IWorkbenchPartReference::Pointer> WorkbenchPage::GetSortedParts()
  {
    return activationList.getParts();
  }

  /**
   * Returns the reference to the given part, or <code>null</code> if it has no reference 
   * (i.e. it is not a top-level part in this workbench page).
   * 
   * @param part the part
   * @return the part's reference or <code>null</code> if the given part does not belong 
   * to this workbench page
   */
  IWorkbenchPartReference::Pointer WorkbenchPage::GetReference(IWorkbenchPart::Pointer part)
  {
    if (part == null)
    {
      return null;
    }
    IWorkbenchPartSite site = part.getSite();
    if (!(site instanceof PartSite))
    {
      return null;
    }
    PartSite partSite = ((PartSite) site);
    PartPane pane = partSite.getPane();
    if (pane instanceof MultiEditorInnerPane)
    {
      MultiEditorInnerPane innerPane = (MultiEditorInnerPane) pane;
      return innerPane.getParentPane().getPartReference();
    }
    return partSite.getPartReference();
  }

  // for dynamic UI
//  void WorkbenchPage::AddPerspective(Perspective::Pointer persp)
//  {
//    perspList.add(persp);
//    window.firePerspectiveOpened(this, persp.getDesc());
//  }

  /**
   * Find the stack of view references stacked with this view part.
   * 
   * @param part
   *            the part
   * @return the stack of references
   * @since 3.0
   */
  std::vector<IViewReference::Pointer> WorkbenchPage::GetViewReferenceStack(IViewPart::Pointer part)
  {
    // Sanity check.
    Perspective persp = getActivePerspective();
    if (persp == null || !certifyPart(part))
    {
      return null;
    }

    ILayoutContainer container = ((PartSite) part.getSite()).getPane()
    .getContainer();
    if (container instanceof ViewStack)
    {
      ViewStack folder = (ViewStack) container;
      final ArrayList list = new ArrayList(folder.getChildren().length);
      for (int i = 0; i < folder.getChildren().length; i++)
      {
        LayoutPart layoutPart = folder.getChildren()[i];
        if (layoutPart instanceof ViewPane)
        {
          IViewReference view = ((ViewPane) layoutPart)
          .getViewReference();
          if (view != null)
          {
            list.add(view);
          }
        }
      }

      // sort the list by activation order (most recently activated first)
      Collections.sort(list, new Comparator()
          {
          public int compare(Object o1, Object o2)
            {
              int pos1 = (-1)
              * activationList
              .indexOf((IWorkbenchPartReference) o1);
              int pos2 = (-1)
              * activationList
              .indexOf((IWorkbenchPartReference) o2);
              return pos1 - pos2;
            }
          });

      return (IViewReference[]) list.toArray(new IViewReference[list
          .size()]);
    }

    return new IViewReference[]
    { (IViewReference) getReference(part)};
  }

  /*
   * (non-Javadoc)
   * 
   * @see org.eclipse.ui.IWorkbenchPage#getViewStack(org.eclipse.ui.IViewPart)
   */
  std::vector<IViewPart::Pointer> WorkbenchPage::GetViewStack(IViewPart::Pointer part)
  {
    IViewReference[] refStack = getViewReferenceStack(part);
    if (refStack == null)
    {
      return null;
    }

    List result = new ArrayList();

    for (int i = 0; i < refStack.length; i++)
    {
      IViewPart next = refStack[i].getView(false);
      if (next != null)
      {
        result.add(next);
      }
    }

    return (IViewPart[]) result.toArray(new IViewPart[result.size()]);
  }

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
  void WorkbenchPage::ResizeView(IViewPart::Pointer part, int width, int height)
  {
    SashInfo sashInfo = new SashInfo();
    PartPane pane = ((PartSite) part.getSite()).getPane();
    ILayoutContainer container = pane.getContainer();
    LayoutTree tree = getPerspectivePresentation().getLayout().root
    .find(((ViewStack) container));

    // retrieve our layout sashes from the layout tree
    findSashParts(tree, pane.findSashes(), sashInfo);

    // first set the width
    float deltaWidth = width - pane.getBounds().width;
    if (sashInfo.right != null)
    {
      Rectangle rightBounds = sashInfo.rightNode.getBounds();
      // set the new ratio 
      sashInfo.right.setRatio(((deltaWidth + sashInfo.right
                  .getBounds().x) - rightBounds.x)
          / rightBounds.width);
      // complete the resize
      sashInfo.rightNode.setBounds(rightBounds);
    }
    else if (sashInfo.left != null)
    {
      Rectangle leftBounds = sashInfo.leftNode.getBounds();
      // set the ratio
      sashInfo.left
      .setRatio(((sashInfo.left.getBounds().x - deltaWidth) - leftBounds.x)
          / leftBounds.width);
      // complete the resize
      sashInfo.leftNode.setBounds(sashInfo.leftNode.getBounds());
    }

    // next set the height
    float deltaHeight = height - pane.getBounds().height;
    if (sashInfo.bottom != null)
    {
      Rectangle bottomBounds = sashInfo.bottomNode.getBounds();
      // set the new ratio 
      sashInfo.bottom.setRatio(((deltaHeight + sashInfo.bottom
                  .getBounds().y) - bottomBounds.y)
          / bottomBounds.height);
      // complete the resize
      sashInfo.bottomNode.setBounds(bottomBounds);
    }
    else if (sashInfo.top != null)
    {
      Rectangle topBounds = sashInfo.topNode.getBounds();
      // set the ratio
      sashInfo.top
      .setRatio(((sashInfo.top.getBounds().y - deltaHeight) - topBounds.y)
          / topBounds.height);
      // complete the resize
      sashInfo.topNode.setBounds(topBounds);
    }

  }

  void WorkbenchPage::FindSashParts(LayoutTree tree, PartPane::Sashes sashes,
      SashInfo info)
  {
    LayoutTree parent = tree.getParent();
    if (parent == null)
    {
      return;
    }

    if (parent.part instanceof LayoutPartSash)
    {
      // get the layout part sash from this tree node
      LayoutPartSash sash = (LayoutPartSash) parent.part;
      // make sure it has a sash control
      Control control = sash.getControl();
      if (control != null)
      {
        // check for a vertical sash
        if (sash.isVertical())
        {
          if (sashes.left == control)
          {
            info.left = sash;
            info.leftNode = parent.findSash(sash);
          }
          else if (sashes.right == control)
          {
            info.right = sash;
            info.rightNode = parent.findSash(sash);
          }
        }
        // check for a horizontal sash

        else
        {
          if (sashes.top == control)
          {
            info.top = sash;
            info.topNode = parent.findSash(sash);
          }
          else if (sashes.bottom == control)
          {
            info.bottom = sash;
            info.bottomNode = parent.findSash(sash);
          }
        }
      }
    }
    // recursive call to continue up the tree
    findSashParts(parent, sashes, info);
  }

  /**
   * Returns all parts that are owned by this page
   * 
   * @return
   */
  std::vector<IWorkbenchPartReference::Pointer> WorkbenchPage::GetAllParts()
  {
    IViewReference[] views = viewFactory.getViews();
    IEditorReference[] editors = getEditorReferences();

    IWorkbenchPartReference[] result = new IWorkbenchPartReference[views.length + editors.length];
    int resultIdx = 0;

    for (int i = 0; i < views.length; i++)
    {
      result[resultIdx++] = views[i];
    }

    for (int i = 0; i < editors.length; i++)
    {
      result[resultIdx++] = editors[i];
    }

    return result;
  }

  /**
   * Returns all open parts that are owned by this page (that is, all parts
   * for which a part opened event would have been sent -- these would be
   * activated parts whose controls have already been created.
   */
  std::vector<IWorkbenchPartReference::Pointer> WorkbenchPage::GetOpenParts()
  {
    IWorkbenchPartReference[] refs = getAllParts();
    List result = new ArrayList();

    for (int i = 0; i < refs.length; i++)
    {
      IWorkbenchPartReference reference = refs[i];

      IWorkbenchPart part = reference.getPart(false);
      if (part != null)
      {
        result.add(reference);
      }
    }

    return (IWorkbenchPartReference[]) result.toArray(new IWorkbenchPartReference[result.size()]);
  }

  /**
   * Sanity-checks the objects in this page. Throws an Assertation exception
   * if an object's internal state is invalid. ONLY INTENDED FOR USE IN THE 
   * UI TEST SUITES. 
   */
  void WorkbenchPage::TestInvariants()
  {
    Perspective persp = getActivePerspective();

    if (persp != null)
    {

      persp.testInvariants();

      // When we have widgets, ensure that there is no situation where the editor area is visible
      // and the perspective doesn't want an editor area. 
      if (!SwtUtil.isDisposed(getClientComposite()) && editorPresentation.getLayoutPart().isVisible())
      {
        Assert.isTrue(persp.isEditorAreaVisible());
      }
    }

  }

  /* (non-Javadoc)
   * @see org.eclipse.ui.IWorkbenchPage#getExtensionTracker()
   */
//  IExtensionTracker WorkbenchPage::GetExtensionTracker()
//  {
//    if (tracker == null)
//    {
//      tracker = new UIExtensionTracker(getWorkbenchWindow().getWorkbench().getDisplay());
//    }
//    return tracker;
//  }

  /*
   * (non-Javadoc)
   * 
   * @see org.eclipse.ui.IWorkbenchPage#getPerspectiveShortcuts()
   */
//  std::vector<std::string> WorkbenchPage::GetPerspectiveShortcuts()
//  {
//    Perspective persp = getActivePerspective();
//    if (persp == null)
//    {
//      return new String[0];
//    }
//    return persp.getPerspectiveShortcuts();
//  }

  /*
   * (non-Javadoc)
   * 
   * @see org.eclipse.ui.IWorkbenchPage#getShowViewShortcuts()
   */
  std::vector<std::string> WorkbenchPage::GetShowViewShortcuts()
  {
    Perspective persp = getActivePerspective();
    if (persp == null)
    {
      return new String[0];
    }
    return persp.getShowViewShortcuts();
  }

  /**
   * @since 3.1
   */
  void WorkbenchPage::SuggestReset()
  {
    final IWorkbench workbench = getWorkbenchWindow().getWorkbench();
    workbench.getDisplay().asyncExec(new Runnable()
        {
        public void WorkbenchPage::run()
          {
            Shell parentShell = null;

            IWorkbenchWindow window = workbench.getActiveWorkbenchWindow();
            if (window == null)
            {
              if (workbench.getWorkbenchWindowCount() == 0)
              {
                return;
              }
              window = workbench.getWorkbenchWindows()[0];
            }

            parentShell = window.getShell();

            if (MessageDialog
                .openQuestion(
                    parentShell,
                    WorkbenchMessages.Dynamic_resetPerspectiveTitle,
                    WorkbenchMessages.Dynamic_resetPerspectiveMessage))
            {
              IWorkbenchPage page = window.getActivePage();
              if (page == null)
              {
                return;
              }
              page.resetPerspective();
            }
          }
        });

  }

  bool WorkbenchPage::IsPartVisible(IWorkbenchPartReference::Pointer reference)
  {
    IWorkbenchPart part = reference.getPart(false);
    // Can't be visible if it isn't created yet
    if (part == null)
    {
      return false;
    }

    return isPartVisible(part);
  }

}
