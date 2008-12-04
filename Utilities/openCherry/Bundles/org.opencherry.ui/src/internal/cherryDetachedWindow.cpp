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

#include "cherryDetachedWindow.h"

#include "../cherryIWorkbenchPartConstants.h"
#include "../cherryISaveablePart.h"

#include "../cherryWorkbenchWindow.h"

#include "../tweaklets/cherryGuiWidgetsTweaklet.h"

#include "cherryEditorManager.h"
#include "cherryDragUtil.h"

namespace cherry
{

DetachedWindow::ShellListener::ShellListener(DetachedWindow* wnd) :
  window(wnd)
{

}

void DetachedWindow::ShellListener::ShellClosed(ShellEvent::Pointer e)
{
  // only continue to close if the handleClose
  // wasn't canceled
  e->doit = window->HandleClose();
}

DetachedWindow::ShellControlListener::ShellControlListener(DetachedWindow* wnd) :
  window(wnd)
{

}

void DetachedWindow::ShellControlListener::ControlResized(
    GuiTk::ControlEvent::Pointer e)
{
  window->folder->SetBounds(Tweaklets::Get(GuiWidgetsTweaklet::KEY)->GetClientArea(e->item));
}

DetachedWindow::DetachedWindow(WorkbenchPage::Pointer workbenchPage)
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
  folder->AddListener(this);

  windowShell = page->GetWorkbenchWindow().Cast<WorkbenchWindow>()
       ->GetDetachedWindowPool()->AllocateShell(shellListener);
  std::cout << "Creating detached shell: " << windowShell->GetTraceId() << std::endl;
  windowShell->SetData(this);
  windowShell->SetText(""); //$NON-NLS-1$

  DragUtil::AddDragTarget(windowShell->GetControl(), this);
  hideViewsOnClose = true;
  if (bounds.IsEmpty())
  {
    Rectangle windowRect = page->GetWorkbenchWindow()->GetShell()->GetBounds();
    Point center(windowRect.x + windowRect.width / 2, windowRect.y
        - windowRect.height / 2);
    bounds = Rectangle(center.x - 150, center.y + 100, 300, 200);
  }

  // Force the rect into the current display
  Rectangle dispBounds = Tweaklets::Get(GuiWidgetsTweaklet::KEY)->GetScreenSize();
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
  return (this->page == workbenchPage);
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
    return 0;
  }

  PartPane::Pointer sourcePart = draggedObject.Cast<PartPane> ();

  if (sourcePart->GetWorkbenchWindow() != page->GetWorkbenchWindow())
  {
    return 0;
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
        StackDropResult::Pointer stackDropResult =
            new StackDropResult(displayBounds, 0);
        target = folder->CreateDropTarget(sourcePart, stackDropResult);
      }
      else
      {
        return 0;
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
  return this->page;
}

void DetachedWindow::RestoreState(IMemento::Pointer memento)
{
  //TODO DetachedWindow restore state
  //  // Read the bounds.
  //  Integer bigInt;
  //  bigInt = memento.getInteger(IWorkbenchConstants.TAG_X);
  //  int x = bigInt.intValue();
  //  bigInt = memento.getInteger(IWorkbenchConstants.TAG_Y);
  //  int y = bigInt.intValue();
  //  bigInt = memento.getInteger(IWorkbenchConstants.TAG_WIDTH);
  //  int width = bigInt.intValue();
  //  bigInt = memento.getInteger(IWorkbenchConstants.TAG_HEIGHT);
  //  int height = bigInt.intValue();
  //  bigInt = memento.getInteger(IWorkbenchConstants.TAG_FLOAT);
  //
  //  // Set the bounds.
  //  bounds = new Rectangle(x, y, width, height);
  //  if (getShell() != 0)
  //  {
  //    getShell().setBounds(bounds);
  //  }
  //
  //  // Create the folder.
  //  IMemento childMem = memento.getChild(IWorkbenchConstants.TAG_FOLDER);
  //  if (childMem != 0)
  //  {
  //    folder.restoreState(childMem);
  //  }
}

void DetachedWindow::SaveState(IMemento::Pointer memento)
{
  //TODO DetachedWindow save state
  //  if (getShell() != 0)
  //  {
  //    bounds = getShell().getBounds();
  //  }
  //
  //  // Save the bounds.
  //  memento.putInteger(IWorkbenchConstants.TAG_X, bounds.x);
  //  memento.putInteger(IWorkbenchConstants.TAG_Y, bounds.y);
  //  memento.putInteger(IWorkbenchConstants.TAG_WIDTH, bounds.width);
  //  memento.putInteger(IWorkbenchConstants.TAG_HEIGHT, bounds.height);
  //
  //  // Save the views.
  //  IMemento childMem = memento.createChild(IWorkbenchConstants.TAG_FOLDER);
  //  folder.saveState(childMem);
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
  this->GetShell()->SetVisible(true);

  if (!(bounds == this->GetShell()->GetBounds()))
  {
    this->GetShell()->SetBounds(bounds);
  }

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
    activePart->RemovePropertyListener(this);
  }
  activePart = partReference;
  if (partReference != 0)
  {
    partReference->AddPropertyListener(this);
  }
  this->UpdateTitle();
}

void DetachedWindow::ConfigureShell(Shell::Pointer shell)
{
  this->UpdateTitle();

  shell->AddControlListener(resizeListener);
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
    return 0;
  }

  return pane.Cast<PartPane> ()->GetPartReference();
}

bool DetachedWindow::HandleClose()
{

  if (hideViewsOnClose)
  {
    std::list<PartPane::Pointer> views;
    this->CollectViewPanes(views, this->GetChildren());

    // Save any drty views
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
    windowShell->RemoveControlListener(resizeListener);
    //    windowShell.removeListener(SWT.Activate, activationListener);
    //    windowShell.removeListener(SWT.Deactivate, activationListener);

    DragUtil::RemoveDragTarget(windowShell->GetControl(), this);
    bounds = windowShell->GetBounds();

    //TODO DetachedWindow unregister key bindings
    //    // Unregister this detached view as a window (for key bindings).
    //    final IContextService contextService = (IContextService) getWorkbenchPage().getWorkbenchWindow().getWorkbench().getService(IContextService.class);
    //    contextService.unregisterShell(windowShell);

    windowShell->SetData(0);
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
