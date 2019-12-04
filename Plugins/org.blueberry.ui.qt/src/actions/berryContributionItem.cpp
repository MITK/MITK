/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#include "berryContributionItem.h"

#include "berryIContributionManager.h"

namespace berry {

void ContributionItem::Fill(QStatusBar* /*parent*/)
{
}

void ContributionItem::Fill(QMenu* /*menu*/, QAction * /*before*/)
{
}

void ContributionItem::Fill(QMenuBar* /*menu*/, QAction* /*before*/)
{
}

void ContributionItem::Fill(QToolBar* /*parent*/, QAction * /*before*/)
{
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
  : visible(true), parent(nullptr)
{
}

ContributionItem::ContributionItem(const QString& id)
  : id(id), visible(true), parent(nullptr)
{
}

}
