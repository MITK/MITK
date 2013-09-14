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

#ifndef BERRYTYPEEXTENSION_H_
#define BERRYTYPEEXTENSION_H_

#include <berryMacros.h>
#include <berryObject.h>

#include "berryIPropertyTester.h"

#include <vector>

namespace berry {

class TypeExtensionManager;
class END_POINT_;

class TypeExtension : public Object {

public:
  berryObjectMacro(TypeExtension);

private:

  /* the type this extension is extending */
  std::string fTypeInfo;
  /* the list of associated extenders */
  std::vector<IPropertyTester::Pointer> fExtenders;
  bool fExtendersLoaded;

  /* the extensions associated with <code>fType</code>'s super classes */
  std::vector<TypeExtension::Pointer> fExtends;
  bool fExtendsLoaded;

  TypeExtension() : fExtendersLoaded(false), fExtendsLoaded(false) {
    // special constructor to create the CONTINUE instance
  }


protected:

  friend class TypeExtensionManager;

  /* a special property tester instance that is used to signal that method searching has to continue */
   /* package */ class CONTINUE_ : public IPropertyTester {

   public:

     berryObjectMacro(CONTINUE_);

     bool Handles(const std::string&  /*namespaze*/, const std::string&  /*method*/) {
       return false;
     }
     bool IsInstantiated() {
       return true;
     }
     bool IsDeclaringPluginActive() {
       return true;
     }
     IPropertyTester* Instantiate() {
       return this;
     }
     bool Test(Object::Pointer /*receiver*/, const std::string& /*method*/,
         std::vector<Object::Pointer>&  /*args*/, Object::Pointer  /*expectedValue*/) {
       return false;
     }
   };

   static const CONTINUE_ CONTINUE;
   static const END_POINT_ END_POINT;

  /* package */
   TypeExtension(const std::string& typeInfo);

  /* package */
   IPropertyTester::Pointer FindTypeExtender(TypeExtensionManager& manager,
       const std::string& namespaze, const std::string& method,
       bool staticMethod, bool forcePluginActivation);
};


/* a special type extension instance that marks the end of an evaluation chain */
class END_POINT_ : public TypeExtension
{
protected:
  IPropertyTester::Pointer FindTypeExtender(TypeExtensionManager& /*manager*/,
      const std::string& /*namespaze*/, const std::string& /*name*/,
      bool  /*staticMethod*/, bool  /*forcePluginActivation*/)
  {
    return CONTINUE_::Pointer(new CONTINUE_());
  }
};

}  // namespace berry

#endif /*BERRYTYPEEXTENSION_H_*/
