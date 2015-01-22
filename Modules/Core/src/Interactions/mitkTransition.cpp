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


#include "mitkTransition.h"
#include "mitkState.h"

mitk::Transition::Transition(std::string name, int nextStateId, int eventId )
  : m_Name(name), m_NextState(NULL),  m_NextStateId(nextStateId), m_EventId(eventId)
{}

mitk::Transition::~Transition()
{
  //needed for correct reference counting of mitkState
  m_NextState = NULL;
  m_Actions.clear();
}

void mitk::Transition::AddAction( Action* action )
{
  m_Actions.push_back( action );
}

std::string mitk::Transition::GetName() const
{
  return m_Name;
}

mitk::State* mitk::Transition::GetNextState() const
{
  return m_NextState.GetPointer();
}

int mitk::Transition::GetNextStateId() const
{
  return m_NextStateId;
}

int mitk::Transition::GetEventId() const
{
  return m_EventId;
}

unsigned int mitk::Transition::GetActionCount() const
{
  return static_cast<unsigned int>(m_Actions.size());
}


mitk::Transition::ActionVectorIterator mitk::Transition::GetActionBeginIterator() const
{
  return m_Actions.begin();
}

mitk::Transition::ActionVectorConstIterator mitk::Transition::GetActionEndIterator() const
{
  return m_Actions.end();
}

bool mitk::Transition::IsEvent(int eventId) const
{
  return (eventId == m_EventId);
}

void mitk::Transition::SetNextState(State* state)
{
  m_NextState = state;
}

