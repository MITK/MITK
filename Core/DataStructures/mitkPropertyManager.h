#ifndef MITKPROPERTYMANAGER_H_HEADER_INCLUDED
#define MITKPROPERTYMANAGER_H_HEADER_INCLUDED
#include "mitkPropertyList.h"
#include <set>
namespace mitk {
  class PropertyManager {

    public:
      typedef std::set<std::string> PropertyNameSet; 
//      typedef PropertyNameSetterator PropertyNameIterator;
      static const PropertyNameSet& GetDefaultPropertyNames();
      static BaseProperty::Pointer CreateDefaultProperty(std::string name);

    protected:
      PropertyManager();
      PropertyNameSet m_DefaultPropertyNameSet;
      static void InitDefaultPropertyNames();
  }      ;
}  


#endif /* MITKPROPERTYMANAGER_H_HEADER_INCLUDED */

