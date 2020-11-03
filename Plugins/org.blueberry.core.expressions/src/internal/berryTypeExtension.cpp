/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryTypeExtension.h"

#include "berryTypeExtensionManager.h"
#include "berryPropertyTesterDescriptor.h"
#include "berryPropertyTester.h"
#include "berryExpressionStatus.h"

#include <berryCoreException.h>

namespace berry {

TypeExtension::TypeExtension()
  : fExtendersLoaded(false)
  , fInheritsLoaded(false)
{
  // special constructor to create the CONTINUE instance
}

TypeExtension::TypeExtension(const Reflection::TypeInfo& typeInfo)
  : fTypeInfo(typeInfo)
  , fExtendersLoaded(false)
  , fInheritsLoaded(false)
{
}

IPropertyTester::Pointer TypeExtension::FindTypeExtender(
    TypeExtensionManager& manager, const QString& namespaze, const QString& method,
    bool staticMethod, bool forcePluginActivation)
{
  if (!fExtendersLoaded)
  {
    fExtenders = manager.LoadTesters(fTypeInfo.GetName());
    fExtendersLoaded = true;
  }
  IPropertyTester::Pointer result;

  // handle extenders associated with this type extender
  for (int i= 0; i < fExtenders.size(); i++)
  {
    IPropertyTester::Pointer extender = fExtenders[i];
    if (extender.IsNull() || !extender->Handles(namespaze, method))
      continue;
    if (extender->IsInstantiated())
    {
      // There is no need to check for an active plug-in here. If a plug-in
      // gets uninstalled we receive an registry event which will flush the whole
      // type extender cache and will reinstantiate the testers. However Bundle#stop
      // isn't handled by this. According to bug https://bugs.blueberry.org/bugs/show_bug.cgi?id=130338
      // we don't have to support stop in 3.2. If we have to in the future we have to
      // reactivate the stopped plug-in if we are in forcePluginActivation mode.
      return extender;
    }
    else
    {
      if (extender->IsDeclaringPluginActive() || forcePluginActivation)
      {
        PropertyTesterDescriptor::Pointer descriptor = extender.Cast<PropertyTesterDescriptor>();
        if (!descriptor.IsNull())
        {
          try
          {
            IPropertyTester::Pointer inst(descriptor->Instantiate());
            inst.Cast<PropertyTester>()->InternalInitialize(descriptor);
            fExtenders[i]= extender = inst;
            return extender;
          }
          catch (const CoreException& e)
          {
            fExtenders[i] = IPropertyTester::Pointer();
            throw e;
          }
        }
        else
        {
          fExtenders[i]= IPropertyTester::Pointer();
          IStatus::Pointer status(new ExpressionStatus(ExpressionStatus::TYPE_EXTENDER_INCORRECT_TYPE,
                                                       "The implementation class is not a sub type of berry::PropertyTester",
                                                       BERRY_STATUS_LOC));
          throw CoreException(status);
        }
      }
      else
      {
        return extender;
      }
    }
  }

  // there is no inheritance for static methods
  if (staticMethod)
    return CONTINUE_::Pointer(new CONTINUE_());

  // handle inheritance chain
  if (!fInheritsLoaded)
  {
    fInherits.clear();
    QList<Reflection::TypeInfo> superClasses = fTypeInfo.GetSuperclasses();
    foreach(const Reflection::TypeInfo& superClass, superClasses)
    {
      fInherits.push_back(manager.Get(superClass));
    }
    fInheritsLoaded = true;
  }
  foreach(const TypeExtension::Pointer& typeExtension, fInherits)
  {
    result = typeExtension->FindTypeExtender(manager, namespaze, method, staticMethod, forcePluginActivation);
    if (result.Cast<CONTINUE_>().IsNull())
    {
      return result;
    }
  }
  return CONTINUE_::Pointer(new CONTINUE_());
}

bool TypeExtension::CONTINUE_::Handles(const QString&, const QString&) {
  return false;
}

bool TypeExtension::CONTINUE_::IsInstantiated() {
  return true;
}

bool TypeExtension::CONTINUE_::IsDeclaringPluginActive() {
  return true;
}

IPropertyTester*TypeExtension::CONTINUE_::Instantiate() {
  return this;
}

bool TypeExtension::CONTINUE_::Test(Object::ConstPointer, const QString&, const QList<Object::Pointer>&, Object::Pointer) {
  return false;
}

IPropertyTester::Pointer END_POINT_::FindTypeExtender(TypeExtensionManager&, const QString&, const QString&, bool, bool)
{
  return CONTINUE_::Pointer(new CONTINUE_());
}


}
