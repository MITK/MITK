#include "PropertyList.h"

//##ModelId=3D6A0E9E0029
mitk::BaseProperty::Pointer mitk::PropertyList::GetProperty(const char *propertyKey) 
{
    std::map<std::string,BaseProperty::Pointer>::const_iterator it;
    
    it=m_Properties.find( propertyKey );

    if(it!=m_Properties.end())
	    return it->second;
    else 
        return NULL;
}

//##ModelId=3D78B966005F
bool mitk::PropertyList::SetProperty(const char* propertyKey, BaseProperty* property)
{
    std::map<std::string,BaseProperty::Pointer>::const_iterator it;
    
    it=m_Properties.find( propertyKey );
    //is the property with the identical value already contained in the list?
    if((it!=m_Properties.end()) && (*it->second==*property))
        //yes? return!
        return false;
    //no? add/replace it.
	m_Properties.insert ( std::pair<std::string,BaseProperty::Pointer>( propertyKey, property ) );	
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

unsigned long mitk::PropertyList::GetMTime() const
{
    std::map<std::string,BaseProperty::Pointer>::const_iterator it=m_Properties.begin();
	for(;it!=m_Properties.end();++it)
	{
		if(Superclass::GetMTime()<it->second->GetMTime())
		{
			Modified();
			break;
		}
	}
    return Superclass::GetMTime();
}
