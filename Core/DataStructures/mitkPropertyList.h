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
//## @ingroup DataTree
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
    //##Documentation
    //## @brief Get a property by its name. 
    mitk::BaseProperty::Pointer GetProperty(const char *propertyKey) const;

    //##ModelId=3D78B966005F
    //##Documentation
    //## @brief Set a property in the list/map (if it is not already - with the identical value - in it). 
    //## 
    //## Each property gets a key (@a propertyKey) to make it accessible. 
    //## This method uses the operator== of BaseProperty.
    bool SetProperty(const char* propertyKey, BaseProperty* property);

    //##ModelId=3ED94AAE0075
    //##Documentation
    //## @brief Get the timestamp of the last change of the map or the last change of one of 
    //## the property store in the list (whichever is later).
    virtual unsigned long GetMTime() const;

    //##ModelId=3EF1B0160286
    //##Documentation
    //## @brief Remove a property from the list/map.
    bool DeleteProperty(const char* propertyKey);

protected:
    //##ModelId=3E38FEFE0125
    PropertyList();


    //##ModelId=3E38FEFE0157
    virtual ~PropertyList();

    //##ModelId=3D6A0F0B00BC
    //##Documentation
    //## @brief Map of properties.
    std::map<std::string,BaseProperty::Pointer> m_Properties;
};

} // namespace mitk



#endif /* PROPERTYLIST_H_HEADER_INCLUDED_C1C77D8D */
