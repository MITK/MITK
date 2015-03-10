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

class Property {

public:
  typedef Property Self;
  typedef Poco::SharedPtr<Self> Pointer;
  typedef Poco::SharedPtr<const Self> ConstPointer;

private:
  Object::ConstPointer fType;
  QString fNamespace;
  QString fName;

  IPropertyTester::Pointer fTester;

  friend class TypeExtensionManager;
  /* package */ Property(Object::ConstPointer type,
                         const QString& namespaze, const QString& name);

  /* package */ void SetPropertyTester(IPropertyTester::Pointer tester);

public:
  bool IsInstantiated();

  bool IsDeclaringPluginActive();

  bool IsValidCacheEntry(bool forcePluginActivation);

  bool Test(Object::ConstPointer receiver, const QList<Object::Pointer>& args,
            Object::Pointer expectedValue);

  bool operator==(Property& obj);

  bool operator==(Property* obj);

  int HashCode();
};

}  // namespace berry

#endif /*BERRYPROPERTY_H_*/
