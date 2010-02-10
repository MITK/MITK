/*=========================================================================

 Program:   BlueBerry Platform
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

#include "berryDetachedWindow.h"

#include "../berryIWorkbenchPartConstants.h"
#include "../berryISaveablePart.h"

#include "../berryWorkbenchWindow.h"

#include "../tweaklets/berryGuiWidgetsTweaklet.h"

#include "berryWorkbenchConstants.h"
#include "berryEditorManager.h"
#include "berryDragUtil.h"

namespace berry
{

DetachedWindow::ShellListener::ShellListener(DetachedWindow* wnd) :
  window(wnd)
{

}

void DetachedWindow::ShellListener::ShellClosed(ShellEvent::Pointer e)
{
  // hold on to a reference of the DetachedWindow instance
  // (otherwise, wnd->HandleClose() woulde delete the DetachedWindow
  // instance too early, trying to write to members afterwards)
  DetachedWindow::Pointer wnd(window);

  // only continue to close if the handleClose
  // wasn't canceled
  e->doit = wnd->HandleClose();
}

DetachedWindow::ShellControlListener::ShellControlListener(DetachedWindow* wnd) :
  window(wnd)
{

}

GuiTk::IControlListener::Events::Types DetachedWindow::ShellControlListener::GetEventTypes() const
{
  return Events::RESIZED;
}

void DetachedWindow::ShellControlListener::ControlResized(
    GuiTk::ControlEvent::Pointer e)
{
  window->folder->SetBounds(
      Tweaklets::Get(GuiWidgetsTweaklet::KEY)->GetClientArea(e->item));
}

DetachedWindow::DetachedWindow(WorkbenchPage* workbenchPage)
{
  shellListener = new ShellListener(this);
  resizeListener = new ShellControlListener(this);

  this->page = workbenchPage;
  hideViewsOnClose = true;

  folder = new PartStack(page, false);
}

void DetachedWindow::PropertyChange(Object::Pointer /*source*/, int propId)
{
  if (propId == IWorkbenchPartConstants::PROP_TITLE)
  {
    this->UpdateTitle();
  }
  else if (propId == PartStack::PROP_SELECTION)
  {
    this->ActivePartChanged(this->GetPartReference(folder->GetSelection()));
  }
}

Shell::Pointer DetachedWindow::GetShell()
{
  return windowShell;
}

void DetachedWindow::Create()
{
  folder->AddListener(IPropertyChangeListener::Pointer(this));

  windowShell
      = page->GetWorkbenchWindow().Cast<WorkbenchWindow> () ->GetDetachedWindowPool()->AllocateShell(
          shellListener);
  windowShell->SetData(Object::Pointer(this));
  windowShell->SetText(""); //$NON-NLS-1$

  DragUtil::AddDragTarget(windowShell->GetControl(),
      IDragOverListener::Pointer(this));
  hideViewsOnClose = true;
  if (bounds.IsEmpty())
  {
    Rectangle windowRect = page->GetWorkbenchWindow()->GetShell()->GetBounds();
    Point center(windowRect.x + windowRect.width / 2, windowRect.y
        - windowRect.height / 2);
    bounds = Rectangle(center.x - 150, center.y + 100, 300, 200);
  }

  // Force the rect into the current display
  Rectangle dispBounds =
      Tweaklets::Get(GuiWidgetsTweaklet::KEY)->GetAvailableScreenSize();
  if (bounds.width > dispBounds.width)
    bounds.width = dispBounds.width;
  if (bounds.height > dispBounds.height)
    bounds.height = dispBounds.height;
  if (bounds.x + bounds.width > dispBounds.width)
    bounds.x = dispBounds.width - bounds.width;
  if (bounds.y + bounds.height > dispBounds.height)
    bounds.y = dispBounds.height - bounds.height;

  this->GetShell()->SetBounds(bounds);

  this->ConfigureShell(windowShell);

  this->CreateContents(windowShell->GetControl());
  //windowShell->Layout(true);
  //folder->SetBounds(Tweaklets::Get(GuiWidgetsTweaklet::KEY)->GetClientArea(windowShell->GetControl()));
}

void DetachedWindow::Add(StackablePart::Pointer part)
{

  Shell::Pointer shell = this->GetShell();
  if (shell != 0)
  {
    part->Reparent(shell->GetControl());
  }
  folder->Add(part);
  this->UpdateMinimumSize();
}

bool DetachedWindow::BelongsToWorkbenchPage(
    IWorkbenchPage::Pointer workbenchPage)
{
  return (workbenchPage == this->page);
}

bool DetachedWindow::Close()
{
  hideViewsOnClose = false;
  Shell::Pointer shell = this->GetShell();
  if (shell != 0)
  {
    shell->Close();
  }
  return true;
}

IDropTarget::Pointer DetachedWindow::Drag(void* /*currentControl*/,
    Object::Pointer draggedObject, const Point& position, const Rectangle& /*dragRectangle*/)
{

  if (draggedObject.Cast<PartPane> () == 0)
  {
    return IDropTarget::Pointer(0);
  }

  PartPane::Pointer sourcePart = draggedObject.Cast<PartPane> ();

  if (sourcePart->GetWorkbenchWindow() != page->GetWorkbenchWindow())
  {
    return IDropTarget::Pointer(0);
  }

  // Only handle the event if the source part is acceptable to the particular PartStack
  IDropTarget::Pointer target;
  if (folder->AllowsDrop(sourcePart))
  {
    target = folder->GetDropTarget(draggedObject, position);

    if (target == 0)
    {
      Rectangle displayBounds =
          DragUtil::GetDisplayBounds(folder->GetControl());
      if (displayBounds.Contains(position))
      {
        StackDropResult::Pointer stackDropResult(new StackDropResult(
            displayBounds, Object::Pointer(0)));
        target = folder->CreateDropTarget(sourcePart, stackDropResult);
      }
      else
      {
        return IDropTarget::Pointer(0);
      }
    }
  }

  return target;
}

IStackableContainer::ChildrenType DetachedWindow::GetChildren() const
{
  return folder->GetChildren();
}

WorkbenchPage::Pointer DetachedWindow::GetWorkbenchPage()
{
  return WorkbenchPage::Pointer(this->page);
}

void DetachedWindow::RestoreState(IMemento::Pointer memento)
{
  // Read the bounds.
  int x = 0;
  memento->GetInteger(WorkbenchConstants::TAG_X, x);
  int y = 0;
  memento->GetInteger(WorkbenchConstants::TAG_Y, y);
  int width = 0;
  memento->GetInteger(WorkbenchConstants::TAG_WIDTH, width);
  int height = 0;
  memento->GetInteger(WorkbenchConstants::TAG_HEIGHT, height);

  // memento->GetInteger(WorkbenchConstants::TAG_FLOAT);

  // Set the bounds.
  bounds = Rectangle(x, y, width, height);
  if (GetShell())
  {
    GetShell()->SetBounds(bounds);
  }

  // Create the folder.
  IMemento::Pointer childMem =
      memento->GetChild(WorkbenchConstants::TAG_FOLDER);
  if (childMem)
  {
    folder->RestoreState(childMem);
  }
}

void DetachedWindow::SaveState(IMemento::Pointer memento)
{
  if (GetShell())
  {
    bounds = GetShell()->GetBounds();
  }

  // Save the bounds.
  memento->PutInteger(WorkbenchConstants::TAG_X, bounds.x);
  memento->PutInteger(WorkbenchConstants::TAG_Y, bounds.y);
  memento->PutInteger(WorkbenchConstants::TAG_WIDTH, bounds.width);
  memento->PutInteger(WorkbenchConstants::TAG_HEIGHT, bounds.height);

  // Save the views.
  IMemento::Pointer childMem = memento->CreateChild(
      WorkbenchConstants::TAG_FOLDER);
  folder->SaveState(childMem);
}

void* DetachedWindow::GetControl()
{
  return folder->GetControl();
}

int DetachedWindow::Open()
{

  if (this->GetShell() == 0)
  {
    this->Create();
  }

  Rectangle bounds = this->GetShell()->GetBounds();

  if (!(bounds == this->GetShell()->GetBounds()))
  {
    this->GetShell()->SetBounds(bounds);
  }

  this->GetShell()->SetVisible(true);

  folder->SetBounds(Tweaklets::Get(GuiWidgetsTweaklet::KEY)->GetClientArea(this->GetShell()->GetControl()));

  return 0;
}

void DetachedWindow::ActivePartChanged(
    IWorkbenchPartReference::Pointer partReference)
{
  if (activePart == partReference)
  {
    return;
  }

  if (activePart != 0)
  {
    activePart->RemovePropertyListener(IPropertyChangeListener::Pointer(this));
  }
  activePart = partReference;
  if (partReference != 0)
  {
    partReference->AddPropertyListener(IPropertyChangeListener::Pointer(this));
  }
  this->UpdateTitle();
}

void DetachedWindow::ConfigureShell(Shell::Pointer shell)
{
  this->UpdateTitle();

  Tweaklets::Get(GuiWidgetsTweaklet::KEY)->AddControlListener(
      shell->GetControl(), resizeListener);
  //shell.addListener(SWT.Activate, activationListener);
  //shell.addListener(SWT.Deactivate, activationListener);

  //TODO DetachedWindow key bindings
  //  // Register this detached view as a window (for key bindings).
  //  IContextService contextService = (IContextService) getWorkbenchPage()
  //  .getWorkbenchWindow().getWorkbench().getService(IContextService.class);
  //  contextService.registerShell(shell, IContextService.TYPE_WINDOW);
  //
  //  page.getWorkbenchWindow().getWorkbench().getHelpSystem().setHelp(shell,
  //      IWorkbenchHelpContextIds.DETACHED_WINDOW);
}

void* DetachedWindow::CreateContents(void* parent)
{
  // Create the tab folder.
  folder->CreateControl(parent);

  // Reparent each view in the tab folder.
  std::list<PartPane::Pointer> detachedChildren;
  this->CollectViewPanes(detachedChildren, this->GetChildren());
  for (std::list<PartPane::Pointer>::iterator itr = detachedChildren.begin(); itr
      != detachedChildren.end(); ++itr)
  {
    PartPane::Pointer part = *itr;
    part->Reparent(parent);
  }

  //TODO DetachedWindow listen to folder events (update size?)
  //  if (folder->GetPresentation()
  //    instanceof TabbedStackPresentation)
  //    {
  //      TabbedStackPresentation stack = (TabbedStackPresentation) folder.getPresentation();
  //      AbstractTabFolder tabFolder = stack.getTabFolder();
  //      tabFolder.addListener(new TabFolderListener()
  //      {
  //      public void handleEvent(TabFolderEvent e)
  //        {
  //          switch (e.type)
  //          {
  //            case TabFolderEvent.EVENT_CLOSE:
  //            {
  //              updateMinimumSize();
  //              break;
  //            }
  //            case TabFolderEvent.EVENT_PREFERRED_SIZE:
  //            {
  //              updateMinimumSize();
  //              break;
  //            }
  //          }
  //        }
  //      });
  //    }

  // Return tab folder control.
  return folder->GetControl();
}

void DetachedWindow::UpdateTitle()
{
  if (activePart != 0)
  {
    // Uncomment to set the shell title to match the title of the active part
    //            String text = activePart.getTitle();
    //
    //            if (!text.equals(s.getText())) {
    //                s.setText(text);
    //            }
  }
}

void DetachedWindow::UpdateMinimumSize()
{
  //  // We can only do this for 'Tabbed' stacked presentations.
  //  if (folder.getPresentation().Cast<TabbedStackPresentation>() != 0)
  //  {
  //    TabbedStackPresentation stack = (TabbedStackPresentation) folder.getPresentation();
  //
  //    if (stack->GetPartList().size() == 1)
  //    {
  //      // Get the minimum space required for the part
  //      int width = stack->ComputePreferredSize(true, Constants::INF, Constants::INF, 0);
  //      int height = stack->ComputePreferredSize(false, Constants::INF, Constants::INF, 0);
  //
  //      // Take the current shell 'trim' into account
  //      int shellHeight = windowShell->GetBounds().height - windowShell->GetClientArea().height;
  //      int shellWidth = windowShell->GetBounds().width - windowShell->GetClientArea().width;
  //
  //      windowShell->SetMinimumSize(width + shellWidth, height + shellHeight);
  //    }
  //  }
}

IWorkbenchPartReference::Pointer DetachedWindow::GetPartReference(
    StackablePart::Pointer pane)
{

  if (pane == 0 || pane.Cast<PartPane> () == 0)
  {
    return IWorkbenchPartReference::Pointer(0);
  }

  return pane.Cast<PartPane> ()->GetPartReference();
}

bool DetachedWindow::HandleClose()
{

  if (hideViewsOnClose)
  {
    std::list<PartPane::Pointer> views;
    this->CollectViewPanes(views, this->GetChildren());

    // Save any dirty views
    if (!this->HandleSaves(views))
    {
      return false; // User canceled the save
    }

    // OK, go on with the closing
    for (std::list<PartPane::Pointer>::iterator itr = views.begin(); itr
        != views.end(); ++itr)
    {
      PartPane::Pointer child = *itr;

      // Only close if closable...
      if (child->IsCloseable())
      {
        page->HideView(child->GetPartReference().Cast<IViewReference> ());

        // Was the close cancelled?
        if (child->GetContainer() != 0)
          return false;
      }
      else
      {
        page->AttachView(child->GetPartReference().Cast<IViewReference> ());
      }
    }
  }

  if (folder != 0)
  {
    folder->Dispose();
  }

  if (windowShell != 0)
  {
    Tweaklets::Get(GuiWidgetsTweaklet::KEY)->RemoveControlListener(
        windowShell->GetControl(), resizeListener);
    //    windowShell.removeListener(SWT.Activate, activationListener);
    //    windowShell.removeListener(SWT.Deactivate, activationListener);

    DragUtil::RemoveDragTarget(windowShell->GetControl(),
        IDragOverListener::Pointer(this));
    bounds = windowShell->GetBounds();

    //TODO DetachedWindow unregister key bindings
    //    // Unregister this detached view as a window (for key bindings).
    //    final IContextService contextService = (IContextService) getWorkbenchPage().getWorkbenchWindow().getWorkbench().getService(IContextService.class);
    //    contextService.unregisterShell(windowShell);

    windowShell->SetData(Object::Pointer(0));
    windowShell = 0;
  }

  return true;
}

bool DetachedWindow::HandleSaves(std::list<PartPane::Pointer> views)
{
  std::vector<IWorkbenchPart::Pointer> dirtyViews;
  for (std::list<PartPane::Pointer>::iterator iterator = views.begin(); iterator
      != views.end(); ++iterator)
  {
    PartPane::Pointer pane = *iterator;
    IViewReference::Pointer ref =
        pane->GetPartReference().Cast<IViewReference> ();
    IViewPart::Pointer part = ref->GetView(false);
    if (part.Cast<ISaveablePart> () != 0)
    {
      ISaveablePart::Pointer saveable = part.Cast<ISaveablePart> ();
      if (saveable->IsDirty() && saveable->IsSaveOnCloseNeeded())
      {
        dirtyViews.push_back(part);
      }
    }
  }

  // If there are any prompt to save -before- any closing happens
  // FIXME: This code will result in a double prompt if the user
  // decides not to save a particular view at this stage they'll
  // get a second one from the 'hideView' call...
  if (dirtyViews.size() > 0)
  {
    IWorkbenchWindow::Pointer window = page->GetWorkbenchWindow();
    bool success =
        EditorManager::SaveAll(dirtyViews, true, true, false, window);
    if (!success)
    {
      return false; // the user canceled.
    }
  }

  return true;
}

void DetachedWindow::CollectViewPanes(std::list<PartPane::Pointer>& result,
    const std::list<StackablePart::Pointer>& parts)
{
  for (std::list<StackablePart::Pointer>::const_iterator iter = parts.begin(); iter
      != parts.end(); ++iter)
  {
    StackablePart::Pointer part = *iter;
    if (part.Cast<PartPane> () != 0)
    {
      result.push_back(part.Cast<PartPane> ());
    }
  }
}

}
