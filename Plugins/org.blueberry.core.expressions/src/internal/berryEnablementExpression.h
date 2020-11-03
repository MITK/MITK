/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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

