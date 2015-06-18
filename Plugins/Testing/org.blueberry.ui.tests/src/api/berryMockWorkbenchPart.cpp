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

#include "berryMockWorkbenchPart.h"

#include <berryImageDescriptor.h>
#include <berryGuiWidgetsTweaklet.h>
#include <berryObjects.h>
#include <berryIWorkbenchPage.h>
#include <berryIWorkbenchWindow.h>
#include <berryIWorkbenchPartConstants.h>

namespace berry
{

MockWorkbenchPart::MockWorkbenchPart()
: titleImage(0), parentWidget(0),
  disposeListener(new GuiTk::ControlDestroyedAdapter<MockWorkbenchPart>(this, &MockWorkbenchPart::ControlDestroyed))
, siteState(false)
{
  callTrace = new CallHistory();
  selectionProvider = new MockSelectionProvider();
}

void MockWorkbenchPart::SetSite(IWorkbenchPartSite::Pointer site)
{
  this->site = site;
  site->SetSelectionProvider(selectionProvider);
}

IWorkbenchPartSite::Pointer MockWorkbenchPart::GetSite() const
{
  return site;
}

std::string MockWorkbenchPart::GetPartName() const
{
  return title;
}

std::string MockWorkbenchPart::GetContentDescription() const
{
  return title;
}

std::string MockWorkbenchPart::GetTitleToolTip() const
{
  return title;
}

CallHistory::Pointer MockWorkbenchPart::GetCallHistory() const
{
  return callTrace;
}

ISelectionProvider::Pointer MockWorkbenchPart::GetSelectionProvider()
{
  return selectionProvider;
}

void MockWorkbenchPart::SetInitializationData(
    IConfigurationElement::Pointer config, const std::string& /*propertyName*/,
    Object::Pointer data)
{

  callTrace->Add("SetInitializationData");

  this->config = config;
  this->data = data;

  // Icon.
  std::string strIcon;
  if (config->GetAttribute("icon", strIcon))
  {
    ImageDescriptor::Pointer imageDesc = ImageDescriptor::CreateFromFile(
        strIcon, config->GetContributor());
    titleImage = imageDesc->CreateImage();
  }

  config->GetAttribute("name", title);
}

void MockWorkbenchPart::ControlDestroyed(GuiTk::ControlEvent::Pointer)
{
  callTrace->Add("WidgetDisposed");
  parentWidget = 0;
}

void MockWorkbenchPart::AddPropertyListener(
    IPropertyChangeListener::Pointer listener)
{
  propertyEvent.AddListener(listener);
}

void MockWorkbenchPart::RemovePropertyListener(
    IPropertyChangeListener::Pointer listener)
{
  propertyEvent.RemoveListener(listener);
}

void MockWorkbenchPart::CreatePartControl(void* parent)
{
  callTrace->Add("CreatePartControl");

  this->parentWidget = parent;
  Tweaklets::Get(GuiWidgetsTweaklet::KEY)->AddControlListener(parent,
      disposeListener);
}

MockWorkbenchPart::~MockWorkbenchPart()
{
  callTrace->Add("PartDestructor");

  if (parentWidget)
  {
    Tweaklets::Get(GuiWidgetsTweaklet::KEY)->RemoveControlListener(parentWidget,
      disposeListener);
  }
}

void* MockWorkbenchPart::GetTitleImage() const
{
  return titleImage;
}

void MockWorkbenchPart::SetFocus()
{
  callTrace->Add("SetFocus");
}

void MockWorkbenchPart::FireSelection()
{
  selectionProvider->FireSelection();
}

bool MockWorkbenchPart::IsSiteInitialized()
{
  return siteState;
}

void MockWorkbenchPart::SetSiteInitialized()
{
  SetSiteInitialized(
  // GetSite()->GetKeyBindingService() != 0 &&
      (GetSite()->GetPage() != 0) && (GetSite()->GetSelectionProvider() != 0)
          && (GetSite()->GetWorkbenchWindow() != 0) // &&
  // TestActionBars(GetActionBars())
  );
}

IConfigurationElement::Pointer MockWorkbenchPart::GetConfig()
{
  return config;
}

Object::Pointer MockWorkbenchPart::GetData()
{
  return data;
}

void MockWorkbenchPart::FirePropertyChange(int propertyId)
{
  ObjectInt::Pointer value(new ObjectInt(propertyId));
  PropertyChangeEvent::Pointer event(new PropertyChangeEvent(Object::Pointer(
      this), IWorkbenchPartConstants::INTEGER_PROPERTY, value, value));
  propertyEvent.propertyChange(event);
}

void MockWorkbenchPart::SetSiteInitialized(bool initialized)
{
  siteState = initialized;
}

std::string MockWorkbenchPart::GetPartProperty(const std::string& key) const
{
  std::map<std::string, std::string>::const_iterator i = properties.find(key);
  if (i != properties.end()) return i->second;
  return "";
}

void MockWorkbenchPart::SetPartProperty(const std::string& key,
    const std::string& value)
{
  properties[key] = value;
}

const std::map<std::string, std::string>& MockWorkbenchPart::GetPartProperties() const
{
  return properties;
}

}
