/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitknnInteractiveInteractor.h>

#include <mitkDisplayActionEventBroadcast.h>
#include <mitkToolManager.h>

#include <usGetModuleContext.h>
#include <usModuleResource.h>
#include <usModuleResourceStream.h>

namespace
{
  std::string ReadSvgResource(const std::string& path)
  {
    auto module = us::GetModuleContext()->GetModule();
    auto resource = module->GetResource(path);

    if (!resource.IsValid())
      return {};

    us::ModuleResourceStream input(resource);

    std::ostringstream output;
    output << input.rdbuf();

    return output.str();
  }
}

namespace mitk::nnInteractive
{
  class Interactor::Impl
  {
  public:
    Impl(InteractionType type, InteractionMode mode)
      : Type(type),
        Mode(mode),
        CurrentPromptType(PromptType::Positive),
        IsEnabled(false)
    {
    }

    void BlockLMBDisplayInteraction()
    {
      if (!m_EventConfigBackup.empty())
        return;

      for (const auto& eventObserverReference : us::GetModuleContext()->GetServiceReferences<InteractionEventObserver>())
      {
        auto eventObserver = us::GetModuleContext()->GetService(eventObserverReference);
        auto eventBroadcast = dynamic_cast<DisplayActionEventBroadcast*>(eventObserver);

        if (eventBroadcast == nullptr)
          continue;

        m_EventConfigBackup.emplace_back(eventObserverReference, eventBroadcast->GetEventConfig());
        eventBroadcast->AddEventConfig("DisplayConfigBlockLMB.xml");
      }
    }

    void UnblockLMBDisplayInteraction()
    {
      for (const auto& [eventObserverReference, eventConfig] : m_EventConfigBackup)
      {
        if (eventObserverReference)
        {
          auto eventObserver = us::GetModuleContext()->GetService<InteractionEventObserver>(eventObserverReference);

          if (eventObserver != nullptr)
            static_cast<DisplayActionEventBroadcast*>(eventObserver)->SetEventConfig(eventConfig);
        }
      }

      m_EventConfigBackup.clear();
    }

    InteractionType Type;
    InteractionMode Mode;
    PromptType CurrentPromptType;
    bool IsEnabled;
    ToolManager::Pointer ToolManager;

  private:
    std::vector<std::pair<us::ServiceReference<InteractionEventObserver>, EventConfig>> m_EventConfigBackup;
  };
}

mitk::nnInteractive::Interactor::Interactor(InteractionType type, InteractionMode mode)
  : m_Impl(std::make_unique<Impl>(type, mode))
{
}

mitk::nnInteractive::Interactor::~Interactor()
{
}

void mitk::nnInteractive::Interactor::SetToolManager(ToolManager* toolManager)
{
  if (m_Impl->ToolManager == toolManager)
    return;

  m_Impl->ToolManager = toolManager;

  this->OnSetToolManager();
}

void mitk::nnInteractive::Interactor::HandleEvent(InteractionEvent* event)
{
  if (this->IsEnabled())
    this->OnHandleEvent(event);
}

void mitk::nnInteractive::Interactor::Enable(PromptType promptType)
{
  if (m_Impl->IsEnabled)
  {
    if (promptType == m_Impl->CurrentPromptType)
      return;

    this->Disable();
  }

  if (m_Impl->Mode == InteractionMode::BlockLMBDisplayInteraction)
    m_Impl->BlockLMBDisplayInteraction();

  m_Impl->CurrentPromptType = promptType;

  this->OnEnable();

  m_Impl->IsEnabled = true;
}

void mitk::nnInteractive::Interactor::Disable()
{
  if (!m_Impl->IsEnabled)
    return;

  this->OnDisable();

  m_Impl->IsEnabled = false;

  if (m_Impl->Mode == InteractionMode::BlockLMBDisplayInteraction)
    m_Impl->UnblockLMBDisplayInteraction();
}

bool mitk::nnInteractive::Interactor::IsEnabled() const
{
  return m_Impl->IsEnabled;
}

void mitk::nnInteractive::Interactor::Reset()
{
  this->OnReset();
}

std::string mitk::nnInteractive::Interactor::GetIcon() const
{
  return ReadSvgResource(
    "nnInteractive/" +
    GetInteractionTypeAsString(this->GetType()) + ".svg");
}

std::string mitk::nnInteractive::Interactor::GetCursor(PromptType promptType) const
{
  return ReadSvgResource(
    "nnInteractive/" +
    GetPromptTypeAsString(promptType) +
    GetInteractionTypeAsString(this->GetType()) +
    "Cursor.svg");
}

mitk::nnInteractive::InteractionType mitk::nnInteractive::Interactor::GetType() const
{
  return m_Impl->Type;
}

mitk::ToolManager* mitk::nnInteractive::Interactor::GetToolManager() const
{
  return m_Impl->ToolManager;
}

mitk::DataStorage* mitk::nnInteractive::Interactor::GetDataStorage() const
{
  if (m_Impl->ToolManager == nullptr)
    return nullptr;

  return m_Impl->ToolManager->GetDataStorage();
}

mitk::nnInteractive::PromptType mitk::nnInteractive::Interactor::GetCurrentPromptType() const
{
  return m_Impl->CurrentPromptType;
}
