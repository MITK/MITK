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

#ifndef BERRYEXPRESSION_H_
#define BERRYEXPRESSION_H_

#include "berryExpressionInfo.h"
#include "berryIEvaluationContext.h"
#include "berryEvaluationResult.h"
#include "berryExpressionsDll.h"

#include <berryMacros.h>
#include <berryObject.h>

#include <string>

namespace berry {

/**
 * Abstract base class for all expressions provided by the common
 * expression language.
 * <p>
 * An expression is evaluated by calling {@link #evaluate(IEvaluationContext)}.
 * </p>
 * <p>
 * This class may be subclassed to provide specific expressions.
 * </p>
 *
 * @since 3.0
 */
class BERRY_EXPRESSIONS Expression : public Object {


public:
  berryObjectMacro(Expression)

  /**
   * The constant integer hash code value meaning the hash code has not yet
   * been computed.
   */
  static const std::size_t HASH_CODE_NOT_COMPUTED;

  /**
   * A factor for computing the hash code for all expressions.
   */
  static const std::size_t HASH_FACTOR;

  /**
   * Name of the value attribute of an expression (value is <code>value</code>).
   */
  static const std::string ATT_VALUE;

private:

  /**
   * The hash code for this object. This value is computed lazily.  If it is
   * not yet computed, it is equal to {@link #HASH_CODE_NOT_COMPUTED}.
   */
  mutable std::size_t fHashCode;

protected:

  /**
   * Checks whether two objects are equal using the
   * <code>equals(Object)</code> method of the <code>left</code> object.
   * This method handles <code>null</code> for either the <code>left</code>
   * or <code>right</code> object.
   *
   * @param left the first object to compare; may be <code>null</code>.
   * @param right the second object to compare; may be <code>null</code>.
   * @return <code>TRUE_EVAL</code> if the two objects are equivalent;
   *         <code>FALSE_EVAL</code> otherwise.
   *
   * @since 3.2
   */
   // static bool Equals(final Object left, final Object right);

  /**
   * Tests whether two arrays of objects are equal to each other. The arrays
   * must not be <code>null</code>, but their elements may be
   * <code>null</code>.
   *
   * @param leftArray the left array to compare; may be <code>null</code>, and
   *  may be empty and may contain <code>null</code> elements.
   * @param rightArray the right array to compare; may be <code>null</code>,
   *  and may be empty and may contain <code>null</code> elements.
   *
   * @return <code>TRUE_EVAL</code> if the arrays are equal length and the elements
   *  at the same position are equal; <code>FALSE_EVAL</code> otherwise.
   *
   * @since 3.2
   */
  static bool Equals(std::vector<Expression::Pointer>& leftArray, std::vector<Expression::Pointer>& rightArray);

  static bool Equals(std::vector<Object::Pointer>& leftArray, std::vector<Object::Pointer>& rightArray);

    /**
   * Returns the hash code for the given <code>object</code>. This method
   * handles <code>null</code>.
   *
   * @param object the object for which the hash code is desired; may be
   *  <code>null</code>.
   *
   * @return The hash code of the object; zero if the object is
   *  <code>null</code>.
   *
   * @since 3.2
   */
   static std::size_t HashCode(Expression::Pointer object);

    /**
   * Returns the hash code for the given array. This method handles
   * <code>null</code>.
   *
   * @param array the array for which the hash code is desired; may be
   *  <code>null</code>.
   * @return the hash code of the array; zero if the object is
   *  <code>null</code>.
   *
   * @since 3.2
   */
  static std::size_t HashCode(std::vector<Expression::Pointer>& array);

  static std::size_t HashCode(std::vector<Object::Pointer>& array);

  /**
     * Method to compute the hash code for this object. The result
     * returned from this method is cached in the <code>fHashCode</code>
     * field. If the value returned from the method equals {@link #HASH_CODE_NOT_COMPUTED}
     * (e.g. <code>-1</code>) then the value is incremented by one.
     * <p>
     * This default implementation calls <code>super.hashCode()</code>
     * </p>
     * @return a hash code for this object.
     *
     * @since 3.2
     */
    virtual std::size_t ComputeHashCode() const;


public:

  /**
   * The expression corresponding to {@link EvaluationResult#TRUE_EVAL}.
   */
  static const Expression::Pointer TRUE_EVAL;

  /**
   * The expression corresponding to {@link EvaluationResult#FALSE_EVAL}.
   */
  static const Expression::Pointer FALSE_EVAL;

  Expression();
  virtual ~Expression();


  virtual std::size_t HashCode() const;

  /**
   * Evaluates this expression.
   *
   * @param context an evaluation context providing information like variable,
   *  name spaces, etc. necessary to evaluate this expression
   *
   * @return the result of the expression evaluation
   *
   * @throws CoreException if the evaluation failed. The concrete reason is
   *  defined by the subclass implementing this method
   */
  virtual EvaluationResult Evaluate(IEvaluationContext* context) = 0;

  /**
   * Computes the expression information for the given expression tree.
   * <p>
   * This is a convenience method for collecting the expression information
   * using {@link Expression#collectExpressionInfo(ExpressionInfo)}.
   * </p>
   *
   * @return the expression information
   *
   * @since 3.2
   */
  virtual const ExpressionInfo* ComputeExpressionInfo() const;

  /**
   * Collects information about this expression tree. This default
   * implementation add the expression's type to the set of misbehaving
   * expression types.
   *
   * @param info the expression information object used
   *  to collect the information
   *
   * @since 3.2
   */
  virtual void CollectExpressionInfo(ExpressionInfo* info) const;

  virtual std::string ToString();

  virtual bool operator==(const Object* object) const;

};

class TRUE_EVALExpression : public Expression
{
public:
  EvaluationResult Evaluate(IEvaluationContext* /*context*/)
  {
      return EvaluationResult::TRUE_EVAL;
  }

  void CollectExpressionInfo(ExpressionInfo* /*info*/) {}
};

class FALSE_EVALExpression : public Expression
{
public:
  EvaluationResult Evaluate(IEvaluationContext* /*context*/)
  {
      return EvaluationResult::FALSE_EVAL;
  }

  void CollectExpressionInfo(ExpressionInfo* /*info*/) {}
};

} // namespace berry

#endif /*BERRYEXPRESSION_H_*/
