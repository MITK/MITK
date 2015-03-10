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

#include "berryElementHandler.h"

#include "internal/berryStandardElementHandler.h"

#include "berryExpression.h"
#include "internal/berryCompositeExpression.h"
#include "berryExpressionConverter.h"

#include <berryIConfigurationElement.h>

#include "Poco/Exception.h"

namespace berry {

ElementHandler::~ElementHandler()
{
}

ElementHandler::Pointer
ElementHandler::GetDefault()
{
  static ElementHandler::Pointer instance(new StandardElementHandler());
  return instance;
}


Expression::Pointer
ElementHandler::Create(ExpressionConverter* /*converter*/, Poco::XML::Element* /*element*/)
{
  throw Poco::NotImplementedException();
}

void
ElementHandler::ProcessChildren(ExpressionConverter* converter, IConfigurationElement::Pointer element, CompositeExpression::Pointer expression)
{
  converter->ProcessChildren(element, expression);
}

void
ElementHandler::ProcessChildren(ExpressionConverter* converter, Poco::XML::Element* element, CompositeExpression::Pointer expression)
{
  converter->ProcessChildren(element, expression);
}

} // namespace berry
