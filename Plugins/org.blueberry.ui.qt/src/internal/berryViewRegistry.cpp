/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryViewRegistry.h"

#include "berryPlatformUI.h"
#include "berryWorkbenchPlugin.h"
#include "berryWorkbenchRegistryConstants.h"

#include "berryPlatform.h"

namespace berry
{

ViewRegistry::ViewCategoryProxy::ViewCategoryProxy(
    IViewDescriptorCategoryPtr rawCategory) :
  rawCategory(rawCategory)
{
}

QList<IViewDescriptor::Pointer> ViewRegistry::ViewCategoryProxy::GetViews() const
{
  return rawCategory->GetElements();
}

QString ViewRegistry::ViewCategoryProxy::GetId() const
{
  return rawCategory->GetId();
}

QStringList ViewRegistry::ViewCategoryProxy::GetPath() const
{
  QList<QString> path;
  QString rawParentPath = rawCategory->GetRawParentPath();
  // nested categories are not supported yet
  // assume an empty raw parent path
  path.push_back(rawParentPath);
  return path;
}

QString ViewRegistry::ViewCategoryProxy::GetLabel() const
{
  return rawCategory->GetLabel();
}

QString ViewRegistry::ViewCategoryProxy::GetLocalId() const
{
  return GetId();
}

QString ViewRegistry::ViewCategoryProxy::GetPluginId() const
{
  return rawCategory->GetPluginId();
}

bool ViewRegistry::ViewCategoryProxy::operator==(const Object* o) const
{
  if (const IViewCategory* other = dynamic_cast<const IViewCategory*>(o))
    return this->GetId() == other->GetId();

  return false;
}

uint ViewRegistry::ViewCategoryProxy::HashCode() const
{
  return qHash(GetId());
}

QString ViewRegistry::EXTENSIONPOINT_UNIQUE_ID = "org.blueberry.ui.views";
    // PlatformUI::PLUGIN_ID + "." + WorkbenchRegistryConstants::PL_VIEWS;


Category<IViewDescriptor::Pointer>::Pointer ViewRegistry::InternalFindCategory(const QString& id)
{
  for (QList<IViewDescriptorCategoryPtr>::iterator itr = categories.begin();
       itr != categories.end(); ++itr)
  {
    if (id == (*itr)->GetRootPath())
    {
      return *itr;
    }
  }
  return IViewDescriptorCategoryPtr(nullptr);
}

IExtensionPoint::Pointer ViewRegistry::GetExtensionPointFilter()
{
  return Platform::GetExtensionRegistry()->GetExtensionPoint(EXTENSIONPOINT_UNIQUE_ID);
}

const QString ViewRegistry::TAG_DESCRIPTION = "description";

ViewRegistry::ViewRegistry() :
  dirtyViewCategoryMappings(true)
{
  miscCategory = new IViewDescriptorCategory();
  this->Add(miscCategory);

  //PlatformUI.getWorkbench().getExtensionTracker().registerHandler(this,
  //    ExtensionTracker.createExtensionPointFilter(getExtensionPointFilter()));
  reader.ReadViews(Platform::GetExtensionRegistry(), this);
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
  for (QList<IViewDescriptor::Pointer>::const_iterator itr = views.begin();
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

IViewDescriptor::Pointer ViewRegistry::Find(const QString& id) const
{
  for (QList<IViewDescriptor::Pointer>::const_iterator itr = views.begin();
       itr != views.end(); ++itr)
  {
    if (id == (*itr)->GetId())
    {
      return *itr;
    }
  }
  return IViewDescriptor::Pointer(nullptr);
}

IViewCategory::Pointer ViewRegistry::FindCategory(const QString& id)
{
  this->MapViewsToCategories();
  IViewDescriptorCategoryPtr category(this->InternalFindCategory(id));
  if (category.IsNull())
  {
    return IViewCategory::Pointer(nullptr);
  }
  IViewCategory::Pointer cat(new ViewCategoryProxy(category));
  return cat;
}

QList<IViewCategory::Pointer> ViewRegistry::GetCategories()
{
  this->MapViewsToCategories();
  QList<IViewCategory::Pointer> retArray;
  for (QList<IViewDescriptorCategoryPtr>::iterator itr = categories.begin();
       itr != categories.end(); ++itr)
  {
    retArray.push_back(IViewCategory::Pointer(new ViewCategoryProxy(*itr)));
  }
  return retArray;
}

QList<IStickyViewDescriptor::Pointer> ViewRegistry::GetStickyViews() const
{
  return sticky;
}

Category<IViewDescriptor::Pointer>::Pointer ViewRegistry::GetMiscCategory() const
{
  return miscCategory;
}

QList<IViewDescriptor::Pointer> ViewRegistry::GetViews() const
{
  return views;
}

void ViewRegistry::MapViewsToCategories()
{
  if (dirtyViewCategoryMappings)
  {
    dirtyViewCategoryMappings = false;
    // clear all category mappings
    for (QList<IViewDescriptorCategoryPtr>::iterator i = categories.begin();
         i != categories.end(); ++i)
    {
      (*i)->Clear(); // this is bad
    }

    miscCategory->Clear();

    for (QList<IViewDescriptor::Pointer>::iterator i = views.begin();
         i != views.end(); ++i)
    {
      IViewDescriptor::Pointer desc(*i);
      IViewDescriptorCategoryPtr cat(nullptr);
      const QList<QString>& catPath = desc->GetCategoryPath();
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
          QString fmt("Category %1 not found for view %2.  This view added to ''%3'' category.");
          WorkbenchPlugin::Log(fmt.arg(catPath[0]).arg(desc->GetId()).arg(miscCategory->GetLabel()));
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

} // namespace berry
