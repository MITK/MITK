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

#include "cherryViewSashContainer.h"

#include "cherryPerspective.h"
#include "cherryPerspectiveHelper.h"
#include "cherryLayoutTree.h"

#include "cherryWorkbenchConstants.h"
#include "cherryWorkbenchPlugin.h"

#include "../cherryImageDescriptor.h"

#include <Poco/HashMap.h>

namespace cherry
{

ViewSashContainer::ViewSashContainer(WorkbenchPage::Pointer page, void* parent) :
  PartSashContainer("root layout container", page, parent)
{
}

ViewSashContainer::Pointer ViewSashContainer::GetRootContainer()
{
  return ViewSashContainer::Pointer(this);
}

void* ViewSashContainer::GetControl()
{
  return this->parent;
}

bool ViewSashContainer::RestoreState(IMemento::Pointer memento)
{
  //TODO ViewSashContainer restore state
  //  MultiStatus
  //      result =
  //          new MultiStatus(PlatformUI.PLUGIN_ID, IStatus.OK, WorkbenchMessages.RootLayoutContainer_problemsRestoringPerspective, null);
  //
  bool result = true;
  // Read the info elements.
  std::vector<IMemento::Pointer> children(memento->GetChildren(WorkbenchConstants::TAG_INFO));

  // Create a part ID to part hashtable.
  Poco::HashMap<std::string, LayoutPart::Pointer> mapIDtoPart(children.size());

  // Loop through the info elements.
  for (std::size_t i = 0; i < children.size(); i++)
  {
  // Get the info details.
  IMemento::Pointer childMem = children[i];
  std::string partID; childMem->GetString(WorkbenchConstants::TAG_PART, partID);
  std::string relativeID; childMem->GetString(WorkbenchConstants::TAG_RELATIVE, relativeID);
  int relationship = 0;
  int left = 0, right = 0;
  if (!relativeID.empty())
  {
    childMem->GetInteger(WorkbenchConstants::TAG_RELATIONSHIP, relationship);

    childMem->GetInteger(WorkbenchConstants::TAG_RATIO_LEFT, left);
    childMem->GetInteger(WorkbenchConstants::TAG_RATIO_RIGHT, right);
  }
  std::string strFolder; childMem->GetString(WorkbenchConstants::TAG_FOLDER, strFolder);

  // Create the part.
  LayoutPart::Pointer part;

  if (strFolder.empty())
  {
    // this is the editor area
    ContainerPlaceholder::Pointer placeholder(new ContainerPlaceholder(partID));
    part = placeholder;
  }
  else
  {
    PartStack::Pointer folder(new PartStack(page));
    folder->SetID(partID);
    //result.add(folder->RestoreState(childMem->GetChild(WorkbenchConstants::TAG_FOLDER)));
    result &= folder->RestoreState(childMem->GetChild(WorkbenchConstants::TAG_FOLDER));
    ContainerPlaceholder::Pointer placeholder(new ContainerPlaceholder(partID));
    placeholder->SetRealContainer(folder);
    part = placeholder;
  }

  // 1FUN70C: ITPUI:WIN - Shouldn't set Container when not active
  part->SetContainer(ILayoutContainer::Pointer(this));

  const int myLeft = left, myRight = right, myRelationship = relationship;
  LayoutPart::Pointer myPart = part;

//  StartupThreading.runWithoutExceptions(new StartupRunnable()
//    {
//
//    public void runWithException() throws Throwable
//      {
        // Add the part to the layout
        if (relativeID.empty())
        {
          this->Add(myPart);
        }
        else
        {
          LayoutPart::Pointer refPart = mapIDtoPart[relativeID];
          if (refPart)
          {
            this->Add(myPart, myRelationship, myLeft, myRight, refPart);
          }
          else
          {
            WorkbenchPlugin::Log("Unable to find part for ID: " + relativeID);
          }
        }
//      }}
//    );

    mapIDtoPart[partID] = part;
  }
  return result;
}

bool ViewSashContainer::SaveState(IMemento::Pointer memento)
{
  std::vector<RelationshipInfo> relationships = this->ComputeRelation();

  bool result = true;
//  MultiStatus
//      result =
//          new MultiStatus(PlatformUI.PLUGIN_ID, IStatus.OK, WorkbenchMessages.RootLayoutContainer_problemsSavingPerspective, null);

  // Loop through the relationship array.
  for (std::size_t i = 0; i < relationships.size(); ++i)
  {
    // Save the relationship info ..
    //    private LayoutPart part;
    //    private int relationship;
    //    private float ratio;
    //    private LayoutPart relative;
    RelationshipInfo& info = relationships[i];
    IMemento::Pointer childMem = memento->CreateChild(WorkbenchConstants::TAG_INFO);
    childMem->PutString(WorkbenchConstants::TAG_PART, info.part->GetID());
    if (info.relative)
    {
      childMem->PutString(WorkbenchConstants::TAG_RELATIVE,
          info.relative->GetID());
      childMem->PutInteger(WorkbenchConstants::TAG_RELATIONSHIP,
          info.relationship);
      childMem->PutInteger(WorkbenchConstants::TAG_RATIO_LEFT, info.left);
      childMem->PutInteger(WorkbenchConstants::TAG_RATIO_RIGHT, info.right);
    }

    // Is this part a folder or a placeholder for one?
    PartStack::Pointer folder(info.part.Cast<PartStack>());
    if (!folder && info.part.Cast<ContainerPlaceholder>())
    {
      IStackableContainer::Pointer part = info.part.Cast<ContainerPlaceholder>()->GetRealContainer();
      folder = part.Cast<PartStack>();
    }

    // If this is a folder (PartStack) save the contents.
    if (folder)
    {
      childMem->PutString(WorkbenchConstants::TAG_FOLDER, "true");

      IMemento::Pointer folderMem(childMem->CreateChild(WorkbenchConstants::TAG_FOLDER));
      //result.add(folder.saveState(folderMem));
      result = folder->SaveState(folderMem);
    }
  }
  return result;
}

bool ViewSashContainer::IsStackType(IStackableContainer::Pointer toTest)
{
  if (toTest.Cast<PartStack> () == 0)
    return false;

  return (toTest.Cast<PartStack> ()->GetAppearance()
      != PresentationFactoryUtil::ROLE_EDITOR);
}

bool ViewSashContainer::IsPaneType(StackablePart::Pointer toTest)
{
  if (toTest.Cast<PartPane> () == 0)
    return false;
  return (toTest.Cast<PartPane> ()->GetPartReference().Cast<IViewReference> ()
      != 0);
}

bool ViewSashContainer::AllowsAdd(LayoutPart::Pointer layoutPart)
{
  return LayoutPart::AllowsAdd(layoutPart);
}

//    void ViewSashContainer::Replace(StackablePart::Pointer oldChild,
//        StackablePart::Pointer newChild)
//    {
//      if (!this->IsChild(oldChild))
//      {
//        return;
//      }
//
//      // Nasty hack: ensure that all views end up inside a tab folder.
//      // Since the view title is provided by the tab folder, this ensures
//      // that views don't get created without a title tab.
//      if (newChild instanceof ViewPane)
//      {
//        ViewStack folder = new ViewStack(page);
//        folder.add(newChild);
//        newChild = folder;
//      }
//
//      super.replace(oldChild, newChild);
//    }

void* ViewSashContainer::CreateParent(void* parentWidget)
{
  return parentWidget;
}

void ViewSashContainer::DisposeParent()
{
  // do nothing
}

float ViewSashContainer::GetDockingRatio(Object::Pointer dragged,
    IStackableContainer::Pointer target)
{
  if (this->IsStackType(target))
  {
    return PartSashContainer::GetDockingRatio(dragged, target);
  }
  else
  {
    return 0.25f;
  }
}

PartStack::Pointer ViewSashContainer::CreateStack()
{
  PartStack::Pointer result(new PartStack(page));
  return result;
}

void ViewSashContainer::SetVisiblePart(IStackableContainer::Pointer container,
    PartPane::Pointer visiblePart)
{
  if (container.Cast<PartStack> () != 0)
  {
    PartStack::Pointer tabFolder = container.Cast<PartStack> ();

    tabFolder->SetSelection(visiblePart);
  }
}

StackablePart::Pointer ViewSashContainer::GetVisiblePart(
    IStackableContainer::Pointer container)
{
  return container.Cast<PartStack> ()->GetSelection();
}

void ViewSashContainer::DerefPart(StackablePart::Pointer sourcePart)
{
  page->GetActivePerspective()->GetPresentation()->DerefPart(sourcePart);
}

//    void ViewSashContainer::AddChild(const RelationshipInfo& info)
//    {
//      LayoutPart child = info.part;
//
//      // Nasty hack: ensure that all views end up inside a tab folder.
//      // Since the view title is provided by the tab folder, this ensures
//      // that views don't get created without a title tab.
//      if (child instanceof ViewPane)
//      {
//        ViewStack folder = new ViewStack(page);
//        folder.add(child);
//        info.part = folder;
//      }
//
//      super.addChild(info);
//    }

}
