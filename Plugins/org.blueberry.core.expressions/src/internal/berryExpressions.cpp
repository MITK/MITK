/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryExpressions.h"

#include "berryExpressionStatus.h"

#include <berryPlatform.h>
#include <berryCoreException.h>

#include <berryIAdapterManager.h>
#include <berryIConfigurationElement.h>

#include <berryObjects.h>
#include <berryObjectString.h>
#include <berryObjectList.h>
#include <berryReflection.h>


namespace berry
{

  bool Expressions::TRACING = false;

  Expressions::Expressions()
  {
    // no instance
  }

  bool
  Expressions::IsInstanceOf(const Object* element, const QString& type)
  {
    // null isn't an instanceof of anything.
    if (element == nullptr)
      return false;

    return IsSubtype(element->GetTypeInfo(), type);
  }

  void
  Expressions::CheckAttribute(const QString& name, const QString& value)
  {
    if (value.isNull())
    {
      IStatus::Pointer status(new ExpressionStatus(
                                ExpressionStatus::MISSING_ATTRIBUTE,
                                QString("Mandatory attribute %1 is missing").arg(name),
                                BERRY_STATUS_LOC));
      throw CoreException(status);
    }
  }

  void
  Expressions::CheckAttribute(const QString& name, const QString& value, QStringList &validValues)
  {
    CheckAttribute(name, value);
    if (validValues.contains(value)) return;
    IStatus::Pointer status(new ExpressionStatus(ExpressionStatus::WRONG_ATTRIBUTE_VALUE,
                                                 QString("Attribute value %1 is not valid").arg(value),
                                                 BERRY_STATUS_LOC));
    throw CoreException(status);
  }

  void
  Expressions::CheckCollection(Object::ConstPointer var, Expression::Pointer expression)
  {
    if (var.Cast<const ObjectList<Object::Pointer> >())
      return;
    IStatus::Pointer status(new ExpressionStatus(ExpressionStatus::VARIABLE_IS_NOT_A_COLLECTION,
                                                 QString("The default variable is not of type ObjectList<Object::Pointer>. "
                                                         "Failed expression: %1").arg(expression->ToString()),
                                                 BERRY_STATUS_LOC));
    throw CoreException(status);
  }

  IIterable::ConstPointer
  Expressions::GetAsIIterable(Object::ConstPointer var, Expression::ConstPointer expression)
  {
    IIterable::ConstPointer iterable(var.Cast<const IIterable>());
    if (!iterable.IsNull())
    {
      return iterable;
    }
    else
    {
      IAdapterManager* manager= Platform::GetAdapterManager();
      iterable = manager->GetAdapter<IIterable>(var.GetPointer());

      if (iterable)
      {
        return iterable;
      }

      if (manager->QueryAdapter<IIterable>(var.GetPointer()) == IAdapterManager::NOT_LOADED)
        return IIterable::Pointer();

      IStatus::Pointer status(new ExpressionStatus(ExpressionStatus::VARIABLE_IS_NOT_A_COLLECTION,
                                                   QString("The default variable is not iterable. Failed expression: %1").arg(expression->ToString()),
                                                   BERRY_STATUS_LOC));
      throw CoreException(status);
    }
  }

  ICountable::ConstPointer
  Expressions::GetAsICountable(Object::ConstPointer var, Expression::ConstPointer expression)
  {
    ICountable::ConstPointer countable(var.Cast<const ICountable>());
    if (!countable.IsNull())
    {
      return countable;
    }
    else
    {
      IAdapterManager* manager = Platform::GetAdapterManager();
      ICountable::ConstPointer result(manager->GetAdapter<ICountable>(var.GetPointer()));
      if (result)
      {
        return result;
      }

      if (manager->QueryAdapter<ICountable>(var.GetPointer()) == IAdapterManager::NOT_LOADED)
        return ICountable::ConstPointer();

      IStatus::Pointer status(new ExpressionStatus(ExpressionStatus::VARIABLE_IS_NOT_A_COLLECTION,
                                                   QString("The default variable is not countable. Failed expression: %1").arg(expression->ToString()),
                                                   BERRY_STATUS_LOC));
      throw CoreException(status);
    }
  }

  bool
  Expressions::GetOptionalBooleanAttribute(IConfigurationElement::Pointer element, const QString& attributeName)
  {
    QString value = element->GetAttribute(attributeName);
    if (value.isNull())
      return false;

    return value.compare("true", Qt::CaseInsensitive) == 0;
  }

  bool
  Expressions::GetOptionalBooleanAttribute(Poco::XML::Element* element, const QString& attributeName)
  {
    QString value = QString::fromStdString(element->getAttribute(attributeName.toStdString()));
    if (value.size() == 0)
      return false;

    return value.compare("true", Qt::CaseInsensitive) == 0;
  }

  QList<Object::Pointer>
  Expressions::GetArguments(const IConfigurationElement::Pointer& element, const QString& attributeName)
  {
    QString value = element->GetAttribute(attributeName);
    if (!value.isNull())
    {
      return ParseArguments(value);
    }
    return QList<Object::Pointer>();
  }

  QList<Object::Pointer>
  Expressions::GetArguments(Poco::XML::Element* element, const QString& attributeName)
  {
    std::string value = element->getAttribute(attributeName.toStdString());
    if (value.size() > 0)
    {
      return ParseArguments(QString::fromStdString(value));
    }
    return QList<Object::Pointer>();
  }

  QList<Object::Pointer>
  Expressions::ParseArguments(const QString& args)
  {
    QList<Object::Pointer> result;
    int start= 0;
    int comma;
    while ((comma = FindNextComma(args, start)) != -1)
    {
      result.push_back(ConvertArgument(args.mid(start, comma-start).trimmed()));
      start= comma + 1;
    }
    result.push_back(ConvertArgument(args.mid(start).trimmed()));
    return result;
  }

  int
  Expressions::FindNextComma(const QString& str, int start)
  {
    bool inString = false;
    for (int i = start; i < str.size(); i++)
    {
      const QChar ch = str.at(i);
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
    {
      IStatus::Pointer status(new ExpressionStatus(ExpressionStatus::STRING_NOT_TERMINATED,
                                                   QString("The String \"%1\" is not correctly terminated with an apostrophe character.").arg(str),
                                                   BERRY_STATUS_LOC));
      throw CoreException(status);
    }

    return -1;
  }

  bool Expressions::IsSubtype(const Reflection::TypeInfo& typeInfo, const QString& type)
  {
    if (typeInfo.GetName() == type) return true;
    QList<Reflection::TypeInfo> superClasses = typeInfo.GetSuperclasses();
    for (int i = 0; i < superClasses.size(); ++i)
    {
      if (IsSubtype(superClasses[i], type))
      {
        return true;
      }
    }
    return false;
  }

  Object::Pointer
  Expressions::ConvertArgument(const QString& arg)
  {
    if (arg.isNull())
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
      ObjectString::Pointer var(new ObjectString(UnEscapeString(arg.mid(1, arg.size() - 2))));
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
    else if (arg.indexOf('.') != -1)
    {
      bool ok = false;
      float num = arg.toFloat(&ok);
      if (ok)
      {
        ObjectFloat::Pointer var(new ObjectFloat(num));
        return var;
      }
      else
      {
        ObjectString::Pointer var(new ObjectString(arg));
        return var;
      }
    }
    else
    {
      bool ok = false;
      int num = arg.toInt(&ok);
      if (ok)
      {
        ObjectInt::Pointer var(new ObjectInt(num));
        return var;
      }
      else
      {
        ObjectString::Pointer var(new ObjectString(arg));
        return var;
      }
    }
  }

  QString
  Expressions::UnEscapeString(const QString& str)
  {
    QString result = "";
    for (int i= 0; i < str.size(); i++)
    {
      const QChar ch= str.at(i);
      if (ch == '\'')
      {
        if (i == str.size() - 1 || str.at(i + 1) != '\'')
        {
          IStatus::Pointer status(new ExpressionStatus(ExpressionStatus::STRING_NOT_CORRECT_ESCAPED,
                                                       QString("The String \"%1\" is not correctly escaped. "
                                                               "Wrong number of apostrophe characters.").arg(str),
                                                       BERRY_STATUS_LOC));
          throw CoreException(status);
        }
        result.append('\'');
        i++;
      }
      else
      {
        result.append(ch);
      }
    }
    return result;
  }

} // namespace berry
