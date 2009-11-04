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

#include <cherryIExtensionPointService.h>

#include "cherryViewRegistry.h"

#include "../cherryPlatformUI.h"
#include "../cherryImageDescriptor.h"
#include "cherryWorkbenchPlugin.h"
#include "cherryWorkbenchRegistryConstants.h"

#include "cherryPlatform.h"

namespace cherry
{

ViewRegistry::ViewCategoryProxy::ViewCategoryProxy(
    IViewDescriptorCategoryPtr rawCategory) :
  rawCategory(rawCategory)
{
}

const std::vector<IViewDescriptor::Pointer>& ViewRegistry::ViewCategoryProxy::GetViews() const
{
  return rawCategory->GetElements();
}

const std::string& ViewRegistry::ViewCategoryProxy::GetId() const
{
  return rawCategory->GetId();
}

std::vector<std::string> ViewRegistry::ViewCategoryProxy::GetPath() const
{
  std::vector<std::string> path;
  std::string rawParentPath = rawCategory->GetRawParentPath();
  // nested categories are not supported yet
  // assume an empty raw parent path
  path.push_back(rawParentPath);
  return path;
}

std::string ViewRegistry::ViewCategoryProxy::GetLabel() const
{
  return rawCategory->GetLabel();
}

bool ViewRegistry::ViewCategoryProxy::operator==(const Object* o) const
{
  if (const IViewCategory* other = dynamic_cast<const IViewCategory*>(o))
    return this->GetId() == other->GetId();

  return false;
}

int ViewRegistry::ViewCategoryProxy::HashCode()
{
  //return GetId().hashCode();
  return 0;
}

std::string ViewRegistry::EXTENSIONPOINT_UNIQUE_ID = "org.opencherry.ui.views";
    // PlatformUI::PLUGIN_ID + "." + WorkbenchRegistryConstants::PL_VIEWS;


Category<IViewDescriptor::Pointer>::Pointer ViewRegistry::InternalFindCategory(const std::string& id)
{
  for (std::vector<IViewDescriptorCategoryPtr>::iterator itr = categories.begin();
       itr != categories.end(); ++itr)
  {
    if (id == (*itr)->GetRootPath())
    {
      return *itr;
    }
  }
  return IViewDescriptorCategoryPtr(0);
}

const IExtensionPoint* ViewRegistry::GetExtensionPointFilter()
{
  return Platform::GetExtensionPointService()->GetExtensionPoint(EXTENSIONPOINT_UNIQUE_ID);
}

const std::string ViewRegistry::TAG_DESCRIPTION = "description";

ViewRegistry::ViewRegistry() :
  dirtyViewCategoryMappings(true)
{
  miscCategory = new IViewDescriptorCategory();
  this->Add(miscCategory);

  //PlatformUI.getWorkbench().getExtensionTracker().registerHandler(this,
  //    ExtensionTracker.createExtensionPointFilter(getExtensionPointFilter()));
  reader.ReadViews(this);
}

void ViewRegistry::Add(IViewDescriptorCategoryPtr desc)
{
  /* fix for 1877 */
  if (this->InternalFindCategory(desc->GetId()).IsNull())
  {
    dirtyViewCategoryMappings = true;
    // Mark categories list as dirty
    categories.push_back(desc);
//    IConfigurationElement::Pointer element(
//        dynamic_cast<IConfigurationElement*>(
//            desc->GetAdapter(typeid(IConfigurationElement))
//            ));
//    if (element.IsNull())
//    {
//      return;
//    }
//    PlatformUI::GetWorkbench()->GetExtensionTracker()
//    .registerObject(element->GetDeclaringExtension(), desc,
//        IExtensionTracker::REF_WEAK);
  }
}

void ViewRegistry::Add(ViewDescriptor::Pointer desc)
{
  for (std::vector<IViewDescriptor::Pointer>::const_iterator itr = views.begin();
       itr != views.end(); ++itr)
  {
    if (desc.GetPointer() == itr->GetPointer()) return;
  }

  views.push_back(desc);
  dirtyViewCategoryMappings = true;
  //desc.activateHandler();
}

void ViewRegistry::Add(StickyViewDescriptor::Pointer desc)
{
  if (std::find(sticky.begin(), sticky.end(), desc) == sticky.end())
  {
    sticky.push_back(desc);
//    PlatformUI::GetWorkbench()->GetExtensionTracker()
//  .registerObject(desc.getConfigurationElement().getDeclaringExtension(),
//      desc, IExtensionTracker.REF_WEAK);
  }
}

IViewDescriptor::Pointer ViewRegistry::Find(const std::string& id) const
{
  for (std::vector<IViewDescriptor::Pointer>::const_iterator itr = views.begin();
       itr != views.end(); ++itr)
  {
    if (id == (*itr)->GetId())
    {
      return *itr;
    }
  }
  return IViewDescriptor::Pointer(0);
}

IViewCategory::Pointer ViewRegistry::FindCategory(const std::string& id)
{
  this->MapViewsToCategories();
  IViewDescriptorCategoryPtr category(this->InternalFindCategory(id));
  if (category.IsNull())
  {
    return IViewCategory::Pointer(0);
  }
  IViewCategory::Pointer cat(new ViewCategoryProxy(category));
  return cat;
}

std::vector<IViewCategory::Pointer> ViewRegistry::GetCategories()
{
  this->MapViewsToCategories();
  std::vector<IViewCategory::Pointer> retArray;
  for (std::vector<IViewDescriptorCategoryPtr>::iterator itr = categories.begin();
       itr != categories.end(); ++itr)
  {
    retArray.push_back(IViewCategory::Pointer(new ViewCategoryProxy(*itr)));
  }
  return retArray;
}

std::vector<IStickyViewDescriptor::Pointer> ViewRegistry::GetStickyViews() const
{
  return sticky;
}

Category<IViewDescriptor::Pointer>::Pointer ViewRegistry::GetMiscCategory() const
{
  return miscCategory;
}

const std::vector<IViewDescriptor::Pointer>& ViewRegistry::GetViews() const
{
  return views;
}

void ViewRegistry::MapViewsToCategories()
{
  if (dirtyViewCategoryMappings)
  {
    dirtyViewCategoryMappings = false;
    // clear all category mappings
    for (std::vector<IViewDescriptorCategoryPtr>::iterator i = categories.begin();
         i != categories.end(); ++i)
    {
      (*i)->Clear(); // this is bad
    }

    miscCategory->Clear();

    for (std::vector<IViewDescriptor::Pointer>::iterator i = views.begin();
         i != views.end(); ++i)
    {
      IViewDescriptor::Pointer desc(*i);
      IViewDescriptorCategoryPtr cat(0);
      const std::vector<std::string>& catPath = desc->GetCategoryPath();
      if (catPath.size() > 0)
      {
        cat = this->InternalFindCategory(catPath[0]);
      }
      if (cat.IsNotNull())
      {
        if (!cat->HasElement(desc))
        {
          cat->AddElement(desc);
        }
      }
      else
      {
        if (catPath.size() > 0)
        {
          // If we get here, this view specified a category which
          // does not exist. Add this view to the 'Other' category
          // but give out a message (to the log only) indicating
          // this has been done.
          std::string fmt(Poco::Logger::format(
              "Category $0 not found for view $1.  This view added to ''$2'' category.",
              catPath[0], desc->GetId(), miscCategory->GetLabel()));
          WorkbenchPlugin::Log(fmt);
        }
        miscCategory->AddElement(desc);
      }
    }
  }
}

//void ViewRegistry::AddExtension(IExtensionTracker tracker,
//    IExtension addedExtension)
//{
//  IConfigurationElement[] addedElements = addedExtension.getConfigurationElements();
//  for (int i = 0; i < addedElements.length; i++)
//  {
//    IConfigurationElement element = addedElements[i];
//    if (element.getName().equals(IWorkbenchRegistryConstants.TAG_VIEW))
//    {
//      reader.readView(element);
//    }
//    else if (element.getName().equals(IWorkbenchRegistryConstants.TAG_CATEGORY))
//    {
//      reader.readCategory(element);
//    }
//    else if (element.getName().equals(IWorkbenchRegistryConstants.TAG_STICKYVIEW))
//    {
//      reader.readSticky(element);
//    }
//  }
//}

} // namespace cherry
