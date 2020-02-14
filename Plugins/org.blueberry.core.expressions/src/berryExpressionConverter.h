/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYEXPRESSIONCONVERTER_H_
#define BERRYEXPRESSIONCONVERTER_H_

#include <org_blueberry_core_expressions_Export.h>

#include <berrySmartPointer.h>

#include <QList>

namespace Poco {
namespace XML {
class Element;
}
}

namespace berry {

struct IConfigurationElement;

class CompositeExpression;
class ElementHandler;
class Expression;

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

  QList<SmartPointer<ElementHandler> > fHandlers;

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
  ExpressionConverter(const QList<SmartPointer<ElementHandler> >& handlers);

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
  SmartPointer<Expression> Perform(const SmartPointer<IConfigurationElement>& root);

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
   */
  SmartPointer<Expression> Perform(Poco::XML::Element* root);


  void ProcessChildren(const SmartPointer<IConfigurationElement>& element,
                       const SmartPointer<CompositeExpression>& result);

  QString GetDebugPath(const SmartPointer<IConfigurationElement>& configurationElement);

  void ProcessChildren(Poco::XML::Element* element,
                       const SmartPointer<CompositeExpression>& result);
};

}  // namespace berry

#endif /*BERRYEXPRESSIONCONVERTER_H_*/
