#ifndef BASEPROPERTY_H_HEADER_INCLUDED_C1F4DF54
#define BASEPROPERTY_H_HEADER_INCLUDED_C1F4DF54

#include <string>
#include "mitkCommon.h"

namespace mitk {

//##ModelId=3D6A0ED703CF
//##Documentation
//## @brief Abstract base class for properties
//## @ingroup DataTree
//## Base class for properties. Properties are arbitrary additional information
//## (to define a new type of information you have to define a subclass of
//## BaseProperty) that can be added to a PropertyList.
//## Concrete subclasses of BaseProperty should define Set-/Get-methods to assess 
//## the property value, which should be stored by value (not by reference).
//## Subclasses must implement an operator==(const BaseProperty& property), which
//## is used by PropertyList to check whether a property has been changed.
class BaseProperty : public itk::Object
{
public:

    //##ModelId=3E3FF04D0369
  mitkClassMacro(BaseProperty,itk::Object);

  //##ModelId=3E3FE04203D1
  //##Documentation
  //## @brief Subclasses must implement this operator==, which
  //## is used by PropertyList to check whether a property has been changed.
  virtual bool operator==(const BaseProperty& property) const = 0;
  virtual std::string GetValueAsString() const;
  itkSetMacro(Enabled,bool);
  itkGetConstMacro(Enabled,bool);
protected:
  //##ModelId=3E38FF0902A7
    BaseProperty();

    //##ModelId=3E38FF0902CF
    virtual ~BaseProperty();
    bool m_Enabled;
};

} // namespace mitk



#endif /* BASEPROPERTY_H_HEADER_INCLUDED_C1F4DF54 */
