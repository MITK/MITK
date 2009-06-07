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

#include "cherryExpressions.h"

#include "cherryPlatform.h"
#include "cherryPlatformException.h"
#include "service/cherryServiceRegistry.h"

#include "cherryIAdapterManager.h"

#include <cherryObjects.h>
#include <cherryObjectString.h>
#include <cherryObjectVector.h>

#include "Poco/String.h"
#include "Poco/NumberParser.h"

#include <deque>

namespace cherry
{

  const bool Expressions::TRACING = true;

  Expressions::Expressions()
  {
    // no instance
  }

  bool
  Expressions::IsInstanceOf(Object::ConstPointer element, const std::string& type)
  {
    // null isn't an instanceof of anything.
    if (element.IsNull())
      return false;

    // TODO Reflection
    // return IsSubtype(element, type)

    return element->GetClassName() == type;
  }


  void
  Expressions::CheckAttribute(const std::string& name, bool value)
  {
    if (!value)
    {
      throw CoreException("Missing attribute", name);
    }
  }

  void
  Expressions::CheckAttribute(const std::string& name, bool result, const std::string& value, std::vector<std::string>& validValues)
  {
    CheckAttribute(name, result);
    for (unsigned int i= 0; i < validValues.size(); i++)
    {
      if (value == validValues[i])
        return;
    }
    throw CoreException("Wrong attribute value", value);
  }

  void
  Expressions::CheckCollection(Object::ConstPointer var, Expression::Pointer expression)
  {
    if (var.Cast<const ObjectVector<Object::Pointer> >())
      return;
    throw CoreException("Expression variable is not of type ObjectVector", expression->ToString());
  }

  IIterable::Pointer
  Expressions::GetAsIIterable(Object::Pointer var, Expression::Pointer expression)
  {
    IIterable::Pointer iterable(var.Cast<IIterable>());
    if (!iterable.IsNull())
    {
      return iterable;
    }
    else
    {
      IAdapterManager::Pointer manager= Platform::GetServiceRegistry().GetServiceById<IAdapterManager>("org.opencherry.service.adaptermanager");
      Object::Pointer result;
      Poco::Any any(manager->GetAdapter(var, IIterable::GetStaticClassName()));
      if (!any.empty() && any.type() == typeid(Object::Pointer))
      {
        result = Poco::AnyCast<Object::Pointer>(any);
      }

      if (result)
      {
        iterable = result.Cast<IIterable>();
        return iterable;
      }

      if (manager->QueryAdapter(var->GetClassName(), IIterable::GetStaticClassName()) == IAdapterManager::NOT_LOADED)
        return IIterable::Pointer();

      throw CoreException("The variable is not iterable", expression->ToString());
    }
  }

  ICountable::Pointer
  Expressions::GetAsICountable(Object::Pointer var, Expression::Pointer expression)
  {
    ICountable::Pointer countable(var.Cast<ICountable>());
    if (!countable.IsNull())
    {
      return countable;
    }
    else
    {
      IAdapterManager::Pointer manager = Platform::GetServiceRegistry().GetServiceById<IAdapterManager>("org.opencherry.service.adaptermanager");
      Object::Pointer result;
      Poco::Any any(manager->GetAdapter(var, ICountable::GetStaticClassName()));
      if (!any.empty() && any.type() == typeid(Object::Pointer))
      {
        result = Poco::AnyCast<Object::Pointer>(any);
      }

      if (result)
      {
        countable = result.Cast<ICountable>();
      }

      if (manager->QueryAdapter(var->GetClassName(), ICountable::GetStaticClassName()) == IAdapterManager::NOT_LOADED)
        return ICountable::Pointer();

      throw CoreException("The variable is not countable", expression->ToString());
    }
  }

  bool
  Expressions::GetOptionalBooleanAttribute(IConfigurationElement::Pointer element, const std::string& attributeName)
  {
    std::string value;
    if (element->GetAttribute(attributeName, value))
      return Poco::toLower<std::string>(value) == "true";

    return false;
  }

  bool
  Expressions::GetOptionalBooleanAttribute(Poco::XML::Element* element, const std::string& attributeName)
  {
    std::string value = element->getAttribute(attributeName);
    if (value.size() == 0)
      return false;

    return Poco::toLower<std::string>(value) == "true";
  }

  void
  Expressions::GetArguments(std::vector<Object::Pointer>& args, IConfigurationElement::Pointer element, const std::string& attributeName)
  {
    std::string value;
    if (element->GetAttribute(attributeName, value))
    {
      ParseArguments(args, value);
    }
  }

  void
  Expressions::GetArguments(std::vector<Object::Pointer>& args, Poco::XML::Element* element, const std::string& attributeName)
  {
    std::string value = element->getAttribute(attributeName);
    if (value.size()> 0)
    {
      ParseArguments(args, value);
    }
  }

  void
  Expressions::ParseArguments(std::vector<Object::Pointer>& result, const std::string& args)
  {
    int start= 0;
    int comma;
    while ((comma = FindNextComma(args, start)) != -1)
    {
      result.push_back(ConvertArgument(Poco::trim<std::string>(args.substr(start, comma-start))));
      start= comma + 1;
    }
    result.push_back(ConvertArgument(Poco::trim<std::string>(args.substr(start))));
  }

  int
  Expressions::FindNextComma(const std::string& str, int start)
  {
    bool inString = false;
    for (unsigned int i = start; i < str.size(); i++)
    {
      char ch = str.at(i);
      if (ch == ',' && ! inString)
      return i;
      if (ch == '\'')
      {
        if (!inString)
        {
          inString= true;
        }
        else
        {
          if (i + 1 < str.size() && str.at(i + 1) == '\'')
          {
            i++;
          }
          else
          {
            inString= false;
          }
        }
      }
      else if (ch == ',' && !inString)
      {
        return i;
      }
    }
    if (inString)
    throw CoreException("String not terminated", str);

    return -1;
  }

  Object::Pointer
  Expressions::ConvertArgument(const std::string& arg, bool result)
  {
    if (!result)
    {
      return Object::Pointer();
    }
    else if (arg.length() == 0)
    {
      ObjectString::Pointer var(new ObjectString(arg));
      return var;
    }
    else if (arg.at(0) == '\'' && arg.at(arg.size() - 1) == '\'')
    {
      ObjectString::Pointer var(new ObjectString(UnEscapeString(arg.substr(1, arg.size() - 2))));
      return var;
    }
    else if ("true" == arg)
    {
      ObjectBool::Pointer var(new ObjectBool(true));
      return var;
    }
    else if ("false" == arg)
    {
      ObjectBool::Pointer var(new ObjectBool(false));
      return var;
    }
    else if (arg.find('.') != std::string::npos)
    {
      try
      {
        double num = Poco::NumberParser::parseFloat(arg);
        ObjectFloat::Pointer var(new ObjectFloat(num));
        return var;
      }
      catch (Poco::SyntaxException)
      {
        ObjectString::Pointer var(new ObjectString(arg));
        return var;
      }
    }
    else
    {
      try
      {
        int num = Poco::NumberParser::parse(arg);
        ObjectInt::Pointer var(new ObjectInt(num));
        return var;
      }
      catch (Poco::SyntaxException e)
      {
        ObjectString::Pointer var(new ObjectString(arg));
        return var;
      }
    }
  }

  std::string
  Expressions::UnEscapeString(const std::string& str)
  {
    std::string result = "";
    for (unsigned int i= 0; i < str.size(); i++)
    {
      char ch= str.at(i);
      if (ch == '\'')
      {
        if (i == str.size() - 1 || str.at(i + 1) != '\'')
        throw CoreException("String not correctly escaped", str);
        result.append(1, '\'');
        i++;
      }
      else
      {
        result.append(1, ch);
      }
    }
    return result;
  }

} // namespace cherry
