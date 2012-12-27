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

#ifndef BERRYSTANDARDELEMENTHANDLER_H_
#define BERRYSTANDARDELEMENTHANDLER_H_

#include "service/berryIConfigurationElement.h"

#include "berryExpression.h"
#include "berryElementHandler.h"
#include "berryExpressionConverter.h"

namespace berry
{

class StandardElementHandler : public ElementHandler
{

public:
  Expression::Pointer Create(ExpressionConverter* converter, SmartPointer<IConfigurationElement> element);

  Expression::Pointer Create(ExpressionConverter* converter, Poco::XML::Element* element);
};

}  // namespace berry

#endif /*BERRYSTANDARDELEMENTHANDLER_H_*/
