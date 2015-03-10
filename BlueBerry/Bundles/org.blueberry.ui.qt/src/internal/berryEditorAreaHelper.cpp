/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "berryEditorAreaHelper.h"

#include "berryEditorSashContainer.h"
#include "berryPartSite.h"
#include "berryIPageLayout.h"
#include "berryPartPane.h"

namespace berry
{

void EditorAreaHelper::CloseEditor(PartPane::Pointer pane)
{
  if (pane != 0)
  {
    editorArea->RemoveEditor(pane);
  }
}

void EditorAreaHelper::AddToLayout(PartPane::Pointer pane,
    PartStack::Pointer stack)
{
  //EditorStack stack = editorArea.getActiveWorkbook();
  pane->SetContainer(stack);

  editorArea->AddEditor(pane, stack);
}

EditorAreaHelper::EditorAreaHelper(WorkbenchPage* page)
{
  this->editorArea
      = new EditorSashContainer(IPageLayout::ID_EDITOR_AREA, page, page->GetClientComposite());

  this->editorArea->CreateControl(page->GetClientComposite());
  this->editorArea->SetActive(true);
}

void EditorAreaHelper::DisplayEditorList()
{
  PartStack::Pointer activeWorkbook = editorArea->GetActiveWorkbook();
  if (activeWorkbook != 0)
  {
    activeWorkbook->ShowPartList();
  }
}

void EditorAreaHelper::CloseEditor(IEditorReference::Pointer ref)
{
  PartPane::Pointer pane = ref.Cast<WorkbenchPartReference>()->GetPane();
  this->CloseEditor(pane);
}

void EditorAreaHelper::CloseEditor(IEditorPart::Pointer part)
{
  PartPane::Pointer pane = part->GetSite().Cast<PartSite>()->GetPane();
  this->CloseEditor(pane);
}

void EditorAreaHelper::DerefPart(LayoutPart::Pointer part)
{

  // Get vital part stats before reparenting.
  ILayoutContainer::Pointer oldContainer = part->GetContainer();

  // Reparent the part back to the main window
  //part.reparent(editorArea.getParent());
  // Update container.
  if (oldContainer == 0)
  {
    return;
  }
  oldContainer->Remove(part);
  QList<LayoutPart::Pointer> children = oldContainer->GetChildren();
  if (children.empty())
  {
    // There are no more children in this container, so get rid of it
    if (oldContainer.Cast<LayoutPart>())
    {
      LayoutPart::Pointer parent = oldContainer.Cast<LayoutPart>();
      ILayoutContainer::Pointer parentContainer = parent->GetContainer();
      if (parentContainer != 0)
      {
        parentContainer->Remove(parent);
        parent->Dispose();
      }
    }
  }
}

EditorAreaHelper::~EditorAreaHelper()
{
  if (editorArea != 0)
  {
    editorArea->SetActive(false);
    editorArea->Dispose();
  }
}

QString EditorAreaHelper::GetActiveEditorWorkbookID()
{
  return editorArea->GetActiveWorkbookID();
}

PartStack::Pointer EditorAreaHelper::GetActiveWorkbook()
{
  return editorArea->GetActiveWorkbook();
}

LayoutPart::Pointer EditorAreaHelper::GetLayoutPart()
{
  LayoutPart::Pointer layoutPart = editorArea.Cast<LayoutPart>();
  return layoutPart;
}

IEditorReference::Pointer EditorAreaHelper::GetVisibleEditor()
{
  PartStack::Pointer activeWorkbook = editorArea->GetActiveWorkbook();
  PartPane::Pointer pane = activeWorkbook->GetSelection().Cast<PartPane>();
  if (pane != 0)
  {
    IEditorReference::Pointer result = pane->GetPartReference().Cast<IEditorReference>();
    return result;
  }
  return IEditorReference::Pointer(0);
}

void EditorAreaHelper::MoveEditor(IEditorPart::Pointer  /*part*/, int  /*position*/)
{

  ///*EditorPane pane = (EditorPane)*/((EditorSite) part.getSite()).getPane();
  //TODO commented this out during presentations works
  //pane.getWorkbook().reorderTab(pane, position);
}

void EditorAreaHelper::AddEditor(EditorReference::Pointer ref,
    const QString& workbookId)
{
  QList<IEditorReference::Pointer> refs = editorArea->GetPage()->GetEditorReferences();
  for (QList<IEditorReference::Pointer>::iterator iter = refs.begin();
       iter != refs.end(); ++iter)
  {
    if (ref == (*iter))
    {
      return;
    }
  }

  PartStack::Pointer stack = this->GetWorkbookFromID(workbookId);

  if (stack == 0)
  {
    stack = this->GetActiveWorkbook();
  }

  this->AddToLayout(ref->GetPane(), stack);


  editorArea->GetPage()->PartAdded(ref);
}

bool EditorAreaHelper::RestoreState(IMemento::Pointer memento)
{
  // Restore the editor area workbooks layout/relationship
  return editorArea->RestoreState(memento);
}

bool EditorAreaHelper::RestorePresentationState(IMemento::Pointer areaMem)
{
  return editorArea->RestorePresentationState(areaMem);
}

bool EditorAreaHelper::SaveState(IMemento::Pointer memento)
{
  // Save the editor area workbooks layout/relationship
  return editorArea->SaveState(memento);
}

void EditorAreaHelper::SetActiveEditorWorkbookFromID(const QString& id)
{
  editorArea->SetActiveWorkbookFromID(id);
}

void EditorAreaHelper::SetActiveWorkbook(PartStack::Pointer workbook, bool hasFocus)
{
  editorArea->SetActiveWorkbook(workbook, hasFocus);
}

bool EditorAreaHelper::SetVisibleEditor(IEditorReference::Pointer ref, bool setFocus)
{
  IEditorReference::Pointer visibleEditor = this->GetVisibleEditor();
  if (ref != visibleEditor)
  {
    IWorkbenchPart::Pointer part = ref->GetPart(true);
    PartPane::Pointer pane;
    if (part != 0)
    {
      pane = part->GetSite().Cast<PartSite>()->GetPane();
    }
    if (pane != 0)
    {
      pane->GetContainer().Cast<PartStack>()->SetSelection(pane);

      if (setFocus)
      {
        part->SetFocus();
      }
      return true;
    }
  }
  return false;
}

QList<PartStack::Pointer> EditorAreaHelper::GetWorkbooks()
{
  return editorArea->GetEditorWorkbooks();
}

QList<IEditorReference::Pointer> EditorAreaHelper::GetEditors()
{
  QList<IEditorReference::Pointer> result;
  QList<PartStack::Pointer> workbooks = editorArea->GetEditorWorkbooks();

  for (QList<PartStack::Pointer>::iterator iter = workbooks.begin();
       iter != workbooks.end(); ++iter)
  {
    PartStack::Pointer stack = *iter;

    QList<LayoutPart::Pointer> children = stack->GetChildren();

    for (QList<LayoutPart::Pointer>::iterator childIter = children.begin();
        childIter != children.end(); ++childIter)
    {
      LayoutPart::Pointer part = *childIter;

      result.push_back(part.Cast<PartPane>()->GetPartReference().Cast<IEditorReference>());
    }
  }

  return result;
}

PartStack::Pointer EditorAreaHelper::GetWorkbookFromID(const QString& workbookId)
{
  return editorArea->GetWorkbookFromID(workbookId);
}

void EditorAreaHelper::UpdateStackButtons()
{
  editorArea->UpdateStackButtons();
}

}
