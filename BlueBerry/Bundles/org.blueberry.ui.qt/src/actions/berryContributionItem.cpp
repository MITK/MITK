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


#include "berryContributionItem.h"

#include "berryIContributionManager.h"

namespace berry {

void ContributionItem::Fill(QStatusBar* /*parent*/)
{
}

QAction* ContributionItem::Fill(QMenu* /*menu*/, QAction * /*before*/)
{
  return 0;
}

QAction *ContributionItem::Fill(QMenuBar* /*menu*/, QAction* /*before*/)
{
  return 0;
}

QAction* ContributionItem::Fill(QToolBar* /*parent*/, QAction * /*before*/)
{
  return 0;
}

void ContributionItem::SaveWidgetState()
{
}

QString ContributionItem::GetId() const
{
  return id;
}

IContributionManager* ContributionItem::GetParent() const
{
  return parent;
}

bool ContributionItem::IsDirty() const
{
  // @issue should this be false instead of calling isDynamic()?
  return IsDynamic();
}

bool ContributionItem::IsEnabled() const
{
  return true;
}

bool ContributionItem::IsDynamic() const
{
  return false;
}

bool ContributionItem::IsGroupMarker() const
{
  return false;
}

bool ContributionItem::IsSeparator() const
{
  return false;
}

bool ContributionItem::IsVisible() const
{
  return visible;
}

void ContributionItem::SetVisible(bool visible)
{
  this->visible = visible;
}

QString ContributionItem::ToString() const
{
  return QString(GetClassName()) + "(id=" + this->GetId() + ")";
}

void ContributionItem::Update()
{
}

void ContributionItem::SetParent(IContributionManager* parent)
{
  this->parent = parent;
}

void ContributionItem::Update(const QString& /*id*/)
{
}

void ContributionItem::SetId(const QString& itemId)
{
  id = itemId;
}

ContributionItem::ContributionItem()
  : visible(true), parent(0)
{
}

ContributionItem::ContributionItem(const QString& id)
  : id(id), visible(true), parent(0)
{
}

}
