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

#ifndef BERRYIPROPERTYTESTER_H_
#define BERRYIPROPERTYTESTER_H_

#include <berryMacros.h>
#include <berryObject.h>

#include <org_blueberry_core_expressions_Export.h>

#include <Poco/Any.h>

#include <string>
#include <vector>

#include <QObject>

namespace berry {

/**
 * A property tester can be used to add additional properties to test to an
 * existing type.
 * <p>
 * This interface is not intended to be implemented by clients. Clients
 * should subclass type <code>PropertyTester</code>.
 * </p>
 */
struct BERRY_EXPRESSIONS IPropertyTester : public Object
{

  berryObjectMacro(berry::IPropertyTester)

  virtual ~IPropertyTester();

  /**
   * Returns whether the property tester can handle the given
   * property or not.
   *
   * @param namespace the name space to be considered
   * @param property the property to test
   * @return <code>true</code> if the tester provides an implementation
   *  for the given property; otherwise <code>false</code> is returned
   */
  virtual bool Handles(const QString& namespaze, const QString& property) = 0;

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
  virtual bool Test(Object::ConstPointer receiver, const QString& property,
                    const QList<Object::Pointer>& args, Object::Pointer expectedValue) = 0;

};

}  // namespace berry

Q_DECLARE_INTERFACE(berry::IPropertyTester, "org.blueberry.IPropertyTester")

#endif /*BERRYIPROPERTYTESTER_H_*/
