/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkTool.h"

#include "mitkDisplayActionEventBroadcast.h"
#include "mitkImageReadAccessor.h"
#include "mitkImageWriteAccessor.h"
#include "mitkLevelWindowProperty.h"
#include "mitkLookupTableProperty.h"
#include "mitkProperties.h"
#include "mitkVtkResliceInterpolationProperty.h"
#include <mitkDICOMSegmentationPropertyHelper.cpp>
#include <mitkToolManager.h>
#include <mitkMultiLabelPredicateHelper.h>

// us
#include <usGetModuleContext.h>
#include <usModuleResource.h>

// itk
#include <itkObjectFactory.h>

namespace mitk
{
  itkEventMacroDefinition(ToolEvent, itk::ModifiedEvent);
}

mitk::Tool::Tool(const char *type, const us::Module *interactorModule)
  : m_EventConfig(""),
    m_ToolManager(nullptr),
    m_DisplayInteractionConfigs(),
    m_InteractorModule(interactorModule)
{
}

mitk::Tool::~Tool()
{
}

bool mitk::Tool::CanHandle(const BaseData* referenceData, const BaseData* /*workingData*/) const
{
  if (referenceData == nullptr)
    return false;

  return true;
}

bool mitk::Tool::IsEligibleForAutoInit() const
{
  return true;
}

void mitk::Tool::InitializeStateMachine()
{
  if (m_InteractorType.empty())
    return;

  try
  {
    auto isThisModule = nullptr == m_InteractorModule;

    auto module = isThisModule
      ? us::GetModuleContext()->GetModule()
      : m_InteractorModule;

    LoadStateMachine(m_InteractorType + ".xml", module);
    SetEventConfig(isThisModule ? "SegmentationToolsConfig.xml" : m_InteractorType + "Config.xml", module);
  }
  catch (const std::exception &e)
  {
    MITK_ERROR << "Could not load statemachine pattern " << m_InteractorType << ".xml with exception: " << e.what();
  }
}

void mitk::Tool::Notify(InteractionEvent *interactionEvent, bool isHandled)
{
  // to use the state machine pattern,
  // the event is passed to the state machine interface to be handled
  if (!isHandled)
  {
    this->HandleEvent(interactionEvent, nullptr);
  }
}

void mitk::Tool::ConnectActionsAndFunctions()
{
}

bool mitk::Tool::FilterEvents(InteractionEvent *, DataNode *)
{
  return true;
}

const char *mitk::Tool::GetGroup() const
{
  return "default";
}

void mitk::Tool::SetToolManager(ToolManager *manager)
{
  m_ToolManager = manager;
}

mitk::ToolManager* mitk::Tool::GetToolManager() const
{
  return m_ToolManager;
}

mitk::DataStorage* mitk::Tool::GetDataStorage() const
{
  if (nullptr != m_ToolManager)
  {
    return m_ToolManager->GetDataStorage();
  }
  return nullptr;
}

void mitk::Tool::Activated()
{
  // As a legacy solution the display interaction of the new interaction framework is disabled here to avoid conflicts
  // with tools
  // Note: this only affects InteractionEventObservers (formerly known as Listeners) all DataNode specific interaction
  // will still be enabled
  m_DisplayInteractionConfigs.clear();
  auto eventObservers = us::GetModuleContext()->GetServiceReferences<InteractionEventObserver>();
  for (const auto& eventObserver : eventObservers)
  {
    auto displayActionEventBroadcast = dynamic_cast<DisplayActionEventBroadcast*>(
      us::GetModuleContext()->GetService<InteractionEventObserver>(eventObserver));
    if (nullptr != displayActionEventBroadcast)
    {
      // remember the original configuration
      m_DisplayInteractionConfigs.insert(std::make_pair(eventObserver, displayActionEventBroadcast->GetEventConfig()));
      // here the alternative configuration is loaded
      displayActionEventBroadcast->AddEventConfig(m_EventConfig.c_str());
    }
  }
}

void mitk::Tool::Deactivated()
{
  // Re-enabling InteractionEventObservers that have been previously disabled for legacy handling of Tools
  // in new interaction framework
  for (const auto& displayInteractionConfig : m_DisplayInteractionConfigs)
  {
    if (displayInteractionConfig.first)
    {
      auto displayActionEventBroadcast = static_cast<mitk::DisplayActionEventBroadcast*>(
        us::GetModuleContext()->GetService<mitk::InteractionEventObserver>(displayInteractionConfig.first));

      if (nullptr != displayActionEventBroadcast)
      {
        // here the regular configuration is loaded again
        displayActionEventBroadcast->SetEventConfig(displayInteractionConfig.second);
      }
    }
  }

  m_DisplayInteractionConfigs.clear();
}

itk::Object::Pointer mitk::Tool::GetGUI(const std::string &toolkitPrefix, const std::string &toolkitPostfix)
{
  itk::Object::Pointer object;

  std::string classname = this->GetNameOfClass();
  std::string guiClassname = toolkitPrefix + classname + toolkitPostfix;

  std::list<itk::LightObject::Pointer> allGUIs = itk::ObjectFactoryBase::CreateAllInstance(guiClassname.c_str());
  for (auto iter = allGUIs.begin(); iter != allGUIs.end(); ++iter)
  {
    if (object.IsNull())
    {
      object = dynamic_cast<itk::Object *>(iter->GetPointer());
    }
    else
    {
      MITK_ERROR << "There is more than one GUI for " << classname << " (several factories claim ability to produce a "
                 << guiClassname << " ) " << std::endl;
      return nullptr; // people should see and fix this error
    }
  }

  return object;
}

mitk::NodePredicateBase::ConstPointer mitk::Tool::GetReferenceDataPreference() const
{
  return GetSegmentationReferenceImagePredicate();
}

mitk::NodePredicateBase::ConstPointer mitk::Tool::GetWorkingDataPreference() const
{
  return GetMultiLabelSegmentationPredicate();
}

us::ModuleResource mitk::Tool::GetIconResource() const
{
  // Each specific tool should load its own resource. This one will be invalid
  return us::ModuleResource();
}

us::ModuleResource mitk::Tool::GetCursorIconResource() const
{
  // Each specific tool should load its own resource. This one will be invalid
  return us::ModuleResource();
}

bool mitk::Tool::ConfirmBeforeDeactivation()
{
  return false;
}
