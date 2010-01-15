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

#include "cherryStandardElementHandler.h"

#include "../cherryExpressionTagNames.h"

#include "cherryInstanceofExpression.h"
#include "cherryTestExpression.h"
#include "cherryOrExpression.h"
#include "cherryAndExpression.h"
#include "cherryNotExpression.h"
#include "cherryWithExpression.h"
#include "cherryAdaptExpression.h"
#include "cherryIterateExpression.h"
#include "cherryCountExpression.h"
#include "cherrySystemTestExpression.h"
#include "cherryResolveExpression.h"
#include "cherryEnablementExpression.h"
#include "cherryEqualsExpression.h"
#include "cherryReferenceExpression.h"

namespace cherry
{

Expression::Pointer
StandardElementHandler::Create(ExpressionConverter* converter, IConfigurationElement::Pointer element)
{
  std::string name = element->GetName();
  if (ExpressionTagNames::INSTANCEOF == name) {
    Expression::Pointer result(new InstanceofExpression(element));
    return result;
  } else if (ExpressionTagNames::TEST == name) {
    Expression::Pointer result(new TestExpression(element));
    return result;
  } else if (ExpressionTagNames::OR == name) {
    CompositeExpression::Pointer result(new OrExpression());
    this->ProcessChildren(converter, element, result);
    return result;
  } else if (ExpressionTagNames::AND == name) {
    CompositeExpression::Pointer result(new AndExpression());
    this->ProcessChildren(converter, element, result);
    return result;
  } else if (ExpressionTagNames::NOT == name) {
    IConfigurationElement::vector children(element->GetChildren());
    Expression::Pointer result(new NotExpression(converter->Perform(children[0])));
    return result;
  } else if (ExpressionTagNames::WITH == name) {
    CompositeExpression::Pointer result(new WithExpression(element));
    this->ProcessChildren(converter, element, result);
    return result;
  } else if (ExpressionTagNames::ADAPT == name) {
    CompositeExpression::Pointer result(new AdaptExpression(element));
    this->ProcessChildren(converter, element, result);
    return result;
  } else if (ExpressionTagNames::ITERATE == name) {
    CompositeExpression::Pointer result(new IterateExpression(element));
    this->ProcessChildren(converter, element, result);
    return result;
  } else if (ExpressionTagNames::COUNT == name) {
    Expression::Pointer result(new CountExpression(element));
    return result;
  } else if (ExpressionTagNames::SYSTEM_TEST == name) {
    Expression::Pointer result(new SystemTestExpression(element));
    return result;
  } else if (ExpressionTagNames::RESOLVE == name) {
    CompositeExpression::Pointer result(new ResolveExpression(element));
    this->ProcessChildren(converter, element, result);
    return result;
  } else if (ExpressionTagNames::ENABLEMENT == name) {
    CompositeExpression::Pointer result(new EnablementExpression(element));
    this->ProcessChildren(converter, element, result);
    return result;
  } else if (ExpressionTagNames::EQUALS == name) {
    Expression::Pointer result(new EqualsExpression(element));
    return result;
  } else if (ExpressionTagNames::REFERENCE == name) {
    Expression::Pointer result(new ReferenceExpression(element));
    return result;
  }
  return Expression::Pointer();
}

Expression::Pointer
StandardElementHandler::Create(ExpressionConverter* converter, Poco::XML::Element* element)
{
  std::string name= element->nodeName();
  if (ExpressionTagNames::INSTANCEOF == name) {
    Expression::Pointer result(new InstanceofExpression(element));
    return result;
  } else if (ExpressionTagNames::TEST == name) {
    Expression::Pointer result(new TestExpression(element));
    return result;
  } else if (ExpressionTagNames::OR == name) {
    CompositeExpression::Pointer result(new OrExpression());
    this->ProcessChildren(converter, element, result);
    return result;
  } else if (ExpressionTagNames::AND == name) {
    CompositeExpression::Pointer result(new AndExpression());
    this->ProcessChildren(converter, element, result);
    return result;
  } else if (ExpressionTagNames::NOT == name) {
    Poco::XML::Node* child = element->firstChild();
    while (child != 0) {
      if (child->nodeType() == Poco::XML::Node::ELEMENT_NODE) {
        Expression::Pointer result(new NotExpression(converter->Perform(static_cast<Poco::XML::Element*>(child))));
        return result;
      }
      child = child->nextSibling();
    }
  } else if (ExpressionTagNames::WITH == name) {
    CompositeExpression::Pointer result(new WithExpression(element));
    this->ProcessChildren(converter, element, result);
    return result;
  } else if (ExpressionTagNames::ADAPT == name) {
    CompositeExpression::Pointer result(new AdaptExpression(element));
    this->ProcessChildren(converter, element, result);
    return result;
  } else if (ExpressionTagNames::ITERATE == name) {
    CompositeExpression::Pointer result(new IterateExpression(element));
    this->ProcessChildren(converter, element, result);
    return result;
  } else if (ExpressionTagNames::COUNT == name) {
    Expression::Pointer result(new CountExpression(element));
    return result;
  } else if (ExpressionTagNames::SYSTEM_TEST == name) {
    Expression::Pointer result(new SystemTestExpression(element));
    return result;
  } else if (ExpressionTagNames::RESOLVE == name) {
    CompositeExpression::Pointer result(new ResolveExpression(element));
    this->ProcessChildren(converter, element, result);
    return result;
  } else if (ExpressionTagNames::ENABLEMENT == name) {
    CompositeExpression::Pointer result(new EnablementExpression(element));
    this->ProcessChildren(converter, element, result);
    return result;
  } else if (ExpressionTagNames::EQUALS == name) {
    Expression::Pointer result(new EqualsExpression(element));
    return result;
  } else if (ExpressionTagNames::REFERENCE == name) {
    Expression::Pointer result(new ReferenceExpression(element));
    return result;
  }
  return Expression::Pointer();
}

}  // namespace cherry
