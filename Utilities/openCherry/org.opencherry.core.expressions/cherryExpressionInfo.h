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

#ifndef CHERRYEXPRESSIONINFO_H_
#define CHERRYEXPRESSIONINFO_H_

#include <string>
#include <typeinfo>
#include <set>

#include <org.opencherry.osgi/cherryDll.h>

namespace cherry {

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
class CHERRY_API ExpressionInfo {

private:
  bool fHasDefaultVariableAccess;
  bool fHasSystemPropertyAccess;
  
  // Although we are using this as sets we use lists since
  // they are faster for smaller numbers of elements
  std::set<std::string> fAccessedVariableNames;
  std::set<std::string> fMisbehavingExpressionTypes;
  std::set<std::string> fAccessedPropertyNames;
  
  
public:
  
  /**
   * Returns <code>true</code> if the default variable is accessed
   * by the expression tree.
   * 
   * @return whether the default variable is accessed or not
   */
  bool HasDefaultVariableAccess();
  
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
  bool HasSystemPropertyAccess();
  
  /**
   * Marks the system property as accessed.
   */
  void MarkSystemPropertyAccessed();

  /**
   * Returns the set off accessed variables.
   * 
   * @return the set off accessed variables
   */
  void GetAccessedVariableNames(std::set<std::string>& names);
  
  /**
   * Marks the given variable as accessed.
   * 
   * @param name the accessed variable
   */
  void AddVariableNameAccess(const std::string& name);
  
  /**
   * Returns the set of accessed properties.
   * 
   * @return the set of accessed properties, or an empty array
   * @since 3.4
   */
  void GetAccessedPropertyNames(std::set<std::string>& names);

  /**
   * Marks that this expression access this property. It should be the fully
   * qualified property name.
   * 
   * @param name
   *            the fully qualified property name
   * @since 3.4
   */
  void AddAccessedPropertyName(const std::string& name);

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
  void GetMisbehavingExpressionTypes(std::set<std::string>&);
  
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

}  // namespace cherry

#endif /*CHERRYEXPRESSIONINFO_H_*/
