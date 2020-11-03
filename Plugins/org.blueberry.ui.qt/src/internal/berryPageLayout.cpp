/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryPageLayout.h"

#include "berryWorkbenchPlugin.h"
#include "berryLayoutHelper.h"
#include "berryViewLayout.h"
#include "berryPresentationFactoryUtil.h"
#include "berryFolderLayout.h"
#include "berryPlaceholderFolderLayout.h"

#include "berryUIException.h"
#include "berryConstants.h"

namespace berry
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
    LayoutPart::Pointer newPart = this->CreateView(ID_EDITOR_AREA);
    if (newPart == 0)
    {
      // this should never happen as long as newID is the editor ID.
      return;
    }

    this->SetRefPart(ID_EDITOR_AREA, newPart);

    // Add it to the layout.
    rootLayoutContainer->Add(newPart);
  }
  catch (PartInitException& e)
  {
    WorkbenchPlugin::Log(this->GetClassName(), "AddEditorArea()", e); //$NON-NLS-1$
  }
}

ViewLayoutRec::Pointer PageLayout::GetViewLayoutRec(const QString& id, bool create)
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

void PageLayout::AddPart(LayoutPart::Pointer newPart,
    const QString& partId, int relationship, float ratio,
    const QString& refId)
{

  this->SetRefPart(partId, newPart);

  // If the referenced part is inside a folder,
  // then use the folder as the reference part.
  LayoutPart::Pointer refPart = this->GetFolderPart(refId);
  if (refPart == 0)
  {
    refPart = this->GetRefPart(refId);
  }

  // Add it to the layout.
  if (refPart != 0)
  {
    ratio = this->NormalizeRatio(ratio);
    rootLayoutContainer->Add(newPart, this->GetPartSashConst(relationship), ratio, refPart);
  }
  else
  {
    WorkbenchPlugin::Log("Reference part does not exist yet: " + refId);
    rootLayoutContainer->Add(newPart);
  }
}

void PageLayout::AddPerspectiveShortcut(const QString& id)
{
  if (!perspectiveShortcuts.contains(id))
  {
    perspectiveShortcuts.push_back(id);
  }
}

void PageLayout::AddPlaceholder(const QString& viewId, int relationship,
    float ratio, const QString& refId)
{
  if (!this->CheckValidPlaceholderId(viewId))
  {
    return;
  }

  // Create the placeholder.
  PartPlaceholder::Pointer newPart(new PartPlaceholder(viewId));
  this->AddPart(newPart, viewId, relationship, ratio, refId);
  // force creation of the view layout rec
  this->GetViewLayoutRec(viewId, true);
}

bool PageLayout::CheckValidPlaceholderId(const QString& id)
{
  // Check that view is not already in layout.
  // This check is done even if the id has a wildcard, since it's incorrect to create
  // multiple placeholders with the same id, wildcard or not.
  if (this->CheckPartInLayout(id))
  {
    return false;
  }

  // check that primary view id is valid, but only if it has no wildcard
  QString primaryId = ViewFactory::ExtractPrimaryId(id);
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

void PageLayout::AddShowInPart(const QString& id)
{
  if (!showInPartIds.contains(id))
  {
    showInPartIds.push_back(id);
  }
}

void PageLayout::AddShowViewShortcut(const QString& id)
{
  if (!showViewShortcuts.contains(id))
  {
    showViewShortcuts.push_back(id);
  }
}

void PageLayout::AddView(const QString& viewId, int relationship,
    float ratio, const QString& refId)
{
  this->AddView(viewId, relationship, ratio, refId, false, false, true);
}

void PageLayout::AddView(const QString& viewId, int relationship,
    float ratio, const QString& refId, bool minimized)
{
  this->AddView(viewId, relationship, ratio, refId, minimized, false, true);
}

void PageLayout::AddView(const QString& viewId, int relationship,
    float ratio, const QString& refId, bool /*minimized*/, bool standalone,
    bool showTitle)
{
  this->AddShowViewShortcut(viewId);
  if (this->CheckPartInLayout(viewId))
  {
    return;
  }

  try
  {
    // Create the part.
    LayoutPart::Pointer newPart = this->CreateView(viewId);
    if (newPart == 0)
    {
      this->AddPlaceholder(viewId, relationship, ratio, refId);
      LayoutHelper::AddViewActivator(PageLayout::Pointer(this), viewId);
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
          true, appearance, nullptr));
      newFolder->Add(newPart);
      this->SetFolderPart(viewId, newFolder);
      this->AddPart(newFolder, viewId, relationship, ratio, refId);
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

bool PageLayout::CheckPartInLayout(const QString& partId)
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

IFolderLayout::Pointer PageLayout::CreateFolder(const QString& folderId,
    int relationship, float ratio, const QString& refId)
{
  if (this->CheckPartInLayout(folderId))
  {
    ILayoutContainer::Pointer folder = this->GetFolderPart(folderId);

    return mapFolderToFolderLayout[folder].Cast<IFolderLayout>();
  }

  // Create the folder.
  PartStack::Pointer folder(new PartStack(rootLayoutContainer->page));
  folder->SetID(folderId);
  this->AddPart(folder, folderId, relationship, ratio, refId);

  // Create a wrapper.
  FolderLayout::Pointer layout(new FolderLayout(PageLayout::Pointer(this), folder, viewFactory));

  mapFolderToFolderLayout.insert(folder, layout);

  return layout;
}

IPlaceholderFolderLayout::Pointer PageLayout::CreatePlaceholderFolder(
    const QString& folderId, int relationship, float ratio, const QString& refId)
{
  if (this->CheckPartInLayout(folderId))
  {
    ContainerPlaceholder::Pointer folder = this->GetRefPart(folderId).Cast<ContainerPlaceholder>();

    return mapFolderToFolderLayout[folder];
  }

  // Create the folder.
  ContainerPlaceholder::Pointer folder(new ContainerPlaceholder(""));
  folder->SetContainer(rootLayoutContainer);
  folder->SetRealContainer(ILayoutContainer::Pointer(new PartStack(rootLayoutContainer->page)));
  folder->SetID(folderId);
  this->AddPart(folder, folderId, relationship, ratio, refId);

  // Create a wrapper.
  IPlaceholderFolderLayout::Pointer layout(new PlaceholderFolderLayout(this, folder));

  mapFolderToFolderLayout.insert(folder, layout);

  return layout;
}

LayoutPart::Pointer PageLayout::CreateView(const QString& partID)
{
  if (partID == ID_EDITOR_AREA)
  {
    return editorFolder;
  }

//  IViewDescriptor::Pointer viewDescriptor = viewFactory->GetViewRegistry()
//    ->Find(ViewFactory::ExtractPrimaryId(partID));
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

QString PageLayout::GetEditorArea()
{
  return ID_EDITOR_AREA;
}

PartStack::Pointer PageLayout::GetFolderPart(const QString& viewId)
{
  return mapIDtoFolder[viewId].Cast<PartStack>();
}

int PageLayout::GetPartSashConst(int nRelationship)
{
  return nRelationship;
}

QList<QString> PageLayout::GetPerspectiveShortcuts()
{
  return perspectiveShortcuts;
}

LayoutPart::Pointer PageLayout::GetRefPart(const QString& partID)
{
  return mapIDtoPart[partID];
}

PartSashContainer::Pointer PageLayout::GetRootLayoutContainer()
{
  return rootLayoutContainer;
}

QList<QString> PageLayout::GetShowInPartIds()
{
  return showInPartIds;
}

QList<QString> PageLayout::GetShowViewShortcuts()
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

void PageLayout::SetFolderPart(const QString& viewId,
    ContainerPlaceholder::Pointer container)
{
  LayoutPart::Pointer tabFolder = container->GetRealContainer();
  mapIDtoFolder[viewId] = tabFolder.Cast<ILayoutContainer>();
}

void PageLayout::SetFolderPart(const QString& viewId,
    PartStack::Pointer folder)
{
  mapIDtoFolder[viewId] = folder.Cast<ILayoutContainer>();
}

void PageLayout::SetFolderPart(const QString& viewId,
    ILayoutContainer::Pointer folder)
{
  mapIDtoFolder[viewId] = folder;
}

void PageLayout::SetRefPart(const QString& partID, LayoutPart::Pointer part)
{
  mapIDtoPart[partID] = part;
}

void PageLayout::StackPart(LayoutPart::Pointer newPart,
    const QString& viewId, const QString& refId)
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
//  LayoutPart::Pointer refPart = this->GetRefPart(refId);
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

void PageLayout::StackPlaceholder(const QString& viewId,
    const QString& refId)
{
  if (this->CheckPartInLayout(viewId))
  {
    return;
  }

  // Create the placeholder.
  PartPlaceholder::Pointer newPart(new PartPlaceholder(viewId));

  LayoutPart::Pointer refPart = this->GetRefPart(refId);
  if (refPart != 0)
  {
    newPart->SetContainer(refPart->GetContainer());
  }

  this->StackPart(newPart, viewId, refId);
}

void PageLayout::StackView(const QString& viewId, const QString& refId)
{
  if (this->CheckPartInLayout(viewId))
  {
    return;
  }

  // Create the new part.
  try
  {
    LayoutPart::Pointer newPart = this->CreateView(viewId);
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

void PageLayout::AddStandaloneView(const QString& viewId, bool showTitle,
    int relationship, float ratio, const QString& refId)
{
  this->AddView(viewId, relationship, ratio, refId, false, true, showTitle);
  ViewLayoutRec::Pointer rec = this->GetViewLayoutRec(viewId, true);
  rec->isStandalone = true;
  rec->showTitle = showTitle;
}

void PageLayout::AddStandaloneViewPlaceholder(const QString& viewId,
    int relationship, float ratio, const QString& refId, bool showTitle)
{

  QString stackId = viewId + ".standalonefolder"; //$NON-NLS-1$

  // Check to see if the view is already in the layout
  if (!this->CheckValidPlaceholderId(viewId))
  {
    return;
  }

  // Create the folder.
  ContainerPlaceholder::Pointer folder(new ContainerPlaceholder(""));
  folder->SetContainer(rootLayoutContainer);
  int appearance = PresentationFactoryUtil::ROLE_STANDALONE;
  if (!showTitle)
  {
    appearance = PresentationFactoryUtil::ROLE_STANDALONE_NOTITLE;
  }
  folder->SetRealContainer(ILayoutContainer::Pointer(new PartStack(rootLayoutContainer->page, true,
      appearance, nullptr)));
  folder->SetID(stackId);
  this->AddPart(folder, stackId, relationship, ratio, refId);

  // Create a wrapper.
  PlaceholderFolderLayout::Pointer placeHolder(new PlaceholderFolderLayout(this, folder));

  // Add the standalone view immediately
  placeHolder->AddPlaceholder(viewId);

  ViewLayoutRec::Pointer rec = this->GetViewLayoutRec(viewId, true);
  rec->isStandalone = true;
  rec->showTitle = showTitle;
}

IViewLayout::Pointer PageLayout::GetViewLayout(const QString& viewId)
{
  ViewLayoutRec::Pointer rec = this->GetViewLayoutRec(viewId, true);
  if (rec == 0)
  {
    return IViewLayout::Pointer(nullptr);
  }
  return IViewLayout::Pointer(new ViewLayout(PageLayout::Pointer(this), rec));
}

QHash<QString, ViewLayoutRec::Pointer> PageLayout::GetIDtoViewLayoutRecMap()
{
  return mapIDtoViewLayoutRec;
}

void PageLayout::RemovePlaceholder(const QString& id)
{
  LayoutPart::Pointer part = this->GetRefPart(id);
  if (part != nullptr && part->IsPlaceHolder())
  {
    ILayoutContainer::Pointer stack = this->GetFolderPart(id);
    if (stack != 0)
    {
      stack->Remove(part);
    }
    else
    {
      //rootLayoutContainer->Remove(part);
      WorkbenchPlugin::Log("Not removing placeholder: Folder for placeholder " + id + " not found");
    }
    mapIDtoPart.remove(id);
    mapIDtoFolder.remove(id);
    mapIDtoViewLayoutRec.remove(id);
  }
}

IPlaceholderFolderLayout::Pointer PageLayout::GetFolderForView(
    const QString& viewId)
{
  if (mapIDtoFolder[viewId] == 0)
    return IPlaceholderFolderLayout::Pointer(nullptr);

  ILayoutContainer::Pointer folder = mapIDtoFolder[viewId];
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
