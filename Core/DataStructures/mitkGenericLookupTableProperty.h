/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2008-06-24 19:37:48 +0200 (Di, 24 Jun 2008) $
Version:   $Revision: 14641 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef MITKGENERICLOOKUPTABLEPROPERTY_H_HEADER_INCLUDED_C1061CEE
#define MITKGENERICLOOKUPTABLEPROPERTY_H_HEADER_INCLUDED_C1061CEE

#include <string>
#include <sstream>
#include <stdlib.h>

#include "mitkGenericLookupTable.h"
#include "mitkVector.h"
#include "mitkCommon.h"
#include "mitkBaseProperty.h"
#include <mitkXMLWriter.h>
#include <mitkXMLReader.h>

namespace mitk {


/**
 * @brief Template class for generating properties for generic lookup-tables.
 * 
 * This class template can be instantiated for any class instance of 
 * mitk::GenericLookupTable<>. The specified template type of both classes need to match.
 *
 * For example, to encapsulate an mitk::GenericLookupTable< float > in a property,
 * instantiate the template as mitk::GenericLookupTableProperty< float >
 * 
 * Note: you can use the macro mitkSpecializeGenericLookupTableProperty to
 * provide specializations for concrete types (e.g. FloatLookupTableProperty).
 * Please see mitkProperties.h for examples. If you don't use the 
 * mitkSpecializeGenericLookupTableProperty Macro, GetNameOfClass() returns
 * a wrong name.
 */
template <typename T>
class GenericLookupTableProperty : public BaseProperty
{
  public:

    mitkClassMacro(GenericLookupTableProperty, BaseProperty);
    
    typedef GenericLookupTable< T > ValueType;

    virtual ~GenericLookupTableProperty() 
    {
    }

    itkSetObjectMacro(Value,ValueType);
    itkGetObjectMacro(Value,ValueType);

    virtual bool operator==(const BaseProperty& other) const 
    {
      try
      {
        const Self *otherProp = dynamic_cast<const Self*>(&other);
        if(otherProp==NULL) return false;
        if (this->m_Value == otherProp->m_Value) return true;
      }
      catch (std::bad_cast)
      {
        // nothing to do now - just return false
      }

      return false;
    }
    
    virtual std::string GetValueAsString() const 
    {
      std::stringstream myStr;
      myStr << m_Value;
      return myStr.str(); 
    }
    
    virtual bool Assignable(const BaseProperty& other) const
    {
      try
      {
        dynamic_cast<const Self&>(other); // dear compiler, please don't optimize this away!
        return true;
      }
      catch (std::bad_cast)
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
      catch (std::bad_cast)
      {
        // nothing to do then
      }

      return *this;
     }

    virtual bool WriteXMLData( XMLWriter& xmlWriter );

    virtual bool ReadXMLData( XMLReader& xmlReader );

  protected:
    GenericLookupTableProperty() {}
    GenericLookupTableProperty(ValueType *x) : m_Value(x) {}

    typename ValueType::Pointer m_Value;
};

template <class T>
bool GenericLookupTableProperty<T>::WriteXMLData( XMLWriter& /*xmlWriter*/ )
{
  //TODO: implement writer for generic lookup tables (c.f. mitkLookupTableProperty)
  return true;
}

template <class T>
bool GenericLookupTableProperty<T>::ReadXMLData( XMLReader& /*xmlReader*/ )
{
  //TODO: implement reader for generic lookup tables (c.f. mitkLookupTableProperty)
  return true;
}

} // namespace mitk


/**
 * Generates a specialized subclass of mitk::GenericLookupTableProperty. 
 * This way, GetNameOfClass() returns the value provided by PropertyName.
 * Please see mitkProperties.h for examples.
 * @param PropertyName the name of the instantiation of GenericLookupTableProperty
 * @param Type the value type of the GenericLookupTableProperty
 */
#define mitkSpecializeGenericLookupTableProperty(PropertyName,Type)  \
class MITK_CORE_EXPORT PropertyName: public GenericLookupTableProperty< Type >        \
{                                                         \
public:                                                   \
  mitkClassMacro(PropertyName, GenericLookupTableProperty< Type >);  \
  itkNewMacro(PropertyName);                              \
  virtual ~PropertyName() {}                              \
protected:                                                \
  PropertyName() { }              \
  PropertyName(ValueType *x) : GenericLookupTableProperty<Type>(x) {}      \
};

#endif /* MITKGENERICLOOKUPTABLEPROPERTY_H_HEADER_INCLUDED_C1061CEE */
