/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryMenuManager.h"

#include "berryIContributionManagerOverrides.h"
#include "berryQActionProperties.h"
#include "berrySubContributionItem.h"

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

  QMenuProxy(Type type, QWidget* parent = nullptr)
    : menu(nullptr), menuBar(nullptr)
  {
    switch (type)
    {
    case MenuBar: menuBar = new QMenuBar(parent); break;
    case Menu: menu = new QMenu(parent); break;
    }
  }

  bool isMenuBar() const
  {
    return menuBar != nullptr;
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

  QAction* getParentItem() const
  {
    return menu ? menu->menuAction() : nullptr;
  }

};

namespace berry
{

struct NullOverrides: public IContributionManagerOverrides
{
public:

  int GetEnabled(const IContributionItem* /*item*/) const override
  {
    return -1;
  }

  int GetVisible(const IContributionItem* /*item*/) const override
  {
    return -1;
  }
};

void MenuManager::HandleAboutToShow()
{
  if (this->removeAllWhenShown)
  {
    this->RemoveAll();
  }
  emit AboutToShow(this);
  this->Update(false, false);
}

void MenuManager::HandleAboutToHide()
{
  emit AboutToHide(this);
}

MenuManager::MenuManager(const QString& text, const QString& id)
  : id(id), menu(nullptr), menuItem(nullptr)
  , menuText(text), parent(nullptr)
  , removeAllWhenShown(false), visible(true)
{

}

MenuManager::MenuManager(const QString& text, const QIcon& image, const QString& id)
  : id(id), menu(nullptr), menuItem(nullptr)
  , menuText(text), image(image)
  , parent(nullptr), removeAllWhenShown(false), visible(true)
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

void MenuManager::AddMenuListener(QObject* listener)
{
  this->connect(this, SIGNAL(AboutToShow(IMenuManager*)), listener, SLOT(MenuAboutToShow(IMenuManager*)));
  this->connect(this, SIGNAL(AboutToHide(IMenuManager*)), listener, SLOT(MenuAboutToHide(IMenuManager*)));
}

void MenuManager::RemoveMenuListener(QObject* listener)
{
  this->disconnect(listener);
}

void MenuManager::Fill(QStatusBar* /*parent*/)
{
}

void MenuManager::Fill(QToolBar* /*parent*/, QAction* /*before*/)
{
}

void MenuManager::Fill(QMenu* parent, QAction* before)
{
  this->FillMenu(parent, before);
}

void MenuManager::Fill(QMenuBar* parent, QAction* before)
{
  this->FillMenu(parent, before);
}

void MenuManager::FillMenu(QWidget* parent, QAction* before)
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
}

IMenuManager::Pointer MenuManager::FindMenuUsingPath(const QString& path) const
{
  IContributionItem::Pointer item(this->FindUsingPath(path));
  if (IMenuManager::Pointer manager = item.Cast<IMenuManager>())
  {
    return manager;
  }
  return IMenuManager::Pointer(nullptr);
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
  return IContributionItem::Pointer(nullptr);
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

void MenuManager::SetCommandId(const QString& definitionId)
{
  this->definitionId = definitionId;
}

void MenuManager::Update()
{
  this->UpdateMenuItem();
}

void MenuManager::Update(const QString& property)
{
  QList<IContributionItem::Pointer> items = GetItems();

  for (int i = 0; i < items.size(); i++)
  {
    items[i]->Update(property);
  }

  if (menu != nullptr && menu->getParentItem() != nullptr)
  {
    if (QActionProperties::TEXT == property)
    {
      QString text = GetMenuText();
      if (!text.isNull())
      {
        menu->getParentItem()->setText(text);
      }
    }
    else if (QActionProperties::IMAGE == property && !image.isNull())
    {
      menu->getParentItem()->setIcon(image);
    }
  }
}

void MenuManager::Update(bool force)
{
  this->Update(force, false);
}

void MenuManager::UpdateAll(bool force)
{
  this->Update(force, true);
}

void MenuManager::InitializeMenu()
{
  menu->setTitle(GetMenuText());
  menu->setIcon(GetImage());

  if (!menu->isMenuBar())
  {
    this->connect(menu->menu, SIGNAL(aboutToShow()), SLOT(HandleAboutToShow()));
    this->connect(menu->menu, SIGNAL(aboutToHide()), SLOT(HandleAboutToHide()));
  }

  // Don't do an update(true) here, in case menu is never opened.
  // Always do it lazily in handleAboutToShow().
}

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
            separator = nullptr;
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
          dest = nullptr;
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
          //qDebug() << "***** Filling item destIx = " << destIx << " (size: " << start << ")";
          this->DoItemFill(src, destIx >= start ? nullptr : menu->actions().at(destIx));
          int newItems = menu->actions().size() - start;
          //qDebug() << "***** New items: " << newItems;
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
    //qDebug() << "QMenuBar [" << menu->menuBar << "]";
    DumpActionInfo(menu->menuBar, 1);
  }
  else
  {
    //qDebug() << "QMenu [" << menu->menu << "]" << menu->menu;
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
