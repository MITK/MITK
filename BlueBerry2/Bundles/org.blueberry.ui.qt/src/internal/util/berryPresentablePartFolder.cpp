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

#include "berryPresentablePartFolder.h"
#include "berryAbstractTabItem.h"

#include "../berryQtWidgetsTweaklet.h"
#include "../berryQtControlWidget.h"

#include <berryConstants.h>
#include <internal/berryDragUtil.h>

namespace berry
{

PresentablePartFolder::ContentProxyListener::ContentProxyListener(
    PresentablePartFolder* folder) :
  folder(folder)
{
}

GuiTk::IControlListener::Events::Types PresentablePartFolder::ContentProxyListener::GetEventTypes() const
{
  return Events::MOVED & Events::RESIZED;
}

void PresentablePartFolder::ContentProxyListener::ControlMoved(
    GuiTk::ControlEvent::Pointer  /*e*/)
{
  folder->LayoutContent();
}

void PresentablePartFolder::ContentProxyListener::ControlResized(
    GuiTk::ControlEvent::Pointer  /*e*/)
{
}

PresentablePartFolder::ShellListener::ShellListener(AbstractTabFolder* _folder) :
  folder(_folder)
{
}

void PresentablePartFolder::ShellListener::ShellActivated(ShellEvent::Pointer  /*e*/)
{
  folder->ShellActive(true);
}

void PresentablePartFolder::ShellListener::ShellDeactivated(
    ShellEvent::Pointer  /*e*/)
{
  folder->ShellActive(false);
}

PresentablePartFolder::ChildPropertyChangeListener::ChildPropertyChangeListener(
    PresentablePartFolder* folder) :
  presentablePartFolder(folder)
{
}

void PresentablePartFolder::ChildPropertyChangeListener::PropertyChange(
    Object::Pointer source, int property)
{

  if (source.Cast<IPresentablePart> () != 0)
  {
    IPresentablePart::Pointer part = source.Cast<IPresentablePart> ();

    presentablePartFolder->ChildPropertyChanged(part, property);
  }
}

PartInfo PresentablePartFolder::tempPartInfo = PartInfo();

void PresentablePartFolder::LayoutContent()
{
  if (current != 0)
  {
    Rectangle clientArea = DragUtil::GetDisplayBounds(contentProxy);

    Rectangle bounds = Tweaklets::Get(GuiWidgetsTweaklet::KEY)->ToControl(
        folder->GetControl()->parentWidget(), clientArea);
    current->SetBounds(bounds);
  }
}

void PresentablePartFolder::InternalRemove(IPresentablePart::Pointer toRemove)
{
  AbstractTabItem* item = this->GetTab(toRemove);
  if (item != 0) {
      item->Dispose();
  }
  // do not use item anymore!

  if (std::find(partList.begin(), partList.end(), toRemove) != partList.end())
  {
    toRemove->RemovePropertyListener(childPropertyChangeListener);
    partList.remove(toRemove);
  }
}

void PresentablePartFolder::ChildPropertyChanged(
    IPresentablePart::Pointer part, int property)
{
  AbstractTabItem* tab = this->GetTab(part);

  if (property == IPresentablePart::PROP_HIGHLIGHT_IF_BACK)
  {
    if (tab != 0 && this->GetCurrent() != part)
    {//Set bold if it doesn't currently have focus
      tab->SetBold(true);
      this->InitTab(tab, part);
    }
  }

  // else if (property == IPresentablePart::PROP_TOOLBAR)
  //  {
  //    if (tab != 0 && this->GetCurrent() == part)
  //    {
  //      folder->FlushToolbarSize();
  //    }
  //    if (tab != 0)
  //      {
  //        this->InitTab(tab, part);
  //        if (this->GetCurrent() == part)
  //        {
  //          this->Layout(true);
  //        }
  //      }
  else if (property == IPresentablePart::PROP_CONTENT_DESCRIPTION || property
      == IPresentablePart::PROP_PANE_MENU || property
      == IPresentablePart::PROP_TITLE)
  {
    if (tab != 0)
    {
      this->InitTab(tab, part);
      if (this->GetCurrent() == part)
      {
        this->Layout(true);
      }
    }
  }
  else if (property == IPresentablePart::PROP_PREFERRED_SIZE)
  {
    TabFolderEvent::Pointer event(
        new TabFolderEvent(TabFolderEvent::EVENT_PREFERRED_SIZE, tab, 0, 0));
    folder->FireEvent(event);
  }
  else
  {
    if (tab != 0)
      this->InitTab(tab, part);
  }
}

PresentablePartFolder::~PresentablePartFolder()
{
  Tweaklets::Get(QtWidgetsTweaklet::KEY)->GetShell(folder->GetControl())->RemoveShellListener(
      shellListener);
  for (std::list<IPresentablePart::Pointer>::iterator iter = partList.begin(); iter
      != partList.end(); ++iter)
  {
    (*iter)->RemovePropertyListener(childPropertyChangeListener);
  }

  BERRY_DEBUG << "DELETING PresentablePartFolder and contentProxy\n";

  delete folder;
}

void PresentablePartFolder::InitTab(AbstractTabItem* item,
    IPresentablePart::Pointer part)
{
  tempPartInfo.Set(part);
  item->SetInfo(tempPartInfo);

  item->SetBusy(part->IsBusy());
  if (part == this->GetCurrent())
  {
    folder->SetSelectedInfo(tempPartInfo);
    //TODO Pane menu
    //folder->EnablePaneMenu(part->GetMenu() != 0);

    //setToolbar(part.getToolBar());
  }
}

PresentablePartFolder::PresentablePartFolder(AbstractTabFolder* _folder) :
  folder(_folder), isVisible(true), shellListener(new ShellListener(folder)),
      childPropertyChangeListener(new ChildPropertyChangeListener(this))
{
  Shell::Pointer controlShell =
      Tweaklets::Get(QtWidgetsTweaklet::KEY)->GetShell(folder->GetControl());
  controlShell->AddShellListener(shellListener);
  folder->ShellActive(Tweaklets::Get(QtWidgetsTweaklet::KEY)->GetActiveShell()
      == controlShell);

  //folder.getControl().addDisposeListener(tabDisposeListener);

  //toolbarProxy = new ProxyControl(folder.getToolbarParent());

  // NOTE: if the shape of contentProxy changes, the fix for bug 85899 in EmptyTabFolder.computeSize may need adjustment.
  contentListener = new ContentProxyListener(this);
  contentProxy = new QtControlWidget(folder->GetContentParent(), Shell::Pointer(0));
  contentProxy->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  //contentProxy->setVisible(false);
  int i = 0;
  for (QWidget* currentWidget = contentProxy; currentWidget != 0 && currentWidget
      != folder->GetControl()->parentWidget(); currentWidget = currentWidget->parentWidget(), ++i)
  {
    Tweaklets::Get(GuiWidgetsTweaklet::KEY)->AddControlListener(currentWidget, contentListener);
  }
  folder->SetContent(contentProxy);
  
  //BERRY_INFO << "listener add: " << i << std::endl;
}

std::vector<IPresentablePart::Pointer> PresentablePartFolder::GetPartList()
{
  std::vector<AbstractTabItem*> items = folder->GetItems();
  std::vector<IPresentablePart::Pointer> result;

  for (unsigned int i = 0; i < items.size(); i++)
  {
    result.push_back(this->GetPartForTab(items[i]));
  }

  return result;
}

void PresentablePartFolder::Insert(IPresentablePart::Pointer part, int idx)
{
  poco_assert(folder->GetControl() != 0);

  if (this->GetTab(part) != 0)
  {
    if (this->IndexOf(part) != idx)
    this->Move(part, idx);
    return;
  }

  idx = std::min<int>(idx, static_cast<int>(folder->GetItemCount()));

  AbstractTabItem* item = 0;

  int style = 0;

  if (part->IsCloseable())
  {
    style |= Constants::CLOSE;
  }

  item = folder->Add(idx, style);

  item->SetData(part);

  this->InitTab(item, part);

  part->AddPropertyListener(childPropertyChangeListener);
  partList.push_back(part);
}

void PresentablePartFolder::Remove(IPresentablePart::Pointer toRemove)
{
  if (toRemove == current)
  {
    this->Select(IPresentablePart::Pointer(0));
  }

  this->InternalRemove(toRemove);
}

void PresentablePartFolder::Move(IPresentablePart::Pointer part, int newIndex)
{
  int currentIndex = this->IndexOf(part);

  if (currentIndex == newIndex)
  {
    return;
  }

  this->InternalRemove(part);
  this->Insert(part, newIndex);

  if (current == part)
  {
    folder->SetSelection(this->GetTab(part));
  }
}

std::size_t PresentablePartFolder::Size()
{
  return folder->GetItemCount();
}

void PresentablePartFolder::SetBounds(const QRect& bounds)
{
  QSize minSize = folder->ComputeSize(bounds.width(), Constants::DEFAULT);

  QRect newBounds(bounds);
  if (folder->GetState() == IStackPresentationSite::STATE_MINIMIZED
      && minSize.height() < bounds.height())
  {
    newBounds.setHeight(minSize.height());
  }

  // Set the tab folder's bounds
  folder->GetControl()->setGeometry(newBounds);

  this->Layout(false);
}

void PresentablePartFolder::Select(IPresentablePart::Pointer toSelect)
{

  if (toSelect == current)
  {
    return;
  }

  if (toSelect != 0)
  {
    toSelect->SetVisible(true);
  }

  if (current != 0)
  {
    current->SetVisible(false);
    //setToolbar(null);
  }

  current = toSelect;

  AbstractTabItem* selectedItem = this->GetTab(toSelect);

  folder->SetSelection(selectedItem);

  if (selectedItem != 0)
  {
    // Determine if we need to un-bold this tab
    selectedItem->SetBold(false);
    this->InitTab(selectedItem, toSelect);
  }
  //  else
  //  {
  //    setToolbar(null);
  //  }

  this->Layout(true);
}

IPresentablePart::Pointer PresentablePartFolder::GetPartForTab(
    AbstractTabItem* tab)
{
  if (tab == 0)
  {
    return IPresentablePart::Pointer(0);
  }

  IPresentablePart::Pointer part = tab->GetData().Cast<IPresentablePart> ();

  return part;
}

AbstractTabItem* PresentablePartFolder::GetTab(IPresentablePart::Pointer part)
{
  return folder->FindItem(part);
}

int PresentablePartFolder::IndexOf(IPresentablePart::Pointer part)
{
  AbstractTabItem* item = this->GetTab(part);

  if (item == 0)
  {
    return -1;
  }

  return folder->IndexOf(item);
}

AbstractTabFolder* PresentablePartFolder::GetTabFolder()
{
  return folder;
}

void PresentablePartFolder::SetVisible(bool isVisible)
{
  this->isVisible = isVisible;
  this->GetTabFolder()->SetVisible(isVisible);
  if (isVisible)
  {
    this->Layout(true);
  }
}

void PresentablePartFolder::Layout(bool changed)
{
  if (!isVisible)
  {
    // Don't bother with layout if we're not visible
    return;
  }
  // Lay out the tab folder and compute the client area
  folder->Layout(changed);

  //toolbarProxy.layout();

  this->LayoutContent();
}

IPresentablePart::Pointer PresentablePartFolder::GetCurrent()
{
  return current;
}

}
