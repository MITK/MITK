#ifndef ACTION_H_HEADER_INCLUDED_C19AE06B
#define ACTION_H_HEADER_INCLUDED_C19AE06B

#include "mitkCommon.h"
#include "mitkPropertyList.h"


namespace mitk {

class Action 
{
  int m_ActionId;
  PropertyList::Pointer m_PropertiesList;

public:
  Action( int actionId );
  ~Action();
  
  void AddProperty(const char* propertyKey, BaseProperty* property);

  int GetActionId() const;
  mitk::BaseProperty* GetProperty( const char *propertyKey ) const;
};

} // namespace mitk

#endif /* ACTION_H_HEADER_INCLUDED_C19AE06B */
