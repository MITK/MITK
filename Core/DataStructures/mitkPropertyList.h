#ifndef PROPERTYLIST_H_HEADER_INCLUDED_C1C77D8D
#define PROPERTYLIST_H_HEADER_INCLUDED_C1C77D8D

#include "mitkCommon.h"
#include <itkObjectFactory.h>
#include "BaseProperty.h"
#include <string>
#include <map> 

namespace mitk {

//##ModelId=3E031F200392
//##Documentation
//## @brief List of properties (instances of subclasses of BaseProperty)
//## List of properties (instances of subclasses of BaseProperty). The
//## properties are stored in a map, thus each property is associated with a
//## key of type string. 
class PropertyList : public itk::Object
{
public:
    mitkClassMacro(PropertyList, itk::Object);

    /** Method for creation through the object factory. */
    itkNewMacro(Self);

public:
    //##ModelId=3D6A0E9E0029
    mitk::BaseProperty::Pointer GetProperty(const char *propertyKey);

    //##ModelId=3D78B966005F
    bool SetProperty(const char* propertyKey, BaseProperty* property);

    //##ModelId=3ED94AAE0075
    virtual unsigned long GetMTime() const;
protected:
    //##ModelId=3E38FEFE0125
    PropertyList();


    //##ModelId=3E38FEFE0157
    virtual ~PropertyList();

    //##ModelId=3D6A0F0B00BC
    std::map<std::string,BaseProperty::Pointer> m_Properties;
};

} // namespace mitk



#endif /* PROPERTYLIST_H_HEADER_INCLUDED_C1C77D8D */
