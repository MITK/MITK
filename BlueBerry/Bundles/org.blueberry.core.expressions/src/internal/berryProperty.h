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

#ifndef BERRYPROPERTY_H_
#define BERRYPROPERTY_H_

#include "berryIPropertyTester.h"

#include "berryObject.h"

#include "Poco/SharedPtr.h"
#include "Poco/Any.h"

#include <vector>
#include <typeinfo>

namespace berry {

class Property : public Object {

public:

  berryObjectMacro(Property)

private:
  Reflection::TypeInfo fType;
  QString fNamespace;
  QString fName;

  IPropertyTester::Pointer fTester;

  friend class TypeExtensionManager;
  /* package */ Property(const Reflection::TypeInfo& typeInfo,
                         const QString& namespaze, const QString& name);

  /* package */ void SetPropertyTester(IPropertyTester::Pointer tester);

public:
  bool IsInstantiated() const;

  bool IsDeclaringPluginActive() const;

  bool IsValidCacheEntry(bool forcePluginActivation) const;

  bool Test(Object::ConstPointer receiver, const QList<Object::Pointer>& args,
            Object::Pointer expectedValue);

  bool operator==(const Object* obj) const;

  bool operator<(const Object* obj) const;

  uint HashCode() const;
};

}  // namespace berry

#endif /*BERRYPROPERTY_H_*/
