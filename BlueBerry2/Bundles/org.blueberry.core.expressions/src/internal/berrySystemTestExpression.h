/*=========================================================================

Program:   BlueBerry Platform
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

#ifndef __BERRY_SYSTEM_TEXT_EXPRESSION_H__
#define __BERRY_SYSTEM_TEXT_EXPRESSION_H__

#include "../berryExpression.h"

#include "service/berryIConfigurationElement.h"

#include "Poco/DOM/Element.h"

namespace berry {

class SystemTestExpression : public Expression {

private:
  std::string fProperty;
	std::string fExpectedValue;

	static const std::string ATT_PROPERTY;

	/**
	 * The seed for the hash code for all system test expressions.
	 */
	static const std::size_t HASH_INITIAL;

public:

  SystemTestExpression(SmartPointer<IConfigurationElement> element);

	SystemTestExpression(Poco::XML::Element* element);

	SystemTestExpression(const std::string& property, const std::string& expectedValue);

	EvaluationResult Evaluate(IEvaluationContext::Pointer context);

	void CollectExpressionInfo(ExpressionInfo* info);

	bool operator==(Expression& object);

protected:

  std::size_t ComputeHashCode();

	// ---- Debugging ---------------------------------------------------

	/* (non-Javadoc)
	 * @see java.lang.Object#toString()
	 */
public:
  std::string ToString();
};

} // namespace berry

#endif // __BERRY_SYSTEM_TEXT_EXPRESSION_H__
