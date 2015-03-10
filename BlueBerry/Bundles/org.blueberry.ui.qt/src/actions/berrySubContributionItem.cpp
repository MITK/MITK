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

#include "berrySubContributionItem.h"

namespace berry
{

SubContributionItem::SubContributionItem(IContributionItem::Pointer item)
: visible(true), innerItem(item)
{

}

void SubContributionItem::Fill(QStatusBar* parent)
{
  if (visible)
  {
    innerItem->Fill(parent);
  }
}

QAction *SubContributionItem::Fill(QMenu *parent, QAction *before)
{
  if (visible)
  {
    return innerItem->Fill(parent, before);
  }
  return 0;
}

QAction *SubContributionItem::Fill(QToolBar *parent, QAction *before)
{
  if (visible)
  {
    return innerItem->Fill(parent, before);
  }
  return 0;
}

QString SubContributionItem::GetId() const
{
  return innerItem->GetId();
}

IContributionItem::Pointer SubContributionItem::GetInnerItem() const
{
  return innerItem;
}

bool SubContributionItem::IsEnabled() const
{
  return innerItem->IsEnabled();
}

bool SubContributionItem::IsDirty() const
{
  return innerItem->IsDirty();
}

bool SubContributionItem::IsDynamic() const
{
  return innerItem->IsDynamic();
}

bool SubContributionItem::IsGroupMarker() const
{
  return innerItem->IsGroupMarker();
}

bool SubContributionItem::IsSeparator() const
{
  return innerItem->IsSeparator();
}

bool SubContributionItem::IsVisible() const
{
  return visible && innerItem->IsVisible();
}

void SubContributionItem::SetParent(IContributionManager* /*parent*/)
{
  // do nothing, the parent of our inner item
  // is its SubContributionManager
}

void SubContributionItem::SetVisible(bool visible)
{
  this->visible = visible;
}

void SubContributionItem::Update()
{
  innerItem->Update();
}

void SubContributionItem::Update(const QString& id)
{
  innerItem->Update(id);
}

void SubContributionItem::SaveWidgetState()
{
}

}
