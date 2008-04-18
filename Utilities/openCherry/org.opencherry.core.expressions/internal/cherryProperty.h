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

#ifndef CHERRYPROPERTY_H_
#define CHERRYPROPERTY_H_

#include "../cherryIPropertyTester.h"

#include "org.opencherry.core.runtime/cherryExpressionVariables.h"

#include "Poco/SharedPtr.h"
#include "Poco/Any.h"

#include <vector>
#include <typeinfo>

namespace cherry {

class Property {
  
public:
  typedef Property Self;
  typedef Poco::SharedPtr<Self> Pointer;
  typedef Poco::SharedPtr<const Self> ConstPointer;
  
private:
  ExpressionVariable::Pointer fType;
  std::string fNamespace;
  std::string fName;
  
  IPropertyTester* fTester;

  friend class TypeExtensionManager;
  /* package */ Property(ExpressionVariable::Pointer type, 
      const std::string& namespaze, const std::string& name);
  
  /* package */ void SetPropertyTester(IPropertyTester* tester);
  
public:
  bool IsInstantiated();
  
  bool IsDeclaringPluginActive();
  
  bool IsValidCacheEntry(bool forcePluginActivation);
  
  bool Test(ExpressionVariable::Pointer receiver, std::vector<ExpressionVariable::Pointer>& args, ExpressionVariable::Pointer expectedValue);
  
  bool operator==(Property& obj);
  
  bool operator==(Property* obj);
  
  int HashCode();
};

}  // namespace cherry

#endif /*CHERRYPROPERTY_H_*/
