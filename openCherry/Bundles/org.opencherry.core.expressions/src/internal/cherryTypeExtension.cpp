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

#include "cherryTypeExtension.h"

#include "cherryTypeExtensionManager.h"
#include "cherryPropertyTesterDescriptor.h"
#include "../cherryPropertyTester.h"

namespace cherry {


TypeExtension::TypeExtension(ExpressionVariable::ExtTypeInfo typeInfo)
 : fTypeInfo(typeInfo), fExtendersLoaded(false), fExtendsLoaded(false) {

}

 IPropertyTester::Pointer
 TypeExtension::FindTypeExtender(TypeExtensionManager& manager,
     const std::string& namespaze, const std::string& method,
     bool staticMethod, bool forcePluginActivation) {
  if (!fExtendersLoaded) {
    manager.LoadTesters(fExtenders, fTypeInfo.m_TypeNames.back());
    fExtendersLoaded = true;
  }
  IPropertyTester::Pointer result;

  // handle extenders associated with this type extender
  for (unsigned int i= 0; i < fExtenders.size(); i++) {
    IPropertyTester::Pointer extender = fExtenders[i];
    if (extender.IsNull() || !extender->Handles(namespaze, method))
      continue;
    if (extender->IsInstantiated()) {
      // There is no need to check for an active plug-in here. If a plug-in
      // gets uninstalled we receive an registry event which will flush the whole
      // type extender cache and will reinstantiate the testers. However Bundle#stop
      // isn't handled by this. According to bug https://bugs.opencherry.org/bugs/show_bug.cgi?id=130338
      // we don't have to support stop in 3.2. If we have to in the future we have to
      // reactivate the stopped plug-in if we are in forcePluginActivation mode.
      return extender;
    } else {
      if (extender->IsDeclaringPluginActive() || forcePluginActivation) {
        PropertyTesterDescriptor::Pointer descriptor = extender.Cast<PropertyTesterDescriptor>();
        if (!descriptor.IsNull())
        {
        try {
          IPropertyTester::Pointer inst(descriptor->Instantiate());
          inst.Cast<PropertyTester>()->InternalInitialize(descriptor);
          fExtenders[i]= extender = inst;
          return extender;
        } catch (CoreException e) {
          fExtenders[i] = IPropertyTester::Pointer();
          throw e;
        }
        }
        else {
          fExtenders[i]= IPropertyTester::Pointer();
          throw CoreException("Type extender has incorrect type");
        }
      } else {
        return extender;
      }
    }
  }

  // there is no inheritance for static methods
  if (staticMethod)
    return CONTINUE_::Pointer(new CONTINUE_());

  // handle inheritance chain
  if (!fExtendsLoaded) {
    fExtends.clear();
    ExpressionVariable::ExtTypeInfo types(fTypeInfo);
    while (!types.m_TypeNames.empty()) {
      types.m_TypeNames.pop_back();
      types.m_TypeInfos.pop_back();
      fExtends.push_back(manager.Get(types));
    }
    fExtendsLoaded = true;
  }
  for (unsigned int i= 0; i < fExtends.size(); i++) {
    result = fExtends[i]->FindTypeExtender(manager, namespaze, method, staticMethod, forcePluginActivation);
    if (result.Cast<CONTINUE_>().IsNull())
      return result;
  }
  return CONTINUE_::Pointer(new CONTINUE_());
}


}
