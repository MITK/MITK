/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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

  berryObjectMacro(Property);

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

  bool operator==(const Object* obj) const override;

  bool operator<(const Object* obj) const override;

  uint HashCode() const override;
};

}  // namespace berry

#endif /*BERRYPROPERTY_H_*/
