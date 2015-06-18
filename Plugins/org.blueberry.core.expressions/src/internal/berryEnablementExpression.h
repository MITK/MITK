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

#ifndef __BERRY_ENABLEMENT_EXPRESSION_H__
#define __BERRY_ENABLEMENT_EXPRESSION_H__

#include "berryCompositeExpression.h"

#include "Poco/DOM/Node.h"

namespace berry {

struct IConfigurationElement;

class EnablementExpression : public CompositeExpression
{

public:

  /**
   * Creates a {@link EnablementExpression}.
   *
   * @param configElement the configuration element
   */
  EnablementExpression(const SmartPointer<IConfigurationElement>& /*configElement*/)
  {
    // config element not used yet.
  }

  /**
   * Creates a {@link EnablementExpression}.
   *
   * @param element the XML element
   */
  EnablementExpression(Poco::XML::Node* /*element*/)
  {
    // element not used yet.
  }

  bool operator==(const Object* object) const override;

  EvaluationResult::ConstPointer Evaluate(IEvaluationContext* context) const override;
};

} // namespace berry

#endif // __BERRY_ENABLEMENT_EXPRESSION_H__

