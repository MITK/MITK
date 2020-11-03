/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryProperty.h"

#include <cassert>

namespace berry {

Property::Property(const Reflection::TypeInfo& typeInfo,
                   const QString &namespaze, const QString &name)
 : fType(typeInfo), fNamespace(namespaze), fName(name)
{
}

void Property::SetPropertyTester(IPropertyTester::Pointer tester)
{
  assert(tester.GetPointer() != nullptr);
  fTester = tester;
}

bool Property::IsInstantiated() const
{
  return fTester->IsInstantiated();
}

bool Property::IsDeclaringPluginActive() const
{
  return fTester->IsDeclaringPluginActive();
}

bool Property::IsValidCacheEntry(bool forcePluginActivation) const
{
  if (forcePluginActivation)
  {
    return this->IsInstantiated() && this->IsDeclaringPluginActive();
  }
  else
  {
    return  (this->IsInstantiated() && this->IsDeclaringPluginActive()) ||
        (!this->IsInstantiated() && !this->IsDeclaringPluginActive());
  }
}

bool Property::Test(Object::ConstPointer receiver, const QList<Object::Pointer>& args,
                    Object::Pointer expectedValue)
{
  return fTester->Test(receiver, fName, args, expectedValue);
}

bool Property::operator==(const Object* obj) const
{
  if (const Property* other = dynamic_cast<const Property*>(obj))
  {
    return fType == other->fType && fNamespace == other->fNamespace &&
        fName == other->fName;
  }
  return false;
}

bool Property::operator<(const Object* obj) const
{
  return this->HashCode() < obj->HashCode();
}

uint Property::HashCode() const
{
  return qHash(fType.GetName()) << 16 |
                                   qHash(fNamespace) << 8 |
                                   qHash(fName);
}

}
