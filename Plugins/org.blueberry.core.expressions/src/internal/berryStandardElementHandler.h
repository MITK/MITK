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
