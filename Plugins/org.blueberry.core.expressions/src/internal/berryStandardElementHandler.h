/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYSTANDARDELEMENTHANDLER_H_
#define BERRYSTANDARDELEMENTHANDLER_H_

#include "berryElementHandler.h"

namespace berry
{

struct IConfigurationElement;

class Expression;
class ExpressionConverter;

class StandardElementHandler : public ElementHandler
{

public:
  SmartPointer<Expression> Create(ExpressionConverter* converter, SmartPointer<IConfigurationElement> element) override;

  SmartPointer<Expression> Create(ExpressionConverter* converter, Poco::XML::Element* element) override;
};

}  // namespace berry

#endif /*BERRYSTANDARDELEMENTHANDLER_H_*/
