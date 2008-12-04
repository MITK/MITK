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

#include "cherryTabbedStackPresentation.h"
#include "cherryAbstractTabItem.h"
#include "cherryLeftToRightTabOrder.h"
#include "cherryReplaceDragHandler.h"

#include <cherryConstants.h>

namespace cherry
{

int TabbedStackPresentation::ComputePreferredMinimumSize(bool width,
    int availablePerpendicular)
{
  int minSize;
  int hint =
          availablePerpendicular == INF ? Constants::DEFAULT
              : availablePerpendicular;
  if (width)
  {
    minSize = folder->GetTabFolder()->ComputeSize(Constants::DEFAULT, hint).width();
  }
  else
  {
    minSize = folder->GetTabFolder()->ComputeSize(hint, Constants::DEFAULT).height();
  }
  return minSize;
}

void TabbedStackPresentation::HandleTabFolderEvent(TabFolderEvent::Pointer e)
{
  int type = e->type;
  if (type == TabFolderEvent::EVENT_MINIMIZE)
  {
    this->GetSite()->SetState(IStackPresentationSite::STATE_MINIMIZED);
  }
  else if (type == TabFolderEvent::EVENT_MAXIMIZE)
  {
    this->GetSite()->SetState(IStackPresentationSite::STATE_MAXIMIZED);
  }
  else if (type == TabFolderEvent::EVENT_RESTORE)
  {
    this->GetSite()->SetState(IStackPresentationSite::STATE_RESTORED);
  }
  else if (type == TabFolderEvent::EVENT_CLOSE)
  {
    IPresentablePart::Pointer part = folder->GetPartForTab(e->tab);

    if (part != 0)
    {
      std::vector<IPresentablePart::Pointer> parts;
      parts.push_back(part);
      this->GetSite()->Close(parts);
    }
  }
  else if (type == TabFolderEvent::EVENT_SHOW_LIST)
  {
    this->ShowPartList();
  }
  else if (type == TabFolderEvent::EVENT_GIVE_FOCUS_TO_PART)
  {
    IPresentablePart::Pointer part = this->GetSite()->GetSelectedPart();
    if (part != 0)
    {
      part->SetFocus();
    }
  }
  else if (type == TabFolderEvent::EVENT_PANE_MENU)
  {
    IPresentablePart::Pointer part = this->GetSite()->GetSelectedPart();
    if (part != 0)
    {
      part->SetFocus();
    }
    //this->ShowPaneMenu(folder->GetPartForTab(e->tab), QPoint(e->x, e->y));
  }
  else if (type == TabFolderEvent::EVENT_DRAG_START)
  {
    AbstractTabItem* beingDragged = e->tab;
    Point initialLocation(e->x, e->y);

    if (beingDragged == 0)
    {
      this->GetSite()->DragStart(initialLocation, false);
    }
    else
    {
      IPresentablePart::Pointer part = folder->GetPartForTab(beingDragged);

      try {
          dragStart = folder->IndexOf(part);
          this->GetSite()->DragStart(part, initialLocation, false);
          dragStart = -1;
      } catch(std::exception& exc) {
          dragStart = -1;
          throw exc;
      }
    }
  }
  else if (type == TabFolderEvent::EVENT_TAB_SELECTED)
  {
    if (ignoreSelectionChanges > 0)
    {
      return;
    }

    IPresentablePart::Pointer part = folder->GetPartForTab(e->tab);

    if (part != 0)
    {
      this->GetSite()->SelectPart(part);
    }
  }
  else if (type == TabFolderEvent::EVENT_SYSTEM_MENU)
  {
    IPresentablePart::Pointer part = folder->GetPartForTab(e->tab);

    if (part == 0)
    {
      part = this->GetSite()->GetSelectedPart();
    }

    if (part != 0)
    {
      //this->ShowSystemMenu(QPoint(e->x, e->y), part);
    }
  }
  else if (type == TabFolderEvent::EVENT_PREFERRED_SIZE)
  {
    IPresentablePart::Pointer part = folder->GetPartForTab(e->tab);
    if (part == 0)
    {
      // Standalone views with no title have no tab, so just get the part.
      std::list<IPresentablePart::Pointer> parts = this->GetSite()->GetPartList();
      if (parts.size() > 0)
        part = parts.front();
    }
    if (part == this->GetSite()->GetSelectedPart())
    {
      this->GetSite()->FlushLayout();
    }
  }

}

TabbedStackPresentation::TabbedStackPresentation(
    IStackPresentationSite::Pointer site, AbstractTabFolder* widget/*, ISystemMenu systemMenu*/)
 : StackPresentation(site)
{
  PresentablePartFolder* folder = new PresentablePartFolder(widget);
  this->Init(site, folder, new LeftToRightTabOrder(folder),
      new ReplaceDragHandler(widget)); // systemMenu);
}

TabbedStackPresentation::TabbedStackPresentation(
    IStackPresentationSite::Pointer site, PresentablePartFolder* folder/*, ISystemMenu systemMenu*/)
 : StackPresentation(site)
{
  this->Init(site, folder, new LeftToRightTabOrder(folder),
      new ReplaceDragHandler(folder->GetTabFolder())); //, systemMenu);
}

TabbedStackPresentation::TabbedStackPresentation(
    IStackPresentationSite::Pointer site, PresentablePartFolder* newFolder,
    TabOrder* tabs, TabDragHandler* dragBehavior /*, ISystemMenu systemMenu*/)
 : StackPresentation(site)
{
  this->Init(site, newFolder, tabs, dragBehavior);
}

void TabbedStackPresentation::Init(IStackPresentationSite::Pointer site,
            PresentablePartFolder* newFolder, TabOrder* tabs, TabDragHandler* dragBehavior /*, ISystemMenu systemMenu*/)
{
  //this->systemMenu = systemMenu;

  initializing = true;
  ignoreSelectionChanges = 0;
  dragStart = -1;

  this->folder = newFolder;
  this->tabs = tabs;
  this->dragBehavior = dragBehavior;

  //        // Add a dispose listener. This will call the presentationDisposed()
  //        // method when the widget is destroyed.
  //        folder.getTabFolder().getControl().addDisposeListener(new DisposeListener() {
  //            public void widgetDisposed(DisposeEvent e) {
  //                presentationDisposed();
  //            }
  //        });

  folder->GetTabFolder()->tabFolderEvent += TabFolderEventDelegate(this, &TabbedStackPresentation::HandleTabFolderEvent);

  //this->partList = new DefaultPartList(site, newFolder);
}

void TabbedStackPresentation::RestoreState(IPresentationSerializer* serializer,
    IMemento::Pointer savedState)
{
  tabs->RestoreState(serializer, savedState);
}

void TabbedStackPresentation::SaveState(IPresentationSerializer* context,
    IMemento::Pointer memento)
{
  StackPresentation::SaveState(context, memento);

  tabs->SaveState(context, memento);
}

void TabbedStackPresentation::SetBounds(const Rectangle& bounds)
{
  QRect rectangle(bounds.x, bounds.y, bounds.width, bounds.height);
  folder->SetBounds(rectangle);
}

Point TabbedStackPresentation::ComputeMinimumSize()
{
  QSize point = folder->GetTabFolder()->ComputeSize(Constants::DEFAULT, Constants::DEFAULT);
  return Point(point.width(), point.height());
}

int TabbedStackPresentation::ComputePreferredSize(bool width,
    int availableParallel, int availablePerpendicular, int preferredResult)
{

  // If there is exactly one part in the stack, this just returns the
  // preferred size of the part as the preferred size of the stack.
  std::list<IPresentablePart::Pointer> parts = this->GetSite()->GetPartList();
  if (parts.size() == 1 && parts.front() != 0 && !(this->GetSite()->GetState()
      == IStackPresentationSite::STATE_MINIMIZED))
  {
    int partSize = parts.front()->ComputePreferredSize(width, availableParallel,
        availablePerpendicular, preferredResult);

    if (partSize == INF)
      return partSize;

    // Adjust preferred size to take into account tab and border trim.
    int minSize = this->ComputePreferredMinimumSize(width, availablePerpendicular);
    if (width)
    {
      // PaneFolder adds some bogus tab spacing, so just find the maximum width.
      partSize = std::max<int>(minSize, partSize);
    }
    else
    {
      // Add them (but only if there's enough room)
      if (INF - minSize > partSize)
        partSize += minSize;
    }

    return partSize;
  }

  if (preferredResult != INF || this->GetSite()->GetState()
      == IStackPresentationSite::STATE_MINIMIZED)
  {
    int minSize = this->ComputePreferredMinimumSize(width, availablePerpendicular);

    if (this->GetSite()->GetState() == IStackPresentationSite::STATE_MINIMIZED)
    {
      return minSize;
    }

    return std::max<int>(minSize, preferredResult);
  }

  return INF;
}

int TabbedStackPresentation::GetSizeFlags(bool width)
{
  int flags = 0;
  // If there is exactly one part in the stack,
  // then take into account the size flags of the part.
  std::list<IPresentablePart::Pointer> parts = this->GetSite()->GetPartList();
  if (parts.size() == 1 && parts.front() != 0)
  {
    flags |= parts.front()->GetSizeFlags(width);
  }

  return flags | StackPresentation::GetSizeFlags(width);
}

void TabbedStackPresentation::ShowPartList()
{
//  if (partList != 0)
//  {
//    int numberOfParts = folder->GetTabFolder()->GetItemCount();
//    if (numberOfParts > 0)
//    {
//      partList.show(getControl(), folder.getTabFolder() .getPartListLocation(),
//          getSite().getSelectedPart());
//    }
//  }
}

TabbedStackPresentation::~TabbedStackPresentation()
{
//  // Dispose the tab folder's widgetry
//  folder.getTabFolder().getControl().dispose();

  std::cout << "DELETING TabbedStackPresentation\n";

  delete tabs;
  delete dragBehavior;
  delete folder;
}

void TabbedStackPresentation::SetActive(int newState)
{
  folder->GetTabFolder()->SetActive(newState);
}

void TabbedStackPresentation::SetVisible(bool isVisible)
{
  IPresentablePart::Pointer current = this->GetSite()->GetSelectedPart();
  if (current != 0)
  {
    current->SetVisible(isVisible);
  }

  folder->SetVisible(isVisible);
}

void TabbedStackPresentation::SetState(int state)
{
  folder->GetTabFolder()->SetState(state);
}

void* TabbedStackPresentation::GetControl()
{
  return folder->GetTabFolder()->GetControl();
}

AbstractTabFolder* TabbedStackPresentation::GetTabFolder()
{
  return folder->GetTabFolder();
}

void TabbedStackPresentation::AddPart(IPresentablePart::Pointer newPart,
    Object::Pointer cookie)
{
  ++ignoreSelectionChanges;
  try
  {
    if (initializing)
    {
      tabs->AddInitial(newPart);
    }
    else
    {
      if (cookie == 0)
      {
        tabs->Add(newPart);
      }
      else
      {
        int insertionPoint = dragBehavior->GetInsertionPosition(cookie);
        tabs->Insert(newPart, insertionPoint);
      }
    }

    --ignoreSelectionChanges;
  } catch (std::exception& e)
  {
    --ignoreSelectionChanges;
    throw e;
  }

  if (tabs->GetPartList().size() == 1)
  {
    if (newPart->GetSizeFlags(true) != 0 || newPart->GetSizeFlags(false) != 0)
    {
      this->GetSite()->FlushLayout();
    }
  }
}

void TabbedStackPresentation::MovePart(IPresentablePart::Pointer toMove,
    Object::Pointer cookie)
{
  ++ignoreSelectionChanges;
  try
  {
    int insertionPoint = dragBehavior->GetInsertionPosition(cookie);

    if (insertionPoint == folder->IndexOf(toMove))
    {
      return;
    }

    tabs->Move(toMove, insertionPoint);

    --ignoreSelectionChanges;
  } catch (std::exception& e)
  {
    --ignoreSelectionChanges;
    throw e;
  }
}

void TabbedStackPresentation::RemovePart(IPresentablePart::Pointer oldPart)
{
  ++ignoreSelectionChanges;
  try
  {
    tabs->Remove(oldPart);

    --ignoreSelectionChanges;
  } catch (std::exception& e)
  {
    --ignoreSelectionChanges;
    throw e;
  }
}

void TabbedStackPresentation::SelectPart(IPresentablePart::Pointer toSelect)
{
  initializing = false;

  tabs->Select(toSelect);
}

StackDropResult::Pointer TabbedStackPresentation::DragOver(void* currentControl, const Point& location)
{
  QWidget* currentWidget = static_cast<QWidget*>(currentControl);
  return dragBehavior->DragOver(currentWidget, location, dragStart);
}

std::vector<void*> TabbedStackPresentation::GetTabList(
    IPresentablePart::Pointer part)
{
  std::vector<void*> list;
  if (folder->GetTabFolder()->GetTabPosition() == Constants::BOTTOM)
  {
    if (part->GetControl() != 0)
    {
      list.push_back(part->GetControl());
    }
  }

  list.push_back(folder->GetTabFolder()->GetControl());

//  if (part->GetToolBar() != 0)
//  {
//    list.push_back(part->GetToolBar());
//  }

  if (folder->GetTabFolder()->GetTabPosition() == Constants::TOP)
  {
    if (part->GetControl() != 0)
    {
      list.push_back(part->GetControl());
    }
  }

  return list;
}

void TabbedStackPresentation::MoveTab(IPresentablePart::Pointer part, int index)
{
  tabs->Move(part, index);
  folder->Layout(true);
}

std::vector<IPresentablePart::Pointer> TabbedStackPresentation::GetPartList()
{
  return tabs->GetPartList();
}

}
