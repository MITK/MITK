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

#ifndef BERRYEXPRESSIONCONVERTER_H_
#define BERRYEXPRESSIONCONVERTER_H_

#include "berryElementHandler.h"
#include "berryExpression.h"

#include "internal/berryCompositeExpression.h"

#include "Poco/DOM/Element.h"

#include <string>
#include <vector>

namespace berry {

/**
 * An expression converter converts an XML expression represented by an
 * {@link IConfigurationElement} or {@link Element} (DOM) subtree into a
 * corresponding expression tree.
 *
 * <p>
 * An expression converter manages a list of {@link ElementHandler}s. Element
 * handlers are responsible to do the actual conversion. The element handlers
 * build a chain of responsibility.
 * </p>
 *
 * @since 3.0
 */
class BERRY_EXPRESSIONS ExpressionConverter {

private:
  std::vector<ElementHandler::Pointer> fHandlers;

  static ExpressionConverter* INSTANCE;

public:

  /**
   * Returns the default expression converter. The default expression converter
   * can cope with all expression elements defined by the common expression
   * language.
   *
   * @return the default expression converter
   */
  static ExpressionConverter* GetDefault();

  /**
   * Creates a new expression converter with the given list of element
   * handlers. The element handlers build a chain of responsibility
   * meaning that the first handler in the list is first used to
   * convert the configuration element. If this handler isn't able
   * to convert the configuration element the next handler in the
   * array is used.
   *
   * @param handlers the array  of element handlers
   */
  ExpressionConverter(std::vector<ElementHandler::Pointer>& handlers);

  /**
   * Converts the tree of configuration elements represented by the given
   * root element and returns a corresponding expression tree.
   *
   * @param root the configuration element to be converted
   *
   * @return the corresponding expression tree or <code>null</code>
   *  if the configuration element cannot be converted
   *
   * @throws CoreException if the configuration element can't be
   *  converted. Reasons include: (a) no handler is available to
   *  cope with a certain configuration element or (b) the XML
   *  expression tree is malformed.
   */
  Expression::Pointer Perform(SmartPointer<IConfigurationElement> root);

  /**
   * Converts the tree of DOM elements represented by the given
   * root element and returns a corresponding expression tree.
   *
   * @param root the element to be converted
   *
   * @return the corresponding expression tree or <code>null</code>
   *  if the element cannot be converted
   *
   * @throws CoreException if the element can't be converted.
   *  Reasons include: (a) no handler is available to cope with
   *  a certain element or (b) the XML expression tree is malformed.
   *
   * @since 3.3
   */
  Expression::Pointer Perform(Poco::XML::Element* root);


  void ProcessChildren(SmartPointer<IConfigurationElement> element, SmartPointer<CompositeExpression> result);

  std::string GetDebugPath(SmartPointer<IConfigurationElement> configurationElement);

  void ProcessChildren(Poco::XML::Element* element, SmartPointer<CompositeExpression> result);
};

}  // namespace berry

#endif /*BERRYEXPRESSIONCONVERTER_H_*/
