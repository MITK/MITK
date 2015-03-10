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

#include "berryMenuManager.h"

#include "berryIContributionManagerOverrides.h"
#include "berrySubContributionItem.h"

#include "internal/berryMMMenuListener.h"

#include <QMenu>
#include <QMenuBar>


class QMenuProxy
{

public:

  QMenu* menu;
  QMenuBar* menuBar;

  enum Type {
    MenuBar,
    Menu
  };

  QMenuProxy(Type type, QWidget* parent = 0)
    : menu(0), menuBar(0)
  {
    switch (type)
    {
    case MenuBar: menuBar = new QMenuBar(parent);
    case Menu: menu = new QMenu(parent);
    }
  }

  bool isMenuBar() const
  {
    return menuBar != 0;
  }

  void setTitle(const QString& title)
  {
    if (menu)
      menu->setTitle(title);
  }

  void setIcon(const QIcon& icon)
  {
    if (menu)
      menu->setIcon(icon);
  }

  QList<QAction*> actions() const
  {
    return menu ? menu->actions() : menuBar->actions();
  }

  void removeAction(QAction* action)
  {
    menu ? menu->removeAction(action) : menuBar->removeAction(action);
  }

  bool isEnabled() const
  {
    return menu ? menu->isEnabled() : menuBar->isEnabled();
  }

  void setEnabled(bool enabled)
  {
    menu ? menu->setEnabled(enabled) : menuBar->setEnabled(enabled);
  }

};

namespace berry
{

struct NullOverrides: public IContributionManagerOverrides
{
public:

  int GetEnabled(const IContributionItem* /*item*/) const
  {
    return -1;
  }

  int GetVisible(const IContributionItem* /*item*/) const
  {
    return -1;
  }
};

//struct GuiTkMenuListener : public GuiTk::IMenuListener
//{
//public:

//  GuiTkMenuListener(MenuManager::Pointer manager) :
//    manager(manager)
//  {

//  }

//  void MenuAboutToHide()
//  {
//    //      ApplicationWindow.resetDescription(e.widget);
//    MenuManager::Pointer(manager)->HandleAboutToHide();
//  }

//  void MenuAboutToShow()
//  {
//    MenuManager::Pointer(manager)->HandleAboutToShow();
//  }

//private:

//  MenuManager::WeakPtr manager;
//};

MenuManager::MenuManager(const QString& text, const QString& id)
  : id(id), menu(0), menuItem(0), menuListener(new MMMenuListener(this))
  , menuText(text), parent(0)
  , removeAllWhenShown(false), visible(true)
{

}

MenuManager::MenuManager(const QString& text, const QIcon& image, const QString& id)
  : id(id), menu(0), menuItem(0), menuListener(new MMMenuListener(this))
  , menuText(text), image(image)
  , parent(0), removeAllWhenShown(false), visible(true)
{

}

bool MenuManager::IsDirty() const
{
  return ContributionManager::IsDirty();
}

MenuManager::~MenuManager()
{
  delete menu;
}

//void MenuManager::AddMenuListener(SmartPointer<IMenuListener> listener)
//{
//  menuEvents.AddListener(listener);
//}

QMenu* MenuManager::CreateContextMenu(QWidget* parent)
{
  if (!menu)
  {
    menu = new QMenuProxy(QMenuProxy::Menu, parent);
    this->InitializeMenu();
  }
  return menu->menu;
}

QMenuBar* MenuManager::CreateMenuBar(QWidget* parent)
{
  if (!menu)
  {
    menu = new QMenuProxy(QMenuProxy::MenuBar, parent);
    this->Update(false);
  }
  return menu->menuBar;
}

void MenuManager::Fill(QStatusBar* /*parent*/)
{
}

QAction* MenuManager::Fill(QToolBar* /*parent*/, QAction* /*before*/)
{
  return 0;
}

QAction* MenuManager::Fill(QMenu* parent, QAction* before)
{
   return this->FillMenu(parent, before);
}

QAction* MenuManager::Fill(QMenuBar* parent, QAction* before)
{
  return this->FillMenu(parent, before);
}

QAction* MenuManager::FillMenu(QWidget* parent, QAction* before)
{
  if (!menuItem)
  {
    menuItem = new QAction(parent);
    if (parent)
    {
      parent->insertAction(before, menuItem);
    }

    menuItem->setText(GetMenuText());
    menuItem->setIcon(image);

    if(!menu)
    {
      menu = new QMenuProxy(QMenuProxy::Menu, parent);
    }

    if (!menu->isMenuBar())
      menuItem->setMenu(menu->menu);

    this->InitializeMenu();

    this->SetDirty(true);
  }
  return menuItem;
}

IMenuManager::Pointer MenuManager::FindMenuUsingPath(const QString& path) const
{
  IContributionItem::Pointer item(this->FindUsingPath(path));
  if (IMenuManager::Pointer manager = item.Cast<IMenuManager>())
  {
    return manager;
  }
  return IMenuManager::Pointer(0);
}

IContributionItem::Pointer MenuManager::FindUsingPath(const QString& path) const
{
  QString id(path);
  QString rest;
  int separator = path.indexOf('/');
  if (separator != -1)
  {
    id = path.left(separator);
    rest = path.mid(separator + 1);
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

QString MenuManager::GetId() const
{
  return id;
}

QMenu* MenuManager::GetMenu() const
{
  return menu->menu;
}

QString MenuManager::GetMenuText() const
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

QIcon MenuManager::GetImage() const
{
  return image;
}

SmartPointer<IContributionManagerOverrides> MenuManager::GetOverrides()
{
  if (!overrides)
  {
    if (!parent)
    {
      overrides = new NullOverrides();
    }
    else
    {
      overrides = parent->GetOverrides();
    }
    ContributionManager::SetOverrides(overrides);
  }
  return overrides;
}

IContributionManager* MenuManager::GetParent() const
{
  return parent;
}

bool MenuManager::GetRemoveAllWhenShown() const
{
  return removeAllWhenShown;
}

bool MenuManager::IsDynamic() const
{
  return false;
}

bool MenuManager::IsEnabled() const
{
  return true;
}

bool MenuManager::IsGroupMarker() const
{
  return false;
}

bool MenuManager::IsSeparator() const
{
  return false;
}

bool MenuManager::IsVisible() const
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
  bool visibleChildren = false;
  foreach(IContributionItem::Pointer item, this->GetItems())
  {
    if (item->IsVisible() && !item->IsSeparator())
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
  IContributionManager* p = this->GetParent();
  if (p)
  {
    p->MarkDirty();
  }
}

//void MenuManager::RemoveMenuListener(SmartPointer<IMenuListener> listener)
//{
//  menuEvents.RemoveListener(listener);
//}

void MenuManager::SaveWidgetState()
{
}

void MenuManager::SetOverrides(SmartPointer<IContributionManagerOverrides> newOverrides)
{
  overrides = newOverrides;
  ContributionManager::SetOverrides(overrides);
}

void MenuManager::SetParent(IContributionManager* manager)
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

void MenuManager::SetActionDefinitionId(const QString& definitionId)
{
  this->definitionId = definitionId;
}

void MenuManager::Update()
{
  this->UpdateMenuItem();
}

void MenuManager::Update(const QString& /*property*/)
{

}

void MenuManager::Update(bool force)
{
  this->Update(force, false);
}

void MenuManager::UpdateAll(bool force)
{
  this->Update(force, true);
}

//void MenuManager::FireAboutToShow(IMenuManager::Pointer manager)
//{
//  menuEvents.menuAboutToShow(manager);
//}

//void MenuManager::FireAboutToHide(IMenuManager::Pointer manager)
//{
//  menuEvents.menuAboutToHide(manager);
//}

//void MenuManager::HandleAboutToShow()
//{
//  if (removeAllWhenShown)
//  {
//    this->RemoveAll();
//  }
//  this->FireAboutToShow(IMenuManager::Pointer(this));
//  this->Update(false, false);
//}

//void MenuManager::HandleAboutToHide()
//{
//  this->FireAboutToHide(IMenuManager::Pointer(this));
//}

void MenuManager::InitializeMenu()
{
  menu->setTitle(GetMenuText());
  menu->setIcon(GetImage());

  //menuListener = new GuiTkMenuListener(MenuManager::Pointer(this));
  if (!menu->isMenuBar())
  {
    QObject::connect(menu->menu, SIGNAL(aboutToShow()), menuListener.data(), SLOT(HandleAboutToShow()));
  }
  //menu->AddMenuListener(menuListener);

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
  if (menuItem && menu)
  {
    bool enabled = removeAllWhenShown || menu->actions().size() > 0;
    if (menu->isEnabled() != enabled)
    {
      menuItem->setEnabled(enabled);
    }
  }
}

//QList<SmartPointer<IMenuItem> > MenuManager::GetMenuItems()
//{
//  if (menu)
//  {
//    return menu->GetItems();
//  }
//  return QList<IMenuItem::Pointer>();
//}

//SmartPointer<IMenuItem> MenuManager::GetMenuItem(unsigned int index)
//{
//  if (menu)
//  {
//    return menu->GetItem(index);
//  }
//  return IMenuItem::Pointer(0);
//}

//unsigned int MenuManager::GetMenuItemCount()
//{
//  if (menu)
//  {
//    return menu->GetItemCount();
//  }
//  return 0;
//}

void MenuManager::DoItemFill(IContributionItem::Pointer ci, QAction* before)
{
  if (menu->isMenuBar())
  {
    ci->Fill(menu->menuBar, before);
  }
  else
  {
    ci->Fill(menu->menu, before);
  }
}

void MenuManager::Update(bool force, bool recursive)
{
  if (ContributionManager::IsDirty() || force)
  {
    if (menu)
    {
      // clean contains all active items without double separators
      QList<IContributionItem::Pointer> items = this->GetItems();
      QList<IContributionItem::Pointer> clean;
      IContributionItem::Pointer separator;
      foreach (IContributionItem::Pointer ci, items)
      {
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
      QList<QAction*> mi = menu->actions();

      for (int i = 0; i < mi.size(); i++)
      {
        Object::Pointer data = mi[i]->data().value<Object::Pointer>();

        if (!data || !clean.contains(data.Cast<IContributionItem>()))
        {
          menu->removeAction(mi[i]);
        }
        else if (IContributionItem::Pointer ci = data.Cast<IContributionItem>())
        {
          if(ci->IsDynamic() && ci->IsDirty())
          {
            menu->removeAction(mi[i]);
            delete mi[i];
          }
        }
      }

      // add new
      mi = menu->actions();
      int srcIx = 0;
      int destIx = 0;

      for (QList<IContributionItem::Pointer>::Iterator e = clean.begin();
           e != clean.end(); ++e)
      {
        IContributionItem::Pointer src(*e);
        IContributionItem::Pointer dest;

        // get corresponding item in widget
        if (srcIx < mi.size())
        {
          dest = mi[srcIx]->data().value<Object::Pointer>().Cast<IContributionItem>();
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
        else if (dest && dest->IsSeparator() && src->IsSeparator())
        {
          mi[srcIx]->setData(QVariant::fromValue<Object::Pointer>(src));
          srcIx++;
          destIx++;
        }
        else
        {
          int start = menu->actions().size();
          qDebug() << "***** Filling item destIx = " << destIx << " (size: " << start << ")";
          this->DoItemFill(src, destIx >= start ? NULL : menu->actions().at(destIx));
          int newItems = menu->actions().size() - start;
          qDebug() << "***** New items: " << newItems;
          for (int i = 0; i < newItems; ++i)
          {
            menu->actions().at(destIx++)->setData(QVariant::fromValue<Object::Pointer>(src));
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
        menu->removeAction(mi[srcIx]);
        delete mi[srcIx];
      }

      this->SetDirty(false);
    }
  }
  else
  {
    // I am not dirty. Check if I must recursivly walk down the hierarchy.
    if (recursive)
    {
      foreach (IContributionItem::Pointer ci, this->GetItems())
      {
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

void MenuManager::DumpActionInfo(QMenuProxy* menu)
{
  if (menu->isMenuBar())
  {
    qDebug() << "QMenuBar [" << menu->menuBar << "]";
    DumpActionInfo(menu->menuBar, 1);
  }
  else
  {
    qDebug() << "QMenu [" << menu->menu << "]" << menu->menu;
    DumpActionInfo(menu->menu, 1);
  }
}

void MenuManager::DumpActionInfo(QWidget* widget, int level)
{
  QString indent = " |";
  for (int i = 0; i < level; ++i) indent += "--";
  foreach(QAction* action, widget->actions())
  {
    qDebug() << qPrintable(indent) << action->text() << "[" << action << "]";
    if (action->menu())
    {
      DumpActionInfo(action->menu(), level+1);
    }
  }
}

}
