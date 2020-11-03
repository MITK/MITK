/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYELEMENTHANDLER_H_
#define BERRYELEMENTHANDLER_H_

#include <berryObject.h>

#include <org_blueberry_core_expressions_Export.h>

namespace Poco {
namespace XML {
class Element;
}
}

namespace berry {

struct IConfigurationElement;

class CompositeExpression;
class Expression;
class ExpressionConverter;

/**
 * An element handler converts an {@link IConfigurationElement} into a
 * corresponding expression object.
 * <p>
 * The class should be subclassed by clients wishing to provide an element
 * handler for special expressions.
 * </p>
 * @since 3.0
 */
class BERRY_EXPRESSIONS ElementHandler : public Object {

public:

  berryObjectMacro(ElementHandler);

  ~ElementHandler() override;

  /**
   * The default element handler which can cope with all XML expression elements
   * defined by the common expression language.
   *
   * @return the default element handler
   */
  static ElementHandler::Pointer GetDefault();

  /**
   * Creates the corresponding expression for the given configuration element.
   *
   * @param converter the expression converter used to initiate the
   *  conversion process
   *
   * @param config the configuration element to convert
   *
   * @return the corresponding expression
   *
   * @throws CoreException if the conversion failed
   */
  virtual SmartPointer<Expression> Create(ExpressionConverter* converter, SmartPointer<IConfigurationElement> config) = 0;

  /**
   * Creates the corresponding expression for the given DOM element. This is
   * an optional operation that is only required if the handler supports conversion
   * of DOM elements.
   *
   * @param converter the expression converter used to initiate the
   *  conversion process
   *
   * @param element the DOM element to convert
   *
   * @return the corresponding expression
   *
   * @throws CoreException if the conversion failed
   *
   * @since 3.3
   */
  virtual SmartPointer<Expression> Create(ExpressionConverter* converter, Poco::XML::Element* element);

protected:

  /**
   * Converts the children of the given configuration element and adds them
   * to the given composite expression.
   * <p>
   * Note this is an internal method and should not be called by clients.
   * </p>
   * @param converter the converter used to do the actual conversion
   * @param element the configuration element for which the children
   *  are to be processed
   * @param expression the composite expression representing the result
   *  of the conversion
   *
   * @throws CoreException if the conversion failed
   */
  virtual void ProcessChildren(ExpressionConverter* converter, SmartPointer<IConfigurationElement> element, SmartPointer<CompositeExpression> expression);

  /**
   * Converts the children of the given DOM element and adds them to the
   * given composite expression.
   * <p>
   * Note this is an internal method and should not be called by clients.
   * </p>
   * @param converter the converter used to do the actual conversion
   * @param element the DOM element for which the children are to be processed
   * @param expression the composite expression representing the result
   *  of the conversion
   *
   * @throws CoreException if the conversion failed
   *
   * @since 3.3
   */
  virtual void ProcessChildren(ExpressionConverter* converter, Poco::XML::Element* element, SmartPointer<CompositeExpression> expression);
};

} // namespace berry

#endif /*BERRYELEMENTHANDLER_H_*/
