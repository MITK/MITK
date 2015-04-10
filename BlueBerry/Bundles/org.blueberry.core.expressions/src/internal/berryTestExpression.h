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

#ifndef __BERRY_TEST_EXPRESSION_H__
#define __BERRY_TEST_EXPRESSION_H__

#include "berryExpression.h"
#include "berryTypeExtensionManager.h"

#include "berryObject.h"

#include "Poco/DOM/Element.h"

namespace berry {

struct IConfigurationElement;

class TestExpression : public Expression {

private:
  QString fNamespace;
  QString fProperty;
  QList<Object::Pointer> fArgs;
  Object::Pointer fExpectedValue;
  bool fForcePluginActivation;

  static const QChar PROP_SEP;
  static const QString ATT_PROPERTY;
  static const QString ATT_ARGS;
  static const QString ATT_FORCE_PLUGIN_ACTIVATION;
  /**
   * The seed for the hash code for all test expressions.
   */
  static const uint HASH_INITIAL;

  static TypeExtensionManager& GetTypeExtensionManager();


public:

  TestExpression(const SmartPointer<IConfigurationElement>& element);

  TestExpression(Poco::XML::Element* element);

  TestExpression(const QString& namespaze, const QString& property,
                 const QList<Object::Pointer>& args, Object::Pointer expectedValue);

  TestExpression(const QString& namespaze, const QString& property,
                 const QList<Object::Pointer>& args, Object::Pointer expectedValue,
                 bool forcePluginActivation);

  EvaluationResult::ConstPointer Evaluate(IEvaluationContext* context) const;

  void CollectExpressionInfo(ExpressionInfo* info) const;

  bool operator==(const Object* object) const;

protected:

  uint ComputeHashCode() const;

  //---- Debugging ---------------------------------------------------

  /* (non-Javadoc)
   * @see java.lang.Object#toString()
   */
public:

  QString ToString() const;

  //---- testing ---------------------------------------------------

  bool TestGetForcePluginActivation();

  static TypeExtensionManager& TestGetTypeExtensionManager();
};

} // namespace berry

#endif // __BERRY_TEST_EXPRESSION_H__
