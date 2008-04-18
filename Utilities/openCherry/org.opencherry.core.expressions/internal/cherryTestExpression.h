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

#ifndef __CHERRY_TEST_EXPRESSION_H__
#define __CHERRY_TEST_EXPRESSION_H__

#include "../cherryExpression.h"
#include "cherryTypeExtensionManager.h"

#include "org.opencherry.osgi/service/cherryIConfigurationElement.h"
#include "org.opencherry.core.runtime/cherryExpressionVariables.h"

#include "Poco/DOM/Element.h"

namespace cherry {

class TestExpression : public Expression {

private:
  std::string fNamespace;
  std::string fProperty;
	std::vector<ExpressionVariable::Pointer> fArgs;
	ExpressionVariable::Pointer fExpectedValue;
	bool fForcePluginActivation;
	
	static const char PROP_SEP;
	static const std::string ATT_PROPERTY;
	static const std::string ATT_ARGS;
	static const std::string ATT_FORCE_PLUGIN_ACTIVATION;
	/**
	 * The seed for the hash code for all test expressions.
	 */
	static const intptr_t HASH_INITIAL;
	
  static TypeExtensionManager fgTypeExtensionManager;
	
  
public:
  
  TestExpression(IConfigurationElement* element);

	TestExpression(Poco::XML::Element* element);

	TestExpression(const std::string& namespaze, const std::string& property, 
	    std::vector<ExpressionVariable::Pointer>& args, ExpressionVariable::Pointer expectedValue);
	
	TestExpression(const std::string& namespaze, const std::string& property, 
	    std::vector<ExpressionVariable::Pointer>& args, ExpressionVariable::Pointer expectedValue, bool forcePluginActivation);

	EvaluationResult Evaluate(IEvaluationContext* context);

	void CollectExpressionInfo(ExpressionInfo* info);

	bool operator==(Expression& object);

protected:
  
  intptr_t ComputeHashCode();
	
	//---- Debugging ---------------------------------------------------
	
	/* (non-Javadoc)
	 * @see java.lang.Object#toString()
	 */
public:
  
  std::string ToString();
	
	//---- testing ---------------------------------------------------
	
	bool TestGetForcePluginActivation();
	
	static TypeExtensionManager& TestGetTypeExtensionManager();
};

} // namespace cherry

#endif // __CHERRY_TEST_EXPRESSION_H__
