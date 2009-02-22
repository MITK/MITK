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

#ifndef CHERRYPROPERTYTESTER_H_
#define CHERRYPROPERTYTESTER_H_

#include <cherryIConfigurationElement.h>

#include "internal/cherryPropertyTesterDescriptor.h"

#include "cherryExpressionsDll.h"

#include <string>

namespace cherry {

/**
 * Abstract superclass of all property testers. Implementation classes of
 * the extension point <code>org.opencherry.core.expresssions.propertyTesters
 * </code> must extend <code>PropertyTester</code>.
 * <p>
 * A property tester implements the property tests enumerated in the property
 * tester extension point. For the following property test extension
 * <pre>
 *   &lt;propertyTester
 *       namespace="org.opencherry.jdt.core"
 *       id="org.opencherry.jdt.core.IPackageFragmentTester"
 *       properties="isDefaultPackage"
 *       type="org.opencherry.jdt.core.IPackageFragment"
 *       class="org.opencherry.demo.MyPackageFragmentTester"&gt;
 *     &lt;/propertyTester&gt;
 * </pre>
 * the corresponding implementation class looks like:
 * <pre>
 *   public class MyPackageFragmentTester {
 *       public boolean test(Object receiver, String property, Object[] args, Object expectedValue) {
 *           IPackageFragment fragement= (IPackageFragment)receiver;
 *           if ("isDefaultPackage".equals(property)) {
 *               return expectedValue == null
 *                ? fragement.isDefaultPackage()
 *                : fragement.isDefaultPackage() == ((Boolean)expectedValue).booleanValue();
 *           }
 *           Assert.isTrue(false);
 *           return false;
 *       }
 *   }
 * </pre>
 * The property can then be used in a test expression as follows:
 * <pre>
 *   &lt;instanceof value="org.opencherry.core.IPackageFragment"/&gt;
 *   &lt;test property="org.opencherry.jdt.core.isDefaultPackage"/&gt;
 * </pre>
 * </p>
 * <p>
 * There is no guarantee that the same instance of a property tester is used
 * to handle &lt;test property="..."/&gt; requests. So property testers
 * should always be implemented in a stateless fashion.
 * </p>
 * @since 3.0
 */
class CHERRY_EXPRESSIONS PropertyTester : public IPropertyTester
{

private:
  IConfigurationElement::Pointer fConfigElement;
  std::string fNamespace;
  std::string fProperties;

public:
  /**
   * Initialize the property tester with the given name space and property.
   * <p>
   * Note: this method is for internal use only. Clients must not call
   * this method.
   * </p>
   * @param descriptor the descriptor object for this tester
   */
  void InternalInitialize(PropertyTesterDescriptor::Pointer descriptor);

  /**
   * Note: this method is for internal use only. Clients must not call
   * this method.
   *
   * @return the property tester descriptor
   */
  PropertyTesterDescriptor::Pointer InternalCreateDescriptor();

  /**
   * {@inheritDoc}
   */
  bool Handles(const std::string& namespaze, const std::string& property);

  /**
   * {@inheritDoc}
   */
  bool IsInstantiated();

  /**
   * {@inheritDoc}
   */
  bool IsDeclaringPluginActive();

  /**
   * {@inheritDoc}
   */
  IPropertyTester* Instantiate();

};

}  // namespace cherry

#endif /*CHERRYPROPERTYTESTER_H_*/
