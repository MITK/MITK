#ifndef BASEPROPERTY_H_HEADER_INCLUDED_C1F4DF54
#define BASEPROPERTY_H_HEADER_INCLUDED_C1F4DF54

#include "mitkCommon.h"

namespace mitk {

//##ModelId=3D6A0ED703CF
//##Documentation
//## @brief Base class for properties
//## Base class for properties. Properties are arbitrary additional information
//## (to define a new type of information you have to define a subclass of
//## BaseProperty) that can be added to a PropertyList.
class BaseProperty : public itk::Object
{
public:

    //##ModelId=3E3FF04D0369
	mitkClassMacro(BaseProperty,itk::Object);

protected:
    //##ModelId=3E38FF0902A7
    BaseProperty();

    //##ModelId=3E38FF0902CF
    virtual ~BaseProperty();

    //##ModelId=3E3FE04203D1
	virtual bool operator==(const BaseProperty& property ) const = 0;
};

} // namespace mitk



#endif /* BASEPROPERTY_H_HEADER_INCLUDED_C1F4DF54 */
