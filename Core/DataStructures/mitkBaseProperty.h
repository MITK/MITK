#ifndef BASEPROPERTY_H_HEADER_INCLUDED_C1F4DF54
#define BASEPROPERTY_H_HEADER_INCLUDED_C1F4DF54

#include "mitkCommon.h"

namespace mitk {

//##ModelId=3D6A0ED703CF
class BaseProperty : public itk::LightObject
{
public:
    /** Standard class typedefs. */
    //##ModelId=3E394E9901A5
    typedef BaseProperty             Self;
    //##ModelId=3E394E9901E1
    typedef itk::LightObject         Superclass;
    //##ModelId=3E394E990209
    typedef itk::SmartPointer<Self>  Pointer;
    //##ModelId=3E394E990231
    typedef itk::SmartPointer<const Self>  ConstPointer;

    /** Method for creation through the object factory. */
    itkNewMacro(Self);

    /** Run-time type information (and related methods). */
    itkTypeMacro(BaseProperty,itk::LightObject);
protected:
    //##ModelId=3E38FF0902A7
    BaseProperty();

    //##ModelId=3E38FF0902CF
    virtual ~BaseProperty();

};

} // namespace mitk



#endif /* BASEPROPERTY_H_HEADER_INCLUDED_C1F4DF54 */
