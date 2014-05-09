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


#ifndef MITKGENERICPROPERTY_H_HEADER_INCLUDED_C1061CEE
#define MITKGENERICPROPERTY_H_HEADER_INCLUDED_C1061CEE

#include <string>
#include <sstream>
#include <stdlib.h>

#include "mitkNumericTypes.h"
#include <MitkCoreExports.h>
#include "mitkBaseProperty.h"

namespace mitk {

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable: 4522)
#endif

/*!
  @ brief Template class for generating properties for int, float, bool, etc.

  This class template can be instantiated for all classes/internal types that fulfills
  these requirements:
    - an operator<< so that the properties value can be put into a std::stringstream
    - an operator== so that two properties can be checked for equality

  Note: you must use the macro mitkSpecializeGenericProperty to provide specializations
  for concrete types (e.g. BoolProperty). Please see mitkProperties.h for examples. If you
  don't use the mitkSpecializeGenericProperty Macro, GetNameOfClass() returns a wrong name.

*/
template <typename T>
class MITK_EXPORT GenericProperty : public BaseProperty
{
  public:

    mitkClassMacro(GenericProperty, BaseProperty);
    mitkNewMacro1Param(GenericProperty<T>, T);
    itkCloneMacro(Self)

    typedef T ValueType;

    itkSetMacro(Value,T);
    itkGetConstMacro(Value,T);

    virtual std::string GetValueAsString() const
    {
      std::stringstream myStr;
      myStr << GetValue() ;
      return myStr.str();
    }

    using BaseProperty::operator=;

  protected:
    GenericProperty() {}
    GenericProperty(T x)
       : m_Value(x) {}
    GenericProperty(const GenericProperty& other)
      : BaseProperty(other)
      , m_Value(other.m_Value)
    {}

    T m_Value;

  private:

    // purposely not implemented
    GenericProperty& operator=(const GenericProperty&);

    virtual itk::LightObject::Pointer InternalClone() const
    {
      itk::LightObject::Pointer result(new Self(*this));
      result->UnRegister();
      return result;
    }

    virtual bool IsEqual(const BaseProperty& other) const
    {
      return (this->m_Value == static_cast<const Self&>(other).m_Value);
    }

    virtual bool Assign(const BaseProperty& other)
    {
      this->m_Value = static_cast<const Self&>(other).m_Value;
      return true;
    }
};

#ifdef _MSC_VER
# pragma warning(pop)
#endif

} // namespace mitk

/**
 * Generates a specialized subclass of mitk::GenericProperty.
 * This way, GetNameOfClass() returns the value provided by PropertyName.
 * Please see mitkProperties.h for examples.
 * @param PropertyName the name of the subclass of GenericProperty
 * @param Type the value type of the GenericProperty
 * @param Export the export macro for DLL usage
 */
#define mitkDeclareGenericProperty(PropertyName,Type,Export)  \
class Export PropertyName: public GenericProperty< Type >     \
{                                                             \
public:                                                       \
  mitkClassMacro(PropertyName, GenericProperty< Type >)       \
  itkFactorylessNewMacro(Self)                                \
  itkCloneMacro(Self)                                         \
  mitkNewMacro1Param(PropertyName, Type)                      \
  using BaseProperty::operator=;                              \
protected:                                                    \
  PropertyName();                                             \
  PropertyName(const PropertyName&);                          \
  PropertyName(Type x);                                       \
private:                                                      \
  itk::LightObject::Pointer InternalClone() const;            \
};

#define mitkDefineGenericProperty(PropertyName,Type,DefaultValue)            \
  mitk::PropertyName::PropertyName()  : Superclass(DefaultValue) { }         \
  mitk::PropertyName::PropertyName(const PropertyName& other) : GenericProperty< Type >(other) {} \
  mitk::PropertyName::PropertyName(Type x) : Superclass(x) {}                \
  itk::LightObject::Pointer mitk::PropertyName::InternalClone() const {      \
    itk::LightObject::Pointer result(new Self(*this));                       \
    result->UnRegister();                                                    \
    return result;                                                           \
  }                                                                          \


#endif /* MITKGENERICPROPERTY_H_HEADER_INCLUDED_C1061CEE */
