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

#include "berryMenuManager.h"

#include "berryIMenu.h"
#include "berryIMenuItem.h"
#include "berryIToolBar.h"
#include "berrySubContributionItem.h"
#include "../berryImageDescriptor.h"

#include "../tweaklets/berryGuiWidgetsTweaklet.h"

namespace berry
{

struct NullOverrides: public IContributionManagerOverrides
{
public:
  ObjectInt::Pointer GetAccelerator(IContributionItem::Pointer /*item*/)
  {
    return ObjectInt::Pointer(0);
  }

  ObjectString::Pointer GetAcceleratorText(IContributionItem::Pointer /*item*/)
  {
    return ObjectString::Pointer(0);
  }

  ObjectBool::Pointer GetEnabled(IContributionItem::Pointer /*item*/)
  {
    return ObjectBool::Pointer(0);
  }

  ObjectString::Pointer GetText(IContributionItem::Pointer /*item*/)
  {
    return ObjectString::Pointer(0);
  }
};

struct GuiTkMenuListener : public GuiTk::IMenuListener
{
public:

  GuiTkMenuListener(MenuManager::Pointer manager) :
    manager(manager)
  {

  }

  void MenuAboutToHide()
  {
    //      ApplicationWindow.resetDescription(e.widget);
    MenuManager::Pointer(manager)->HandleAboutToHide();
  }

  void MenuAboutToShow()
  {
    MenuManager::Pointer(manager)->HandleAboutToShow();
  }

private:

  MenuManager::WeakPtr manager;
};

MenuManager::MenuManager(const std::string text, const std::string& id)
: id(id), menuText(text), removeAllWhenShown(false), visible(true)
{

}

MenuManager::MenuManager(const std::string& text,
    SmartPointer<ImageDescriptor> image, const std::string& id)
: id(id), menuText(text), image(image), removeAllWhenShown(false), visible(true)
{

}

MenuManager::~MenuManager()
{
  this->Dispose();
}

void MenuManager::AddMenuListener(SmartPointer<IMenuListener> listener)
{
  menuEvents.AddListener(listener);
}

SmartPointer<IMenu> MenuManager::CreateContextMenu(void* parent)
{
  if (!this->MenuExist())
  {
    menu = Tweaklets::Get(GuiWidgetsTweaklet::KEY)->CreateMenu(parent);
    this->InitializeMenu();
  }
  return menu;
}

SmartPointer<IMenu> MenuManager::CreateMenuBar(void* parent)
{
  if (!this->MenuExist())
  {
    menu = Tweaklets::Get(GuiWidgetsTweaklet::KEY)->CreateMenu(parent, IMenu::BAR);
    this->Update(false);
  }
  return menu;
}

void MenuManager::Dispose()
{
  if (this->MenuExist())
  {
    menu->RemoveMenuListener(menuListener);
    menu->Dispose();
  }
  menu = 0;

  if (menuItem)
  {
    menuItem->Dispose();
    menuItem = 0;
  }

  //this->DisposeOldImages();

  std::vector<IContributionItem::Pointer> items = this->GetItems();
  for (unsigned int i = 0; i < items.size(); i++)
  {
    items[i]->Dispose();
  }

  this->MarkDirty();
}

void MenuManager::Fill(void* /*parent*/)
{
}

void MenuManager::Fill(SmartPointer<IToolBar> /*parent*/, int /*index*/)
{
}

void MenuManager::Fill(SmartPointer<IMenu> parent, int index)
{
  if (!menuItem || menuItem->IsDisposed())
  {
    menuItem = Tweaklets::Get(GuiWidgetsTweaklet::KEY)->CreateMenuItem(parent, UIElement::STYLE_CASCADE, index);
    menuItem->SetText(this->GetMenuText());

    if (image)
    {
      //LocalResourceManager localManager =
      //    new LocalResourceManager(JFaceResources.getResources());
      menuItem->SetIcon(image); //localManager.createImage(image));
      //disposeOldImages();
      //imageManager = localManager;
    }

    if (!this->MenuExist())
    {
      menu = Tweaklets::Get(GuiWidgetsTweaklet::KEY)->CreateMenu(parent);
    }

    menuItem->SetMenu(menu);

    this->InitializeMenu();

    this->SetDirty(true);
  }
}

IMenuManager::Pointer MenuManager::FindMenuUsingPath(const std::string& path)
{
  IContributionItem::Pointer item(this->FindUsingPath(path));
  if (IMenuManager::Pointer manager = item.Cast<IMenuManager>())
  {
    return manager;
  }
  return IMenuManager::Pointer(0);
}

IContributionItem::Pointer MenuManager::FindUsingPath(const std::string& path)
{
  std::string id(path);
  std::string rest;
  std::string::size_type separator(path.find_first_of('/'));
  if (separator != std::string::npos)
  {
    id = path.substr(0, separator);
    rest = path.substr(separator + 1);
  }
  else
  {
    return ContributionManager::Find(path);
  }

  IContributionItem::Pointer item(ContributionManager::Find(id));
  if (IMenuManager::Pointer manager = item.Cast<IMenuManager>())
  {
    return manager->FindUsingPath(rest);
  }
  return IContributionItem::Pointer(0);
}

std::string MenuManager::GetId()
{
  return id;
}

SmartPointer<IMenu> MenuManager::GetMenu()
{
  return menu;
}

std::string MenuManager::GetMenuText()
{
  if (definitionId == "")
  {
    return menuText;
  }
//  ExternalActionManager::ICallback callback = ExternalActionManager
//  .getInstance().getCallback();
//  if (callback != null)
//  {
//    String shortCut = callback.getAcceleratorText(definitionId);
//    if (shortCut == null)
//    {
//      return menuText;
//    }
//    return menuText + "\t" + shortCut; //$NON-NLS-1$
//  }
  return menuText;
}

SmartPointer<ImageDescriptor> MenuManager::GetImageDescriptor()
{
  return image;
}

SmartPointer<IContributionManagerOverrides> MenuManager::GetOverrides()
{
  if (!overrides)
  {
    IContributionManager::Pointer p(parent.Lock());
    if (!p)
    {
      overrides = new NullOverrides();
    }
    else
    {
      overrides = p->GetOverrides();
    }
    ContributionManager::SetOverrides(overrides);
  }
  return overrides;
}

IContributionManager::Pointer MenuManager::GetParent()
{
  return IContributionManager::Pointer(parent);
}

bool MenuManager::GetRemoveAllWhenShown()
{
  return removeAllWhenShown;
}

bool MenuManager::IsDynamic()
{
  return false;
}

bool MenuManager::IsEnabled()
{
  return true;
}

bool MenuManager::IsGroupMarker()
{
  return false;
}

bool MenuManager::IsSeparator()
{
  return false;
}

bool MenuManager::IsVisible()
{
  if (!visible)
  {
    return false; // short circuit calculations in this case
  }

  if (removeAllWhenShown)
  {
    // we have no way of knowing if the menu has children
    return true;
  }

  // menus aren't visible if all of its children are invisible (or only contains visible separators).
  std::vector<IContributionItem::Pointer> childItems(this->GetItems());
  bool visibleChildren = false;
  for (unsigned int j = 0; j < childItems.size(); j++)
  {
    if (childItems[j]->IsVisible() && !childItems[j]->IsSeparator())
    {
      visibleChildren = true;
      break;
    }
  }

  return visibleChildren;
}

void MenuManager::MarkDirty()
{
  ContributionManager::MarkDirty();
  // Can't optimize by short-circuiting when the first dirty manager is encountered,
  // since non-visible children are not even processed.
  // That is, it's possible to have a dirty sub-menu under a non-dirty parent menu
  // even after the parent menu has been updated.
  // If items are added/removed in the sub-menu, we still need to propagate the dirty flag up,
  // even if the sub-menu is already dirty, since the result of isVisible() may change
  // due to the added/removed items.
  IContributionManager::Pointer parent(this->GetParent());
  if (parent)
  {
    parent->MarkDirty();
  }
}

void MenuManager::RemoveMenuListener(SmartPointer<IMenuListener> listener)
{
  menuEvents.RemoveListener(listener);
}

void MenuManager::SaveWidgetState()
{
}

void MenuManager::SetOverrides(SmartPointer<IContributionManagerOverrides> newOverrides)
{
  overrides = newOverrides;
  ContributionManager::SetOverrides(overrides);
}

void MenuManager::SetParent(IContributionManager::Pointer manager)
{
  parent = manager;
}

void MenuManager::SetRemoveAllWhenShown(bool removeAll)
{
  this->removeAllWhenShown = removeAll;
}

void MenuManager::SetVisible(bool visible)
{
  this->visible = visible;
}

void MenuManager::SetActionDefinitionId(const std::string& definitionId)
{
  this->definitionId = definitionId;
}

void MenuManager::Update()
{
  this->UpdateMenuItem();
}

void MenuManager::Update(bool force)
{
  this->Update(force, false);
}

void MenuManager::Update(const std::string& property)
{
  std::vector<IContributionItem::Pointer> items(this->GetItems());

  for (unsigned int i = 0; i < items.size(); i++)
  {
    items[i]->Update(property);
  }

  if (menu && !menu->IsDisposed() && menu->GetParentItem())
  {
    if (IMenuItem::TEXT == property)
    {
      ObjectString::Pointer text(this->GetOverrides()->GetText(IMenuManager::Pointer(this)));

      if (!text)
      {
        text = new ObjectString(this->GetMenuText());
      }

      if (text)
      {
//        ExternalActionManager::ICallback callback = ExternalActionManager
//        .getInstance().getCallback();
//
//        if (callback != null)
//        {
//          int index = text.indexOf('&');
//
//          if (index >= 0 && index < text.length() - 1)
//          {
//            char character = Character.toUpperCase(text
//                .charAt(index + 1));
//
//            if (callback.isAcceleratorInUse(SWT.ALT | character))
//            {
//              if (index == 0)
//              {
//                text = text.substring(1);
//              }
//              else
//              {
//                text = text.substring(0, index)
//                + text.substring(index + 1);
//              }
//            }
//          }
//        }

        menu->GetParentItem()->SetText(*text);
      }
    }
    else if (IMenuItem::IMAGE == property && image)
    {
//      LocalResourceManager localManager = new LocalResourceManager(JFaceResources
//          .getResources());
      menu->GetParentItem()->SetIcon(image); //localManager.createImage(image));
      // disposeOldImages();
      // imageManager = localManager;
    }
  }
}

void MenuManager::UpdateAll(bool force)
{
  this->Update(force, true);
}

void MenuManager::FireAboutToShow(IMenuManager::Pointer manager)
{
  menuEvents.menuAboutToShow(manager);
}

void MenuManager::FireAboutToHide(IMenuManager::Pointer manager)
{
  menuEvents.menuAboutToHide(manager);
}

void MenuManager::HandleAboutToShow()
{
  if (removeAllWhenShown)
  {
    this->RemoveAll();
  }
  this->FireAboutToShow(IMenuManager::Pointer(this));
  this->Update(false, false);
}

void MenuManager::HandleAboutToHide()
{
  this->FireAboutToHide(IMenuManager::Pointer(this));
}

void MenuManager::InitializeMenu()
{
  menuListener = new GuiTkMenuListener(MenuManager::Pointer(this));
  menu->AddMenuListener(menuListener);
  // Don't do an update(true) here, in case menu is never opened.
  // Always do it lazily in handleAboutToShow().
}

//void MenuManager::DisposeOldImages()
//{
//  if (imageManager)
//  {
//    imageManager.dispose();
//    imageManager = null;
//  }
//}

void MenuManager::UpdateMenuItem()
{
  /*
   * Commented out until proper solution to enablement of
   * menu item for a sub-menu is found. See bug 30833 for
   * more details.
   *
   if (menuItem != null && !menuItem.isDisposed() && menuExist()) {
   IContributionItem items[] = getItems();
   boolean enabled = false;
   for (int i = 0; i < items.length; i++) {
   IContributionItem item = items[i];
   enabled = item.isEnabled();
   if(enabled) break;
   }
   // Workaround for 1GDDCN2: SWT:Linux - MenuItem.setEnabled() always causes a redraw
   if (menuItem.getEnabled() != enabled)
   menuItem.setEnabled(enabled);
   }
   */
  // Partial fix for bug #34969 - disable the menu item if no
  // items in sub-menu (for context menus).
  if (menuItem && !menuItem->IsDisposed() && this->MenuExist())
  {
    bool enabled = removeAllWhenShown || menu->GetItemCount() > 0;
    // Workaround for 1GDDCN2: SWT:Linux - MenuItem.setEnabled() always causes a redraw
    if (menuItem->GetEnabled() != enabled)
    {
      // We only do this for context menus (for bug #34969)
      IMenu::Pointer topMenu(menu);
      while (topMenu->GetParentMenu())
      {
        topMenu = topMenu->GetParentMenu();
      }
      if (topMenu->GetStyle() != IMenu::BAR)
      {
        menuItem->SetEnabled(enabled);
      }
    }
  }
}

bool MenuManager::MenuExist()
{
  return menu != 0 && !menu->IsDisposed();
}

std::vector<SmartPointer<IMenuItem> > MenuManager::GetMenuItems()
{
  if (menu)
  {
    return menu->GetItems();
  }
  return std::vector<IMenuItem::Pointer>();
}

SmartPointer<IMenuItem> MenuManager::GetMenuItem(unsigned int index)
{
  if (menu)
  {
    return menu->GetItem(index);
  }
  return IMenuItem::Pointer(0);
}

unsigned int MenuManager::GetMenuItemCount()
{
  if (menu)
  {
    return menu->GetItemCount();
  }
  return 0;
}

void MenuManager::DoItemFill(IContributionItem::Pointer ci, int index)
{
  ci->Fill(menu, index);
}

void MenuManager::Update(bool force, bool recursive)
{
  if (ContributionManager::IsDirty() || force)
  {
    if (this->MenuExist())
    {
      // clean contains all active items without double separators
      std::vector<IContributionItem::Pointer> items(this->GetItems());
      std::vector<IContributionItem::Pointer> clean;
      IContributionItem::Pointer separator;
      for (unsigned int i = 0; i < items.size(); ++i)
      {
        IContributionItem::Pointer ci(items[i]);
        if (!ci->IsVisible())
        {
          continue;
        }
        if (ci->IsSeparator())
        {
          // delay creation until necessary
          // (handles both adjacent separators, and separator at end)
          separator = ci;
        }
        else
        {
          if (separator)
          {
            if (clean.size() > 0)
            {
              clean.push_back(separator);
            }
            separator = 0;
          }
          clean.push_back(ci);
        }
      }

      // remove obsolete (removed or non active)
      std::vector<IMenuItem::Pointer> mi(this->GetMenuItems());

      for (unsigned int i = 0; i < mi.size(); i++)
      {
        Object::Pointer data(mi[i]->GetData());

        if (!data || std::find(clean.begin(), clean.end(), data) == clean.end())
        {
          mi[i]->Dispose();
        }
        else if (IContributionItem::Pointer ci = data.Cast<IContributionItem>())
        {
          if(ci->IsDynamic() && ci->IsDirty())
          {
            mi[i]->Dispose();
          }
        }
      }

      // add new
      mi = this->GetMenuItems();
      unsigned int srcIx = 0;
      unsigned int destIx = 0;

      for (std::vector<IContributionItem::Pointer>::iterator e = clean.begin();
           e != clean.end(); ++e)
      {
        IContributionItem::Pointer src(*e);
        IContributionItem::Pointer dest;

        // get corresponding item in widget
        if (srcIx < mi.size())
        {
          dest = mi[srcIx]->GetData().Cast<IContributionItem>();
        }
        else
        {
          dest = 0;
        }

        if (dest && src == dest)
        {
          srcIx++;
          destIx++;
        }
        else if (dest && dest->IsSeparator()
            && src->IsSeparator())
        {
          mi[srcIx]->SetData(src);
          srcIx++;
          destIx++;
        }
        else
        {
          unsigned int start = this->GetMenuItemCount();
          this->DoItemFill(src, destIx);
          unsigned int newItems = this->GetMenuItemCount() - start;
          for (unsigned int i = 0; i < newItems; i++)
          {
            IMenuItem::Pointer item(this->GetMenuItem(destIx++));
            item->SetData(src);
          }
        }

        // May be we can optimize this call. If the menu has just
        // been created via the call src.fill(fMenuBar, destIx) then
        // the menu has already been updated with update(true)
        // (see MenuManager). So if force is true we do it again. But
        // we can't set force to false since then information for the
        // sub sub menus is lost.
        if (recursive)
        {
          IContributionItem::Pointer item(src);
          if (SubContributionItem::Pointer subItem = item.Cast<SubContributionItem>())
          {
            item = subItem->GetInnerItem();
          }
          if (IMenuManager::Pointer mm = item.Cast<IMenuManager>())
          {
            mm->UpdateAll(force);
          }
        }

      }

      // remove any old menu items not accounted for
      for (; srcIx < mi.size(); srcIx++)
      {
        mi[srcIx]->Dispose();
      }

      this->SetDirty(false);
    }
  }
  else
  {
    // I am not dirty. Check if I must recursivly walk down the hierarchy.
    if (recursive)
    {
      std::vector<IContributionItem::Pointer> items(this->GetItems());
      for (unsigned int i = 0; i < items.size(); ++i)
      {
        IContributionItem::Pointer ci(items[i]);
        if (IMenuManager::Pointer mm = ci.Cast<IMenuManager>())
        {
          if (mm->IsVisible())
          {
            mm->UpdateAll(force);
          }
        }
      }
    }
  }
  this->UpdateMenuItem();
}

}

