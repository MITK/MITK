#include "PropertyList.h"

//##ModelId=3D6A0E9E0029
mitk::BaseProperty::Pointer mitk::PropertyList::GetProperty(const char *propertyKey) const
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
    std::map<std::string,BaseProperty::Pointer>::iterator it;
    
    it=m_Properties.find( propertyKey );
    //is a property with key @a propertyKey contained in the list?
    if(it!=m_Properties.end())
        //yes?
    {
        //is the property contained in the list identical to the new one?
        if( *it->second == *property)
            // yes? do nothing and return.
            return false;
        //no? erase the old entry.
        m_Properties.erase(it);
    }

    //no? add/replace it.
	//std::pair<std::map<std::string,BaseProperty::Pointer>::iterator, bool> o=
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
//##ModelId=3EF1B0160286
bool mitk::PropertyList::DeleteProperty(const char* propertyKey)
{
    return m_Properties.erase(propertyKey)>0;
}

