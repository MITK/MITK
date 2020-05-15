/*===================================================================

 The Medical Imaging Interaction Toolkit (MITK)

 Copyright (c) German Cancer Research Center,
 Division of Medical and Biological Informatics.
 All rights reserved.

 This software is distributed WITHOUT ANY WARRANTY; without
 even the implied warranty of MERCHANTABILITY or FITNESS FOR
 A PARTICULAR PURPOSE.

 See LICENSE.txt or http://www.mitk.org for details.

 ===================================================================*/

#include <boost/scoped_ptr.hpp>

#include "mitkMouseModeSwitcher.h"
// us
#include "usGetModuleContext.h"
#include "usModuleContext.h"

#include "mitkInteractionEventObserver.h"

namespace {
  boost::scoped_ptr<mitk::MouseModeSwitcher> s_impl;

  std::string getNameButton(const unsigned int& button)
  {
    std::string buttonName;
    switch (button) {
    case 1:
      buttonName = "Left";
      break;

    case 2:
      buttonName = "Right";
      break;

    case 4:
      buttonName = "Middle";
      break;

    default:
      buttonName = "";
    }
    return buttonName;
  }

  void updateMouseModes(mitk::MouseModeSwitcher::MouseModeMap& activeMouseModes, mitk::MouseModeSwitcher::MouseMode mode, const unsigned int& button)
  {
    activeMouseModes[mode].insert(button);
    for (auto &activeMode : activeMouseModes) {
      auto &buttonSet = activeMode.second;
      if (activeMode.first != mode) {
        buttonSet.erase(button);
      }
    }
  }
}

mitk::MouseModeSwitcher& mitk::MouseModeSwitcher::GetInstance()
{
  if (!s_impl) {
    s_impl.reset(new mitk::MouseModeSwitcher());
  }
  return *s_impl;
}

void mitk::MouseModeSwitcher::DestroyInstance()
{
  s_impl.reset();
}

mitk::MouseModeSwitcher::MouseModeSwitcher() :
  m_ActiveInteractionScheme(MITK), m_ActiveMouseMode(MousePointer)
{
  m_ActiveMouseModes.clear();
  m_ActiveMouseModes[mitk::MouseModeSwitcher::CrossHair] = { 0x00000001 }; // Qt::LeftButton
  m_ActiveMouseModes[mitk::MouseModeSwitcher::Zoom] = { 0x00000002 }; // Qt::RightButton
  m_ActiveMouseModes[mitk::MouseModeSwitcher::Pan] = { 0x00000004 }; // Qt::MiddleButton

  this->SetInteractionScheme(m_ActiveInteractionScheme);
}

mitk::MouseModeSwitcher::MouseModeSwitcher(mitk::BaseRenderer::Pointer renderer) :
    mitk::MouseModeSwitcher::MouseModeSwitcher()
{
  this->AddRenderer(renderer);
}

mitk::MouseModeSwitcher::~MouseModeSwitcher()
{
  for (auto serviceRegistration : m_ServiceRegistrations) {
    serviceRegistration.Unregister();
  }
}

void mitk::MouseModeSwitcher::AddRenderer(mitk::BaseRenderer::Pointer renderer)
{
  std::string rendererName = renderer->GetName();
  if (m_RegisteredRendererNames.find(rendererName) != m_RegisteredRendererNames.end()) {
    return; // Already registered renderer with same name
  }

  DisplayInteractor::Pointer currentObserver = mitk::DisplayInteractor::New();
  currentObserver->SetEventConfig("DisplayConfigMITK.xml");

  if (renderer->GetMapperID() == mitk::BaseRenderer::Standard2D) {
    currentObserver->LoadStateMachine("DisplayInteraction.xml");
    currentObserver->SetOnly3D(false);
  } else if (renderer->GetMapperID() == mitk::BaseRenderer::Standard3D) {
    currentObserver->LoadStateMachine("DisplayInteraction3D.xml");
    currentObserver->SetOnly3D(true);
  } else {
    assert(false);
  }

  // Register as listener via micro services
  us::ServiceProperties props;
  props["name"] = std::string("DisplayInteractor");

  if (!rendererName.empty()) {
    props["rendererName"] = rendererName;
  }

  us::ServiceRegistration<InteractionEventObserver> serviceRegistration = 
    us::GetModuleContext()->RegisterService<InteractionEventObserver>(currentObserver.GetPointer(),props);

  m_Observers.push_back(currentObserver);
  m_ServiceRegistrations.push_back(serviceRegistration);
  m_RegisteredRendererNames.insert(rendererName);
}

void mitk::MouseModeSwitcher::SetInteractionScheme(InteractionScheme scheme)
{
  std::string eventConfig;
  switch (scheme)
  {
  case MITK:
  {
    eventConfig = "DisplayConfigMITK.xml";
  }
    break;

  case SWIVEL:
  {
    eventConfig = "DisplayConfigMITKSwivel.xml";
  }
    break;
  }

  if (!eventConfig.empty()) {
    for (auto observer : m_Observers) {
      observer->SetEventConfig(eventConfig);
    }
  }
  m_ActiveInteractionScheme = scheme;
  for (auto mode: m_ActiveMouseModes) { // restore mouse modes
    for (auto button: mode.second) {
      if (scheme == MITK || button != 1) {
        SelectMouseMode(mode.first, button);
      }
    }
  }
  this->InvokeEvent(MouseModeChangedEvent());
}

void mitk::MouseModeSwitcher::SetSelectionMode(bool selection)
{
  for (auto observer : m_Observers) {
    observer->SetSelectionMode(selection);
  }
}

void mitk::MouseModeSwitcher::SelectMouseMode(MouseMode mode, const unsigned int& button)
{
  std::string eventConfig;
  switch (mode)
  {
  case MousePointer:
  {
    eventConfig = "DisplayConfigMITK";
    break;
  } // case 0
  case Scroll:
  {
    eventConfig = "DisplayConfigScroll";
    break;
  }
  case LevelWindow:
  {
    eventConfig = "DisplayConfigLevelWindow";
    break;
  }
  case Zoom:
  {
    eventConfig = "DisplayConfigZoom";
    break;
  }
  case Pan:
  {
    eventConfig = "DisplayConfigPan";
    break;
  }
  case CrossHair:
  {
    eventConfig = "DisplayConfigCrossHair";
    break;
  }
  case MouseRotation:
  {
    eventConfig = "DisplayConfigMouseRotation";
    break;
  }
  } // end switch (mode)

  if (eventConfig != "DisplayConfigMITK") {

    eventConfig = eventConfig + getNameButton(button);
  }
  eventConfig = eventConfig + ".xml";

  if (!eventConfig.empty()) {
    for (auto observer : m_Observers) {
      observer->AddEventConfig(eventConfig);
    }
  }

  m_ActiveMouseMode = mode;

  updateMouseModes(m_ActiveMouseModes, mode, button);

  this->InvokeEvent(MouseModeChangedEvent());
}

mitk::MouseModeSwitcher::MouseMode mitk::MouseModeSwitcher::GetCurrentMouseMode() const
{
  return m_ActiveMouseMode;
}

mitk::MouseModeSwitcher::MouseModeMap& mitk::MouseModeSwitcher::GetActiveMouseModes()
{
  return m_ActiveMouseModes;
}
