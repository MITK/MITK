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

#include "cherryDefinitionRegistry.h"

#include "../cherryExpressionConverter.h"

#include "cherryPlatform.h"
#include "cherryPlatformException.h"
#include "service/cherryIExtensionPointService.h"

namespace cherry {

std::map<std::string, Expression::Pointer>& DefinitionRegistry::GetCache()
{
  return cache;
}

DefinitionRegistry::DefinitionRegistry()
{
  //Platform.getExtensionRegistry().addRegistryChangeListener(this, "org.opencherry.core.expressions"); //$NON-NLS-1$
}

Expression::Pointer DefinitionRegistry::GetExpression(const std::string& id)
{
  Expression::Pointer cachedExpression= this->GetCache()[id];
  if (!cachedExpression.IsNull())
  {
    return cachedExpression;
  }

  IExtensionPointService* service = Platform::GetExtensionPointService();
  IConfigurationElement::vector ces(
    service->GetConfigurationElementsFor("org.opencherry.core.expressions.definitions"));

  Expression::Pointer foundExpression;
  for (IConfigurationElement::vector::iterator i= ces.begin(); i != ces.end(); ++i)
  {
    std::string cid;
    if ((*i)->GetAttribute("id", cid))
    {
      if (cid == id)
      {
        try
        {
          foundExpression= this->GetExpression(id, *i);
          break;
        }
        catch (InvalidServiceObjectException e)
        {
          throw CoreException("Missing expression", id);
        }
      }
    }
  }
  if (foundExpression.IsNull())
  {
    throw CoreException("Missing expression", id);
  }
  
  return foundExpression;
}

Expression::Pointer DefinitionRegistry::GetExpression(const std::string& id,
    IConfigurationElement* element)
{
  IConfigurationElement::vector children(element->GetChildren());
  Expression::Pointer expr= ExpressionConverter::GetDefault()->Perform(children[0]);
  if (!expr.IsNull())
  {
    this->GetCache()[id] = expr;
  }
  
  return expr;
}

}
