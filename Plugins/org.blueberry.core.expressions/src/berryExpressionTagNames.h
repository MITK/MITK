/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYEXPRESSIONTAGNAMES_H_
#define BERRYEXPRESSIONTAGNAMES_H_

#include <QString>

#include <org_blueberry_core_expressions_Export.h>

namespace berry {

/**
 * Class defining the tag names of the XML elements of the common
 * expression language.
 */
class BERRY_EXPRESSIONS ExpressionTagNames {

public:

  /** The tag name of the enablement expression (value: <code>enablement</code>) */
  static const QString ENABLEMENT;

  /** The tag name of the and expression (value: <code>and</code>) */
  static const QString AND;

  /** The tag name of the or expression (value: <code>or</code>) */
  static const QString OR;

  /** The tag name of the not expression (value: <code>not</code>) */
  static const QString NOT;

  /** The tag name of the instanceof expression (value: <code>instanceof</code>) */
  static const QString INSTANCEOF;

  /** The tag name of the test expression (value: <code>test</code>) */
  static const QString TEST;

  /** The tag name of the with expression (value: <code>with</code>) */
  static const QString WITH;

  /** The tag name of the adapt expression (value: <code>adapt</code>) */
  static const QString ADAPT;

  /** The tag name of the count expression (value: <code>count</code>) */
  static const QString COUNT;

  /** The tag name of the adapt expression (value: <code>iterate</code>) */
  static const QString ITERATE;

  /** The tag name of the resolve expression (value: <code>resolve</code>) */
  static const QString RESOLVE;

  /** The tag name of the systemTest expression (value: <code>systemTest</code>) */
  static const QString SYSTEM_TEST;

  /** The tag name of the equals expression (value: <code>equals</code>) */
  static const QString EQUALS;

  /**
   * The tag name of the reference expression (value: <code>reference</code>)
   */
  static const QString REFERENCE;
};

}  // namespace berry

#endif /*BERRYEXPRESSIONTAGNAMES_H_*/
