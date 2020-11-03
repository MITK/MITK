/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "internal/berryQtWidgetsTweaklet.h"

#include "berryPresentablePartFolder.h"
#include "berryAbstractTabItem.h"

#include <internal/berryQtControlWidget.h>

#include <berryConstants.h>
#include <internal/berryDragUtil.h>

namespace berry
{

PresentablePartFolder::ShellListener::ShellListener(AbstractTabFolder* _folder) :
  folder(_folder)
{
}

void PresentablePartFolder::ShellListener::ShellActivated(const ShellEvent::Pointer& /*e*/)
{
  folder->ShellActive(true);
}

void PresentablePartFolder::ShellListener::ShellDeactivated(
    const ShellEvent::Pointer& /*e*/)
{
  folder->ShellActive(false);
}

PresentablePartFolder::ChildPropertyChangeListener::ChildPropertyChangeListener(
    PresentablePartFolder* folder) :
  presentablePartFolder(folder)
{
}

void PresentablePartFolder::ChildPropertyChangeListener::PropertyChange(
    const Object::Pointer& source, int property)
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
    QRect clientArea = DragUtil::GetDisplayBounds(contentProxy);

    QRect bounds = Tweaklets::Get(GuiWidgetsTweaklet::KEY)->ToControl(
        folder->GetControl()->parentWidget(), clientArea);
    current->SetBounds(bounds);
  }
}

void PresentablePartFolder::InternalRemove(IPresentablePart::Pointer toRemove)
{
  AbstractTabItem* item = this->GetTab(toRemove);
  if (item != nullptr) {
      item->Dispose();
  }
  // do not use item anymore!

  if (std::find(partList.begin(), partList.end(), toRemove) != partList.end())
  {
    toRemove->RemovePropertyListener(childPropertyChangeListener.data());
    partList.removeAll(toRemove);
  }
}

void PresentablePartFolder::ChildPropertyChanged(
    IPresentablePart::Pointer part, int property)
{
  AbstractTabItem* tab = this->GetTab(part);

  if (property == IPresentablePart::PROP_HIGHLIGHT_IF_BACK)
  {
    if (tab != nullptr && this->GetCurrent() != part)
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
    if (tab != nullptr)
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
    if (tab != nullptr)
      this->InitTab(tab, part);
  }
}

PresentablePartFolder::~PresentablePartFolder()
{
  Tweaklets::Get(QtWidgetsTweaklet::KEY)->GetShell(folder->GetControl())->RemoveShellListener(
      shellListener.data());
  for (QList<IPresentablePart::Pointer>::iterator iter = partList.begin(); iter
      != partList.end(); ++iter)
  {
    (*iter)->RemovePropertyListener(childPropertyChangeListener.data());
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
  controlShell->AddShellListener(shellListener.data());
  folder->ShellActive(Tweaklets::Get(QtWidgetsTweaklet::KEY)->GetActiveShell()
      == controlShell);

  //folder.getControl().addDisposeListener(tabDisposeListener);

  //toolbarProxy = new ProxyControl(folder.getToolbarParent());

  contentProxy = new QWidget(folder->GetContentParent());
  contentProxy->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  folder->SetContent(contentProxy);

}

QList<IPresentablePart::Pointer> PresentablePartFolder::GetPartList()
{
  QList<AbstractTabItem*> items = folder->GetItems();
  QList<IPresentablePart::Pointer> result;

  for (int i = 0; i < items.size(); i++)
  {
    result.push_back(this->GetPartForTab(items[i]));
  }

  return result;
}

void PresentablePartFolder::Insert(IPresentablePart::Pointer part, int idx)
{
  poco_assert(folder->GetControl() != nullptr);

  if (this->GetTab(part) != nullptr)
  {
    if (this->IndexOf(part) != idx)
    this->Move(part, idx);
    return;
  }

  idx = std::min<int>(idx, static_cast<int>(folder->GetItemCount()));

  AbstractTabItem* item = nullptr;

  int style = 0;

  if (part->IsCloseable())
  {
    style |= Constants::CLOSE;
  }

  item = folder->Add(idx, style);

  item->SetData(part);

  this->InitTab(item, part);

  part->AddPropertyListener(childPropertyChangeListener.data());
  partList.push_back(part);
}

void PresentablePartFolder::Remove(IPresentablePart::Pointer toRemove)
{
  if (toRemove == current)
  {
    this->Select(IPresentablePart::Pointer(nullptr));
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

  folder->Move(currentIndex, newIndex);

  //this->InternalRemove(part);
  //this->Insert(part, newIndex);

  //if (current == part)
  //{
  //  folder->SetSelection(this->GetTab(part));
  //}
}

int PresentablePartFolder::Size()
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

  if (selectedItem != nullptr)
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
  if (tab == nullptr)
  {
    return IPresentablePart::Pointer(nullptr);
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

  if (item == nullptr)
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
