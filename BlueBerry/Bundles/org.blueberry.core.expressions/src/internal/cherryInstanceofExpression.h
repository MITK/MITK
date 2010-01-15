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

#ifndef __CHERRY_INSTANCEOF_EXPRESSION_H__
#define __CHERRY_INSTANCEOF_EXPRESSION_H__

#include "../cherryExpression.h"

#include "service/cherryIConfigurationElement.h"

#include "Poco/DOM/Element.h"

namespace cherry {

struct InstanceofExpression : public Expression {

private:

  /**
    * The seed for the hash code for all instance of expressions.
    */
  static const std::size_t HASH_INITIAL;

	std::string fTypeName;


public:

  InstanceofExpression(SmartPointer<IConfigurationElement> element);

	InstanceofExpression(Poco::XML::Element* element);

	InstanceofExpression(const std::string& typeName);

	/* (non-Javadoc)
	 * @see org.opencherry.jdt.internal.corext.refactoring.participants.Expression#evaluate(java.lang.Object)
	 */
	EvaluationResult Evaluate(IEvaluationContext* context);

	void CollectExpressionInfo(ExpressionInfo* info);

	bool operator==(Expression& object);

	//---- Debugging ---------------------------------------------------

	  /* (non-Javadoc)
	   * @see java.lang.Object#toString()
	   */
	std::string ToString();

protected:

  std::size_t ComputeHashCode();


};

} // namespace cherry

#endif // __CHERRY_INSTANCEOF_EXPRESSION_H__
