/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef BERRYEVALUATIONRESULT_
#define BERRYEVALUATIONRESULT_

#include <string>

#include <org_blueberry_core_expressions_Export.h>

#include <berryObject.h>

namespace berry {

/**
 * An evaluation result represents the result of an expression
 * evaluation. There are exact three instances of evaluation
 * result. They are: <code>FALSE_EVAL</code>, <code>TRUE_EVAL</code> and
 * <code>NOT_LOADED</code>. <code>NOT_LOADED</code> represents
 * the fact that an expression couldn't be evaluated since a
 * plug-in providing certain test expressions isn't loaded yet.
 * <p>
 * In addition the class implements the three operation <code>and
 * </code>, <code>or</code> and <code>not</code>. The operation are
 * defined as follows:
 * </p>
 * <p>
 * The and operation:
 * </p>
 * <table border="1" cellpadding="5">
 *   <colgroup>
 *     <col width="120">
 *     <col width="120">
 *     <col width="120">
 *     <col width="120">
 *   </colgroup>
 *   <tbody>
 *     <tr>
 *       <td><em>AND</em></td>
 *       <td>FALSE_EVAL</td>
 *       <td>TRUE_EVAL</td>
 *       <td>NOT_LOADED</td>
 *     </tr>
 *     <tr>
 *       <td>FALSE_EVAL</td>
 *       <td>FALSE_EVAL</td>
 *       <td>FALSE_EVAL</td>
 *       <td>FALSE_EVAL</td>
 *     </tr>
 *     <tr>
 *       <td>TRUE_EVAL</td>
 *       <td>FALSE_EVAL</td>
 *       <td>TRUE_EVAL</td>
 *       <td>NOT_LOADED</td>
 *     </tr>
 *     <tr>
 *       <td>NOT_LOADED</td>
 *       <td>FALSE_EVAL</td>
 *       <td>NOT_LOADED</td>
 *       <td>NOT_LOADED</td>
 *     </tr>
 *   </tbody>
 * </table>
 * <p>
 * The or operation:
 * </p>
 * <table border="1" cellpadding="5">
 *   <colgroup>
 *     <col width="120">
 *     <col width="120">
 *     <col width="120">
 *     <col width="120">
 *   </colgroup>
 *   <tbody>
 *     <tr>
 *       <td><em>OR</em></td>
 *       <td>FALSE_EVAL</td>
 *       <td>TRUE_EVAL</td>
 *       <td>NOT_LOADED</td>
 *     </tr>
 *     <tr>
 *       <td>FALSE_EVAL</td>
 *       <td>FALSE_EVAL</td>
 *       <td>TRUE_EVAL</td>
 *       <td>NOT_LOADED</td>
 *     </tr>
 *     <tr>
 *       <td>TRUE_EVAL</td>
 *       <td>TRUE_EVAL</td>
 *       <td>TRUE_EVAL</td>
 *       <td>TRUE_EVAL</td>
 *     </tr>
 *     <tr>
 *       <td>NOT_LOADED</td>
 *       <td>NOT_LOADED</td>
 *       <td>TRUE_EVAL</td>
 *       <td>NOT_LOADED</td>
 *     </tr>
 *   </tbody>
 * </table>
 * <p>
 * The not operation:
 * </p>
 * <table border="1" cellpadding="5">
 *   <colgroup>
 *     <col width="120">
 *     <col width="120">
 *     <col width="120">
 *     <col width="120">
 *   </colgroup>
 *   <tbody>
 *     <tr>
 *       <td><em>NOT<em></td>
 *       <td>FALSE_EVAL</td>
 *       <td>TRUE_EVAL</td>
 *       <td>NOT_LOADED</td>
 *     </tr>
 *     <tr>
 *       <td></td>
 *       <td>TRUE_EVAL</td>
 *       <td>FALSE_EVAL</td>
 *       <td>NOT_LOADED</td>
 *     </tr>
 *   </tbody>
 * </table>
 *
 * <p>
 * The class is not intended to be subclassed by clients.
 * </p>
 * @since 3.0
 */
class BERRY_EXPRESSIONS EvaluationResult : public Object
{

private:
  int fValue;

  static const int FALSE_VALUE;
  static const int TRUE_VALUE;
  static const int NOT_LOADED_VALUE;

public:

  /** The evaluation result representing the value FALSE */
  static const SmartPointer<const EvaluationResult> FALSE_EVAL;
  /** The evaluation result representing the value TRUE */
  static const SmartPointer<const EvaluationResult> TRUE_EVAL;
  /** The evaluation result representing the value NOT_LOADED */
  static const SmartPointer<const EvaluationResult> NOT_LOADED;

private:

  static const SmartPointer<const EvaluationResult> AND[3][3];
  static const SmartPointer<const EvaluationResult> OR[3][3];
  static const SmartPointer<const EvaluationResult> NOT[3];

  /*
   * No instances outside of <code>EvaluationResult</code>
   */
  EvaluationResult(int value);

  EvaluationResult(const EvaluationResult& o);

public:

  berryObjectMacro(berry::EvaluationResult)

  bool operator==(const Object*) const override;

  bool operator!=(const Object*) const;

  /**
   * Returns an <code>EvaluationResult</code> whose value is <code>this &amp;&amp; other)</code>.
   *
   * @param other the right hand side of the and operation.
   *
   * @return <code>this &amp;&amp; other</code> as defined by the evaluation result
   */
  EvaluationResult::ConstPointer And(const EvaluationResult::ConstPointer& other) const;

  /**
   * Returns an <code>EvaluationResult</code> whose value is <code>this || other)</code>.
   *
   * @param other the right hand side of the or operation.
   *
   * @return <code>this || other</code> as defined by the evaluation result
   */
  EvaluationResult::ConstPointer Or(const EvaluationResult::ConstPointer& other) const;

  /**
   * Returns the inverted value of this evaluation result
   *
   * @return the inverted value of this evaluation result
   */
  EvaluationResult::ConstPointer Not() const;

  /**
   * Returns an evaluation result instance representing the
   * given boolean value. If the given boolean value is
   * <code>TRUE_EVAL</code> then <code>ExpressionResult.TRUE_EVAL</code>
   * is returned. If the value is <code>FALSE_EVAL</code> then <code>
   * ExpressionResult.FALSE_EVAL</code> is returned.
   *
   * @param b a boolean value
   *
   * @return the expression result representing the boolean
   *  value
   */
  static EvaluationResult::ConstPointer ValueOf(bool b);

  /**
   * For debugging purpose only
   *
   * @return a string representing this object. The result is not
   *  human readable
   */
  QString ToString() const override;
};

}  // namespace berry

#endif /*BERRYEVALUATIONRESULT_*/
