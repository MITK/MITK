#ifndef MITKGENERICPROPERTY_H_HEADER_INCLUDED_C1061CEE
#define MITKGENERICPROPERTY_H_HEADER_INCLUDED_C1061CEE

#include "mitkCommon.h"
#include "BaseProperty.h"

namespace mitk {

//##ModelId=3EF99DB6009C
template <class T>
class GenericProperty : public BaseProperty
{
  public:
    mitkClassMacro(GenericProperty, BaseProperty);
    itkNewMacro(Self);
    //##ModelId=3EF99E2C0290
    GenericProperty();

    //##ModelId=3EF99E2C02AE
    virtual ~GenericProperty();
    //##ModelId=3EF99E3A0196
    virtual bool operator==(const BaseProperty& property) const;


  protected:
    //##ModelId=3EF99E45001F
    float m_Value;


};

} // namespace mitk



#endif /* MITKGENERICPROPERTY_H_HEADER_INCLUDED_C1061CEE */
