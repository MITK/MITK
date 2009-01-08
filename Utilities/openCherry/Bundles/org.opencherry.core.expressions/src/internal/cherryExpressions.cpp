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
  Expressions::IsInstanceOf(ExpressionVariable::ConstPointer element, const std::string& type)
  {
    // null isn't an instanceof of anything.
    if (element.IsNull())
      return false;

    const std::deque<std::string> types = element->GetExtTypeInfo().m_TypeNames;
    for (std::deque<std::string>::const_iterator i = types.begin(); i != types.end(); ++i)
    {
      if (*i == type) return true;
    }

    return true;
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
  Expressions::CheckCollection(ExpressionVariable::ConstPointer var, Expression::Pointer expression)
  {
    if (!var.Cast<const VectorExpressionVariable>().IsNull())
      return;
    throw CoreException("Expression variable is not of type VectorExpressionVariable", expression->ToString());
  }

  IIterable::Pointer
  Expressions::GetAsIIterable(ExpressionVariable::Pointer var, Expression::Pointer expression)
  {
    IIterable::Pointer iterable(var.Cast<IIterable>());
    if (!iterable.IsNull())
    {
      return iterable;
    }
    else
    {
      IAdapterManager::Pointer manager= Platform::GetServiceRegistry().GetServiceById<IAdapterManager>("org.opencherry.service.adaptermanager");
      ExpressionVariable::Pointer result(manager->GetAdapter(var, typeid(IIterable)));
      if (!result.IsNull())
      {
        iterable = result.Cast<IIterable>();
        return iterable;
      }

      if (manager->QueryAdapter(var, typeid(IIterable).name()) == IAdapterManager::NOT_LOADED)
      return IIterable::Pointer();

      throw CoreException("The variable is not iterable", expression->ToString());
    }
  }

  ICountable::Pointer
  Expressions::GetAsICountable(ExpressionVariable::Pointer var, Expression::Pointer expression)
  {
    ICountable::Pointer countable(var.Cast<ICountable>());
    if (!countable.IsNull())
    {
      return countable;
    }
    else
    {
      IAdapterManager::Pointer manager = Platform::GetServiceRegistry().GetServiceById<IAdapterManager>("org.opencherry.service.adaptermanager");
      ExpressionVariable::Pointer result(manager->GetAdapter(var, typeid(ICountable)));
      if (!result.IsNull())
      {
        countable = result.Cast<ICountable>();
      }

      if (manager->QueryAdapter(var, typeid(ICountable).name()) == IAdapterManager::NOT_LOADED)
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
  Expressions::GetArguments(std::vector<ExpressionVariable::Pointer>& args, IConfigurationElement::Pointer element, const std::string& attributeName)
  {
    std::string value;
    if (element->GetAttribute(attributeName, value))
    {
      ParseArguments(args, value);
    }
  }

  void
  Expressions::GetArguments(std::vector<ExpressionVariable::Pointer>& args, Poco::XML::Element* element, const std::string& attributeName)
  {
    std::string value = element->getAttribute(attributeName);
    if (value.size()> 0)
    {
      ParseArguments(args, value);
    }
  }

  void
  Expressions::ParseArguments(std::vector<ExpressionVariable::Pointer>& result, const std::string& args)
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

  ExpressionVariable::Pointer
  Expressions::ConvertArgument(const std::string& arg, bool result)
  {
    if (!result)
    {
      return ExpressionVariable::Pointer();
    }
    else if (arg.length() == 0)
    {
      StringExpressionVariable::Pointer var(new StringExpressionVariable(arg));
      return var;
    }
    else if (arg.at(0) == '\'' && arg.at(arg.size() - 1) == '\'')
    {
      StringExpressionVariable::Pointer var(new StringExpressionVariable(UnEscapeString(arg.substr(1, arg.size() - 2))));
      return var;
    }
    else if ("true" == arg)
    {
      BooleanExpressionVariable::Pointer var(new BooleanExpressionVariable(true));
      return var;
    }
    else if ("false" == arg)
    {
      BooleanExpressionVariable::Pointer var(new BooleanExpressionVariable(false));
      return var;
    }
    else if (arg.find('.') != std::string::npos)
    {
      try
      {
        double num = Poco::NumberParser::parseFloat(arg);
        FloatExpressionVariable::Pointer var(new FloatExpressionVariable(num));
        return var;
      }
      catch (Poco::SyntaxException)
      {
        StringExpressionVariable::Pointer var(new StringExpressionVariable(arg));
        return var;
      }
    }
    else
    {
      try
      {
        int num = Poco::NumberParser::parse(arg);
        IntegerExpressionVariable::Pointer var(new IntegerExpressionVariable(num));
        return var;
      }
      catch (Poco::SyntaxException e)
      {
        StringExpressionVariable::Pointer var(new StringExpressionVariable(arg));
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
