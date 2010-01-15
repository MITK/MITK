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

#include "cherryElementHandler.h"

#include "internal/cherryStandardElementHandler.h"

#include "Poco/Exception.h"

namespace cherry {

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

} // namespace cherry
