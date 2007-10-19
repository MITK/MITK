#include "mitkTool.h"

mitk::Tool::Tool(const char* type)
:StateMachine(type)
{
}

mitk::Tool::~Tool()
{
}
    
const char* mitk::Tool::GetGroup() const
{
  return "default";
}

void mitk::Tool::SetToolManager(ToolManager* manager)
{
  m_ToolManager = manager;
}

void mitk::Tool::Activated()
{
}

void mitk::Tool::Deactivated()
{
  StateMachine::ResetStatemachineToStartState(); // forget about the past
}


//--------------------------------------------------------------------------------
//----          ToolLogger
//--------------------------------------------------------------------------------

unsigned int mitk::Tool::ToolLogger::s_Verboseness = 0;
mitk::Tool::ToolLogger::NullStream mitk::Tool::ToolLogger::s_NullStream;

std::ostream& mitk::Tool::ToolLogger::Logger(unsigned int verboseness)
{
  if (verboseness <= s_Verboseness)
  {
    return std::cout;
  }
  else
  {
    return s_NullStream;
  }
}

unsigned int mitk::Tool::ToolLogger::GetVerboseness()
{
  return s_Verboseness;
}

void mitk::Tool::ToolLogger::SetVerboseness( unsigned int verboseness )
{
  s_Verboseness = verboseness;
}

