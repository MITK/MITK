/*=========================================================================

Program:   openCherry Platform
Language:  C++
Date:      $Date: 2008-06-12 19:58:42 +0200 (Do, 12 Jun 2008) $
Version:   $Revision: 14610 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "cherryExpressionVariables.h"

#include <algorithm>

namespace cherry {

  void ExpressionVariable::ExtTypeInfo_::Add(const std::string& name, const std::type_info& typeinfo) {
    if (std::find(m_TypeInfos.begin(), m_TypeInfos.end(), &typeinfo) == m_TypeInfos.end())
      m_TypeInfos.push_back(&typeinfo);

    if (std::find(m_TypeNames.begin(), m_TypeNames.end(), name) == m_TypeNames.end())
      m_TypeNames.push_back(name);
  }

  ExpressionVariable::ExpressionVariable() {
    m_TypeInfo.Add("cherry::ExpressionVariable", typeid(this));
  }

  const ExpressionVariable::ExtTypeInfo& ExpressionVariable::GetExtTypeInfo() const {
    return m_TypeInfo;
  }

  std::string ExpressionVariable::GetClassName() const {
    return m_TypeInfo.m_TypeNames.back();
  }

  std::string ExpressionVariable::ToString() const {
    return GetClassName();
  }


  StringExpressionVariable::StringExpressionVariable(const std::string& str) : m_Var(str)
  {
    m_TypeInfo.Add("cherry::StringExpressionVariable", typeid(this));
  }

  bool StringExpressionVariable::operator==(const Object* var) const {
    if (const StringExpressionVariable* that = dynamic_cast<const StringExpressionVariable*>(var))
      return this->m_Var == that->m_Var;

    return false;
  }

  void StringExpressionVariable::SetVariable(const std::string& var) {
    this->m_Var = var;
  }

  std::string& StringExpressionVariable::GetVariable() {
    return m_Var;
  }

  int StringExpressionVariable::HashCode() {
    return Poco::Hash<std::string>()(m_Var);
  }

  VectorExpressionVariable::VectorExpressionVariable() {
    m_TypeInfo.Add("cherry::VectorExpressionVariable", typeid(this));
  }

  bool VectorExpressionVariable::operator==(const Object* var) const {
    if (const VectorExpressionVariable* that = dynamic_cast<const VectorExpressionVariable*>(var))
      return this->m_Var == that->m_Var;

    return false;
  }

  VectorExpressionVariable::VectorType& VectorExpressionVariable::GetVariable() {
    return m_Var;
  }

  int VectorExpressionVariable::HashCode() {
    static int HASH_FACTOR = Poco::Hash<std::string>()("cherry::VectorExpressionVariable");
    if (m_Var.size() == 0)
    {
      return 0;
    }
    int hashCode = Poco::Hash<std::string>()("std::vector<ExpressionVariable::Pointer>");
    for (unsigned int i= 0; i < m_Var.size(); i++)
    {
      hashCode = hashCode * HASH_FACTOR + m_Var[i]->HashCode();
    }
    return hashCode;
  }

  BooleanExpressionVariable::BooleanExpressionVariable(bool b) : m_Var(b)
  {
    m_TypeInfo.Add("cherry::BooleanExpressionVariable", typeid(this));
  }

  bool BooleanExpressionVariable::operator==(const Object* var) const {
    if (const BooleanExpressionVariable* that = dynamic_cast<const BooleanExpressionVariable*>(var))
      return this->m_Var == that->m_Var;

    return false;
  }

  void BooleanExpressionVariable::SetVariable(bool var) {
    this->m_Var = var;
  }

  bool& BooleanExpressionVariable::GetVariable() {
    return m_Var;
  }

  int BooleanExpressionVariable::HashCode() {
    return m_Var ? 1 : 0;
  }


  FloatExpressionVariable::FloatExpressionVariable(double d) : m_Var(d)
  {
    m_TypeInfo.Add("cherry::FloatExpressionVariable", typeid(this));
  }

  bool FloatExpressionVariable::operator==(const Object* var) const {
    if (const FloatExpressionVariable* that = dynamic_cast<const FloatExpressionVariable*>(var))
      return this->m_Var == that->m_Var;

    return false;
  }

  void FloatExpressionVariable::SetVariable(double var) {
    this->m_Var = var;
  }

  int FloatExpressionVariable::HashCode() {
    return (int)m_Var;
  }

  double& FloatExpressionVariable::GetVariable() {
    return m_Var;
  }


  IntegerExpressionVariable::IntegerExpressionVariable(int i) : m_Var(i)
  {
    m_TypeInfo.Add("cherry::IntegerExpressionVariable", typeid(this));
  }

  bool IntegerExpressionVariable::operator==(const Object* var) const {
    if (const IntegerExpressionVariable* that = dynamic_cast<const IntegerExpressionVariable*>(var))
      return this->m_Var == that->m_Var;

    return false;
  }

  void IntegerExpressionVariable::SetVariable(int var) {
    this->m_Var = var;
  }

  int& IntegerExpressionVariable::GetVariable() {
    return m_Var;
  }

  int IntegerExpressionVariable::HashCode() {
    return m_Var;
  }

}
