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

#include "cherryWorkbenchPart.h"

#include "cherryIWorkbenchPartConstants.h"

#include "internal/cherryWorkbenchPlugin.h"

#include <cherryObjectString.h>
#include <cherryObjects.h>

namespace cherry
{

WorkbenchPart::WorkbenchPart() :
  m_Title(""), m_ToolTip(""), m_PartName(""), m_ContentDescription("")
{

}

void WorkbenchPart::InternalSetContentDescription(
    const std::string& description)
{
  //assert(description != 0)

  //Do not send changes if they are the same
  if (this->m_ContentDescription == description)
  {
    return;
  }
  this->m_ContentDescription = description;

  this->FirePropertyChange(IWorkbenchPartConstants::PROP_CONTENT_DESCRIPTION);
}

void WorkbenchPart::InternalSetPartName(const std::string& partName)
{
  //partName = Util.safeString(partName);

  //assert(partName != 0);

  //Do not send changes if they are the same
  if (this->m_PartName == partName)
  {
    return;
  }
  this->m_PartName = partName;

  this->FirePropertyChange(IWorkbenchPartConstants::PROP_PART_NAME);

}

//  IConfigurationElement* GetConfigurationElement()
//  {
//    return m_ConfigElement;
//  }


//  protected Image getDefaultImage() {
//    return PlatformUI.getWorkbench().getSharedImages().getImage(
//                ISharedImages.IMG_DEF_VIEW);
//  }

void WorkbenchPart::SetSite(IWorkbenchPartSite::Pointer site)
{
  this->CheckSite(site);
  this->m_PartSite = site;
}

void WorkbenchPart::CheckSite(IWorkbenchPartSite::Pointer /*site*/)
{
  // do nothing
}

void WorkbenchPart::SetTitleImage(void* titleImage)
{
  //assert(titleImage == 0 || !titleImage.isDisposed());
  //Do not send changes if they are the same
  if (this->m_TitleImage == titleImage)
  {
    return;
  }
  this->m_TitleImage = titleImage;
  //firePropertyChange(IWorkbenchPart.PROP_TITLE);
  //    if (imageDescriptor != null) {
  //      JFaceResources.getResources().destroyImage(imageDescriptor);
  //      imageDescriptor = null;
  //    }
}

void WorkbenchPart::SetTitleToolTip(const std::string& toolTip)
{
  //toolTip = Util.safeString(toolTip);
  //Do not send changes if they are the same
  if (this->m_ToolTip == toolTip)
  {
    return;
  }
  this->m_ToolTip = toolTip;
  this->FirePropertyChange(IWorkbenchPartConstants::PROP_TITLE);
}

void WorkbenchPart::SetPartName(const std::string& partName)
{

  InternalSetPartName(partName);

  //setDefaultTitle();
}

void WorkbenchPart::SetContentDescription(const std::string& description)
{
  InternalSetContentDescription(description);

  //setDefaultTitle();
}

void WorkbenchPart::FirePropertyChanged(const std::string& key,
    const std::string& oldValue, const std::string& newValue)
{
  ObjectString::Pointer objOldVal = new ObjectString(oldValue);
  ObjectString::Pointer objNewVal = new ObjectString(newValue);
  PropertyChangeEvent::Pointer event =
      new PropertyChangeEvent(this, key, objOldVal, objNewVal);
  typedef IPropertyChangeListener::Events::EventType::ListenerList ListenerList;
  const ListenerList& listeners =
      partChangeEvents.propertyChange.GetListeners();
  for (ListenerList::const_iterator iter = listeners.begin(); iter
      != listeners.end(); ++iter)
  {
    try
    {
      (*iter)->Execute(event);
    } catch (Poco::RuntimeException& e)
    {
      WorkbenchPlugin::Log(e);
    }
  }
}

void WorkbenchPart::FirePropertyChange(int propertyId)
{
  ObjectInt::Pointer val = new ObjectInt(propertyId);

  PropertyChangeEvent::Pointer event =
      new PropertyChangeEvent(this, IWorkbenchPartConstants::INTEGER_PROPERTY, val, val);
  typedef IPropertyChangeListener::Events::EventType::ListenerList ListenerList;
  const ListenerList& listeners =
      partChangeEvents.propertyChange.GetListeners();
  for (ListenerList::const_iterator iter = listeners.begin(); iter
      != listeners.end(); ++iter)
  {
    try
    {
      (*iter)->Execute(event);
    } catch (Poco::RuntimeException& e)
    {
      WorkbenchPlugin::Log(e);
    }
  }
}

void WorkbenchPart::AddPropertyListener(IPropertyChangeListener::Pointer l)
{
  partChangeEvents.AddListener(l);
}

void WorkbenchPart::RemovePropertyListener(IPropertyChangeListener::Pointer l)
{
  partChangeEvents.RemoveListener(l);
}

void WorkbenchPart::SetPartProperty(const std::string& key,
    const std::string& value)
{
  std::map<std::string, std::string>::iterator iter = partProperties.find(
      key);
  std::string oldValue;
  if (iter != partProperties.end())
    oldValue = iter->second;

  if (value == "")
  {
    partProperties.erase(key);
  }
  else
  {
    partProperties.insert(std::make_pair(key, value));
  }
  this->FirePropertyChanged(key, oldValue, value);
}

std::string WorkbenchPart::GetPartProperty(const std::string& key)
{
  return partProperties[key];
}

const std::map<std::string, std::string>& WorkbenchPart::GetPartProperties()
{
  return partProperties;
}

IWorkbenchPartSite::Pointer WorkbenchPart::GetSite()
{
  return this->m_PartSite;
}

std::string WorkbenchPart::GetPartName()
{
  return this->m_PartName;
}

std::string WorkbenchPart::GetContentDescription()
{
  return this->m_ContentDescription;
}

void* WorkbenchPart::GetTitleImage()
{
  if (this->m_TitleImage != 0)
  {
    return this->m_TitleImage;
  }

  return 0;
  //return GetDefaultImage();
}

std::string WorkbenchPart::GetTitleToolTip()
{
  return this->m_ToolTip;
}

void WorkbenchPart::SetInitializationData(IConfigurationElement::Pointer cfig,
    const std::string& propertyName, Object::Pointer data)
{
  // Save config element.
  m_ConfigElement = cfig;

  // Part name and title.
  cfig->GetAttribute("name", m_PartName);
  m_Title = m_PartName;

  // Icon.
  //String strIcon = cfig.getAttribute("icon");//$NON-NLS-1$
  //if (strIcon == null) {
  //  return;
  //}

  //imageDescriptor = AbstractUIPlugin.imageDescriptorFromPlugin(
  //        configElement.getNamespace(), strIcon);

  //if (imageDescriptor == null) {
  //  return;
  //}

  //titleImage = JFaceResources.getResources().createImageWithDefault(imageDescriptor);
}

} // namespace cherry
