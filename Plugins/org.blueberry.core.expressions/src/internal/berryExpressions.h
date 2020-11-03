/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYEXPRESSIONS_H_
#define BERRYEXPRESSIONS_H_

#include "Poco/Any.h"
#include "Poco/DOM/Element.h"

#include "berryExpression.h"
#include "berryIIterable.h"
#include "berryICountable.h"

#include <vector>
#include <typeinfo>

namespace berry
{

struct IConfigurationElement;

namespace Reflection { class TypeInfo; }

class Expressions {

private:

  Expressions();

  static int FindNextComma(const QString& str, int start);

  static bool IsSubtype(const Reflection::TypeInfo& typeInfo, const QString& type);

public:

  /* debugging flag to enable tracing */
  static bool TRACING;

  static bool IsInstanceOf(const Object *element, const QString& type);

  static void CheckAttribute(const QString& name, const QString& value);

  static void CheckAttribute(const QString& name, const QString& value, QStringList& validValues);

  static void CheckCollection(Object::ConstPointer var, Expression::Pointer expression);

  /**
   * Converts the given variable into an <code>IIterable</code>. If a corresponding adapter can't be found an
   * exception is thrown. If the corresponding adapter isn't loaded yet, <code>null</code> is returned.
   *
   * @param var the variable to turn into an <code>IIterable</code>
   * @param expression the expression referring to the variable
   *
   * @return the <code>IIterable</code> or <code>null<code> if a corresponding adapter isn't loaded yet
   *
   * @throws CoreException if the var can't be adapted to an <code>IIterable</code>
   */
  static IIterable::ConstPointer GetAsIIterable(Object::ConstPointer var, Expression::ConstPointer expression);

  /**
   * Converts the given variable into an <code>ICountable</code>. If a corresponding adapter can't be found an
   * exception is thrown. If the corresponding adapter isn't loaded yet, <code>null</code> is returned.
   *
   * @param var the variable to turn into an <code>ICountable</code>
   * @param expression the expression referring to the variable
   *
   * @return the <code>ICountable</code> or <code>null<code> if a corresponding adapter isn't loaded yet
   *
   * @throws CoreException if the var can't be adapted to an <code>ICountable</code>
   */
  static ICountable::ConstPointer GetAsICountable(Object::ConstPointer var, Expression::ConstPointer expression);

  static bool GetOptionalBooleanAttribute(SmartPointer<IConfigurationElement> element, const QString& attributeName);

  static bool GetOptionalBooleanAttribute(Poco::XML::Element* element, const QString& attributeName);

  //---- Argument parsing --------------------------------------------

  static QList<Object::Pointer> GetArguments(const SmartPointer<IConfigurationElement>& element,
                                             const QString& attributeName);

  static QList<Object::Pointer> GetArguments(Poco::XML::Element* element,
                                             const QString& attributeName);

  static QList<Object::Pointer> ParseArguments(const QString& args);

  static Object::Pointer ConvertArgument(const QString& arg);

  static QString UnEscapeString(const QString& str);

};

}

#endif /*BERRYEXPRESSIONS_H_*/
