/*=========================================================================

 Program:   BlueBerry Platform
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

#include "berryWorkbenchPart.h"

#include "berryIWorkbenchPartConstants.h"
#include "berryImageDescriptor.h"
#include "berrySafeRunner.h"
#include "util/berrySafeRunnable.h"

#include "internal/berryWorkbenchPlugin.h"

#include <osgi/framework/ObjectString.h>
#include <osgi/framework/Objects.h>

namespace berry
{

class PropChangedRunnable : public SafeRunnable
{
public:

  berryObjectMacro(PropChangedRunnable)

  IPropertyChangeListener::Events::EventType::AbstractDelegate* delegate;

  PropChangedRunnable(PropertyChangeEvent::Pointer event)
  : event(event)
  {}

  void Run()
  {
    delegate->Execute(event);
  }

private:

  PropertyChangeEvent::Pointer event;
};

WorkbenchPart::~WorkbenchPart()
{
  if (m_TitleImage && m_ImageDescriptor)
    m_ImageDescriptor->DestroyImage(m_TitleImage);
}

WorkbenchPart::WorkbenchPart() 
:m_Title(""), 
 m_TitleImage(0),
 m_ToolTip(""), 
 m_PartName(""), 
 m_ContentDescription("")
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
  m_ImageDescriptor->DestroyImage(m_TitleImage);
  this->m_TitleImage = titleImage;
  this->FirePropertyChange(IWorkbenchPartConstants::PROP_TITLE);

  if (m_ImageDescriptor) {
    //JFaceResources.getResources().destroyImage(imageDescriptor);
    m_ImageDescriptor = 0;
  }
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
  ObjectString::Pointer objOldVal(new ObjectString(oldValue));
  ObjectString::Pointer objNewVal(new ObjectString(newValue));
  Object::Pointer source(this);
  PropertyChangeEvent::Pointer event(
      new PropertyChangeEvent(source, key, objOldVal, objNewVal));
  typedef IPropertyChangeListener::Events::EventType::ListenerList ListenerList;
  PropChangedRunnable::Pointer runnable(new PropChangedRunnable(event));

  const ListenerList& listeners =
      partChangeEvents.propertyChange.GetListeners();
  for (ListenerList::const_iterator iter = listeners.begin(); iter
      != listeners.end(); ++iter)
  {
    runnable->delegate = *iter;
    SafeRunner::Run(runnable);
  }
}

void WorkbenchPart::FirePropertyChange(int propertyId)
{
  ObjectInt::Pointer val(new ObjectInt(propertyId));
  Object::Pointer source(this);

  PropertyChangeEvent::Pointer event(
      new PropertyChangeEvent(source, IWorkbenchPartConstants::INTEGER_PROPERTY, val, val));
  typedef IPropertyChangeListener::Events::EventType::ListenerList ListenerList;
  PropChangedRunnable::Pointer runnable(new PropChangedRunnable(event));

  const ListenerList& listeners =
      partChangeEvents.propertyChange.GetListeners();
  for (ListenerList::const_iterator iter = listeners.begin(); iter
      != listeners.end(); ++iter)
  {
    runnable->delegate = *iter;
    SafeRunner::Run(runnable);
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

std::string WorkbenchPart::GetPartProperty(const std::string& key) const
{
  std::map<std::string, std::string>::const_iterator itr = partProperties.find(key);
  if (itr == partProperties.end()) return "";

  return itr->second;
}

const std::map<std::string, std::string>& WorkbenchPart::GetPartProperties() const
{
  return partProperties;
}

IWorkbenchPartSite::Pointer WorkbenchPart::GetSite() const
{
  return this->m_PartSite;
}

std::string WorkbenchPart::GetPartName() const
{
  return this->m_PartName;
}

std::string WorkbenchPart::GetContentDescription() const
{
  return this->m_ContentDescription;
}

void* WorkbenchPart::GetTitleImage() const
{
  if (this->m_TitleImage != 0)
  {
    return this->m_TitleImage;
  }

  return 0;
  //return GetDefaultImage();
}

std::string WorkbenchPart::GetTitleToolTip() const
{
  return this->m_ToolTip;
}

void WorkbenchPart::SetInitializationData(IConfigurationElement::Pointer cfig,
    const std::string&  /*propertyName*/, Object::Pointer  /*data*/)
{
  // Save config element.
  m_ConfigElement = cfig;

  // Part name and title.
  cfig->GetAttribute("name", m_PartName);
  m_Title = m_PartName;

  // Icon.
  std::string strIcon;
  cfig->GetAttribute("icon", strIcon);//$NON-NLS-1$
  if (strIcon.empty()) {
    return;
  }

  m_ImageDescriptor = AbstractUIPlugin::ImageDescriptorFromPlugin(
     m_ConfigElement->GetContributor(), strIcon);

  if (!m_ImageDescriptor) {
    return;
  }

  //titleImage = JFaceResources.getResources().createImageWithDefault(imageDescriptor);
  m_TitleImage = m_ImageDescriptor->CreateImage();
}

} // namespace berry
