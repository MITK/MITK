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

#include "berryExpressionConverter.h"

#include "berryPlatformException.h"

#include "Poco/DOM/Node.h"

namespace berry {

ExpressionConverter* ExpressionConverter::INSTANCE = 0;

ExpressionConverter*
ExpressionConverter::GetDefault()
{
  if (INSTANCE) return INSTANCE;

  std::vector<ElementHandler::Pointer> handlers;
  handlers.push_back(ElementHandler::GetDefault());
  INSTANCE = new ExpressionConverter(handlers);

  return INSTANCE;
}

ExpressionConverter::ExpressionConverter(std::vector<ElementHandler::Pointer>& handlers)
{
  fHandlers = handlers;
}

Expression::Pointer
ExpressionConverter::Perform(IConfigurationElement::Pointer root)
{
  for (unsigned int i = 0; i < fHandlers.size(); i++) {
    ElementHandler::Pointer handler = fHandlers[i];
    Expression::Pointer result = handler->Create(this, root);
    if (!result.IsNull())
      return result;
  }
  return Expression::Pointer();
}

Expression::Pointer
ExpressionConverter::Perform(Poco::XML::Element* root)
{
  for (unsigned int i = 0; i < fHandlers.size(); i++) {
    ElementHandler::Pointer handler = fHandlers[i];
    Expression::Pointer result = handler->Create(this, root);
    if (!result.IsNull())
      return result;
  }
  return Expression::Pointer();
}

void
ExpressionConverter::ProcessChildren(IConfigurationElement::Pointer element, CompositeExpression::Pointer result)
{
  IConfigurationElement::vector children(element->GetChildren());

  IConfigurationElement::vector::iterator iter;
    for (iter = children.begin(); iter != children.end(); ++iter)
    {
      Expression::Pointer child = this->Perform(*iter);
      if (child.IsNull())
        throw new CoreException("Unknown element", GetDebugPath(*iter));

      result->Add(child);
    }

}

std::string
ExpressionConverter::GetDebugPath(IConfigurationElement::Pointer configurationElement)
{
  std::string buf = "";
  buf.append(configurationElement->GetName());
  const IConfigurationElement* parent= configurationElement->GetParent();
  while (parent) {
    if (parent->GetParent())
    {
      buf.append(" > ");
      buf.append(parent->GetName());
      parent = parent->GetParent();
    }
    else
    {
      buf.append(" : "); //$NON-NLS-1$
      std::string point;
      parent->GetAttribute("point", point);
      buf.append(point);
      buf.append(" @ "); //$NON-NLS-1$
      buf.append(parent->GetContributor());
      parent= 0;
    }
  }
  return buf;
}

void
ExpressionConverter::ProcessChildren(Poco::XML::Element* element, CompositeExpression::Pointer result)
{
  Poco::XML::Node* child = element->firstChild();
  while (child != 0) {
    if (child->nodeType() == Poco::XML::Node::ELEMENT_NODE) {
      Poco::XML::Element* elem = static_cast<Poco::XML::Element*>(child);
      Expression::Pointer exp = this->Perform(elem);
      if (exp.IsNull())
        throw CoreException("org.blueberry.core.expressions unknown element", elem->nodeName());

      result->Add(exp);
    }
    child = child->nextSibling();
  }
}

}  // namespace berry
