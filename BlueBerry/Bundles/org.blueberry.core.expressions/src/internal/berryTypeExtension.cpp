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

#include "berryTypeExtension.h"

#include "berryTypeExtensionManager.h"
#include "berryPropertyTesterDescriptor.h"
#include "berryPropertyTester.h"
#include "berryExpressionStatus.h"

#include <berryCoreException.h>

namespace berry {


TypeExtension::TypeExtension(const QString& typeInfo)
 : fTypeInfo(typeInfo), fExtendersLoaded(false), fExtendsLoaded(false)
{

}

IPropertyTester::Pointer TypeExtension::FindTypeExtender(
    TypeExtensionManager& manager, const QString& namespaze, const QString& method,
    bool staticMethod, bool forcePluginActivation)
{
  if (!fExtendersLoaded)
  {
    fExtenders = manager.LoadTesters(fTypeInfo);
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
  // TODO Reflection
//  if (!fExtendsLoaded) {
//    fExtends.clear();
//    Object::ExtTypeInfo types(fTypeInfo);
//    while (!types.m_TypeNames.empty()) {
//      types.m_TypeNames.pop_back();
//      types.m_TypeInfos.pop_back();
//      fExtends.push_back(manager.Get(types));
//    }
//    fExtendsLoaded = true;
//  }
//  for (unsigned int i= 0; i < fExtends.size(); i++) {
//    result = fExtends[i]->FindTypeExtender(manager, namespaze, method, staticMethod, forcePluginActivation);
//    if (result.Cast<CONTINUE_>().IsNull())
//      return result;
//  }
  return CONTINUE_::Pointer(new CONTINUE_());
}


}
