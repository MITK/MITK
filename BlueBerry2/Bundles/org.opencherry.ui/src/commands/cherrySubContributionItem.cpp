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

#include "cherrySubContributionItem.h"

#include "cherryIMenu.h"
#include "cherryIToolBar.h"
#include "cherryIContributionManager.h"

namespace cherry
{

SubContributionItem::SubContributionItem(IContributionItem::Pointer item)
: visible(true), innerItem(item)
{

}

void SubContributionItem::Dispose()
{
  innerItem->Dispose();
}

void SubContributionItem::Fill(void* parent)
{
  if (visible)
  {
    innerItem->Fill(parent);
  }
}

void SubContributionItem::Fill(SmartPointer<IMenu> parent, int index)
{
  if (visible)
  {
    innerItem->Fill(parent, index);
  }
}

void SubContributionItem::Fill(SmartPointer<IToolBar> parent, int index)
{
  if (visible)
  {
    innerItem->Fill(parent, index);
  }
}

std::string SubContributionItem::GetId() const
{
  return innerItem->GetId();
}

IContributionItem::Pointer SubContributionItem::GetInnerItem()
{
  return innerItem;
}

bool SubContributionItem::IsEnabled()
{
  return innerItem->IsEnabled();
}

bool SubContributionItem::IsDirty()
{
  return innerItem->IsDirty();
}

bool SubContributionItem::IsDynamic()
{
  return innerItem->IsDynamic();
}

bool SubContributionItem::IsGroupMarker()
{
  return innerItem->IsGroupMarker();
}

bool SubContributionItem::IsSeparator()
{
  return innerItem->IsSeparator();
}

bool SubContributionItem::IsVisible()
{
  return visible && innerItem->IsVisible();
}

void SubContributionItem::SetParent(SmartPointer<IContributionManager> parent)
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

void SubContributionItem::Update(const std::string& id)
{
  innerItem->Update(id);
}

void SubContributionItem::SaveWidgetState()
{
}

}
