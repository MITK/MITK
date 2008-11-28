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

#ifndef CHERRYTYPEEXTENSION_H_
#define CHERRYTYPEEXTENSION_H_

#include <cherryMacros.h>
#include <cherryExpressionVariables.h>

#include "../cherryIPropertyTester.h"

#include <vector>

namespace cherry {

class TypeExtensionManager;
class END_POINT_;

class TypeExtension : public Object {   
 
public:
  cherryClassMacro(TypeExtension);
  
private:
    
  /* the type this extension is extending */
  ExpressionVariable::ExtTypeInfo fTypeInfo;
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
     
     cherryClassMacro(CONTINUE_)
     
     bool Handles(const std::string& namespaze, const std::string& method) {
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
     bool Test(ExpressionVariable::Pointer receiver, const std::string& method, 
         std::vector<ExpressionVariable::Pointer>& args, ExpressionVariable::Pointer expectedValue) {
       return false;
     }
   };
    
   static const CONTINUE_ CONTINUE;
   static const END_POINT_ END_POINT;
   
  /* package */ 
   TypeExtension(ExpressionVariable::ExtTypeInfo typeInfo);
  
  /* package */ 
   IPropertyTester::Pointer FindTypeExtender(TypeExtensionManager& manager, 
       const std::string& namespaze, const std::string& method, 
       bool staticMethod, bool forcePluginActivation);
};


/* a special type extension instance that marks the end of an evaluation chain */
class END_POINT_ : public TypeExtension
{
protected:
  IPropertyTester::Pointer FindTypeExtender(TypeExtensionManager& manager,
      const std::string& namespaze, const std::string& name,
      bool staticMethod, bool forcePluginActivation)
  {
    return CONTINUE_::Pointer(new CONTINUE_());
  }
};

}  // namespace cherry

#endif /*CHERRYTYPEEXTENSION_H_*/
