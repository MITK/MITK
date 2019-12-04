/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYPROPERTYTESTER_H_
#define BERRYPROPERTYTESTER_H_

#include <berryIConfigurationElement.h>

#include "internal/berryPropertyTesterDescriptor.h"

#include <org_blueberry_core_expressions_Export.h>


namespace berry {

/**
 * Abstract superclass of all property testers. Implementation classes of
 * the extension point <code>org.blueberry.core.expresssions.propertyTesters
 * </code> must extend <code>PropertyTester</code>.
 * <p>
 * A property tester implements the property tests enumerated in the property
 * tester extension point. For the following property test extension
 * <pre>
 *   &lt;propertyTester
 *       namespace="org.blueberry.jdt.core"
 *       id="org.blueberry.jdt.core.IPackageFragmentTester"
 *       properties="isDefaultPackage"
 *       type="org.blueberry.jdt.core.IPackageFragment"
 *       class="org.blueberry.demo.MyPackageFragmentTester"&gt;
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
 *   &lt;instanceof value="org.blueberry.core.IPackageFragment"/&gt;
 *   &lt;test property="org.blueberry.jdt.core.isDefaultPackage"/&gt;
 * </pre>
 * </p>
 * <p>
 * There is no guarantee that the same instance of a property tester is used
 * to handle &lt;test property="..."/&gt; requests. So property testers
 * should always be implemented in a stateless fashion.
 * </p>
 */
class BERRY_EXPRESSIONS PropertyTester : public QObject, public IPropertyTester
{
  Q_OBJECT
  Q_INTERFACES(berry::IPropertyTester)

private:
  IConfigurationElement::Pointer fConfigElement;
  QString fNamespace;
  QString fProperties;

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
  bool Handles(const QString& namespaze, const QString& property) override;

  /**
   * {@inheritDoc}
   */
  bool IsInstantiated() override;

  /**
   * {@inheritDoc}
   */
  bool IsDeclaringPluginActive() override;

  /**
   * {@inheritDoc}
   */
  IPropertyTester* Instantiate() override;

};

}  // namespace berry

#endif /*BERRYPROPERTYTESTER_H_*/
