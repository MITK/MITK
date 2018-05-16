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

#include "mitkStateMachineContainer.h"
#include <algorithm>
#include <mitkStandardFileLocations.h>
#include <vtkObjectFactory.h>
#include <vtkXMLDataElement.h>

// us
#include "usGetModuleContext.h"
#include "usModule.h"
#include "usModuleResource.h"
#include "usModuleResourceStream.h"

/**
 * @brief This class builds up all the necessary structures for a statemachine.
 * and stores one start-state for all built statemachines.
 **/
// XML StateMachine Tags
const std::string NAME = "name";
const std::string CONFIG = "statemachine";
const std::string STATE = "state";
const std::string STATEMODE = "state_mode";
const std::string TRANSITION = "transition";
const std::string EVENTCLASS = "event_class";
const std::string EVENTVARIANT = "event_variant";
const std::string STARTSTATE = "startstate";
const std::string TARGET = "target";
const std::string ACTION = "action";
const std::string CONDITION = "condition";
const std::string INVERTED = "inverted";

namespace mitk
{
  vtkStandardNewMacro(StateMachineContainer);
}

mitk::StateMachineContainer::StateMachineContainer() : m_StartStateFound(false), m_errors(false)
{
}

mitk::StateMachineContainer::~StateMachineContainer()
{
}

/**
 * @brief Loads the xml file filename and generates the necessary instances.
 **/
bool mitk::StateMachineContainer::LoadBehavior(const std::string &fileName, const us::Module *module)
{
  if (module == nullptr)
  {
    module = us::GetModuleContext()->GetModule();
  }
  us::ModuleResource resource = module->GetResource("Interactions/" + fileName);
  if (!resource.IsValid())
  {
    mitkThrow() << ("Resource not valid. State machine pattern not found:" + fileName);
  }
  us::ModuleResourceStream stream(resource);
  this->SetStream(&stream);
  m_Filename = fileName;
  return this->Parse() && !m_errors;
}

mitk::StateMachineState::Pointer mitk::StateMachineContainer::GetStartState() const
{
  return m_StartState;
}

/**
 * @brief sets the pointers in Transition (setNextState(..)) according to the extracted xml-file content
 **/
void mitk::StateMachineContainer::ConnectStates()
{
  for (auto it = m_States.begin(); it != m_States.end(); ++it)
  {
    if ((*it)->ConnectTransitions(&m_States) == false)
      m_errors = true;
  }
}

void mitk::StateMachineContainer::StartElement(const char *elementName, const char **atts)
{
  std::string name(elementName);

  if (name == CONFIG)
  {
    //
  }
  else if (name == STATE)
  {
    std::string stateName = ReadXMLStringAttribut(NAME, atts);
    std::transform(stateName.begin(), stateName.end(), stateName.begin(), ::toupper);
    std::string stateMode = ReadXMLStringAttribut(STATEMODE, atts);
    std::transform(stateMode.begin(), stateMode.end(), stateMode.begin(), ::toupper);
    bool isStartState = ReadXMLBooleanAttribut(STARTSTATE, atts);

    if (isStartState)
    {
      m_StartStateFound = true;
    }

    // sanitize state modes
    if (stateMode == "" || stateMode == "REGULAR")
    {
      stateMode = "REGULAR";
    }
    else if (stateMode != "GRAB_INPUT" && stateMode != "PREFER_INPUT")
    {
      MITK_WARN << "Invalid State Modus " << stateMode << ". Mode assumed to be REGULAR";
      stateMode = "REGULAR";
    }
    m_CurrState = mitk::StateMachineState::New(stateName, stateMode);

    if (isStartState)
      m_StartState = m_CurrState;
  }
  else if (name == TRANSITION)
  {
    std::string eventClass = ReadXMLStringAttribut(EVENTCLASS, atts);
    std::string eventVariant = ReadXMLStringAttribut(EVENTVARIANT, atts);
    std::string target = ReadXMLStringAttribut(TARGET, atts);
    std::transform(target.begin(), target.end(), target.begin(), ::toupper);

    mitk::StateMachineTransition::Pointer transition =
      mitk::StateMachineTransition::New(target, eventClass, eventVariant);
    if (m_CurrState)
    {
      m_CurrState->AddTransition(transition);
    }
    else
    {
      MITK_WARN << "Malformed Statemachine Pattern. Transition has no origin. \n Will be ignored.";
      MITK_WARN << "Malformed Transition details: target=" << target << ", event class:" << eventClass
                << ", event variant:" << eventVariant;
    }
    m_CurrTransition = transition;
  }

  else if (name == ACTION)
  {
    std::string actionName = ReadXMLStringAttribut(NAME, atts);
    mitk::StateMachineAction::Pointer action = mitk::StateMachineAction::New(actionName);
    if (m_CurrTransition)
      m_CurrTransition->AddAction(action);
    else
      MITK_WARN << "Malformed state machine Pattern. Action without transition. \n Will be ignored.";
  }

  else if (name == CONDITION)
  {
    if (!m_CurrTransition)
      MITK_WARN << "Malformed state machine Pattern. Condition without transition. \n Will be ignored.";

    std::string conditionName = ReadXMLStringAttribut(NAME, atts);
    std::string inverted = ReadXMLStringAttribut(INVERTED, atts);
    if (inverted == "" || inverted == "false")
    {
      m_CurrTransition->AddCondition(mitk::StateMachineCondition(conditionName, false));
    }
    else
    {
      m_CurrTransition->AddCondition(mitk::StateMachineCondition(conditionName, true));
    }
  }
}

void mitk::StateMachineContainer::EndElement(const char *elementName)
{
  std::string name(elementName);

  if (name == CONFIG)
  {
    if (m_StartState.IsNull())
    {
      MITK_ERROR << "State machine pattern has no start state and cannot be used: " << m_Filename;
    }
    ConnectStates();
  }
  else if (name == TRANSITION)
  {
    m_CurrTransition = nullptr;
  }
  else if (name == ACTION)
  {
    //
  }
  else if (name == CONDITION)
  {
    //
  }
  else if (name == STATE)
  {
    m_States.push_back(m_CurrState);
    m_CurrState = nullptr;
  }
}

std::string mitk::StateMachineContainer::ReadXMLStringAttribut(std::string name, const char **atts)
{
  if (atts)
  {
    const char **attsIter = atts;

    while (*attsIter)
    {
      if (name == *attsIter)
      {
        attsIter++;
        return *attsIter;
      }
      attsIter++;
      attsIter++;
    }
  }

  return std::string();
}

bool mitk::StateMachineContainer::ReadXMLBooleanAttribut(std::string name, const char **atts)
{
  std::string s = ReadXMLStringAttribut(name, atts);
  std::transform(s.begin(), s.end(), s.begin(), ::toupper);
  if (s == "TRUE")
    return true;
  else
    return false;
}
