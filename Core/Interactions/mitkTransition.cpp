#include "mitkTransition.h"

//##ModelId=3E5A3A77006F
mitk::Transition::Transition(std::string name, int nextStateId, int eventId )
: m_Name(name), m_NextStateId(nextStateId), m_EventId(eventId), m_NextState(NULL)
{	 
  static int counter = 0;
  std::cout << name << " " << counter++ << std::endl;
}

//##
void mitk::Transition::AddActionID( int actionId ) 
{
  m_ActionIds.push_back( actionId );

  int count = m_ActionIds.size();
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
  int size = m_ActionIds.size();
  return size; 
}

//##
std::vector<int>::iterator mitk::Transition::GetActionIdBeginIterator() const
{
  return m_ActionIds.begin();
}

//##
const std::vector<int>::iterator mitk::Transition::GetActionIdEndIterator() const
{
  return m_ActionIds.end();
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

