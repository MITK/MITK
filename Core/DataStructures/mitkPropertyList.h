#ifndef PROPERTYLIST_H_HEADER_INCLUDED_C1C77D8D
#define PROPERTYLIST_H_HEADER_INCLUDED_C1C77D8D

#include "mitkCommon.h"
#include <itkObjectFactory.h>
#include "BaseProperty.h"
#include <string>
#include <map> 

namespace mitk {

//##ModelId=3E031F200392
class PropertyList : public itk::Object
{
public:
    mitkClassMacro(PropertyList, itk::Object);

    /** Method for creation through the object factory. */
    itkNewMacro(Self);

public:
    //##ModelId=3D6A0E9E0029
    mitk::BaseProperty* GetProperty(const char *propertyKey);

    //##ModelId=3D78B966005F
    bool SetProperty(const char* propertyKey, BaseProperty* property);

protected:
    //##ModelId=3E38FEFE0125
    PropertyList();


    //##ModelId=3E38FEFE0157
    virtual ~PropertyList();

    //##ModelId=3D6A0F0B00BC
    std::map<std::string,BaseProperty::Pointer> properties;
};

} // namespace mitk



#endif /* PROPERTYLIST_H_HEADER_INCLUDED_C1C77D8D */
