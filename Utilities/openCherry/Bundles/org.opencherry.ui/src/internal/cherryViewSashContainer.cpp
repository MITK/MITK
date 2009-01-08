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
  //  // Read the info elements.
  //  IMemento[] children = memento.getChildren(IWorkbenchConstants.TAG_INFO);
  //
  //  // Create a part ID to part hashtable.
  //  final Map mapIDtoPart = new HashMap(children.length);
  //
  //  // Loop through the info elements.
  //  for (int i = 0; i < children.length; i++)
  //  {
  //    // Get the info details.
  //    IMemento childMem = children[i];
  //    String partID = childMem.getString(IWorkbenchConstants.TAG_PART);
  //    final String relativeID = childMem
  //    .getString(IWorkbenchConstants.TAG_RELATIVE);
  //    int relationship = 0;
  //    float ratio = 0.0f;
  //    int left = 0, right = 0;
  //    if (relativeID != null)
  //    {
  //      relationship
  //          = childMem.getInteger(IWorkbenchConstants.TAG_RELATIONSHIP).intValue();
  //
  //      // Note: the ratio is used for reading pre-3.0 eclipse workspaces. It should be ignored
  //      // if "left" and "right" are available.
  //      Float ratioFloat = childMem .getFloat(IWorkbenchConstants.TAG_RATIO);
  //      Integer leftInt =
  //          childMem .getInteger(IWorkbenchConstants.TAG_RATIO_LEFT);
  //      Integer rightInt = childMem .getInteger(
  //          IWorkbenchConstants.TAG_RATIO_RIGHT);
  //      if (leftInt != null && rightInt != null)
  //      {
  //        left = leftInt.intValue();
  //        right = rightInt.intValue();
  //      }
  //      else
  //      {
  //        if (ratioFloat != null)
  //        {
  //          ratio = ratioFloat.floatValue();
  //        }
  //      }
  //    }
  //    String strFolder = childMem .getString(IWorkbenchConstants.TAG_FOLDER);
  //
  //    // Create the part.
  //    LayoutPart part = null;
  //    if (strFolder == null)
  //    {
  //      part = new PartPlaceholder(partID);
  //    }
  //    else
  //    {
  //      ViewStack folder = new ViewStack(page);
  //      folder.setID(partID);
  //      result.add(folder.restoreState(childMem .getChild(
  //          IWorkbenchConstants.TAG_FOLDER)));
  //      ContainerPlaceholder placeholder = new ContainerPlaceholder(partID);
  //      placeholder.setRealContainer(folder);
  //      part = placeholder;
  //    }
  //    // 1FUN70C: ITPUI:WIN - Shouldn't set Container when not active
  //    part.setContainer(this);
  //
  //    final int myLeft = left, myRight = right, myRelationship = relationship;
  //    final float myRatio = ratio;
  //    final LayoutPart myPart = part;
  //
  //    StartupThreading.runWithoutExceptions(new StartupRunnable()
  //        {
  //
  //        public void runWithException() throws Throwable
  //          {
  //            // Add the part to the layout
  //            if (relativeID == null)
  //            {
  //              add(myPart);
  //            }
  //            else
  //            {
  //              LayoutPart refPart = (LayoutPart) mapIDtoPart.get(relativeID);
  //              if (refPart != null)
  //              {
  //                if (myLeft != 0)
  //                {
  //                  add(myPart, myRelationship, myLeft, myRight, refPart);
  //                }
  //                else
  //                {
  //                  add(myPart, myRelationship, myRatio, refPart);
  //                }
  //              }
  //              else
  //              {
  //                WorkbenchPlugin
  //                .log("Unable to find part for ID: " + relativeID);//$NON-NLS-1$
  //              }
  //            }
  //          }}
  //        );
  //
  //        mapIDtoPart.put(partID, part);
  //      }
  //      return result;
  return true;
}

bool ViewSashContainer::SaveState(IMemento::Pointer memento)
{
  //TODO ViewSashContainer save state
  //      RelationshipInfo[] relationships = computeRelation();
  //
  //      MultiStatus
  //          result =
  //              new MultiStatus(PlatformUI.PLUGIN_ID, IStatus.OK, WorkbenchMessages.RootLayoutContainer_problemsSavingPerspective, null);
  //
  //      // Loop through the relationship array.
  //      for (int i = 0; i < relationships.length; i++)
  //      {
  //        // Save the relationship info ..
  //        //    private LayoutPart part;
  //        //    private int relationship;
  //        //    private float ratio;
  //        //    private LayoutPart relative;
  //        RelationshipInfo info = relationships[i];
  //        IMemento childMem = memento .createChild(IWorkbenchConstants.TAG_INFO);
  //        childMem.putString(IWorkbenchConstants.TAG_PART, info.part.getID());
  //        if (info.relative != null)
  //        {
  //          childMem.putString(IWorkbenchConstants.TAG_RELATIVE,
  //              info.relative.getID());
  //          childMem.putInteger(IWorkbenchConstants.TAG_RELATIONSHIP,
  //              info.relationship);
  //          childMem.putInteger(IWorkbenchConstants.TAG_RATIO_LEFT, info.left);
  //          childMem.putInteger(IWorkbenchConstants.TAG_RATIO_RIGHT, info.right);
  //
  //          // The ratio is only needed for saving workspaces that can be read by old versions
  //          // of Eclipse. It is not used in newer versions of Eclipse, which use the "left"
  //          // and "right" attributes instead.
  //          childMem.putFloat(IWorkbenchConstants.TAG_RATIO, info .getRatio());
  //        }
  //
  //        // Is this part a folder or a placeholder for one?
  //        ViewStack folder = null;
  //        if (info.part instanceof ViewStack)
  //        {
  //          folder = (ViewStack) info.part;
  //        }
  //        else if (info.part instanceof ContainerPlaceholder)
  //        {
  //          LayoutPart part = ((ContainerPlaceholder) info.part)
  //          .getRealContainer();
  //          if (part instanceof ViewStack)
  //          {
  //            folder = (ViewStack) part;
  //          }
  //        }
  //
  //        // If this is a folder (ViewStack) save the contents.
  //        if (folder != null)
  //        {
  //          childMem.putString(IWorkbenchConstants.TAG_FOLDER, "true");//$NON-NLS-1$
  //
  //          IMemento folderMem = childMem .createChild(
  //              IWorkbenchConstants.TAG_FOLDER);
  //          result.add(folder.saveState(folderMem));
  //        }
  //      }
  //      return result;
  return true;
}

bool ViewSashContainer::IsStackType(IStackableContainer::Pointer toTest)
{
  if (toTest.Cast<PartStack> () == 0)
    return false;

  return (toTest.Cast<PartStack> ()->GetAppearance()
      == PresentationFactoryUtil::ROLE_VIEW);
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
