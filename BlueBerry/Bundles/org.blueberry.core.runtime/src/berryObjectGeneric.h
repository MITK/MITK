/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#ifndef BERRYOBJECTGENERIC_H_
#define BERRYOBJECTGENERIC_H_

#include <sstream>
#include <org_blueberry_core_runtime_Export.h>
#include "berryMacros.h"
#include "berryObject.h"

namespace berry {

template <typename T>
class org_blueberry_core_runtime_EXPORT ObjectGeneric : public Object
{
  public:

    berryObjectMacro(ObjectGeneric);

    typedef T ValueType;

    ObjectGeneric() : m_Value(0) {}
    ObjectGeneric(T x) : m_Value(x) {}
    //ObjectGeneric(const Self& o) { m_Value = o.m_Value; }

    virtual ~ObjectGeneric()
    {
    }

    void SetValue(T val) {
      m_Value = val;
    }

    T GetValue() const {
      return m_Value;
    }

    bool operator==(const Object* o) const
    {
      if(const Self* other = dynamic_cast<const Self*>(o))
        return (this->m_Value == other->m_Value);

      return false;
    }

    virtual std::string GetValueAsString() const
    {
      std::stringstream myStr;
      std::locale originalLocale = myStr.getloc();
      std::locale C("C");
      myStr.imbue(C);
      myStr << GetValue() ;
      myStr.imbue(originalLocale);
      return myStr.str();
    }

    virtual bool Assignable(Object::ConstPointer other) const
    {
      return other.Cast<const Self>() != 0;
    }

    virtual void Assign(Object::ConstPointer other)
    {
      ConstPointer specOther = other.Cast<const Self>();

      if (specOther && this->m_Value != specOther->m_Value)
      {
        this->m_Value = specOther->m_Value;
      }
    }

  protected:

    T m_Value;
};


} // namespace berry

/**
 * Generates a specialized subclass of berry::ObjectGeneric.
 * This way, GetClassName() returns the value provided by ObjectName.
 * Please see berryObjects.h for examples
 * @param ObjectName the name of the instantiation of ObjectGeneric
 * @param Type the value type
 */
#define berrySpecializeGenericObject(ObjectName,Type,DefaultValue)   \
class org_blueberry_core_runtime_EXPORT ObjectName: public ::berry::ObjectGeneric< Type > \
{                                                                    \
public:                                                              \
  berryObjectMacro(ObjectName);                                      \
  ObjectName() : ::berry::ObjectGeneric< Type >(DefaultValue) { }    \
  ObjectName(Type x) : ::berry::ObjectGeneric<Type>(x) {}            \
  QString ToString() const { QString txt; QTextStream ts(&txt); ts << m_Value; return txt; } \
};


#endif /* BERRYOBJECTGENERIC_H_ */
