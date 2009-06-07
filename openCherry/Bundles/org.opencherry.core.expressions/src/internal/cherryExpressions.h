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

#ifndef CHERRYEXPRESSIONS_H_
#define CHERRYEXPRESSIONS_H_

#include "Poco/Any.h"
#include "Poco/DOM/Element.h"

#include "service/cherryIConfigurationElement.h"

#include "../cherryExpression.h"
#include "../cherryIIterable.h"
#include "../cherryICountable.h"

#include <string>
#include <vector>
#include <typeinfo>

namespace cherry
{

class Expressions {

private:

  Expressions();

  static int FindNextComma(const std::string& str, int start);

public:

  /* debugging flag to enable tracing */
  static const bool TRACING;

  static bool IsInstanceOf(Object::ConstPointer element, const std::string& type);

  static void CheckAttribute(const std::string& name, bool value);

  static void CheckAttribute(const std::string& name, bool result, const std::string& value, std::vector<std::string>& validValues);

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
  static IIterable::Pointer GetAsIIterable(Object::Pointer var, Expression::Pointer expression);

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
  static ICountable::Pointer GetAsICountable(Object::Pointer var, Expression::Pointer expression);

  static bool GetOptionalBooleanAttribute(SmartPointer<IConfigurationElement> element, const std::string& attributeName);

  static bool GetOptionalBooleanAttribute(Poco::XML::Element* element, const std::string& attributeName);

  //---- Argument parsing --------------------------------------------

  static void GetArguments(std::vector<Object::Pointer>& args, SmartPointer<IConfigurationElement> element, const std::string& attributeName);

  static void GetArguments(std::vector<Object::Pointer>& args, Poco::XML::Element* element, const std::string& attributeName);

  static void ParseArguments(std::vector<Object::Pointer>&, const std::string& args);

  static Object::Pointer ConvertArgument(const std::string& arg, bool result = true);

  static std::string UnEscapeString(const std::string& str);

};

}

#endif /*CHERRYEXPRESSIONS_H_*/
