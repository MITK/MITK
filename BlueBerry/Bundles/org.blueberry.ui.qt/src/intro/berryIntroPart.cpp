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

#include "berryIntroPart.h"

#include <berrySafeRunner.h>
#include <berryObjects.h>
#include <berryIContributor.h>
#include <berryIWorkbenchPartConstants.h>
#include <berryAbstractUICTKPlugin.h>

#include "util/berrySafeRunnable.h"
#include "internal/berryWorkbenchRegistryConstants.h"

namespace berry
{

class PropChangedRunnable: public SafeRunnable
{
public:

  berryObjectMacro(PropChangedRunnable)

  IPropertyChangeListener::Events::EventType::AbstractDelegate* delegate;

  PropChangedRunnable(PropertyChangeEvent::Pointer event) :
    event(event)
  {
  }

  void Run()
  {
    delegate->Execute(event);
  }

private:

  PropertyChangeEvent::Pointer event;
};

QString IntroPart::GetDefaultTitle() const
{
  return "Welcome";
}

void IntroPart::FirePropertyChange(int propertyId)
{
  ObjectInt::Pointer val(new ObjectInt(propertyId));
  Object::Pointer source(this);

  PropertyChangeEvent::Pointer event(new PropertyChangeEvent(source,
      IWorkbenchPartConstants::INTEGER_PROPERTY, val, val));
  typedef IPropertyChangeListener::Events::EventType::ListenerList ListenerList;
  PropChangedRunnable::Pointer runnable(new PropChangedRunnable(event));

  const ListenerList& listeners =
      propChangeEvents.propertyChange.GetListeners();
  for (ListenerList::const_iterator iter = listeners.begin(); iter
      != listeners.end(); ++iter)
  {
    runnable->delegate = *iter;
    SafeRunner::Run(runnable);
  }

}

IConfigurationElement::Pointer IntroPart::GetConfigurationElement()
{
  return configElement;
}

QIcon IntroPart::GetDefaultImage() const
{
  return QIcon();
}

void IntroPart::SetSite(IIntroSite::Pointer site)
{
  this->partSite = site;
}

void IntroPart::SetTitleImage(const QIcon& titleImage)
{
  //Do not send changes if they are the same
  if (this->imageDescriptor.cacheKey() == titleImage.cacheKey())
  {
    return;
  }
  this->imageDescriptor = titleImage;
  FirePropertyChange(IWorkbenchPartConstants::PROP_TITLE);
}

void IntroPart::SetTitle(const QString& titleLabel)
{
  if (this->titleLabel == titleLabel)
    return;
  this->titleLabel = titleLabel;
  FirePropertyChange(IWorkbenchPartConstants::PROP_TITLE);
}

void IntroPart::AddPropertyListener(IPropertyChangeListener *l)
{
  propChangeEvents.AddListener(l);
}

IntroPart::~IntroPart()
{

}

IIntroSite::Pointer IntroPart::GetIntroSite() const
{
  return partSite;
}

QIcon IntroPart::GetTitleImage() const
{
  if (!this->imageDescriptor.isNull())
  {
    return this->imageDescriptor;
  }
  return GetDefaultImage();
}

QString IntroPart::GetPartName() const
{
  if (!titleLabel.isEmpty())
  {
    return titleLabel;
  }
  return GetDefaultTitle();
}

void IntroPart::Init(IIntroSite::Pointer site, IMemento::Pointer /*memento*/)
    throw (PartInitException)
{
  SetSite(site);
}

void IntroPart::RemovePropertyListener(IPropertyChangeListener *l)
{
  propChangeEvents.RemoveListener(l);
}

void IntroPart::SaveState(IMemento::Pointer /*memento*/)
{
  //no-op
}

void IntroPart::SetInitializationData(const IConfigurationElement::Pointer& cfig,
                                      const QString& /*propertyName*/, const Object::Pointer& /*data*/)
{

  // Save config element.
  configElement = cfig;

  titleLabel = cfig->GetAttribute(WorkbenchRegistryConstants::ATT_LABEL);

  // Icon.
  QString strIcon = cfig->GetAttribute(WorkbenchRegistryConstants::ATT_ICON);
  if (strIcon.isEmpty())
  {
    return;
  }

  imageDescriptor = AbstractUICTKPlugin::ImageDescriptorFromPlugin(
        configElement->GetContributor()->GetName(), strIcon);

}

}
