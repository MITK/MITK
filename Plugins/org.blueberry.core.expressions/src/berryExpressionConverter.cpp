/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryExpressionConverter.h"

#include "berryPlatformException.h"
#include "berryIConfigurationElement.h"
#include "berryIExtension.h"
#include "berryIContributor.h"

#include "berryCoreException.h"
#include "berryElementHandler.h"
#include "berryExpression.h"
#include "berryStatus.h"

#include "internal/berryExpressionPlugin.h"
#include "internal/berryCompositeExpression.h"

#include "Poco/DOM/Node.h"
#include "Poco/DOM/Element.h"


namespace berry {

ExpressionConverter* ExpressionConverter::INSTANCE = nullptr;

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
  for (int i = 0; i < fHandlers.size(); i++) {
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
  for (int i = 0; i < fHandlers.size(); i++) {
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
    {
      IStatus::Pointer status(new Status(IStatus::ERROR_TYPE, ExpressionPlugin::GetPluginId(),
                                         IStatus::ERROR_TYPE, QString("Unknown expression element ") + GetDebugPath(*iter),
                                         BERRY_STATUS_LOC));
      throw CoreException(status);
    }

    result->Add(child);
  }
}

QString
ExpressionConverter::GetDebugPath(const IConfigurationElement::Pointer& configurationElement)
{
  QString buf;
  buf.append(configurationElement->GetName());
  Object::Pointer parent= configurationElement->GetParent();
  while (parent)
  {
    if (IConfigurationElement::Pointer parent2 = parent.Cast<IConfigurationElement>())
    {
      buf.append(" > ");
      buf.append(parent2->GetName());
      QString id= parent2->GetAttribute("id");
      if (!id.isEmpty())
      {
        buf.append(" (id=").append(id).append(')');
      }
      parent= parent2->GetParent();
    }
    else if (IExtension::Pointer parent2 = parent.Cast<IExtension>())
    {
      buf.append(" : ");
      buf.append(parent2->GetExtensionPointUniqueIdentifier());
      buf.append(" @ ");
      buf.append(parent2->GetContributor()->GetName());
      parent = nullptr;
    }
    else
    {
      parent = nullptr;
    }
  }
  return buf;
}

void
ExpressionConverter::ProcessChildren(Poco::XML::Element* element,
                                     const CompositeExpression::Pointer& result)
{
  Poco::XML::Node* child = element->firstChild();
  while (child != nullptr) {
    if (child->nodeType() == Poco::XML::Node::ELEMENT_NODE) {
      Poco::XML::Element* elem = static_cast<Poco::XML::Element*>(child);
      Expression::Pointer exp = this->Perform(elem);
      if (exp.IsNull())
      {
        IStatus::Pointer status(new Status(IStatus::ERROR_TYPE, ExpressionPlugin::GetPluginId(),
                                           IStatus::ERROR_TYPE, QString("Unknown expression element ")
                                           + QString::fromStdString(elem->nodeName()), BERRY_STATUS_LOC));

        throw CoreException(status);
      }
      result->Add(exp);
    }
    child = child->nextSibling();
  }
}

}  // namespace berry
