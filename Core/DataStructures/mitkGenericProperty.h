#ifndef MITKGENERICPROPERTY_H_HEADER_INCLUDED_C1061CEE
#define MITKGENERICPROPERTY_H_HEADER_INCLUDED_C1061CEE

#include <string>
#include <sstream>

#include "mitkCommon.h"
#include "mitkBaseProperty.h"

namespace mitk {

//##ModelId=3EF99DB6009C
template <typename T>
class GenericProperty : public BaseProperty
{
  public:
   
    mitkClassMacro(GenericProperty<T>, BaseProperty);
//    itkNewMacro(Self);

    //##ModelId=3EF99E2C0290
    GenericProperty() {};
    GenericProperty(T x) : m_Value(x) {};	
    //##ModelId=3EF99E2C02AE
    virtual ~GenericProperty() {}; 
    //##ModelId=3EF99E3A0196
    virtual bool operator==(const BaseProperty& property) const {
        const Self *other = dynamic_cast<const Self*>(&property);
        if(other==NULL) return false;
        return other->m_Value==m_Value;
    }

    virtual T GetValue() const
    {
      return m_Value;
    }


    virtual void SetValue(T x) {
	m_Value = x;
    }	
    std::string GetValueAsString() const {
       std::stringstream myStr;
       myStr << GetValue() ;
       return myStr.str(); 
    }
    
  protected:
    //##ModelId=3EF99E45001F
    T m_Value;

};

} // namespace mitk

#endif /* MITKGENERICPROPERTY_H_HEADER_INCLUDED_C1061CEE */
