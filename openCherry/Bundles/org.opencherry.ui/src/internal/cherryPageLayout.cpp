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

#include "cherryPageLayout.h"

#include "cherryWorkbenchPlugin.h"
#include "cherryLayoutHelper.h"
#include "cherryViewLayout.h"
#include "cherryPresentationFactoryUtil.h"
#include "cherryFolderLayout.h"
#include "cherryPlaceholderFolderLayout.h"

#include "../cherryUIException.h"
#include "../cherryConstants.h"

namespace cherry
{

PageLayout::PageLayout()
 : editorVisible(true)
{
  //no-op
}

PageLayout::PageLayout(ViewSashContainer::Pointer container,
    ViewFactory* viewFactory, LayoutPart::Pointer editorFolder,
    IPerspectiveDescriptor::Pointer descriptor)
 : editorVisible(true)
{
  this->viewFactory = viewFactory;
  this->rootLayoutContainer = container;
  this->editorFolder = editorFolder;
  this->descriptor = descriptor;
  this->Prefill();
}

void PageLayout::AddEditorArea()
{
  try
  {
    // Create the part.
//    StackablePart::Pointer newPart = this->CreateView(ID_EDITOR_AREA);
//    if (newPart == 0)
//    {
//      // this should never happen as long as newID is the editor ID.
//      return;
//    }
//
 //   this->SetFolderPart(ID_EDITOR_AREA, editorFolder.Cast<ILayoutContainer>());

    // Add it to the layout.
    rootLayoutContainer->Add(editorFolder);
  }
  catch (PartInitException& e)
  {
    WorkbenchPlugin::Log(this->GetClassName(), "AddEditorArea()", e); //$NON-NLS-1$
  }
}

ViewLayoutRec::Pointer PageLayout::GetViewLayoutRec(const std::string& id, bool create)
{
  ViewLayoutRec::Pointer rec = mapIDtoViewLayoutRec[id];
  if (rec == 0 && create)
  {
    rec = new ViewLayoutRec();
    // set up the view layout appropriately if the page layout is fixed
    if (this->IsFixed())
    {
      rec->isCloseable = false;
      rec->isMoveable = false;
    }
    mapIDtoViewLayoutRec[id] = rec;
  }
  return rec;
}

void PageLayout::AddStack(IStackableContainer::Pointer newPart,
    const std::string& partId, int relationship, float ratio,
    const std::string& refId)
{

  //this->SetRefPart(partId, newPart);
  this->SetFolderPart(partId, newPart);

  // If the referenced part is inside a folder,
  // then use the folder as the reference part.
  IStackableContainer::Pointer refPart = this->GetFolderPart(refId);

  //  if (refPart == 0)
//  {
//    refPart = this->GetRefPart(refId);
//  }

  // Add it to the layout.
  if (refPart != 0)
  {
    ratio = this->NormalizeRatio(ratio);
    rootLayoutContainer->Add(newPart.Cast<LayoutPart>(), this->GetPartSashConst(relationship), ratio,
        refPart.Cast<LayoutPart>());
  }
  else if (refId == ID_EDITOR_AREA)
  {
    ratio = this->NormalizeRatio(ratio);
    rootLayoutContainer->Add(newPart.Cast<LayoutPart>(), this->GetPartSashConst(relationship), ratio,
        this->editorFolder.Cast<LayoutPart>());
  }
  else
  {
    WorkbenchPlugin::Log("Reference part does not exist yet: " + refId);
    rootLayoutContainer->Add(newPart.Cast<LayoutPart>());
  }
}

void PageLayout::AddPerspectiveShortcut(const std::string& id)
{
  if (std::find(perspectiveShortcuts.begin(),
      perspectiveShortcuts.end(), id) == perspectiveShortcuts.end())
  {
    perspectiveShortcuts.push_back(id);
  }
}

void PageLayout::AddPlaceholder(const std::string& viewId, int relationship,
    float ratio, const std::string& refId)
{
  if (!this->CheckValidPlaceholderId(viewId))
  {
    return;
  }

  // Create a folder.
  ContainerPlaceholder::Pointer folder(new ContainerPlaceholder(viewId));
  folder->SetContainer(rootLayoutContainer);
  folder->SetRealContainer(PartStack::Pointer(new PartStack(rootLayoutContainer->page)));
  //folder->SetId(folderId);
  // Create the placeholder.
  PartPlaceholder::Pointer newPart(new PartPlaceholder(viewId));
  folder->Add(newPart);
  this->AddStack(folder, viewId, relationship, ratio, refId);
  // force creation of the view layout rec
  this->GetViewLayoutRec(viewId, true);
}

bool PageLayout::CheckValidPlaceholderId(const std::string& id)
{
  // Check that view is not already in layout.
  // This check is done even if the id has a wildcard, since it's incorrect to create
  // multiple placeholders with the same id, wildcard or not.
  if (this->CheckPartInLayout(id))
  {
    return false;
  }

  // check that primary view id is valid, but only if it has no wildcard
  std::string primaryId = ViewFactory::ExtractPrimaryId(id);
  if (!ViewFactory::HasWildcard(primaryId))
  {
    IViewRegistry* reg = WorkbenchPlugin::GetDefault()->GetViewRegistry();
    IViewDescriptor::Pointer desc = reg->Find(primaryId);
    if (desc == 0)
    {
      // cannot safely open the dialog so log the problem
      WorkbenchPlugin::Log("Unable to find view with id: " + primaryId
          + ", when creating perspective " + this->GetDescriptor()->GetId()); //$NON-NLS-1$ //$NON-NLS-2$
      return false;
    }
  }

  return true;
}

void PageLayout::AddShowInPart(const std::string& id)
{
  if (std::find(showInPartIds.begin(), showInPartIds.end(), id) == showInPartIds.end())
  {
    showInPartIds.push_back(id);
  }
}

void PageLayout::AddShowViewShortcut(const std::string& id)
{
  if (std::find(showViewShortcuts.begin(), showViewShortcuts.end(), id) == showInPartIds.end())
  {
    showViewShortcuts.push_back(id);
  }
}

void PageLayout::AddView(const std::string& viewId, int relationship,
    float ratio, const std::string& refId)
{
  this->AddView(viewId, relationship, ratio, refId, false, false, true);
}

void PageLayout::AddView(const std::string& viewId, int relationship,
    float ratio, const std::string& refId, bool minimized)
{
  this->AddView(viewId, relationship, ratio, refId, minimized, false, true);
}

void PageLayout::AddView(const std::string& viewId, int relationship,
    float ratio, const std::string& refId, bool minimized, bool standalone,
    bool showTitle)
{
  if (this->CheckPartInLayout(viewId))
  {
    return;
  }

  try
  {
    // Create the part.
    StackablePart::Pointer newPart = this->CreateView(viewId);
    if (newPart == 0)
    {
      this->AddPlaceholder(viewId, relationship, ratio, refId);
      //TODO ViewActivator for IIdentifier
      //LayoutHelper::AddViewActivator(this, viewId);
    }
    else
    {
      int appearance = PresentationFactoryUtil::ROLE_VIEW;
      if (standalone)
      {
        if (showTitle)
        {
          appearance = PresentationFactoryUtil::ROLE_STANDALONE;
        }
        else
        {
          appearance = PresentationFactoryUtil::ROLE_STANDALONE_NOTITLE;
        }
      }

      // PartStack for views
      PartStack::Pointer newFolder(new PartStack(rootLayoutContainer->page,
          true, appearance, 0));
      newFolder->Add(newPart);
      this->SetFolderPart(viewId, newFolder);
      this->AddStack(newFolder, viewId, relationship, ratio, refId);
      // force creation of the view layout rec
      this->GetViewLayoutRec(viewId, true);

      // Capture any minimized stacks
//      if (minimized)
//      {
//        // Remember the minimized stacks so we can
//        // move them to the trim when the Perspective
//        // activates...
//        minimizedStacks.add(newFolder);
//      }
    }
  }
  catch (PartInitException& e)
  {
    WorkbenchPlugin::Log(this->GetClassName(), "AddView()", e); //$NON-NLS-1$
  }
}

//      List getMinimizedStacks() {
//      return minimizedStacks;
//    }

bool PageLayout::CheckPartInLayout(const std::string& partId)
{
  if (partId == ID_EDITOR_AREA) return true;

  if (this->GetRefPart(partId) != 0) // || this->IsFastViewId(partId))
  {
    WorkbenchPlugin::Log("Part already exists in page layout: " + partId);
    return true;
  }

  if (this->GetFolderPart(partId) != 0) // || this->IsFastViewId(partId))
  {
    WorkbenchPlugin::Log("Part already exists in page layout: " + partId);
    return true;
  }

  return false;
}

IFolderLayout::Pointer PageLayout::CreateFolder(const std::string& folderId,
    int relationship, float ratio, const std::string& refId)
{
  if (this->CheckPartInLayout(folderId))
  {
    IStackableContainer::Pointer folder = this->GetFolderPart(folderId);

    return mapFolderToFolderLayout[folder].Cast<IFolderLayout>();
  }

  // Create the folder.
  PartStack::Pointer folder(new PartStack(rootLayoutContainer->page));
  folder->SetID(folderId);
  this->AddStack(folder, folderId, relationship, ratio, refId);

  // Create a wrapper.
  FolderLayout::Pointer layout(new FolderLayout(PageLayout::Pointer(this), folder, viewFactory));

  mapFolderToFolderLayout.insert(std::make_pair(folder, layout));

  return layout;
}

IPlaceholderFolderLayout::Pointer PageLayout::CreatePlaceholderFolder(
    const std::string& folderId, int relationship, float ratio, const std::string& refId)
{
  if (this->CheckPartInLayout(folderId))
  {
    ContainerPlaceholder::Pointer folder = this->GetRefPart(folderId).Cast<ContainerPlaceholder>();

    return mapFolderToFolderLayout[folder];
  }

  // Create the folder.
  ContainerPlaceholder::Pointer folder(new ContainerPlaceholder(""));
  folder->SetContainer(rootLayoutContainer);
  folder->SetRealContainer(IStackableContainer::Pointer(new PartStack(rootLayoutContainer->page)));
  folder->SetID(folderId);
  this->AddStack(folder, folderId, relationship, ratio, refId);

  // Create a wrapper.
  IPlaceholderFolderLayout::Pointer layout(new PlaceholderFolderLayout(PageLayout::Pointer(this), folder));

  mapFolderToFolderLayout.insert(std::make_pair(folder, layout));

  return layout;
}

StackablePart::Pointer PageLayout::CreateView(const std::string& partID)
{
//  if (partID == ID_EDITOR_AREA)
//  {
//    return editorFolder;
//  }

  IViewDescriptor::Pointer viewDescriptor = viewFactory->GetViewRegistry()
    ->Find(ViewFactory::ExtractPrimaryId(partID));
//  if (WorkbenchActivityHelper.filterItem(viewDescriptor))
//  {
//    return null;
//  }
  return LayoutHelper::CreateView(this->GetViewFactory(), partID);
}

IPerspectiveDescriptor::Pointer PageLayout::GetDescriptor()
{
  return descriptor;
}

std::string PageLayout::GetEditorArea()
{
  return ID_EDITOR_AREA;
}

IStackableContainer::Pointer PageLayout::GetFolderPart(const std::string& viewId)
{
  return mapIDtoFolder[viewId];
}

int PageLayout::GetPartSashConst(int nRelationship)
{
  return nRelationship;
}

std::vector<std::string> PageLayout::GetPerspectiveShortcuts()
{
  return perspectiveShortcuts;
}

StackablePart::Pointer PageLayout::GetRefPart(const std::string& partID)
{
  return mapIDtoPart[partID];
}

PartSashContainer::Pointer PageLayout::GetRootLayoutContainer()
{
  return rootLayoutContainer;
}

std::vector<std::string> PageLayout::GetShowInPartIds()
{
  return showInPartIds;
}

std::vector<std::string> PageLayout::GetShowViewShortcuts()
{
  return showViewShortcuts;
}

ViewFactory* PageLayout::GetViewFactory()
{
  return viewFactory;
}

bool PageLayout::IsEditorAreaVisible()
{
  return editorVisible;
}

float PageLayout::NormalizeRatio(float in)
{
  if (in < RATIO_MIN)
  {
    in = RATIO_MIN;
  }
  if (in > RATIO_MAX)
  {
    in = RATIO_MAX;
  }
  return in;
}

void PageLayout::Prefill()
{
  this->AddEditorArea();

  //TODO action sets
  // Add default action sets.
//  ActionSetRegistry reg = WorkbenchPlugin.getDefault()
//  .getActionSetRegistry();
//  IActionSetDescriptor[] array = reg.getActionSets();
//  int count = array.length;
//  for (int nX = 0; nX < count; nX++)
//  {
//    IActionSetDescriptor desc = array[nX];
//    if (desc.isInitiallyVisible())
//    {
//      addActionSet(desc.getId());
//    }
//  }
}

void PageLayout::SetEditorAreaVisible(bool showEditorArea)
{
  editorVisible = showEditorArea;
}

void PageLayout::SetFixed(bool fixed)
{
  this->fixed = fixed;
}

bool PageLayout::IsFixed()
{
  return fixed;
}

void PageLayout::SetFolderPart(const std::string& viewId,
    ContainerPlaceholder::Pointer container)
{
  IStackableContainer::Pointer tabFolder = container->GetRealContainer();
  mapIDtoFolder[viewId] = tabFolder;
}

void PageLayout::SetFolderPart(const std::string& viewId,
    PartStack::Pointer folder)
{
  mapIDtoFolder[viewId] = folder.Cast<IStackableContainer>();
}

void PageLayout::SetFolderPart(const std::string& viewId,
    IStackableContainer::Pointer folder)
{
  mapIDtoFolder[viewId] = folder;
}

void PageLayout::SetRefPart(const std::string& partID, StackablePart::Pointer part)
{
  mapIDtoPart[partID] = part;
}

void PageLayout::StackPart(StackablePart::Pointer newPart,
    const std::string& viewId, const std::string& refId)
{
  this->SetRefPart(viewId, newPart);
  // force creation of the view layout rec
  this->GetViewLayoutRec(viewId, true);

  // If ref part is in a folder than just add the
  // new view to that folder.
  PartStack::Pointer folder = this->GetFolderPart(refId).Cast<PartStack>();
  if (folder != 0)
  {
    folder->Add(newPart);
    this->SetFolderPart(viewId, folder);
    return;
  }

// parts are now always contained in folders

//  // If the ref part is in the page layout then create
//  // a new folder and add the new view.
//  StackablePart::Pointer refPart = this->GetRefPart(refId);
//  if (refPart != 0) // && (refPart instanceof PartPane || refPart instanceof PartPlaceholder))
//  {
//    PartStack::Pointer newFolder(new PartStack(rootLayoutContainer->page));
//    rootLayoutContainer->Replace(refPart, newFolder);
//    newFolder->Add(refPart);
//    newFolder->Add(newPart);
//    this->SetFolderPart(refId, newFolder);
//    this->SetFolderPart(viewId, newFolder);
//    return;
//  }

  // If ref part is not found then just do add.
  WorkbenchPlugin::Log("Referenced part does not exist yet: " + refId);
  PartStack::Pointer newFolder(new PartStack(rootLayoutContainer->page));
  newFolder->Add(newPart);
  this->SetFolderPart(viewId, newFolder);
  rootLayoutContainer->Add(newFolder);
}

void PageLayout::StackPlaceholder(const std::string& viewId,
    const std::string& refId)
{
  if (this->CheckPartInLayout(viewId))
  {
    return;
  }

  // Create the placeholder.
  PartPlaceholder::Pointer newPart(new PartPlaceholder(viewId));

  StackablePart::Pointer refPart = this->GetRefPart(refId);
  if (refPart != 0)
  {
    newPart->SetContainer(refPart->GetContainer());
  }

  this->StackPart(newPart, viewId, refId);
}

void PageLayout::StackView(const std::string& viewId, const std::string& refId)
{
  if (this->CheckPartInLayout(viewId))
  {
    return;
  }

  // Create the new part.
  try
  {
    StackablePart::Pointer newPart = this->CreateView(viewId);
    if (newPart == 0)
    {
      this->StackPlaceholder(viewId, refId);
      LayoutHelper::AddViewActivator(PageLayout::Pointer(this), viewId);
    }
    else
    {
      this->StackPart(newPart, viewId, refId);
    }
  }
  catch (PartInitException& e)
  {
    WorkbenchPlugin::Log(this->GetClassName(), "StackView", e); //$NON-NLS-1$
  }
}

int PageLayout::ConstantToLayoutPosition(int constant) {
  if (constant == Constants::TOP)
      return IPageLayout::TOP;
  if (constant == Constants::BOTTOM)
      return IPageLayout::BOTTOM;
  if (constant == Constants::RIGHT)
      return IPageLayout::RIGHT;
  if (constant == Constants::LEFT)
      return IPageLayout::LEFT;

  return -1;
}

void PageLayout::AddStandaloneView(const std::string& viewId, bool showTitle,
    int relationship, float ratio, const std::string& refId)
{
  this->AddView(viewId, relationship, ratio, refId, false, true, showTitle);
  ViewLayoutRec::Pointer rec = this->GetViewLayoutRec(viewId, true);
  rec->isStandalone = true;
  rec->showTitle = showTitle;
}

void PageLayout::AddStandaloneViewPlaceholder(const std::string& viewId,
    int relationship, float ratio, const std::string& refId, bool showTitle)
{

  std::string stackId = viewId + ".standalonefolder"; //$NON-NLS-1$

  // Check to see if the view is already in the layout
  if (!this->CheckValidPlaceholderId(viewId))
  {
    return;
  }

  // Create the folder.
  ContainerPlaceholder::Pointer folder(new ContainerPlaceholder(0));
  folder->SetContainer(rootLayoutContainer);
  int appearance = PresentationFactoryUtil::ROLE_STANDALONE;
  if (!showTitle)
  {
    appearance = PresentationFactoryUtil::ROLE_STANDALONE_NOTITLE;
  }
  folder->SetRealContainer(IStackableContainer::Pointer(new PartStack(rootLayoutContainer->page, true,
      appearance, 0)));
  folder->SetID(stackId);
  this->AddStack(folder, stackId, relationship, ratio, refId);

  // Create a wrapper.
  PlaceholderFolderLayout::Pointer placeHolder(new PlaceholderFolderLayout(PageLayout::Pointer(this),
      folder));

  // Add the standalone view immediately
  placeHolder->AddPlaceholder(viewId);

  ViewLayoutRec::Pointer rec = this->GetViewLayoutRec(viewId, true);
  rec->isStandalone = true;
  rec->showTitle = showTitle;
}

IViewLayout::Pointer PageLayout::GetViewLayout(const std::string& viewId)
{
  ViewLayoutRec::Pointer rec = this->GetViewLayoutRec(viewId, true);
  if (rec == 0)
  {
    return IViewLayout::Pointer(0);
  }
  return IViewLayout::Pointer(new ViewLayout(PageLayout::Pointer(this), rec));
}

std::map<std::string, ViewLayoutRec::Pointer> PageLayout::GetIDtoViewLayoutRecMap()
{
  return mapIDtoViewLayoutRec;
}

void PageLayout::RemovePlaceholder(const std::string& id)
{
  StackablePart::Pointer part = this->GetRefPart(id);
  if (part->IsPlaceHolder())
  {
    IStackableContainer::Pointer stack = this->GetFolderPart(id);
    if (stack != 0)
    {
      stack->Remove(part);
    }
    else
    {
      //rootLayoutContainer->Remove(part);
      WorkbenchPlugin::Log("Not removing placeholder: Folder for placeholder " + id + " not found");
    }
    mapIDtoPart.erase(id);
    mapIDtoFolder.erase(id);
    mapIDtoViewLayoutRec.erase(id);
  }
}

IPlaceholderFolderLayout::Pointer PageLayout::GetFolderForView(
    const std::string& viewId)
{
  if (mapIDtoFolder[viewId] == 0)
    return IPlaceholderFolderLayout::Pointer(0);

  IStackableContainer::Pointer folder = mapIDtoFolder[viewId];
  IPlaceholderFolderLayout::Pointer layout;
  if (mapFolderToFolderLayout[folder] == 0)
  {
    layout = new FolderLayout(PageLayout::Pointer(this), folder.Cast<PartStack>(), viewFactory);
    mapFolderToFolderLayout[folder] = layout;
  }
  else
  {
    layout = mapFolderToFolderLayout[folder];
  }
  return layout;
}

}
