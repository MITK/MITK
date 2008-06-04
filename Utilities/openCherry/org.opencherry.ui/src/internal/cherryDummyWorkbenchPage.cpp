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
#include "cherryViewSite.h"
#include "cherryWorkbenchRegistryConstants.h"
#include "cherryWorkbenchPlugin.h"

#include "../cherryUIException.h"
#include "../cherryPlatformUI.h"
#include "../cherryPartPane.h"

#include <org.opencherry.osgi/cherryPlatform.h>

#include <Poco/Exception.h>

namespace cherry
{

void WorkbenchPage::ActionSwitcher::UpdateActivePart(
    IWorkbenchPart::Pointer newPart)
{

}

void WorkbenchPage::ActionSwitcher::UpdateTopEditor(
    IEditorPart::Pointer newEditor)
{

}

void WorkbenchPage::ActionSwitcher::ActivateContributions(
    IWorkbenchPart::Pointer part, bool enable)
{

}

void WorkbenchPage::ActionSwitcher::DeactivateContributions(
    IWorkbenchPart::Pointer part, bool remove)
{

}

IExtensionPoint* WorkbenchPage::GetPerspectiveExtensionPoint()
{
  return 0;
}

WorkbenchPage::WorkbenchPage(WorkbenchWindow::Pointer w,
    const std::string& layoutID, IAdaptable* input)
{
  this->Init(w, "", input, true);
}

WorkbenchPage::WorkbenchPage(WorkbenchWindow::Pointer w, IAdaptable* input)
{
  this->Init(w, "", input, true);
}

void WorkbenchPage::Activate(IWorkbenchPart::Pointer part)
{
  IWorkbenchPartReference::Pointer ref = this->GetReference(part);
  this->InternalBringToTop(ref);
  this->SetActivePart(part);
}

void WorkbenchPage::ActivatePart(const IWorkbenchPart::Pointer part)
{
  if (part.IsNotNull())
  {
    //part.setFocus();
    PartPane::Pointer pane = this->GetPane(part);
    pane->SetFocus();
    PartSite::Pointer site = part->GetSite().Cast<PartSite>();
    pane->ShowFocus(true);
    //this->UpdateTabList(part);
    //SubActionBars bars = (SubActionBars) site.getActionBars();
    //bars.partChanged(part);
  }
}

PartPane::Pointer WorkbenchPage::GetPane(IWorkbenchPart::Pointer part)
{
  if (part.IsNull()) 
  {
    return 0;
  }
  
  return this->GetPane(this->GetReference(part));
}

PartPane::Pointer WorkbenchPage::GetPane(IWorkbenchPartReference::Pointer part)
{
  if (part.IsNull()) {
    return 0;
  }
          
  return part.Cast<WorkbenchPartReference>()->GetPane();
}

bool WorkbenchPage::InternalBringToTop(IWorkbenchPartReference::Pointer part)
{
  return false;
}

void WorkbenchPage::BringToTop(IWorkbenchPart::Pointer part)
{

}

IViewPart::Pointer WorkbenchPage::BusyShowView(const std::string& viewID,
    const std::string& secondaryID, int mode)
{
  return 0;
}

void WorkbenchPage::BusyShowView(IViewPart::Pointer part, int mode)
{
  if (mode == VIEW_ACTIVATE)
  {
    this->Activate(part);
  }
  else if (mode == VIEW_VISIBLE)
  {
//    IWorkbenchPartReference ref = getActivePartReference();
//    // if there is no active part or it's not a view, bring to top
//    if (ref == null || !(ref instanceof IViewReference))
//    {
//      bringToTop(part);
//    }
//    else
//    {
//      // otherwise check to see if the we're in the same stack as the active view
//      IViewReference activeView = (IViewReference) ref;
//      IViewReference[] viewStack = getViewReferenceStack(part);
//      for (int i = 0; i < viewStack.length; i++)
//      {
//        if (viewStack[i].equals(activeView))
//        {
//          return;
//        }
//      }
//      bringToTop(part);
//    }
  }
}

bool WorkbenchPage::CertifyPart(IWorkbenchPart::Pointer part)
{
  return false;
}

bool WorkbenchPage::Close()
{
  return false;
}

bool WorkbenchPage::CloseAllSavedEditors()
{
  return false;
}

bool WorkbenchPage::CloseAllEditors(bool save)
{
  return false;
}

void WorkbenchPage::UpdateActivePart()
{

}

void WorkbenchPage::MakeActive(IWorkbenchPartReference::Pointer ref)
{

}

void WorkbenchPage::MakeActiveEditor(IEditorReference::Pointer ref)
{

}

bool WorkbenchPage::CloseEditors(
    const std::vector<IEditorReference::Pointer>& refArray, bool save)
{
  return false;
}

void WorkbenchPage::DeferUpdates(bool shouldDefer)
{

}

void WorkbenchPage::StartDeferring()
{

}

void WorkbenchPage::HandleDeferredEvents()
{

}

bool WorkbenchPage::IsDeferred()
{
  return false;
}

bool WorkbenchPage::CloseEditor(IEditorReference::Pointer editorRef, bool save)
{
  return false;
}

bool WorkbenchPage::CloseEditor(IEditorPart::Pointer editor, bool save)
{
  return false;
}

void WorkbenchPage::PartAdded(WorkbenchPartReference::Pointer ref)
{

}

void WorkbenchPage::PartRemoved(WorkbenchPartReference::Pointer ref)
{

}

void WorkbenchPage::DisposePart(WorkbenchPartReference::Pointer ref)
{

}

void WorkbenchPage::DeactivatePart(IWorkbenchPart::Pointer part)
{

}

void WorkbenchPage::DetachView(IViewReference::Pointer ref)
{

}

void WorkbenchPage::AttachView(IViewReference::Pointer ref)
{

}

WorkbenchPage::~WorkbenchPage()
{

}

IViewPart::Pointer WorkbenchPage::FindView(const std::string& id)
{
  return 0;
}

IViewReference::Pointer WorkbenchPage::FindViewReference(
    const std::string& viewId)
{
  return 0;
}

IViewReference::Pointer WorkbenchPage::FindViewReference(
    const std::string& viewId, const std::string& secondaryId)
{
  return 0;
}

IEditorPart::Pointer WorkbenchPage::GetActiveEditor()
{
  return 0;
}

IEditorReference::Pointer WorkbenchPage::GetActiveEditorReference()
{
  return 0;
}

IWorkbenchPart::Pointer WorkbenchPage::GetActivePart()
{
  return 0;
}

IWorkbenchPartReference::Pointer WorkbenchPage::GetActivePartReference()
{
  return 0;
}

void* WorkbenchPage::GetClientComposite()
{
  return composite;
}

EditorManager* WorkbenchPage::GetEditorManager()
{
  return 0;
}

PerspectiveHelper* GetPerspectivePresentation()
{
  return 0;
}

/**
 * Answer the editor presentation.
 */
IEditorAreaHelper* GetEditorPresentation()
{
  return 0;
}

std::vector<IEditorPart::Pointer> WorkbenchPage::GetEditors()
{
  return std::vector<IEditorPart::Pointer>();
}

std::vector<IEditorPart::Pointer> WorkbenchPage::GetDirtyEditors()
{
  return std::vector<IEditorPart::Pointer>();
}

std::vector<IWorkbenchPart::Pointer> WorkbenchPage::GetDirtyParts()
{
  return std::vector<IWorkbenchPart::Pointer>();
}

/**
 * See IWorkbenchPage.
 */
IEditorPart::Pointer WorkbenchPage::FindEditor(IEditorInput::Pointer input)
{
  return 0;
}

/**
 * See IWorkbenchPage.
 */
std::vector<IEditorReference::Pointer> WorkbenchPage::FindEditors(IEditorInput::Pointer input,
    const std::string& editorId, int matchFlags)
{
  return std::vector<IEditorReference::Pointer>();
}

/**
 * See IWorkbenchPage.
 */
std::list<IEditorReference::Pointer> WorkbenchPage::GetEditorReferences()
{
  return std::list<IEditorReference::Pointer>();
}

IEditorAreaHelper* WorkbenchPage::GetEditorPresentation()
{
  return editorPresentation;
}

/**
 * @see IWorkbenchPage
 */
IAdaptable* WorkbenchPage::GetInput()
{
  return 0;
}

std::string WorkbenchPage::GetLabel()
{
  return "";
}

ViewFactory* WorkbenchPage::GetViewFactory()
{
  if (viewFactory == 0)
  {
    viewFactory = new ViewFactory(this, WorkbenchPlugin::GetDefault()->GetViewRegistry());
  }
  return viewFactory;
}

std::vector<IViewReference::Pointer> WorkbenchPage::GetViewReferences()
{
  return std::vector<IViewReference::Pointer>();
}

std::vector<IViewPart::Pointer> WorkbenchPage::GetViews()
{
  return std::vector<IViewPart::Pointer>();
}

//std::vector<IViewPart::Pointer> WorkbenchPage::GetViews(
//    Perspective::Pointer persp, bool restore)
//{
//  return std::vector<IViewPart::Pointer>();
//}

/**
 * See IWorkbenchPage.
 */
IWorkbenchWindow::Pointer WorkbenchPage::GetWorkbenchWindow()
{
  return 0;
}

/*
 * (non-Javadoc)
 * 
 * @see org.eclipse.ui.IWorkbenchPage#hideView(org.eclipse.ui.IViewReference)
 */
void WorkbenchPage::HideView(IViewReference::Pointer ref)
{

}

void WorkbenchPage::RefreshActiveView()
{

}

void WorkbenchPage::HideView(IViewPart::Pointer view)
{

}

void WorkbenchPage::Init(WorkbenchWindow::Pointer w,
    const std::string& layoutID, IAdaptable* input, bool openExtras)
{
  // Save args.
  this->window = w;
  this->input = input;
  this->viewFactory = 0;
  //actionSets = new ActionSetManager(w);

  deferCount = 0;
  
  // Create presentation.
  this->CreateClientComposite();
  editorPresentation = window->GetWorkbench().Cast<Workbench>()->CreateEditorPresentation();
  //editorMgr = new EditorManager(window, this, editorPresentation);
}

bool WorkbenchPage::IsPartVisible(IWorkbenchPart::Pointer part)
{
  return false;
}

/**
 * See IWorkbenchPage.
 */
bool WorkbenchPage::IsEditorAreaVisible()
{
  return false;
}

/**
 * Returns whether the view is fast.
 */
bool WorkbenchPage::IsFastView(IViewReference::Pointer ref)
{
  return false;
}

bool WorkbenchPage::IsCloseable(IViewReference::Pointer ref)
{
  return false;
}

bool WorkbenchPage::IsMoveable(IViewReference::Pointer ref)
{
  return false;
}

bool WorkbenchPage::IsSaveNeeded()
{
  return false;
}

void WorkbenchPage::OnActivate()
{

}

void WorkbenchPage::OnDeactivate()
{

}

void WorkbenchPage::ReuseEditor(IReusableEditor::Pointer editor,
    IEditorInput* input)
{

}

IEditorPart::Pointer WorkbenchPage::OpenEditor(IEditorInput* input,
    const std::string& editorID)
{
  return 0;
}

IEditorPart::Pointer WorkbenchPage::OpenEditor(IEditorInput* input,
    const std::string& editorID, bool activate)
{
  return 0;
}

IEditorPart::Pointer WorkbenchPage::OpenEditor(const IEditorInput* input,
    const std::string& editorID, bool activate, int matchFlags)
{
  return 0;
}

IEditorPart::Pointer WorkbenchPage::OpenEditor(const IEditorInput* input,
    const std::string& editorID, bool activate, int matchFlags,
    IMemento::ConstPointer editorState)
{
  return 0;
}

IEditorPart::Pointer WorkbenchPage::OpenEditorFromDescriptor(
    const IEditorInput* input,
    const IEditorDescriptor::ConstPointer editorDescriptor, bool activate,
    const IMemento* editorState)
{
  return 0;
}

/**
 * @see #openEditor(IEditorInput, String, boolean, int)
 */
IEditorPart::Pointer WorkbenchPage::BusyOpenEditor(IEditorInput* input,
    const std::string& editorID, bool activate, int matchFlags,
    IMemento* editorState)
{
  return 0;
}

IEditorPart::Pointer WorkbenchPage::BusyOpenEditorFromDescriptor(
    IEditorInput* input, EditorDescriptor::Pointer editorDescriptor,
    bool activate, IMemento* editorState)
{
  return 0;
}

IEditorPart::Pointer WorkbenchPage::BusyOpenEditorBatched(IEditorInput* input,
    const std::string& editorID, bool activate, int matchFlags,
    IMemento* editorState)
{
  return 0;
}

IEditorPart::Pointer WorkbenchPage::BusyOpenEditorFromDescriptorBatched(
    IEditorInput* input, EditorDescriptor::Pointer editorDescriptor,
    bool activate, IMemento* editorState)
{
  return 0;
}

void WorkbenchPage::OpenEmptyTab()
{

}

void WorkbenchPage::ShowEditor(bool activate, IEditorPart::Pointer editor)
{

}

bool WorkbenchPage::IsEditorPinned(IEditorPart::Pointer editor)
{
  return false;
}

void WorkbenchPage::RequestActivation(IWorkbenchPart::Pointer part)
{

}

void WorkbenchPage::ResetPerspective()
{
  
}

bool WorkbenchPage::RestoreState(IMemento::Pointer memento,
    const IPerspectiveDescriptor::ConstPointer activeDescriptor)
{
  return false;
}

bool WorkbenchPage::SaveAllEditors(bool confirm)
{
  return false;
}

bool WorkbenchPage::SaveAllEditors(bool confirm, bool addNonPartSources)
{
  return false;
}

bool WorkbenchPage::SavePart(IWorkbenchPart::Pointer saveable, bool confirm)
{
  return false;
}

bool WorkbenchPage::SaveEditor(IEditorPart::Pointer editor, bool confirm)
{
  return false;
}

void WorkbenchPage::SavePerspective()
{
  
}

bool WorkbenchPage::SaveState(IMemento::Pointer memento)
{
  return false;
}

std::string WorkbenchPage::GetId(IWorkbenchPart::Pointer part)
{
  return "";
}

std::string WorkbenchPage::GetId(IWorkbenchPartReference::Pointer ref)
{
  return "";
}

void WorkbenchPage::SetActivePart(IWorkbenchPart::Pointer newPart)
{
   IWorkbenchPartReference::Pointer partref = this->GetReference(newPart);
   IWorkbenchPartReference::Pointer realPartRef = 0;
   if (newPart.IsNotNull())
   {
     IWorkbenchPartSite::Pointer site = newPart->GetSite();
     if (site.Cast<PartSite>().IsNotNull())
     {
       realPartRef = site.Cast<PartSite>()->GetPane()->GetPartReference();
     }
   }

   partBeingActivated = realPartRef;

   // Deactivate old part
//   IWorkbenchPart oldPart = getActivePart();
//   if (oldPart != null)
//   {
//     deactivatePart(oldPart);
//   }

   // Set active part.
   if (newPart.IsNotNull())
   {
     //activationList.setActive(newPart);
     if (newPart.Cast<IEditorPart>().IsNotNull())
     {
       this->MakeActiveEditor(realPartRef.Cast<IEditorReference>());
     }
   }
   this->ActivatePart(newPart);

   //actionSwitcher.updateActivePart(newPart);

   //partList.setActivePart(partref);
}

void WorkbenchPage::SetEditorAreaVisible(bool showEditorArea)
{

}

//void WorkbenchPage::UpdateVisibility(Perspective::Pointer oldPersp,
//    Perspective::Pointer newPersp)
//{
//
//}

//PartService* WorkbenchPage::GetPartService()
//{
//  return 0;
//}

void WorkbenchPage::ResetToolBarLayout()
{

}

IViewPart::Pointer WorkbenchPage::ShowView(const std::string& viewID)
{
  return this->ShowView(viewID, "", VIEW_ACTIVATE);
}

IViewPart::Pointer WorkbenchPage::ShowView(const std::string& viewID,
    const std::string& secondaryID, int mode)
{
  if (secondaryID != "")
  {
    if (secondaryID.find_first_of(ViewFactory::ID_SEP) != std::string::npos)
    {
      throw Poco::InvalidArgumentException("Illegal secondary id");
    }
  }
  
  std::cout << "Trying to show view: " << viewID << std::endl;
  
  IViewReference::Pointer ref;
  
  // look if view is already created
  std::vector<IViewReference::Pointer> refs(this->GetViewReferences());
  for (unsigned int i = 0; i < refs.size(); i++)
  {
    if (viewID == refs[i]->GetId()
        && (secondaryID == refs[i]->GetSecondaryId()))
    {
      ref = refs[i];
      break;
    }
  }
  
  IViewPart::Pointer view;
  if (ref.IsNotNull())
  {
    view = ref->GetView(true);
  }
  if (view.IsNotNull())
  {
    this->BusyShowView(view, mode);
    return view;
  }
  
  
  // perspective show view part
  ViewFactory* factory = this->GetViewFactory();
  ref = factory->CreateView(viewID, secondaryID);
  view = ref->GetPart(true).Cast<IViewPart>();
  if (view.IsNull()) {
      throw PartInitException("View Factory: Could not create view " + ref->GetId());
  }
  ViewSite::Pointer site = view->GetSite().Cast<ViewSite>();
  PartPane::Pointer pane = site->GetPane(); 
    
  // should add pane to main window here
  window->GetWorkbench().Cast<Workbench>()->AddViewPane(pane);
  
  // Ensure that the newly showing part is enabled
  if (pane.IsNotNull())
    pane->SetControlEnabled(true);
  
  this->BusyShowView(view, mode);   
  
  return view;
}

bool WorkbenchPage::CertifyMode(int mode)
{
  return false;
}

std::vector<IEditorReference::Pointer> WorkbenchPage::GetSortedEditors()
{
  return std::vector<IEditorReference::Pointer>();
}

std::vector<IWorkbenchPartReference::Pointer> WorkbenchPage::GetSortedParts()
{
  return std::vector<IWorkbenchPartReference::Pointer>();
}

IWorkbenchPartReference::Pointer WorkbenchPage::GetReference(
    IWorkbenchPart::Pointer part)
{
  if (part.IsNull()) {
      return 0;
  }
  IWorkbenchPartSite::Pointer site = part->GetSite();
  if (site.Cast<PartSite>().IsNull()) {
    return 0;
  }
  PartSite::Pointer partSite = site.Cast<PartSite>();
  
  return partSite->GetPartReference();
}

//std::vector<IViewReference::Pointer> WorkbenchPage::GetViewReferenceStack(
//    IViewPart::Pointer part)
//{
//  return std::vector<IViewReference::Pointer>();
//}

//std::vector<IViewPart::Pointer> WorkbenchPage::GetViewStack(
//    IViewPart::Pointer part)
//{
//  return std::vector<IViewPart::Pointer>();
//}

void WorkbenchPage::ResizeView(IViewPart::Pointer part, int width, int height)
{

}

//void WorkbenchPage::FindSashParts(LayoutTree tree, PartPane::Sashes sashes,
//    SashInfo info)
//{
//
//}

std::vector<IWorkbenchPartReference::Pointer> WorkbenchPage::GetAllParts()
{
  return std::vector<IWorkbenchPartReference::Pointer>();
}

std::vector<IWorkbenchPartReference::Pointer> WorkbenchPage::GetOpenParts()
{
  return std::vector<IWorkbenchPartReference::Pointer>();
}

void WorkbenchPage::TestInvariants()
{

}

std::vector<std::string> WorkbenchPage::GetPerspectiveShortcuts()
{
  return std::vector<std::string>();
}

std::vector<std::string> WorkbenchPage::GetShowViewShortcuts()
{
  return std::vector<std::string>();
}

void WorkbenchPage::CloseAllPerspectives(bool saveEditors, bool closePage)
{
  
}

void WorkbenchPage::CreateClientComposite()
{
  composite = window->GetWorkbench().Cast<Workbench>()->CreateWorkbenchPageControl();
}

void WorkbenchPage::SuggestReset()
{

}

bool WorkbenchPage::IsPartVisible(IWorkbenchPartReference::Pointer reference)
{
  return false;
}

}
