#ifndef MITKSMARTPOINTERPROPERTY_H_HEADER_INCLUDED_C126B791
#define MITKSMARTPOINTERPROPERTY_H_HEADER_INCLUDED_C126B791

#include "mitkCommon.h"
#include "mitkBaseProperty.h"

namespace mitk {

//##ModelId=3ED94AC90382
//##Documentation
//## @brief Property containing a smart-pointer
//## @ingroup DataTree
class SmartPointerProperty : public BaseProperty
{
  public:
    mitkClassMacro(SmartPointerProperty, BaseProperty);

    //##ModelId=3ED94B4203C1
    virtual bool operator==(const BaseProperty& property) const;
    //##ModelId=3ED94B7500F2
    SmartPointerProperty(itk::Object* pointer);

    //##ModelId=3ED94B750111
    virtual ~SmartPointerProperty();

    //##ModelId=3ED952AD02B7
    itk::Object::Pointer GetSmartPointer() const;

    //##ModelId=3ED952AD02F6
    void SetSmartPointer(itk::Object* pointer);
  protected:
    //##ModelId=3ED95309021C
    itk::Object::Pointer m_SmartPointer;
};

} // namespace mitk



#endif /* MITKSMARTPOINTERPROPERTY_H_HEADER_INCLUDED_C126B791 */
