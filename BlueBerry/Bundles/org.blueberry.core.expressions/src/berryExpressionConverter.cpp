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

#include "berryExpressionConverter.h"

#include "berryPlatformException.h"
#include "berryIConfigurationElement.h"

#include "berryElementHandler.h"
#include "berryExpression.h"

#include "internal/berryCompositeExpression.h"

#include "Poco/DOM/Node.h"
#include "Poco/DOM/Element.h"


namespace berry {

ExpressionConverter* ExpressionConverter::INSTANCE = 0;

ExpressionConverter*
ExpressionConverter::GetDefault()
{
  if (INSTANCE) return INSTANCE;

  QList<ElementHandler::Pointer> handlers;
  handlers.push_back(ElementHandler::GetDefault());
  INSTANCE = new ExpressionConverter(handlers);

  return INSTANCE;
}

ExpressionConverter::ExpressionConverter(const QList<ElementHandler::Pointer>& handlers)
{
  fHandlers = handlers;
}

Expression::Pointer
ExpressionConverter::Perform(const IConfigurationElement::Pointer& root)
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
ExpressionConverter::ProcessChildren(const IConfigurationElement::Pointer& element,
                                     const CompositeExpression::Pointer& result)
{
  QList<IConfigurationElement::Pointer> children(element->GetChildren());

  QList<IConfigurationElement::Pointer>::iterator iter;
  for (iter = children.begin(); iter != children.end(); ++iter)
  {
    Expression::Pointer child = this->Perform(*iter);
    if (child.IsNull())
      throw CoreException(QString("Unknown element: ") + GetDebugPath(*iter));

    result->Add(child);
  }
}

QString
ExpressionConverter::GetDebugPath(const IConfigurationElement::Pointer& configurationElement)
{
  QString buf;
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
      buf.append(" : ");
      QString point = parent->GetAttribute("point");
      buf.append(point);
      buf.append(" @ ");
      buf.append(parent->GetContributor());
      parent= 0;
    }
  }
  return buf;
}

void
ExpressionConverter::ProcessChildren(Poco::XML::Element* element,
                                     const CompositeExpression::Pointer& result)
{
  Poco::XML::Node* child = element->firstChild();
  while (child != 0) {
    if (child->nodeType() == Poco::XML::Node::ELEMENT_NODE) {
      Poco::XML::Element* elem = static_cast<Poco::XML::Element*>(child);
      Expression::Pointer exp = this->Perform(elem);
      if (exp.IsNull())
        throw CoreException(QString("org.blueberry.core.expressions unknown element: ")
                            + QString::fromStdString(elem->nodeName()));

      result->Add(exp);
    }
    child = child->nextSibling();
  }
}

}  // namespace berry
