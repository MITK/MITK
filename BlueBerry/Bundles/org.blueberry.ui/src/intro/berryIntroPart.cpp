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

#include "berryIntroPart.h"

#include <berrySafeRunner.h>
#include <berryObjects.h>
#include <berryIWorkbenchPartConstants.h>
#include <berryAbstractUIPlugin.h>

#include "../util/berrySafeRunnable.h"
#include "../internal/berryWorkbenchRegistryConstants.h"

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

std::string IntroPart::GetDefaultTitle() const
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

void* IntroPart::GetDefaultImage() const
{
  return 0;
}

void IntroPart::SetSite(IIntroSite::Pointer site)
{
  this->partSite = site;
}

void IntroPart::SetTitleImage(void* titleImage)
{
  //Do not send changes if they are the same
  if (this->titleImage == titleImage)
  {
    return;
  }
  this->titleImage = titleImage;
  FirePropertyChange(IWorkbenchPartConstants::PROP_TITLE);
}

void IntroPart::SetTitle(const std::string& titleLabel)
{
  if (this->titleLabel == titleLabel)
    return;
  this->titleLabel = titleLabel;
  FirePropertyChange(IWorkbenchPartConstants::PROP_TITLE);
}

void IntroPart::AddPropertyListener(IPropertyChangeListener::Pointer l)
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

void* IntroPart::GetTitleImage() const
{
  if (titleImage != 0)
  {
    return titleImage;
  }
  return GetDefaultImage();
}

std::string IntroPart::GetPartName() const
{
  if (!titleLabel.empty())
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

void IntroPart::RemovePropertyListener(IPropertyChangeListener::Pointer l)
{
  propChangeEvents.RemoveListener(l);
}

void IntroPart::SaveState(IMemento::Pointer /*memento*/)
{
  //no-op
}

void IntroPart::SetInitializationData(IConfigurationElement::Pointer cfig,
    const std::string& /*propertyName*/, Object::Pointer /*data*/)
{

  // Save config element.
  configElement = cfig;

  cfig->GetAttribute(WorkbenchRegistryConstants::ATT_LABEL, titleLabel);

  // Icon.
  std::string strIcon;
  if (!cfig->GetAttribute(WorkbenchRegistryConstants::ATT_ICON, strIcon))
  {
    return;
  }

  imageDescriptor = AbstractUIPlugin::ImageDescriptorFromPlugin(
      configElement->GetContributor(), strIcon);

  if (!imageDescriptor)
  {
    return;
  }

  titleImage = imageDescriptor->CreateImage(true);
}

}
