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

#ifndef CHERRYEVALUATIONRESULT_
#define CHERRYEVALUATIONRESULT_

#include <string>

#include "cherryExpressionsDll.h"

namespace cherry {

/**
 * An evaluation result represents the result of an expression
 * evaluation. There are exact three instances of evaluation 
 * result. They are: <code>FALSE</code>, <code>TRUE</code> and
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
 *       <td>FALSE</td>
 *       <td>TRUE</td>
 *       <td>NOT_LOADED</td>
 *     </tr>
 *     <tr>
 *       <td>FALSE</td>
 *       <td>FALSE</td>
 *       <td>FALSE</td>
 *       <td>FALSE</td>
 *     </tr>
 *     <tr>
 *       <td>TRUE</td>
 *       <td>FALSE</td>
 *       <td>TRUE</td>
 *       <td>NOT_LOADED</td>
 *     </tr>
 *     <tr>
 *       <td>NOT_LOADED</td>
 *       <td>FALSE</td>
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
 *       <td>FALSE</td>
 *       <td>TRUE</td>
 *       <td>NOT_LOADED</td>
 *     </tr>
 *     <tr>
 *       <td>FALSE</td>
 *       <td>FALSE</td>
 *       <td>TRUE</td>
 *       <td>NOT_LOADED</td>
 *     </tr>
 *     <tr>
 *       <td>TRUE</td>
 *       <td>TRUE</td>
 *       <td>TRUE</td>
 *       <td>TRUE</td>
 *     </tr>
 *     <tr>
 *       <td>NOT_LOADED</td>
 *       <td>NOT_LOADED</td>
 *       <td>TRUE</td>
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
 *       <td>FALSE</td>
 *       <td>TRUE</td>
 *       <td>NOT_LOADED</td>
 *     </tr>
 *     <tr>
 *       <td></td>
 *       <td>TRUE</td>
 *       <td>FALSE</td>
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
class CHERRY_EXPRESSIONS EvaluationResult {
  
private:
  int fValue;
  
  static const int FALSE_VALUE;
  static const int TRUE_VALUE;
  static const int NOT_LOADED_VALUE;
  
public:
  
  /** The evaluation result representing the value FALSE */
  static const EvaluationResult FALSE;
  /** The evaluation result representing the value TRUE */
  static const EvaluationResult TRUE;
  /** The evaluation result representing the value NOT_LOADED */
  static const EvaluationResult NOT_LOADED;

private:
  
  static const EvaluationResult AND[3][3];
  static const EvaluationResult OR[3][3];
  static const EvaluationResult NOT[3];

  /*
   * No instances outside of <code>EvaluationResult</code>
   */
  EvaluationResult(int value);
  
public:
  
  bool operator==(const EvaluationResult&);
  
  bool operator!=(const EvaluationResult&);
  
  /**
   * Returns an <code>EvaluationResult</code> whose value is <code>this &amp;&amp; other)</code>.
   * 
   * @param other the right hand side of the and operation.
   * 
   * @return <code>this &amp;&amp; other</code> as defined by the evaluation result
   */
  EvaluationResult And(EvaluationResult other);
  
  /**
   * Returns an <code>EvaluationResult</code> whose value is <code>this || other)</code>.
   * 
   * @param other the right hand side of the or operation.
   * 
   * @return <code>this || other</code> as defined by the evaluation result
   */
  EvaluationResult Or(EvaluationResult other);
  
  /**
   * Returns the inverted value of this evaluation result
   * 
   * @return the inverted value of this evaluation result
   */
  EvaluationResult Not();
  
  /**
   * Returns an evaluation result instance representing the
   * given boolean value. If the given boolean value is
   * <code>true</code> then <code>ExpressionResult.TRUE</code>
   * is returned. If the value is <code>false</code> then <code>
   * ExpressionResult.FALSE</code> is returned.
   * 
   * @param b a boolean value
   * 
   * @return the expression result representing the boolean
   *  value
   */
  static EvaluationResult ValueOf(bool b);
  
  /**
   * For debugging purpose only
   * 
   * @return a string representing this object. The result is not
   *  human readable
   */
  std::string ToString();
};

}  // namespace cherry

#endif /*CHERRYEVALUATIONRESULT_*/
