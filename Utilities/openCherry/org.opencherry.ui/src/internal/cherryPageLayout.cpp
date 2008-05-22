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

namespace cherry
{

PageLayout::PageLayout()
 : boolEditorVisible(true)
{
  //no-op
}

PageLayout::PageLayout(ViewSashContainer::Pointer container,
    ViewFactory* viewFactory, LayoutPart::Pointer editorFolder,
    IPerspectiveDescriptor::Pointer descriptor)
 : boolEditorVisible(true)
{
  super();
  this.viewFactory = viewFactory;
  this.rootLayoutContainer = container;
  this.editorFolder = editorFolder;
  this.descriptor = descriptor;
  prefill();
}

void PageLayout::AddEditorArea()
{
  try
  {
    // Create the part.
    LayoutPart newPart = createView(ID_EDITOR_AREA);
    if (newPart == null)
    {
      // this should never happen as long as newID is the editor ID.
      return;
    }

    setRefPart(ID_EDITOR_AREA, newPart);

    // Add it to the layout.
    rootLayoutContainer.add(newPart);
  }
  catch (PartInitException e)
  {
    WorkbenchPlugin.log(getClass(), "addEditorArea()", e); //$NON-NLS-1$
  }
}

ViewLayoutRec PageLayout::GetViewLayoutRec(const std::string& id, bool create)
{
  ViewLayoutRec rec = (ViewLayoutRec) mapIDtoViewLayoutRec.get(id);
  if (rec == null && create)
  {
    rec = new ViewLayoutRec();
    // set up the view layout appropriately if the page layout is fixed
    if (isFixed())
    {
      rec.isCloseable = false;
      rec.isMoveable = false;
    }
    mapIDtoViewLayoutRec.put(id, rec);
  }
  return rec;
}

void PageLayout::AddPart(LayoutPart::Pointer newPart,
    const std::string& partId, int relationship, float ratio,
    const std::string& refId)
{

  setRefPart(partId, newPart);

  // If the referenced part is inside a folder,
  // then use the folder as the reference part.
  LayoutPart refPart = getFolderPart(refId);
  if (refPart == null)
  {
    refPart = getRefPart(refId);
  }

  // Add it to the layout.
  if (refPart != null)
  {
    ratio = normalizeRatio(ratio);
    rootLayoutContainer.add(newPart, getPartSashConst(relationship), ratio,
        refPart);
  }
  else
  {
    WorkbenchPlugin.log(NLS.bind(WorkbenchMessages.PageLayout_missingRefPart,
        refId));
    rootLayoutContainer.add(newPart);
  }
}

void PageLayout::AddPerspectiveShortcut(const std::string& id)
{
  if (!perspectiveShortcuts.contains(id))
  {
    perspectiveShortcuts.add(id);
  }
}

void PageLayout::AddPlaceholder(const std::string& viewId, int relationship,
    float ratio, const std::string& refId)
{
  if (!checkValidPlaceholderId(viewId))
  {
    return;
  }

  // Create the placeholder.
  PartPlaceholder newPart = new PartPlaceholder(viewId);
  addPart(newPart, viewId, relationship, ratio, refId);
  // force creation of the view layout rec
  getViewLayoutRec(viewId, true);
}

bool PageLayout::CheckValidPlaceholderId(const std::string& id)
{
  // Check that view is not already in layout.
  // This check is done even if the id has a wildcard, since it's incorrect to create
  // multiple placeholders with the same id, wildcard or not.
  if (checkPartInLayout(id))
  {
    return false;
  }

  // check that primary view id is valid, but only if it has no wildcard
  String primaryId = ViewFactory.extractPrimaryId(id);
  if (!ViewFactory.hasWildcard(primaryId))
  {
    IViewRegistry reg = WorkbenchPlugin.getDefault().getViewRegistry();
    IViewDescriptor desc = reg.find(primaryId);
    if (desc == null)
    {
      // cannot safely open the dialog so log the problem
      WorkbenchPlugin.log("Unable to find view with id: " + primaryId
          + ", when creating perspective " + getDescriptor().getId()); //$NON-NLS-1$ //$NON-NLS-2$
      return false;
    }
  }

  return true;
}

void PageLayout::AddShowInPart(const std::string& id)
{
  if (!showInPartIds.contains(id))
  {
    showInPartIds.add(id);
  }
}

void PageLayout::AddShowViewShortcut(const std::string& id)
{
  if (!showViewShortcuts.contains(id))
  {
    showViewShortcuts.add(id);
  }
}

void PageLayout::AddView(const std::string& viewId, int relationship,
    float ratio, const std::string& refId)
{
  addView(viewId, relationship, ratio, refId, false, false, true);
}

void PageLayout::AddView(const std::string& viewId, int relationship,
    float ratio, const std::string& refId, bool minimized)
{
  addView(viewId, relationship, ratio, refId, minimized, false, true);
}

void PageLayout::AddView(const std::string& viewId, int relationship,
    float ratio, const std::string& refId, bool minimized, bool standalone,
    bool showTitle)
{
  if (checkPartInLayout(viewId))
  {
    return;
  }

  try
  {
    // Create the part.
    LayoutPart newPart = createView(viewId);
    if (newPart == null)
    {
      addPlaceholder(viewId, relationship, ratio, refId);
      LayoutHelper.addViewActivator(this, viewId);
    }
    else
    {
      int appearance = PresentationFactoryUtil.ROLE_VIEW;
      if (standalone)
      {
        if (showTitle)
        {
          appearance = PresentationFactoryUtil.ROLE_STANDALONE;
        }
        else
        {
          appearance = PresentationFactoryUtil.ROLE_STANDALONE_NOTITLE;
        }
      }

      ViewStack newFolder = new ViewStack(rootLayoutContainer.page,
          true, appearance, null);
      newFolder.add(newPart);
      setFolderPart(viewId, newFolder);
      addPart(newFolder, viewId, relationship, ratio, refId);
      // force creation of the view layout rec
      getViewLayoutRec(viewId, true);

      // Capture any minimized stacks
      if (minimized)
      {
        // Remember the minimized stacks so we can
        // move them to the trim when the Perspective
        // activates...
        minimizedStacks.add(newFolder);
      }
    }
  }
  catch (PartInitException e)
  {
    WorkbenchPlugin.log(getClass(), "addView", e); //$NON-NLS-1$
  }
}

//      List getMinimizedStacks() {
//      return minimizedStacks;
//    }

bool PageLayout::CheckPartInLayout(const std::string& partId)
{
  if (getRefPart(partId) != null || isFastViewId(partId))
  {
    WorkbenchPlugin.log(NLS.bind(WorkbenchMessages.PageLayout_duplicateRefPart,
        partId));
    return true;
  }

  return false;
}

IFolderLayout::Pointer PageLayout::CreateFolder(const std::string& folderId,
    int relationship, float ratio, const std::string& refId)
{
  if (checkPartInLayout(folderId))
  {
    ViewStack folder = (ViewStack) getRefPart(folderId);

    return (IFolderLayout) mapFolderToFolderLayout.get(folder);
  }

  // Create the folder.
  ViewStack folder = new ViewStack(rootLayoutContainer.page);
  folder.setID(folderId);
  addPart(folder, folderId, relationship, ratio, refId);

  // Create a wrapper.
  FolderLayout layout = new FolderLayout(this, folder, viewFactory);

  mapFolderToFolderLayout.put(folder, layout);

  return layout;
}

IPlaceholderFolderLayout::Pointer PageLayout::CreatePlaceholderFolder(
    const std::string& folderId, int relationship, float ratio, String refId)
{
  if (checkPartInLayout(folderId))
  {
    ContainerPlaceholder folder = (ContainerPlaceholder) getRefPart(folderId);

    return (IPlaceholderFolderLayout) mapFolderToFolderLayout.get(folder);
  }

  // Create the folder.
  ContainerPlaceholder folder = new ContainerPlaceholder(null);
  folder.setContainer(rootLayoutContainer);
  folder.setRealContainer(new ViewStack(rootLayoutContainer.page));
  folder.setID(folderId);
  addPart(folder, folderId, relationship, ratio, refId);

  // Create a wrapper.
  IPlaceholderFolderLayout layout = new PlaceholderFolderLayout(this, folder);

  mapFolderToFolderLayout.put(folder, layout);

  return layout;
}

LayoutPart::Pointer PageLayout::CreateView(const std::string& partID)
{
  if (partID.equals(ID_EDITOR_AREA))
  {
    return editorFolder;
  }
  IViewDescriptor viewDescriptor = viewFactory.getViewRegistry()
  .find(ViewFactory.extractPrimaryId(partID));
  if (WorkbenchActivityHelper.filterItem(viewDescriptor))
  {
    return null;
  }
  return LayoutHelper.createView(getViewFactory(), partID);
}

IPerspectiveDescriptor::Pointer PageLayout::GetDescriptor()
{
  return descriptor;
}

std::string PageLayout::GetEditorArea()
{
  return ID_EDITOR_AREA;
}

PartStack::Pointer PageLayout::GetFolderPart(const std::string& viewId)
{
  return (ViewStack) mapIDtoFolder.get(viewId);
}

int PageLayout::GetPartSashConst(int nRelationship)
{
  return nRelationship;
}

std::vector<std::string> PageLayout::GetPerspectiveShortcuts()
{
  return perspectiveShortcuts;
}

LayoutPart::Pointer PageLayout::GetRefPart(const std::string& partID)
{
  return (LayoutPart) mapIDtoPart.get(partID);
}

ViewSashContainer::Pointer PageLayout::GetRootLayoutContainer()
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
  addEditorArea();

  // Add default action sets.
  ActionSetRegistry reg = WorkbenchPlugin.getDefault()
  .getActionSetRegistry();
  IActionSetDescriptor[] array = reg.getActionSets();
  int count = array.length;
  for (int nX = 0; nX < count; nX++)
  {
    IActionSetDescriptor desc = array[nX];
    if (desc.isInitiallyVisible())
    {
      addActionSet(desc.getId());
    }
  }
}

void PageLayout::SetEditorAreaVisible(bool showEditorArea)
{
  editorVisible = showEditorArea;
}

void PageLayout::SetFixed(bool fixed)
{
  this.fixed = fixed;
}

bool PageLayout::IsFixed()
{
  return fixed;
}

void PageLayout::SetFolderPart(const std::string& viewId,
    ContainerPlaceholder::Pointer container)
{
  LayoutPart tabFolder = container.getRealContainer();
  mapIDtoFolder.put(viewId, tabFolder);
}

void PageLayout::SetFolderPart(const std::string& viewId,
    PartStack::Pointer folder)
{
  mapIDtoFolder.put(viewId, folder);
}

void PageLayout::SetRefPart(const std::string& partID, LayoutPart::Pointer part)
{
  mapIDtoPart.put(partID, part);
}

void PageLayout::StackPart(LayoutPart::Pointer newPart,
    const std::string& viewId, const std::string& refId)
{
  setRefPart(viewId, newPart);
  // force creation of the view layout rec
  getViewLayoutRec(viewId, true);

  // If ref part is in a folder than just add the
  // new view to that folder.
  ViewStack folder = getFolderPart(refId);
  if (folder != null)
  {
    folder.add(newPart);
    setFolderPart(viewId, folder);
    return;
  }

  // If the ref part is in the page layout then create
  // a new folder and add the new view.
  LayoutPart refPart = getRefPart(refId);
  if (refPart != null && (refPart instanceof PartPane || refPart instanceof PartPlaceholder)
) {
            ViewStack newFolder = new ViewStack(rootLayoutContainer.page);
            rootLayoutContainer.replace(refPart, newFolder);
            newFolder.add(refPart);
            newFolder.add(newPart);
            setFolderPart(refId, newFolder);
            setFolderPart(viewId, newFolder);
            return;
        }

        // If ref part is not found then just do add.
            WorkbenchPlugin.log(NLS.bind(WorkbenchMessages.PageLayout_missingRefPart,
        refId));
  rootLayoutContainer.add(newPart);
}

void PageLayout::StackPlaceholder(const std::string& viewId,
    const std::string& refId)
{
  if (checkPartInLayout(viewId))
  {
    return;
  }

  // Create the placeholder.
  PartPlaceholder newPart = new PartPlaceholder(viewId);

  LayoutPart refPart = getRefPart(refId);
  if (refPart != null)
  {
    newPart.setContainer(refPart.getContainer());
  }

  stackPart(newPart, viewId, refId);
}

void PageLayout::StackView(const std::string& viewId, const std::string& refId)
{
  if (checkPartInLayout(viewId))
  {
    return;
  }

  // Create the new part.
  try
  {
    LayoutPart newPart = createView(viewId);
    if (newPart == null)
    {
      stackPlaceholder(viewId, refId);
      LayoutHelper.addViewActivator(this, viewId);
    }
    else
    {
      stackPart(newPart, viewId, refId);
    }
  }
  catch (PartInitException e)
  {
    WorkbenchPlugin.log(getClass(), "stackView", e); //$NON-NLS-1$
  }
}

void PageLayout::AddStandaloneView(const std::string& viewId, bool showTitle,
    int relationship, float ratio, const std::string& refId)
{
  addView(viewId, relationship, ratio, refId, false, true, showTitle);
  ViewLayoutRec rec = getViewLayoutRec(viewId, true);
  rec.isStandalone = true;
  rec.showTitle = showTitle;
}

void PageLayout::AddStandaloneViewPlaceholder(const std::string& viewId,
    int relationship, float ratio, const std::string& refId, bool showTitle)
{

  String stackId = viewId + ".standalonefolder"; //$NON-NLS-1$

  // Check to see if the view is already in the layout
  if (!checkValidPlaceholderId(viewId))
  {
    return;
  }

  // Create the folder.
  ContainerPlaceholder folder = new ContainerPlaceholder(null);
  folder.setContainer(rootLayoutContainer);
  int appearance = PresentationFactoryUtil.ROLE_STANDALONE;
  if (!showTitle)
  {
    appearance = PresentationFactoryUtil.ROLE_STANDALONE_NOTITLE;
  }
  folder.setRealContainer(new ViewStack(rootLayoutContainer.page, true,
      appearance, null));
  folder.setID(stackId);
  addPart(folder, stackId, relationship, ratio, refId);

  // Create a wrapper.
  PlaceholderFolderLayout placeHolder = new PlaceholderFolderLayout(this,
      folder);

  // Add the standalone view immediately
  placeHolder.addPlaceholder(viewId);

  ViewLayoutRec rec = getViewLayoutRec(viewId, true);
  rec.isStandalone = true;
  rec.showTitle = showTitle;
}

IViewLayout::Pointer PageLayout::GetViewLayout(const std::string& viewId)
{
  ViewLayoutRec rec = getViewLayoutRec(viewId, true);
  if (rec == null)
  {
    return null;
  }
  return new ViewLayout(this, rec);
}

std::map<std::string, ViewLayoutRec> PageLayout::GetIDtoViewLayoutRecMap()
{
  return mapIDtoViewLayoutRec;
}

void PageLayout::RemovePlaceholder(const std::string& id)
{
  LayoutPart part = getRefPart(id);
  if (part instanceof PartPlaceholder)
  {
    ViewStack stack = getFolderPart(id);
    if (stack != null)
    {
      stack.remove(part);
    }
    else
    {
      rootLayoutContainer.remove(part);
    }
    mapIDtoPart.remove(id);
    mapIDtoFolder.remove(id);
    mapIDtoViewLayoutRec.remove(id);
  }
}

IPlaceholderFolderLayout::Pointer PageLayout::GetFolderForView(
    const std::string& viewId)
{
  if (!mapIDtoFolder.containsKey(viewId))
    return null;

  ViewStack folder = (ViewStack) mapIDtoFolder.get(viewId);
  IPlaceholderFolderLayout layout = null;
  if (!mapFolderToFolderLayout.containsKey(folder))
  {
    layout = new FolderLayout(this, folder, viewFactory);
    mapFolderToFolderLayout.put(folder, layout);
  }
  else
  {
    layout = (IPlaceholderFolderLayout)mapFolderToFolderLayout.get(folder);
  }
  return layout;
}

}
