/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYEXPRESSIONSTATUS_H
#define BERRYEXPRESSIONSTATUS_H

#include "berryStatus.h"

namespace berry {

/**
 * Represents the outcome of an expression evaluation. Status objects are
 * used inside {@link CoreException} objects to
 * indicate what went wrong.
 *
 * @see CoreException
 */
class ExpressionStatus : public Status
{

public:

  /** Error code indicating that the variable in focus in not a collection */
  static const int VARIABLE_IS_NOT_A_COLLECTION; // = 3;

  /** Error code indicating that the variable in focus in not a list */
  static const int VARIABLE_IS_NOT_A_LIST; // = 4;

  /** Error code indicating that an attribute value doesn't present an integer */
  static const int VALUE_IS_NOT_AN_INTEGER; // = 5;

  /** Error code indicating that a mandatory attribute is missing */
  static const int MISSING_ATTRIBUTE; // = 50;

  /** Error code indicating that the value specified for an attribute is invalid */
  static const int WRONG_ATTRIBUTE_VALUE; // = 51;

  /** Error code indicating that we are unable to find an expression */
  static const int MISSING_EXPRESSION ; // = 52;

  /** Error code indicating that the number of arguments passed to resolve variable is incorrect. */
  static const int VARAIBLE_POOL_WRONG_NUMBER_OF_ARGUMENTS; // = 100;

  /** Error code indicating that the argument passed to resolve a variable is not of type java.lang.String */
  static const int VARAIBLE_POOL_ARGUMENT_IS_NOT_A_STRING; // = 101;

  /** Error code indicating that a plugin providing a certain type extender isn't loaded yet */
  static const int TYPE_EXTENDER_PLUGIN_NOT_LOADED; // = 200;

  /** Error indicating that a property referenced in a test expression can't be resolved */
  static const int TYPE_EXTENDER_UNKOWN_METHOD; // = 201;

  /** Error code indicating that the implementation class of a type extender is not of type TypeExtender */
  static const int TYPE_EXTENDER_INCORRECT_TYPE; // = 202;

  /** Error indicating that the value returned from a type extender isn't of type boolean */
  static const int TEST_EXPRESSION_NOT_A_BOOLEAN; // = 203;

  /** Error indicating that the property attribute of the test element doesn't have a name space */
  static const int NO_NAMESPACE_PROVIDED; // = 300;

  /** Error indicating that a variable accessed in a with expression isn't available in the evaluation context */
  static const int VARIABLE_NOT_DEFINED; // = 301;

  /** Error indicating that in a string passed via a arg attribute the apostrophe character isn't correctly escaped */
  static const int STRING_NOT_CORRECT_ESCAPED; // = 302;

  /** Error indicating that a string passed via a arg attribute isn't correctly terminated with an apostrophe */
  static const int STRING_NOT_TERMINATED; // = 303;

  /**
   * Creates a new expression status.
   *
   * @param errorCode the error code of the status
   * @param message a human-readable message, localized to the current locale
   */
  ExpressionStatus(int errorCode, const QString& message, const SourceLocation &sl);

  /**
   * Creates a new expression status.
   *
   * @param errorCode the error code of the status
   * @param message a human-readable message, localized to the current locale
   * @param exception a low-level exception, or <code>null</code> if not applicable
   */
  ExpressionStatus(int errorCode, const QString& message, const ctkException& exc, const SourceLocation &sl);
};

}

#endif // BERRYEXPRESSIONSTATUS_H
