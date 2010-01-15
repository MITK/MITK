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

#include "cherryContributionItem.h"
#include "cherryIContributionManager.h"
#include "cherryIMenu.h"
#include "cherryIToolBar.h"

namespace cherry
{

ContributionItem::ContributionItem(const std::string& id) :
  id(id), visible(true)
{

}

void ContributionItem::Dispose()
{
}

void ContributionItem::Fill(void* parent)
{
}

void ContributionItem::Fill(SmartPointer<IMenu> menu, int index)
{
}

void ContributionItem::Fill(SmartPointer<IToolBar> parent, int index)
{
}

void ContributionItem::SaveWidgetState()
{
}

std::string ContributionItem::GetId() const
{
  return id;
}

IContributionManager::Pointer ContributionItem::GetParent()
{
  return IContributionManager::Pointer(parent);
}

bool ContributionItem::IsDirty()
{
  // @issue should this be false instead of calling isDynamic()?
  return this->IsDynamic();
}

bool ContributionItem::IsEnabled()
{
  return true;
}

bool ContributionItem::IsDynamic()
{
  return false;
}

bool ContributionItem::IsGroupMarker()
{
  return false;
}

bool ContributionItem::IsSeparator()
{
  return false;
}

bool ContributionItem::IsVisible()
{
  return visible;
}

void ContributionItem::SetVisible(bool visible)
{
  this->visible = visible;
}

void ContributionItem::PrintSelf(std::ostream& os, Indent indent) const
{
  os << indent << "ContributionItem (id=" << this->GetId() << ")";
}

void ContributionItem::Update()
{
}

void ContributionItem::SetParent(IContributionManager::Pointer parent)
{
  this->parent = parent;
}

void ContributionItem::Update(const std::string& id)
{
}

void ContributionItem::SetId(const std::string& itemId)
{
  id = itemId;
}

}
