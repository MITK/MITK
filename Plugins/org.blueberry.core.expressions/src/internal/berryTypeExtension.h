/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYTYPEEXTENSION_H_
#define BERRYTYPEEXTENSION_H_

#include <berryMacros.h>
#include <berryObject.h>

#include "berryIPropertyTester.h"


namespace berry {

class TypeExtensionManager;
class END_POINT_;

class TypeExtension : public Object {

public:
  berryObjectMacro(TypeExtension);

private:

  /* the type this extension is extending */
  Reflection::TypeInfo fTypeInfo;
  /* the list of associated extenders */
  QList<IPropertyTester::Pointer> fExtenders;
  bool fExtendersLoaded;

  /* the extensions associated with <code>fType</code>'s super classes */
  QList<TypeExtension::Pointer> fInherits;
  bool fInheritsLoaded;

  TypeExtension();

protected:

  friend class TypeExtensionManager;

  /* a special property tester instance that is used to signal that method searching has to continue */
  class CONTINUE_ : public IPropertyTester {

  public:

    berryObjectMacro(CONTINUE_);

    bool Handles(const QString&  /*namespaze*/, const QString&  /*method*/) override;
    bool IsInstantiated() override;
    bool IsDeclaringPluginActive() override;
    IPropertyTester* Instantiate() override;
    bool Test(Object::ConstPointer /*receiver*/, const QString& /*method*/,
              const QList<Object::Pointer>&  /*args*/, Object::Pointer  /*expectedValue*/) override;
  };

  static const CONTINUE_ CONTINUE;
  static const END_POINT_ END_POINT;

  TypeExtension(const Reflection::TypeInfo& typeInfo);

  IPropertyTester::Pointer FindTypeExtender(TypeExtensionManager& manager,
                                            const QString &namespaze, const QString &method,
                                            bool staticMethod, bool forcePluginActivation);
};


/* a special type extension instance that marks the end of an evaluation chain */
class END_POINT_ : public TypeExtension
{
protected:
  IPropertyTester::Pointer FindTypeExtender(TypeExtensionManager& /*manager*/,
                                            const QString& /*namespaze*/, const QString& /*name*/,
                                            bool  /*staticMethod*/, bool  /*forcePluginActivation*/);
};

}  // namespace berry

#endif /*BERRYTYPEEXTENSION_H_*/
