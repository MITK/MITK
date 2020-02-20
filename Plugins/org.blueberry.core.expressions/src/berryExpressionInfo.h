/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYEXPRESSIONINFO_H_
#define BERRYEXPRESSIONINFO_H_

#include <QSet>

#include <typeinfo>

#include <org_blueberry_core_expressions_Export.h>

namespace berry {

/**
 * A status object describing information about an expression tree.
 * This information can for example be used to decide whether a
 * expression tree as to be reevaluated if the value of some
 * variables changes.
 * <p>
 * This class is not intended to be extended by clients.
 * </p>
 *
 * @since 3.2
 */
class BERRY_EXPRESSIONS ExpressionInfo {

private:
  bool fHasDefaultVariableAccess;
  bool fHasSystemPropertyAccess;

  // Although we are using this as sets we use lists since
  // they are faster for smaller numbers of elements
  QSet<QString> fAccessedVariableNames;
  QSet<QString> fMisbehavingExpressionTypes;
  QSet<QString> fAccessedPropertyNames;


public:

  ExpressionInfo();

  /**
   * Returns <code>true</code> if the default variable is accessed
   * by the expression tree.
   *
   * @return whether the default variable is accessed or not
   */
  bool HasDefaultVariableAccess() const;

  /**
   * Marks the default variable as accessed.
   */
  void MarkDefaultVariableAccessed();

  /**
   * Returns <code>true</code> if the system property is accessed
   * by the expression tree.
   *
   * @return whether the system property is accessed or not
   */
  bool HasSystemPropertyAccess() const;

  /**
   * Marks the system property as accessed.
   */
  void MarkSystemPropertyAccessed();

  /**
   * Returns the set off accessed variables.
   *
   * @return the set off accessed variables
   */
  QSet<QString> GetAccessedVariableNames() const;

  /**
   * Marks the given variable as accessed.
   *
   * @param name the accessed variable
   */
  void AddVariableNameAccess(const QString& name);

  /**
   * Returns the set of accessed properties.
   *
   * @return the set of accessed properties, or an empty array
   */
  QSet<QString> GetAccessedPropertyNames() const;

  /**
   * Marks that this expression access this property. It should be the fully
   * qualified property name.
   *
   * @param name
   *            the fully qualified property name
   */
  void AddAccessedPropertyName(const QString& name);

  /**
   * Returns the set of expression types which don't implement the
   * new (@link Expression#computeReevaluationInfo(IEvaluationContext)}
   * method. If one expression didn't implement the method the expression
   * tree no optimizations can be done. Returns <code>null</code> if
   * all expressions implement the method.
   *
   * @return the set of expression types which don't implement the
   *  <code>computeReevaluationInfo</code> method.
   */
  QSet<QString> GetMisbehavingExpressionTypes() const;

  /**
   * Adds the given class to the list of misbehaving classes.
   *
   * @param clazz the class to add.
   */
  void AddMisBehavingExpressionType(const std::type_info& clazz);

  /**
   * Merges this reevaluation information with the given info.
   *
   * @param other the information to merge with
   */
  void Merge(ExpressionInfo* other);

  /**
   * Merges this reevaluation information with the given info
   * ignoring the default variable access.
   *
   * @param other the information to merge with
   */
  void MergeExceptDefaultVariable(ExpressionInfo* other);


private:

  /**
   * Merges only the default variable access.
   *
   * @param other the information to merge with
   */
  void MergeDefaultVariableAccess(ExpressionInfo* other);

  /**
   * Merges only the system property access.
   *
   * @param other the information to merge with
   */
  void MergeSystemPropertyAccess(ExpressionInfo* other);

  /**
   * Merges only the accessed variable names.
   *
   * @param other the information to merge with
   */
  void MergeAccessedVariableNames(ExpressionInfo* other);

  /**
   * Merges only the accessed property names.
   *
   * @param other the information to merge with
   * @since 3.4
   */
  void MergeAccessedPropertyNames(ExpressionInfo* other);

  /**
   * Merges only the misbehaving expression types.
   *
   * @param other the information to merge with
   */
  void MergeMisbehavingExpressionTypes(ExpressionInfo* other);

};

}  // namespace berry

#endif /*BERRYEXPRESSIONINFO_H_*/
