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

#include "cherryContributionManager.h"

#include "cherryIContributionItem.h"

#include <Poco/String.h>
#include <sstream>
#include <algorithm>

namespace cherry {

ObjectBool::Pointer ContributionManager::ContributionManagerOverrides::GetEnabled(IContributionItem::Pointer /*item*/) {
  return ObjectBool::Pointer(0);
}

ObjectInt::Pointer ContributionManager::ContributionManagerOverrides::GetAccelerator(IContributionItem::Pointer /*item*/) {
  return ObjectInt::Pointer(0);
}

ObjectString::Pointer ContributionManager::ContributionManagerOverrides::GetAcceleratorText(IContributionItem::Pointer /*item*/) {
  return ObjectString::Pointer(0);
}

ObjectString::Pointer ContributionManager::ContributionManagerOverrides::GetText(IContributionItem::Pointer /*item*/) {
  return ObjectString::Pointer(0);
}

ContributionManager::ContributionManager()
: isDirty(true), dynamicItems(0)
{
    // Do nothing.
  }

  bool ContributionManager::AllowItem(SmartPointer<IContributionItem> /*itemToAdd*/) {
    return true;
  }

  void ContributionManager::DumpStatistics() {
    int size = contributions.size();

    this->Print(std::cout);
    std::cout << "   Number of elements: " << size << std::endl;
    int sum = 0;
    for (IContributionContainer::iterator iter = contributions.begin();
      iter != contributions.end(); ++iter)
    {
      if ((*iter)->IsVisible()) ++sum;
    }
    std::cout << "   Number of visible elements: " << sum << std::endl;
    std::cout << "   Is dirty: " << this->IsDirty() << std::endl;
  }

  bool ContributionManager::HasDynamicItems() {
    return (dynamicItems > 0);
  }

  ContributionManager::IContributionContainer::iterator
  ContributionManager::FindIter(const std::string& id)
  {
    for (IContributionContainer::iterator iter = contributions.begin();
      iter != contributions.end(); ++iter)
    {
      IContributionItem::Pointer item = *iter;
      std::string itemId = item->GetId();
      if (Poco::icompare(itemId, id) == 0) {
        return iter;
      }
    }
    return contributions.end();
  }

  int ContributionManager::IndexOf(SmartPointer<IContributionItem> item) {
    IContributionContainer::iterator iter = std::find(contributions.begin(),
        contributions.end(), item);

    return iter != contributions.end() ? (iter - contributions.begin()) : -1;
  }

  void ContributionManager::ItemAdded(SmartPointer<IContributionItem> item) {
    item->SetParent(IContributionManager::Pointer(this));
    this->MarkDirty();
    if (item->IsDynamic()) {
      ++dynamicItems;
    }
  }

  void ContributionManager::ItemRemoved(SmartPointer<IContributionItem> item) {
    item->SetParent(IContributionManager::Pointer(0));
    this->MarkDirty();
    if (item->IsDynamic()) {
      --dynamicItems;
    }
  }

  void ContributionManager::SetDirty(bool dirty) {
    isDirty = dirty;
  }

  void ContributionManager::InternalSetItems(const std::vector<SmartPointer<IContributionItem> >& items) {
    contributions.clear();
    for (unsigned int i = 0; i < items.size(); i++) {
      if (this->AllowItem(items[i])) {
        contributions.push_back(items[i]);
      }
    }
  }

  void ContributionManager::Add(SmartPointer<IContributionItem> item) {
    poco_assert(item != 0); // "Item must not be null"; //$NON-NLS-1$
    if (this->AllowItem(item)) {
      contributions.push_back(item);
      this->ItemAdded(item);
    }
  }

  void ContributionManager::AppendToGroup(const std::string& groupName, SmartPointer<IContributionItem> item) {
    this->AddToGroup(groupName, item, true);
  }

  IContributionItem::Pointer ContributionManager::Find(const std::string& id) {
    IContributionContainer::iterator iter = this->FindIter(id);
    if (iter == contributions.end())
      return IContributionItem::Pointer(0);

    return *iter;
  }

  std::vector<SmartPointer<IContributionItem> > ContributionManager::GetItems() {
    return std::vector<IContributionItem::Pointer>(contributions.begin(), contributions.end());
  }

  int ContributionManager::GetSize() {
    return contributions.size();
  }

  SmartPointer<IContributionManagerOverrides> ContributionManager::GetOverrides() {
    if (overrides == 0) {
      overrides = new ContributionManagerOverrides();
    }
    return overrides;
  }

  int ContributionManager::IndexOf(const std::string& id) {
    IContributionContainer::iterator iter = this->FindIter(id);
    if (iter == contributions.end()) return -1;

    return iter - contributions.begin();
  }

  void ContributionManager::Insert(int index, SmartPointer<IContributionItem> item) {
    if (index > contributions.size()) {
      std::stringstream msg;
      msg << "inserting " << item->GetId() << " at " << index;
      throw std::out_of_range(msg.str());
    }
    if (this->AllowItem(item)) {
      IContributionContainer::iterator insertLoc = contributions.begin() + index;
      contributions.insert(insertLoc, item);
      this->ItemAdded(item);
    }
  }

  void ContributionManager::InsertAfter(const std::string& ID, SmartPointer<IContributionItem> item) {
    IContributionContainer::iterator ci = this->FindIter(ID);
    if (ci == contributions.end()) {
      std::stringstream msg;
      msg << "can't find ID" << ID;
      throw std::invalid_argument(msg.str());
    }
    // System.out.println("insert after: " + ix);
    if (this->AllowItem(item)) {
      contributions.insert(++ci, item);
      this->ItemAdded(item);
    }
  }

  void ContributionManager::InsertBefore(const std::string& ID, SmartPointer<IContributionItem> item) {
    IContributionContainer::iterator ci = this->FindIter(ID);
    if (ci == contributions.end()) {
      std::stringstream msg;
      msg << "can't find ID" << ID;
      throw std::invalid_argument(msg.str());//$NON-NLS-1$
    }
    // System.out.println("insert before: " + ix);
    if (this->AllowItem(item)) {
      contributions.insert(ci, item);
      this->ItemAdded(item);
    }
  }

  bool ContributionManager::IsDirty() {
    if (isDirty) {
      return true;
    }
    if (this->HasDynamicItems()) {
      for (IContributionContainer::iterator iter = contributions.begin();
           iter != contributions.end(); ++iter)
      {
        if ((*iter)->IsDirty()) {
          return true;
        }
      }
    }
    return false;
  }

  bool ContributionManager::IsEmpty() {
    return contributions.empty();
  }

  void ContributionManager::MarkDirty() {
    this->SetDirty(true);
  }

  void ContributionManager::PrependToGroup(const std::string& groupName, SmartPointer<IContributionItem> item) {
    this->AddToGroup(groupName, item, false);
  }

  SmartPointer<IContributionItem> ContributionManager::Remove(const std::string& ID) {
    IContributionContainer::iterator ci = this->FindIter(ID);
    if (ci == contributions.end()) {
      return IContributionItem::Pointer(0);
    }
    IContributionItem::Pointer removedItem = *ci;
    contributions.erase(ci);
    this->ItemRemoved(removedItem);
    return removedItem;
  }

  SmartPointer<IContributionItem> ContributionManager::Remove(SmartPointer<IContributionItem> item) {
    IContributionContainer::iterator ci = std::find(contributions.begin(),
      contributions.end(), item);
    if (ci == contributions.end())
      return IContributionItem::Pointer(0);

    IContributionItem::Pointer removedItem = *ci;
    contributions.erase(ci);
    this->ItemRemoved(item);
    return removedItem;
  }

  void ContributionManager::RemoveAll() {
    std::vector<IContributionItem::Pointer> items = this->GetItems();
    contributions.clear();
    for (unsigned int i = 0; i < items.size(); ++i) {
      this->ItemRemoved(items[i]);
    }
    dynamicItems = 0;
    this->MarkDirty();
  }

  bool ContributionManager::ReplaceItem(const std::string& identifier,
      SmartPointer<IContributionItem> replacementItem) {

    IContributionContainer::iterator index = this->FindIter(identifier);
    if (index == contributions.end()) {
      return false; // couldn't find the item.
    }

    // Remove the old item.
    IContributionItem::Pointer oldItem = *index;
    this->ItemRemoved(oldItem);

    // Add the new item.
    *index = replacementItem;
    this->ItemAdded(replacementItem); // throws NPE if (replacementItem == null)

    // Go through and remove duplicates.
    for (IContributionContainer::iterator iter = index; iter != contributions.end(); )
    {
      IContributionItem::Pointer item = *iter;
      if (item && identifier == item->GetId())
      {
//        if (Policy.TRACE_TOOLBAR) {
//          System.out
//              .println("Removing duplicate on replace: " + identifier); //$NON-NLS-1$
//        }
        iter = contributions.erase(iter);
        this->ItemRemoved(item);
      }
      else
        ++iter;
    }

    return true; // success
  }

  void ContributionManager::SetOverrides(SmartPointer<IContributionManagerOverrides> newOverrides) {
    overrides = newOverrides;
  }

  void ContributionManager::AddToGroup(const std::string& groupName, SmartPointer<IContributionItem> item,
      bool append) {
    for (IContributionContainer::iterator iter = contributions.begin(); iter != contributions.end(); ++iter)
    {
      IContributionItem::Pointer o = *iter;
      if (o->IsGroupMarker()) {
        std::string id = o->GetId();
        if (Poco::icompare(id, groupName) == 0)
        {
          ++iter;
          if (append) {
            for (; iter != contributions.end(); ++iter) {
              IContributionItem::Pointer ci = *(++iter);
              if (ci->IsGroupMarker())
              {
                break;
              }
            }
          }
          if (this->AllowItem(item)) {
            contributions.insert(iter, item);
            this->ItemAdded(item);
          }
          return;
        }
      }
    }
    std::stringstream msg;
    msg << "Group not found: " << groupName;
    throw std::invalid_argument(msg.str());
  }

}
