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
#include <vtkXMLDataElement.h>
#include <mitkStandardFileLocations.h>
#include <vtkObjectFactory.h>

/**
 * @brief This class builds up all the necessary structures for a statemachine.
 * and stores one start-state for all built statemachines.
 **/
//mitk::StateMachineFactory::StartStateMap mitk::StateMachineFactory::m_StartStates;
//mitk::StateMachineFactory::AllStateMachineMapType mitk::StateMachineFactory::m_AllStateMachineMap;
//std::string mitk::StateMachineFactory::s_LastLoadedBehavior;
//XML StateMachine Tags
const std::string NAME = "name";
const std::string STATE_MACHINE = "statemachine";
const std::string STATE = "state";
const std::string TRANSITION = "transition";
const std::string EVENTCLASS = "event_class";
const std::string EVENTVARIANT = "event_variant";
const std::string STARTSTATE = "startstate";
const std::string TARGET = "target";
const std::string ACTION = "action";

namespace mitk
{
  vtkStandardNewMacro(StateMachineContainer);
}

mitk::StateMachineContainer::StateMachineContainer()
{
}

mitk::StateMachineContainer::~StateMachineContainer()
{
}

/**
 * @brief Loads the xml file filename and generates the necessary instances.
 **/
bool mitk::StateMachineContainer::LoadBehavior(std::string fileName)
{
  if (fileName.empty())
    return false;

  this->SetFileName(fileName.c_str());

  return this->Parse();
  return true;
}

mitk::StateMachineState::Pointer mitk::StateMachineContainer::GetStartState()
{
  return m_StartState;
}

/**
 * @brief sets the pointers in Transition (setNextState(..)) according to the extracted xml-file content
 **/
bool mitk::StateMachineContainer::ConnectStates()
{
  return false;
}

void mitk::StateMachineContainer::StartElement(const char* elementName, const char **atts)
{

  /*
   * TODO: Check if necessary information has been collected i.o. to construct the states/actions/transition, if not, warn user.
   * TODO: test if it works ..
   */

  std::string name(elementName);

  if (name == STATE_MACHINE)
  {
    //
  }
  else if (name == STATE)
  {
    std::string stateName = ReadXMLStringAttribut(NAME, atts);
    bool isStartState = ReadXMLBooleanAttribut(STARTSTATE, atts);

    m_CurrState = mitk::StateMachineState::New(stateName);

    if (isStartState)
      m_StartState = m_CurrState;
  }
  else if (name == TRANSITION)
  {
    std::string eventClass = ReadXMLStringAttribut(EVENTCLASS, atts);
    std::string eventVariant = ReadXMLStringAttribut(EVENTVARIANT, atts);
    std::string target = ReadXMLStringAttribut(TARGET, atts);

    mitk::StateMachineTransition::Pointer transition = mitk::StateMachineTransition::New(target, eventClass, eventVariant);
    if (m_CurrState)
      m_CurrState->AddTransition(transition);
    else
      MITK_WARN<< "Malformed Statemachine Pattern. Transition has no origin. \n Will be ignored.";

    m_CurrTransition = transition;
  }

  else if (name == ACTION)
  {
    std::string actionName = ReadXMLStringAttribut(ACTION, atts);
    mitk::StateMachineAction::Pointer action = mitk::StateMachineAction::New(actionName);
    if (m_CurrTransition)
      m_CurrTransition->AddAction(action);
    else
      MITK_WARN<< "Malformed Statemachine Pattern. Action without transition. \n Will be ignored.";
    }

  }

void mitk::StateMachineContainer::EndElement(const char* elementName)
{
  std::string name(elementName);

  if (name == STATE_MACHINE)
  {
    //
  }
  else if (name == TRANSITION)
  {
    m_CurrTransition = NULL;
  }
  else if (name == ACTION)
  {
    //
  }
  else if (name == STATE)
  {
    m_States.push_back(m_CurrState);
    m_CurrState = NULL;
  }
}

std::string mitk::StateMachineContainer::ReadXMLStringAttribut(std::string name, const char** atts)
{
  if (atts)
  {
    const char** attsIter = atts;

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

bool mitk::StateMachineContainer::ReadXMLBooleanAttribut(std::string name, const char** atts)
{
  std::string s = ReadXMLStringAttribut(name, atts);

  if (s == "TRUE")
    return true;
  else
    return false;
}

