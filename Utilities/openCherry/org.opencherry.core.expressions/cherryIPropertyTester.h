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

#ifndef CHERRYIPROPERTYTESTER_H_
#define CHERRYIPROPERTYTESTER_H_

#include "org.opencherry.osgi/cherryMacros.h"

#include "org.opencherry.core.runtime/cherryExpressionVariables.h"

#include "Poco/Any.h"

#include <string>
#include <vector>

namespace cherry {

/**
 * A property tester can be used to add additional properties to test to an 
 * existing type.
 * <p>
 * This interface is not intended to be implemented by clients. Clients
 * should subclass type <code>PropertyTester</code>.
 * </p>
 * 
 * @since 3.0
 */
struct CHERRY_API IPropertyTester : public Object {
  
  cherryClassMacro(IPropertyTester)

  virtual ~IPropertyTester() {}
  
  /**
   * Returns whether the property tester can handle the given
   * property or not.
   * 
   * @param namespace the name space to be considered
   * @param property the property to test
   * @return <code>true</code> if the tester provides an implementation
   *  for the given property; otherwise <code>false</code> is returned
   */
  virtual bool Handles(const std::string& namespaze, const std::string& property) = 0;
  
  /**
   * Returns whether the implementation class for this property tester is
   * loaded or not.
   * 
   * @return <code>true</code>if the implementation class is loaded; 
   *  <code>false</code> otherwise
   */
  virtual bool IsInstantiated() = 0;
  
  /**
   * Returns <code>true</code> if the implementation class of this property
   * tester can be loaded. This is the case if the plug-in providing
   * the implementation class is active. Returns <code>false</code> otherwise.
   * 
   * @return whether the implementation class can be loaded or not
   */
  virtual bool IsDeclaringPluginActive() = 0;
  
  /**
   * Loads the implementation class for this property tester and returns an
   * instance of this class.
   * 
   * @return an instance of the implementation class for this property tester
   * 
   * @throws CoreException if the implementation class cannot be loaded
   */
  virtual IPropertyTester* Instantiate() = 0;
  
  /**
   * Executes the property test determined by the parameter <code>property</code>. 
   * 
   * @param receiver the receiver of the property test
   * @param property the property to test
   * @param args additional arguments to evaluate the property. If no arguments
   *  are specified in the <code>test</code> expression an array of length 0
   *  is passed
   * @param expectedValue the expected value of the property. The value is either 
   *  of type <code>java.lang.String</code> or a boxed base type. If no value was
   *  specified in the <code>test</code> expressions then <code>null</code> is passed
   * 
   * @return returns <code>true<code> if the property is equal to the expected value; 
   *  otherwise <code>false</code> is returned
   */
  virtual bool Test(ExpressionVariable::Pointer receiver, const std::string& property, std::vector<ExpressionVariable::Pointer>& args, ExpressionVariable::Pointer expectedValue) = 0;

};

}  // namespace cherry

#endif /*CHERRYIPROPERTYTESTER_H_*/
