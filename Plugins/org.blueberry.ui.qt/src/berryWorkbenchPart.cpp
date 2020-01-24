/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryWorkbenchPart.h"

#include "berryIWorkbenchPartConstants.h"
#include <berryIContributor.h>
#include <berrySafeRunner.h>
#include <util/berrySafeRunnable.h>

#include "berryWorkbenchPlugin.h"

#include <berryObjectString.h>
#include <berryObjects.h>

namespace berry
{

class PropChangedRunnable : public SafeRunnable
{
public:

  berryObjectMacro(PropChangedRunnable);

  IPropertyChangeListener::Events::EventType::AbstractDelegate* delegate;

  PropChangedRunnable(PropertyChangeEvent::Pointer event)
  : event(event)
  {}

  void Run() override
  {
    delegate->Execute(event);
  }

private:

  PropertyChangeEvent::Pointer event;
};

WorkbenchPart::~WorkbenchPart()
{
}

WorkbenchPart::WorkbenchPart()
:m_Title(""),
 m_ToolTip(""),
 m_PartName(""),
 m_ContentDescription("")
{

}

void WorkbenchPart::InternalSetContentDescription(
    const QString& description)
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

void WorkbenchPart::InternalSetPartName(const QString& partName)
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

void WorkbenchPart::SetTitleImage(const QIcon& titleImage)
{
  //assert(titleImage == 0 || !titleImage.isDisposed());
  //Do not send changes if they are the same
  if (this->m_TitleImage.cacheKey() == titleImage.cacheKey())
  {
    return;
  }
  this->m_TitleImage = titleImage;
  this->FirePropertyChange(IWorkbenchPartConstants::PROP_TITLE);
}

void WorkbenchPart::SetTitleToolTip(const QString& toolTip)
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

void WorkbenchPart::SetPartName(const QString& partName)
{

  InternalSetPartName(partName);

  //setDefaultTitle();
}

void WorkbenchPart::SetContentDescription(const QString& description)
{
  InternalSetContentDescription(description);

  //setDefaultTitle();
}

void WorkbenchPart::FirePropertyChanged(const QString& key,
    const QString& oldValue, const QString& newValue)
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
  for (auto iter = listeners.begin(); iter
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
  for (auto iter = listeners.begin(); iter
      != listeners.end(); ++iter)
  {
    runnable->delegate = *iter;
    SafeRunner::Run(runnable);
  }
}

void WorkbenchPart::AddPropertyListener(IPropertyChangeListener* l)
{
  partChangeEvents.AddListener(l);
}

void WorkbenchPart::RemovePropertyListener(IPropertyChangeListener* l)
{
  partChangeEvents.RemoveListener(l);
}

void WorkbenchPart::SetPartProperty(const QString& key,
    const QString& value)
{
  QHash<QString, QString>::iterator iter = partProperties.find(key);
  QString oldValue;
  if (iter != partProperties.end())
    oldValue = iter.value();

  if (value == "")
  {
    partProperties.remove(key);
  }
  else
  {
    partProperties.insert(key, value);
  }
  this->FirePropertyChanged(key, oldValue, value);
}

QString WorkbenchPart::GetPartProperty(const QString& key) const
{
  QHash<QString, QString>::const_iterator itr = partProperties.find(key);
  if (itr == partProperties.end()) return "";

  return itr.value();
}

const QHash<QString, QString> &WorkbenchPart::GetPartProperties() const
{
  return partProperties;
}

IWorkbenchPartSite::Pointer WorkbenchPart::GetSite() const
{
  return this->m_PartSite;
}

QString WorkbenchPart::GetPartName() const
{
  return this->m_PartName;
}

QString WorkbenchPart::GetContentDescription() const
{
  return this->m_ContentDescription;
}

QIcon WorkbenchPart::GetTitleImage() const
{
  return this->m_TitleImage;
  //return GetDefaultImage();
}

QString WorkbenchPart::GetTitleToolTip() const
{
  return this->m_ToolTip;
}

void WorkbenchPart::SetInitializationData(const IConfigurationElement::Pointer& cfig,
                                          const QString& /*propertyName*/, const Object::Pointer& /*data*/)
{
  // Save config element.
  m_ConfigElement = cfig;

  // Part name and title.
  m_PartName = cfig->GetAttribute("name");
  m_Title = m_PartName;

  // Icon.
  QString strIcon = cfig->GetAttribute("icon");
  if (strIcon.isEmpty())
  {
    return;
  }

  m_TitleImage = AbstractUICTKPlugin::ImageDescriptorFromPlugin(
        m_ConfigElement->GetContributor()->GetName(), strIcon);
}

} // namespace berry
