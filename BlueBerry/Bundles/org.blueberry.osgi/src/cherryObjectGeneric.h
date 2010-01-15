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


#ifndef CHERRYOBJECTGENERIC_H_
#define CHERRYOBJECTGENERIC_H_

#include <sstream>
#include "cherryOSGiDll.h"
#include "cherryMacros.h"
#include "cherryObject.h"

namespace cherry {

template <typename T>
class CHERRY_OSGI ObjectGeneric : public Object
{
  public:

    cherryObjectMacro(ObjectGeneric);

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


} // namespace cherry

/**
 * Generates a specialized subclass of cherry::ObjectGeneric.
 * This way, GetClassName() returns the value provided by ObjectName.
 * Please see cherryObjects.h for examples
 * @param ObjectName the name of the instantiation of ObjectGeneric
 * @param Type the value type
 */
#define cherrySpecializeGenericObject(ObjectName,Type,DefaultValue)  \
class CHERRY_OSGI ObjectName: public ::cherry::ObjectGeneric< Type >           \
{                                                                    \
public:                                                              \
  cherryObjectMacro(ObjectName);                                      \
  ObjectName() : ::cherry::ObjectGeneric< Type >(DefaultValue) { }                           \
  ObjectName(Type x) : ::cherry::ObjectGeneric<Type>(x) {}                     \
  /*ObjectName(const ObjectName& o) : ObjectGeneric< Type >(o) {} */ \
};


#endif /* CHERRYOBJECTGENERIC_H_ */
