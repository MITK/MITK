#include "mitkAction.h"


namespace mitk {

/**
 *
 */
Action::Action( int actionId )
:m_ActionId( actionId )
{ 
  m_PropertiesList = PropertyList::New();
}

/**
 *
 */
Action::~Action() 
{
}

/**
 *
 */
void Action::AddProperty(const char* propertyKey, BaseProperty* property) 
{
  return;
}


/**
 *
 */
int Action::GetActionId() const
{
  return m_ActionId;
}


/**
 *
 */
mitk::BaseProperty* Action::GetProperty( const char *propertyKey ) const 
{
  return m_PropertiesList->GetProperty( propertyKey );
}

} // namespace mitk
