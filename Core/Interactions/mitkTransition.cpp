/*=========================================================================

  Program:   Medical Imaging & Interaction Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/ for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkTransition.h"
#include "mitkAction.h"

//##ModelId=3E5A3A77006F
mitk::Transition::Transition(std::string name, int nextStateId, int eventId )
  : m_Name(name), m_NextState(NULL),  m_NextStateId(nextStateId), m_EventId(eventId)
{	 
  static int counter = 0;
  std::cout << name << " " << counter++ << std::endl;
}

//##
void mitk::Transition::AddAction( Action* action ) 
{
  m_Actions.push_back( action );

  int count = m_Actions.size();
  std::cout << count << std::cout;  
}

//##ModelId=3E5A3BAE03B5
std::string mitk::Transition::GetName() const
{
	return m_Name;
}

//##ModelId=3E5A3BCE00E0
mitk::State* mitk::Transition::GetNextState() const
{
	return m_NextState;
}

int mitk::Transition::GetNextStateId() const
{
	return m_NextStateId;
}

int mitk::Transition::GetEventId() const 
{
  return m_EventId;
}

//##
int mitk::Transition::GetActionCount() const 
{
  int size = m_Actions.size();
  return size; 
}

//##
std::vector<mitk::Action*>::iterator mitk::Transition::GetActionBeginIterator() const
{
  return m_Actions.begin();
}

//##
const std::vector<mitk::Action*>::iterator mitk::Transition::GetActionEndIterator() const
{
  return m_Actions.end();
}

//##ModelId=3E5A3C1D0289
bool mitk::Transition::IsEvent(int eventId) const
{
	return (eventId == m_EventId);
}

//##ModelId=3E6370080067
void mitk::Transition::SetNextState(State* state)
{
	m_NextState = state;
}

