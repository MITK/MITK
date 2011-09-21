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
class GenericProperty : public BaseProperty
{
  public:

    mitkClassMacro(GenericProperty, BaseProperty);
    mitkNewMacro1Param(GenericProperty<T>, T);
    
    typedef T ValueType;

    virtual ~GenericProperty() 
    {
    }

    itkSetMacro(Value,T);
    itkGetConstMacro(Value,T);

    virtual bool operator==(const BaseProperty& other) const 
    {
      try
      {
        const Self *otherProp = dynamic_cast<const Self*>(&other);
        if(otherProp==NULL) return false;
        if (this->m_Value == otherProp->m_Value) return true;
      }
      catch (std::bad_cast&)
      {
        // nothing to do now - just return false
      }

      return false;
    }
    
    virtual std::string GetValueAsString() const 
    {
      std::stringstream myStr;
      myStr << GetValue() ;
      return myStr.str(); 
    }
    
    virtual bool Assignable(const BaseProperty& other) const
    {
      try
      {
        dynamic_cast<const Self&>(other); // dear compiler, please don't optimize this away!
        return true;
      }
      catch (std::bad_cast&)
      {
      }
      return false;
    }

    virtual BaseProperty& operator=(const BaseProperty& other)
    {
      try
      {
        const Self& otherProp( dynamic_cast<const Self&>(other) );

        if (this->m_Value != otherProp.m_Value)
        {
          this->m_Value = otherProp.m_Value;
          this->Modified();
        }
      }
      catch (std::bad_cast&)
      {
        // nothing to do then
      }

      return *this;
     }

  protected:
    GenericProperty() {}
    GenericProperty(T x) 
       : m_Value(x) {}
    T m_Value;
};

} // namespace mitk

/**
 * Generates a specialized subclass of mitk::GenericProperty. 
 * This way, GetNameOfClass() returns the value provided by PropertyName.
 * Please see mitkProperties.h for examples.
 * @param PropertyName the name of the instantiation of GenericProperty
 * @param Type the value type of the GenericProperty
 */
#define mitkSpecializeGenericProperty(PropertyName,Type,DefaultValue)  \
class MITK_EXPORT PropertyName: public GenericProperty< Type >        \
{                                                         \
public:                                                   \
  mitkClassMacro(PropertyName, GenericProperty< Type >);  \
  itkNewMacro(PropertyName);                              \
  mitkNewMacro1Param(PropertyName, Type);                 \
  virtual ~PropertyName() {}                              \
  virtual BaseProperty& operator=(const BaseProperty& other) { return Superclass::operator=(other); } \
protected:                                                \
  PropertyName() { m_Value = DefaultValue; }              \
  PropertyName(Type x) : GenericProperty<Type>(x) {}      \
};

#endif /* MITKGENERICPROPERTY_H_HEADER_INCLUDED_C1061CEE */


