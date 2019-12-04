/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryDefinitionRegistry.h"

#include "berryExpressionConverter.h"
#include "berryExpressionStatus.h"
#include "berryCoreException.h"
#include "berryElementHandler.h"

#include "berryPlatform.h"
#include "berryPlatformException.h"
#include "berryStatus.h"
#include "berryIExtensionRegistry.h"
#include "berryIConfigurationElement.h"
#include "berryInvalidRegistryObjectException.h"

namespace berry {

QHash<QString, Expression::Pointer>& DefinitionRegistry::GetCache()
{
  return cache;
}

DefinitionRegistry::DefinitionRegistry()
{
  //Platform.getExtensionRegistry().addRegistryChangeListener(this, "org.blueberry.core.expressions"); //$NON-NLS-1$
}

Expression::Pointer DefinitionRegistry::GetExpression(const QString& id)
{
  Expression::Pointer cachedExpression= this->GetCache()[id];
  if (!cachedExpression.IsNull())
  {
    return cachedExpression;
  }

  IExtensionRegistry* registry = Platform::GetExtensionRegistry();
  QList<IConfigurationElement::Pointer> ces =
      registry->GetConfigurationElementsFor("org.blueberry.core.expressions.definitions");

  Expression::Pointer foundExpression;
  for (QList<IConfigurationElement::Pointer>::iterator i= ces.begin(); i != ces.end(); ++i)
  {
    QString cid = (*i)->GetAttribute("id");
    if (!cid.isNull())
    {
      if (cid == id)
      {
        try
        {
          foundExpression= this->GetExpression(id, *i);
          break;
        }
        catch (const InvalidRegistryObjectException& /*e*/)
        {
          IStatus::Pointer status(new ExpressionStatus(ExpressionStatus::MISSING_EXPRESSION,
                                                       QString("Unable to locate expression definition ") + id,
                                                       BERRY_STATUS_LOC));
          throw CoreException(status);
        }
      }
    }
  }
  if (foundExpression.IsNull())
  {
    IStatus::Pointer status(new ExpressionStatus(ExpressionStatus::MISSING_EXPRESSION,
                                                 QString("Unable to locate expression definition ") + id,
                                                 BERRY_STATUS_LOC));
    throw CoreException(status);
  }

  return foundExpression;
}

Expression::Pointer DefinitionRegistry::GetExpression(const QString& id,
                                                      const IConfigurationElement::Pointer& element)
{
  QList<IConfigurationElement::Pointer> children(element->GetChildren());
  Expression::Pointer expr= ExpressionConverter::GetDefault()->Perform(children[0]);
  if (!expr.IsNull())
  {
    this->GetCache()[id] = expr;
  }

  return expr;
}

}
