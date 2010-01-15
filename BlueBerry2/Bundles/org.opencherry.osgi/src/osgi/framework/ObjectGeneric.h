/*=========================================================================

 Program:   openCherry Platform
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


#ifndef OSGI_FRAMEWORK_OBJECTGENERIC_H_
#define OSGI_FRAMEWORK_OBJECTGENERIC_H_

#include <sstream>
#include "../../cherryOSGiDll.h"
#include "osgi/framework/Macros.h"
#include "osgi/framework/Object.h"

namespace osgi {

namespace framework {

template <typename T>
class CHERRY_OSGI ObjectGeneric : public Object
{
  public:

    osgiObjectMacro(osgi::framework::ObjectGeneric<T>);

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

    Object::Pointer Clone() const
    {
      Object::Pointer clone(new Self(m_Value));
      return clone;
    }

    virtual std::string GetValueAsString() const
    {
      std::stringstream myStr;
      myStr << GetValue() ;
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


}
} // namespace

/**
 * Generates a specialized subclass of cherry::ObjectGeneric.
 * This way, GetClassName() returns the value provided by ObjectName.
 * Please see osgi/framework/Objects.h for examples
 * @param ObjectName the name of the instantiation of ObjectGeneric
 * @param Type the value type
 */
#define osgiSpecializeGenericObject(ns,ObjectName,Type,DefaultValue)  \
class CHERRY_OSGI ObjectName: public ::osgi::framework::ObjectGeneric< Type >           \
{                                                                    \
public:                                                              \
  osgiObjectMacro(ns::ObjectName);                                      \
  ObjectName() : ::osgi::framework::ObjectGeneric< Type >(DefaultValue) { }                           \
  ObjectName(Type x) : ::osgi::framework::ObjectGeneric<Type>(x) {}                     \
  /*ObjectName(const ObjectName& o) : ObjectGeneric< Type >(o) {} */ \
};


#endif /* OSGI_FRAMEWORK_OBJECTGENERIC_H_ */
