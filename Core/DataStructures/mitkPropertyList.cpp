#include "PropertyList.h"

//##ModelId=3D6A0E9E0029
mitk::BaseProperty* mitk::PropertyList::GetProperty(const char *propertyKey) 
{
	return properties.find( propertyKey )->second;
}

//##ModelId=3D78B966005F
bool mitk::PropertyList::SetProperty(const char* propertyKey, BaseProperty* property)
{
	properties.insert ( std::pair<std::string,BaseProperty::Pointer>( propertyKey, property ) );	
    return true;
}

//##ModelId=3E38FEFE0125
mitk::PropertyList::PropertyList()
{
}


//##ModelId=3E38FEFE0157
mitk::PropertyList::~PropertyList()
{
}

