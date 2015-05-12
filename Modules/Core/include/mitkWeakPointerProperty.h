/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#ifndef MITKWEAKPOINTERPROPERTY_H_HEADER_INCLUDED_C126B791
#define MITKWEAKPOINTERPROPERTY_H_HEADER_INCLUDED_C126B791

#include <MitkCoreExports.h>
#include "mitkBaseProperty.h"
#include "itkWeakPointer.h"

namespace mitk {

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable: 4522)
#endif

  //##Documentation
  //## @brief Property containing a smart-pointer
  //##
  //## @ingroup DataManagement
  class MITKCORE_EXPORT WeakPointerProperty : public BaseProperty
  {
  public:
    mitkClassMacro(WeakPointerProperty, BaseProperty);

    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)
    mitkNewMacro1Param(WeakPointerProperty, itk::Object*);

    virtual ~WeakPointerProperty();

    typedef itk::WeakPointer<itk::Object> ValueType;

    ValueType GetWeakPointer() const;
    ValueType GetValue() const;

    void SetWeakPointer(itk::Object* pointer);
    void SetValue(const ValueType& value);

    virtual std::string GetValueAsString() const override;

    using BaseProperty::operator=;

  protected:
    itk::WeakPointer<itk::Object> m_WeakPointer;

    WeakPointerProperty(const WeakPointerProperty&);

    WeakPointerProperty(itk::Object* pointer = nullptr);

  private:
    // purposely not implemented
    WeakPointerProperty& operator=(const WeakPointerProperty&);

    itk::LightObject::Pointer InternalClone() const override;

    virtual bool IsEqual(const BaseProperty& property) const override;
    virtual bool Assign(const BaseProperty& property) override;
  };

#ifdef _MSC_VER
# pragma warning(pop)
#endif

} // namespace mitk



#endif /* MITKWEAKPOINTERPROPERTY_H_HEADER_INCLUDED_C126B791 */
