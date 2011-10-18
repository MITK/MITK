/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef MITKGENERICPROPERTY_H_HEADER_INCLUDED_C1061CEE
#define MITKGENERICPROPERTY_H_HEADER_INCLUDED_C1061CEE

#include <string>
#include <sstream>
#include <stdlib.h>

#include "mitkVector.h"
#include <MitkExports.h>
#include "mitkBaseProperty.h"

namespace mitk {

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
    T m_Value;

  private:

    // purposely not implemented
    GenericProperty(const GenericProperty&);
    GenericProperty& operator=(const GenericProperty&);

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
  mitkClassMacro(PropertyName, GenericProperty< Type >);      \
  itkNewMacro(PropertyName);                                  \
  mitkNewMacro1Param(PropertyName, Type);                     \
  using BaseProperty::operator=;                              \
protected:                                                    \
  PropertyName();                                             \
  PropertyName(Type x);                                       \
};

#define mitkDefineGenericProperty(PropertyName,Type,DefaultValue)    \
  mitk::PropertyName::PropertyName()  : Superclass(DefaultValue) { } \
  mitk::PropertyName::PropertyName(Type x) : Superclass(x) {}


#endif /* MITKGENERICPROPERTY_H_HEADER_INCLUDED_C1061CEE */
