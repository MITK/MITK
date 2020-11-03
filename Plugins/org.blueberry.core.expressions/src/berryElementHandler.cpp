/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
