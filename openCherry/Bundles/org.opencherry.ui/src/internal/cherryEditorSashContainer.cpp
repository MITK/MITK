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

#include "cherryEditorSashContainer.h"

#include "cherryPresentationSerializer.h"

#include "cherryLayoutTree.h"

#include "../tweaklets/cherryGuiWidgetsTweaklet.h"

#include <sstream>

namespace cherry
{

const std::string EditorSashContainer::DEFAULT_WORKBOOK_ID =
    "DefaultEditorWorkbook";

void EditorSashContainer::AddChild(const RelationshipInfo& info)
{
  PartSashContainer::AddChild(info);

  this->UpdateStackButtons();
}

void EditorSashContainer::ChildAdded(LayoutPart::Pointer child)
{
  PartSashContainer::ChildAdded(child);

  if (child.Cast<PartStack> () != 0)
  {
    editorWorkbooks.push_back(child.Cast<PartStack> ());
  }
}

void EditorSashContainer::ChildRemoved(LayoutPart::Pointer child)
{
  PartSashContainer::ChildRemoved(child);

  if (child.Cast<PartStack> () != 0)
  {
    editorWorkbooks.remove(child.Cast<PartStack>());
    if (activeEditorWorkbook == child)
    {
      this->SetActiveWorkbook(PartStack::Pointer(0), false);
    }

    this->UpdateStackButtons();
  }
}

PartStack::Pointer EditorSashContainer::CreateDefaultWorkbook()
{
  PartStack::Pointer newWorkbook = this->NewEditorWorkbook();
  newWorkbook->SetID(DEFAULT_WORKBOOK_ID);
  this->Add(newWorkbook);
  return newWorkbook;
}

PartStack::Pointer EditorSashContainer::NewEditorWorkbook()
{
  PartStack::Pointer newWorkbook(new PartStack(page, true, PresentationFactoryUtil::ROLE_EDITOR));
  std::stringstream buf;
  buf << newWorkbook->GetClassName() << newWorkbook.GetPointer();
  newWorkbook->SetID(buf.str());

  return newWorkbook;
}

void* EditorSashContainer::CreateParent(void* parentWidget)
{
  return Tweaklets::Get(GuiWidgetsTweaklet::KEY)->CreateComposite(parentWidget);
}

void EditorSashContainer::DisposeParent()
{
  this->parent = 0;
}

bool EditorSashContainer::IsActiveWorkbook(PartStack::Pointer workbook)
{
  return activeEditorWorkbook == workbook;
}

PartStack::Pointer EditorSashContainer::CreateStack()
{
  return this->NewEditorWorkbook();
}

void EditorSashContainer::SetVisiblePart(
    IStackableContainer::Pointer container, PartPane::Pointer visiblePart)
{
  PartStack::Pointer stack = container.Cast<PartStack>();
  if (stack == 0) return;

  stack->GetContainer().Cast<EditorSashContainer>()->SetActiveWorkbook(stack, true);

  stack->SetSelection(visiblePart);
}

StackablePart::Pointer EditorSashContainer::GetVisiblePart(
    IStackableContainer::Pointer container)
{
  PartStack::Pointer refPart = container.Cast<PartStack>();

  return refPart->GetSelection();
}

EditorSashContainer::EditorSashContainer(const std::string& editorId,
    WorkbenchPage::Pointer page, void* parent)
 : PartSashContainer(editorId, page, parent)
{
  this->CreateDefaultWorkbook();
}

bool EditorSashContainer::AllowsAdd(LayoutPart::Pointer layoutPart)
{
  return LayoutPart::AllowsAdd(layoutPart);
}

void EditorSashContainer::AddEditor(PartPane::Pointer pane,
    PartStack::Pointer stack)
{
  //EditorStack workbook = getActiveWorkbook();
  stack->Add(pane);
}

void EditorSashContainer::UpdateStackButtons()
{
  //  // This is applicable only when the new
  //  // min/max behaviour is being used
  //  Perspective persp = getPage().getActivePerspective();
  //  if (!Perspective.useNewMinMax(persp))
  //    return;
  //
  //  // Find the upper Right editor stack
  //  LayoutPart[] stacks = getChildren();
  //  EditorStack winner = getUpperRightEditorStack(stacks);
  //
  //  // Now hide the buttons for all but the upper right stack
  //  for (int i = 0; i < stacks.length; i++)
  //  {
  //    if (!(stacks[i] instanceof EditorStack)
  //      )
  //      continue;
  //      ((EditorStack) stacks[i]).showMinMax(stacks[i] == winner);
  //  }
  //
  //  // Force the stack's presentation state to match its perspective
  //  persp.refreshEditorAreaVisibility();
}

PartStack::Pointer EditorSashContainer::GetUpperRightEditorStack()
{
  return this->GetUpperRightEditorStack(this->GetChildren());
}

PartStack::Pointer EditorSashContainer::GetUpperRightEditorStack(
    const ILayoutContainer::ChildrenType& stacks)
{

  // Find the upper Right editor stack
  PartStack::Pointer winner;
  Rectangle winnerRect;

  for (ILayoutContainer::ChildrenType::const_iterator iter = stacks.begin();
       iter != stacks.end(); ++iter)
  {
    LayoutPart::Pointer part = *iter;
    if (part.Cast<PartStack>() == 0)
      continue;

    PartStack::Pointer stack = part.Cast<PartStack>();
    Rectangle bb = stack->GetBounds();
    if (iter == stacks.begin() || bb.y < winnerRect.y || (bb.y == winnerRect.y && bb.x
        > winnerRect.x))
    {
      winner = stack;
      winnerRect = bb;
    }
  }

  return winner;
}

PartStack::Pointer EditorSashContainer::GetActiveWorkbook()
{
  if (activeEditorWorkbook == 0)
  {
    if (editorWorkbooks.size() < 1)
    {
      this->SetActiveWorkbook(this->CreateDefaultWorkbook(), false);
    }
    else
    {
      this->SetActiveWorkbook(editorWorkbooks.front(), false);
    }
  }

  return activeEditorWorkbook;
}

std::string EditorSashContainer::GetActiveWorkbookID()
{
  return this->GetActiveWorkbook()->GetID();
}

std::list<PartStack::Pointer> EditorSashContainer::GetEditorWorkbooks()
{
  return editorWorkbooks;
}

int EditorSashContainer::GetEditorWorkbookCount()
{
  return editorWorkbooks.size();
}

void EditorSashContainer::FindSashes(LayoutPart::Pointer pane,
    PartPane::Sashes& sashes)
{
  //Find the sashes around the current editor and
  //then the sashes around the editor area.
  PartSashContainer::FindSashes(pane, sashes);

  ILayoutContainer::Pointer container = this->GetContainer();
  if (container != 0)
  {
    container->FindSashes(LayoutPart::Pointer(this), sashes);
  }
}

void EditorSashContainer::RemoveAllEditors()
{
  PartStack::Pointer currentWorkbook = this->GetActiveWorkbook();

  // Iterate over a copy so the original can be modified.
  std::list<PartStack::Pointer> workbooks(editorWorkbooks);
  for (std::list<PartStack::Pointer>::iterator iter = workbooks.begin();
       iter != workbooks.end(); ++iter)
  {
    PartStack::Pointer workbook = *iter;
    std::list<StackablePart::Pointer> children = workbook->GetChildren();
    for (std::list<StackablePart::Pointer>::iterator childIter = children.begin();
         childIter != children.end(); ++childIter)
    {
      workbook->Remove(*childIter);
    }

    if (workbook != currentWorkbook)
    {
      this->Remove(workbook);
      workbook->Dispose();
    }
  }
}

void EditorSashContainer::RemoveEditor(PartPane::Pointer pane)
{
  PartStack::Pointer workbook = pane->GetContainer().Cast<PartStack>();
  if (workbook == 0)
  {
    return;
  }

  workbook->Remove(pane);

  // remove the editor workbook if empty
  if (workbook->GetItemCount() < 1 /* && editorWorkbooks.size() > 1*/)
  {
    //    // If the user closes the last editor and the editor area
    //    // is maximized, restore it
    //    Perspective persp = getPage().getActivePerspective();
    //    if (Perspective.useNewMinMax(persp))
    //    {
    //      if (persp.getPresentation().getMaximizedStack() instanceof EditorStack)
    //      persp.getPresentation().getMaximizedStack().
    //      setState(IStackPresentationSite.STATE_RESTORED);
    //    }

    this->Remove(workbook);
    workbook->Dispose();
  }
}

bool EditorSashContainer::RestoreState(IMemento::Pointer memento)
{
  //TODO EditorSashContainer restore state
  //  MultiStatus
  //      result =
  //          new MultiStatus(PlatformUI.PLUGIN_ID, IStatus.OK, WorkbenchMessages.RootLayoutContainer_problemsRestoringPerspective, 0);
  //
  //  // Remove the default editor workbook that is
  //  // initialy created with the editor area.
  //  if (children != 0)
  //  {
  //    StartupThreading.runWithoutExceptions(new StartupRunnable()
  //        {
  //
  //        public void runWithException() throws Throwable
  //          {
  //            EditorStack defaultWorkbook = 0;
  //            for (int i = 0; i < children.size(); i++)
  //            {
  //              LayoutPart child = (LayoutPart) children.get(i);
  //              if (child.getID() == DEFAULT_WORKBOOK_ID)
  //              {
  //                defaultWorkbook = (EditorStack) child;
  //                if (defaultWorkbook.getItemCount()> 0)
  //                {
  //                  defaultWorkbook = 0;
  //                }
  //              }
  //            }
  //            if (defaultWorkbook != 0)
  //            {
  //              remove(defaultWorkbook);
  //            }
  //          }}
  //        );
  //
  //      }
  //
  //  // Restore the relationship/layout
  //  IMemento[] infos = memento.getChildren(IWorkbenchConstants.TAG_INFO);
  //  final Map mapIDtoPart = new HashMap(infos.length);
  //
  //  for (int i = 0; i < infos.length; i++)
  //  {
  //    // Get the info details.
  //    IMemento childMem = infos[i];
  //    final String partID = childMem.getString(IWorkbenchConstants.TAG_PART);
  //    final String relativeID = childMem
  //    .getString(IWorkbenchConstants.TAG_RELATIVE);
  //    int relationship = 0;
  //    int left = 0, right = 0;
  //    float ratio = 0.5f;
  //    if (relativeID != 0)
  //    {
  //      relationship
  //          = childMem.getInteger(IWorkbenchConstants.TAG_RELATIONSHIP).intValue();
  //      Float ratioFloat = childMem .getFloat(IWorkbenchConstants.TAG_RATIO);
  //      Integer leftInt =
  //          childMem .getInteger(IWorkbenchConstants.TAG_RATIO_LEFT);
  //      Integer rightInt = childMem .getInteger(
  //          IWorkbenchConstants.TAG_RATIO_RIGHT);
  //      if (leftInt != 0 && rightInt != 0)
  //      {
  //        left = leftInt.intValue();
  //        right = rightInt.intValue();
  //      }
  //      else if (ratioFloat != 0)
  //      {
  //        ratio = ratioFloat.floatValue();
  //      }
  //    }
  //
  //    final EditorStack workbook [] = new EditorStack[1];
  //    StartupThreading.runWithoutExceptions(new StartupRunnable()
  //        {
  //
  //        public void runWithException() throws Throwable
  //          {
  //            // Create the part.
  //            workbook[0] = EditorStack.newEditorWorkbook(EditorSashContainer.this, page);
  //            workbook[0].setID(partID);
  //            // 1FUN70C: ITPUI:WIN - Shouldn't set Container when not active
  //            workbook[0].setContainer(EditorSashContainer.this);
  //          }}
  //        );
  //
  //        IMemento workbookMemento = childMem .getChild(
  //            IWorkbenchConstants.TAG_FOLDER);
  //        if (workbookMemento != 0)
  //        {
  //          result.add(workbook[0].restoreState(workbookMemento));
  //        }
  //
  //        final int myLeft = left, myRight = right, myRelationship = relationship;
  //        final float myRatio = ratio;
  //        StartupThreading.runWithoutExceptions(new StartupRunnable()
  //            {
  //
  //            public void runWithException() throws Throwable
  //              {
  //                // Add the part to the layout
  //                if (relativeID == 0)
  //                {
  //                  add(workbook[0]);
  //                }
  //                else
  //                {
  //                  LayoutPart refPart = (LayoutPart) mapIDtoPart.get(relativeID);
  //                  if (refPart != 0)
  //                  {
  //                    //$TODO pass in left and right
  //                    if (myLeft == 0 || myRight == 0)
  //                    {
  //                      add(workbook[0], myRelationship, myRatio, refPart);
  //                    }
  //                    else
  //                    {
  //                      add(workbook[0], myRelationship, myLeft, myRight, refPart);
  //                    }
  //                  }
  //                  else
  //                  {
  //                    WorkbenchPlugin
  //                    .log("Unable to find part for ID: " + relativeID);//$NON-NLS-1$
  //                  }
  //                }
  //              }}
  //            );
  //
  //            mapIDtoPart.put(partID, workbook[0]);
  //          }
  //
  //          return result;
  return true;
}

bool EditorSashContainer::SaveState(IMemento::Pointer memento)
{
  //TODO EditorSashContainer save state
  //          RelationshipInfo[] relationships = computeRelation();
  //          MultiStatus
  //              result =
  //                  new MultiStatus(PlatformUI.PLUGIN_ID, IStatus.OK, WorkbenchMessages.RootLayoutContainer_problemsSavingPerspective, 0);
  //
  //          for (int i = 0; i < relationships.length; i++)
  //          {
  //            // Save the relationship info ..
  //            //    private LayoutPart part;
  //            //    private int relationship;
  //            //    private float ratio;
  //            //    private LayoutPart relative;
  //            RelationshipInfo info = relationships[i];
  //            IMemento childMem = memento .createChild(
  //                IWorkbenchConstants.TAG_INFO);
  //            childMem.putString(IWorkbenchConstants.TAG_PART, info.part.getID());
  //
  //            EditorStack stack = (EditorStack) info.part;
  //            if (stack != 0)
  //            {
  //              IMemento folderMem = childMem .createChild(
  //                  IWorkbenchConstants.TAG_FOLDER);
  //              result.add(stack.saveState(folderMem));
  //            }
  //
  //            if (info.relative != 0)
  //            {
  //              childMem.putString(IWorkbenchConstants.TAG_RELATIVE,
  //                  info.relative.getID());
  //              childMem.putInteger(IWorkbenchConstants.TAG_RELATIONSHIP,
  //                  info.relationship);
  //              childMem.putInteger(IWorkbenchConstants.TAG_RATIO_LEFT, info.left);
  //              childMem.putInteger(IWorkbenchConstants.TAG_RATIO_RIGHT,
  //                  info.right);
  //              // Note: "ratio" is not used in newer versions of Eclipse, which use "left"
  //              // and "right" (above) instead
  //              childMem.putFloat(IWorkbenchConstants.TAG_RATIO, info .getRatio());
  //            }
  //          }
  //
  //          return result;
  return true;
}

void EditorSashContainer::SetActiveWorkbook(PartStack::Pointer newWorkbook,
    bool hasFocus)
{
  if (newWorkbook != 0)
  {
    if (std::find(editorWorkbooks.begin(), editorWorkbooks.end(), newWorkbook) == editorWorkbooks.end())
    {
      return;
    }
  }

  PartStack::Pointer oldWorkbook = activeEditorWorkbook;
  activeEditorWorkbook = newWorkbook;

  if (oldWorkbook != 0 && oldWorkbook != newWorkbook)
  {
    oldWorkbook->SetActive(StackPresentation::AS_INACTIVE);
  }

  if (newWorkbook != 0)
  {
    if (hasFocus)
    {
      newWorkbook->SetActive(StackPresentation::AS_ACTIVE_FOCUS);
    }
    else
    {
      newWorkbook->SetActive(StackPresentation::AS_ACTIVE_NOFOCUS);
    }
  }

  this->UpdateTabList();
}

void EditorSashContainer::SetActiveWorkbookFromID(const std::string& id)
{
  for (std::list<PartStack::Pointer>::iterator iter = editorWorkbooks.begin();
       iter != editorWorkbooks.end(); ++iter)
  {
    PartStack::Pointer workbook = *iter;
    if (workbook->GetID() == id)
    {
      this->SetActiveWorkbook(workbook, false);
    }
  }
}

PartStack::Pointer EditorSashContainer::GetWorkbookFromID(const std::string& id)
{
  for (std::list<PartStack::Pointer>::iterator iter = editorWorkbooks.begin();
       iter != editorWorkbooks.end(); ++iter)
  {
    PartStack::Pointer workbook = *iter;
    if (workbook->GetID() == id)
    {
      return workbook;
    }
  }

  return PartStack::Pointer(0);
}

void EditorSashContainer::UpdateTabList()
{
  void* parent = this->GetParent();
  if (parent != 0)
  { // parent may be 0 on startup
    PartStack::Pointer wb(this->GetActiveWorkbook());
//TODO EditorSashContainer update tab list
//    if (wb == 0)
//    {
//      parent.setTabList(new Control[0]);
//    }
//    else
//    {
//      parent.setTabList(wb.getTabList());
//    }
  }
}

void EditorSashContainer::CreateControl(void* parent)
{
  PartSashContainer::CreateControl(parent);

  //TODO DND
  //let the user drop files/editor input on the editor area
  //this->AddDropSupport();
}

bool EditorSashContainer::IsCompressible()
{
  //Added for bug 19524
  return true;
}

bool EditorSashContainer::IsStackType(IStackableContainer::Pointer toTest)
{
  if (toTest.Cast<PartStack>() == 0)
    return false;

  return (toTest.Cast<PartStack> ()->GetAppearance()
      == PresentationFactoryUtil::ROLE_EDITOR);
}

bool EditorSashContainer::IsPaneType(StackablePart::Pointer toTest)
{
  if (toTest.Cast<PartPane>() == 0)
    return false;

  return (toTest.Cast<PartPane> ()->GetPartReference().Cast<IEditorReference> ()
      != 0);
}

bool EditorSashContainer::RestorePresentationState(IMemento::Pointer areaMem)
{
  std::list<PartStack::Pointer> workbooks = this->GetEditorWorkbooks();
  for (std::list<PartStack::Pointer>::iterator iter = workbooks.begin();
       iter != workbooks.end(); ++iter)
  {
    PartStack::Pointer workbook = *iter;
    IMemento::Pointer memento = workbook->GetSavedPresentationState();
    if (memento == 0)
    {
      continue;
    }
    std::list<IPresentablePart::Pointer> listParts = workbook->GetPresentableParts();
    std::vector<IPresentablePart::Pointer> parts(listParts.begin(), listParts.end());
    PresentationSerializer serializer(parts);
    //StartupThreading.runWithoutExceptions(new StartupRunnable()
    //    {

    // public void runWithException() throws Throwable
    //   {
    workbook->GetPresentation()->RestoreState(&serializer, memento);
    //   }}
    // );

  }
  //return new Status(IStatus.OK, PlatformUI.PLUGIN_ID, 0, "", 0); //$NON-NLS-1$
  return true;
}

}
