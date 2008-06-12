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

#ifndef CHERRYIEXPRESSIONVARIABLE_H_
#define CHERRYIEXPRESSIONVARIABLE_H_

#include <Poco/Hash.h>

#include <cherryMacros.h>

#include "cherryRuntimeDll.h"

#include <typeinfo>
#include <string>
#include <deque>
#include <vector>
#include <algorithm>

namespace cherry {

struct ExpressionVariable : public Object
{
  cherryClassMacro(ExpressionVariable)
  
  typedef std::deque<std::string> TypeNames;
  typedef std::deque<const std::type_info*> TypeInfos;
  
  struct ExtTypeInfo {

    std::deque<std::string> m_TypeNames;
    std::deque<const std::type_info*> m_TypeInfos;
    
  };
  
  struct ExtTypeInfo_ : public ExtTypeInfo {
   
    void Add(const std::string& name, const std::type_info& typeinfo) {
          if (std::find(m_TypeInfos.begin(), m_TypeInfos.end(), &typeinfo) == m_TypeInfos.end())
                  m_TypeInfos.push_back(&typeinfo);
          
          if (std::find(m_TypeNames.begin(), m_TypeNames.end(), name) == m_TypeNames.end())
            m_TypeNames.push_back(name);
        }
  };
  
  virtual bool operator==(ExpressionVariable& var) = 0;
  
  virtual bool operator==(ExpressionVariable* var) {
    return this->operator==(*var);
  }
  
  virtual int HashCode() = 0;
  
  ExpressionVariable() {
    m_TypeInfo.Add("cherry::ExpressionVariable", typeid(this));
  }
  
  const ExtTypeInfo& GetExtTypeInfo() const {
    return m_TypeInfo;
  }
  
  virtual std::string GetClassName() const {
    return m_TypeInfo.m_TypeNames.back();
  }
  
  virtual std::string ToString() const {
    return GetClassName();
  }
  
protected:
  ExtTypeInfo_ m_TypeInfo;
  
};

struct StringExpressionVariable : public ExpressionVariable
{
  cherryClassMacro(StringExpressionVariable)
  
  StringExpressionVariable(const std::string& str) : m_Var(str)
  {
    m_TypeInfo.Add("cherry::StringExpressionVariable", typeid(this));
  }
  
  bool operator==(ExpressionVariable& var) {
    try {
      StringExpressionVariable& that = dynamic_cast<StringExpressionVariable&>(var);
      return this->m_Var == that.m_Var;
    }
    catch (std::bad_cast e)
    {
      return false;
    }
  }
  
  void SetVariable(const std::string& var) {
    this->m_Var = var;
  }
  
  std::string& GetVariable() {
    return m_Var;
  }
  
  int HashCode() {
    return Poco::Hash<std::string>()(m_Var);
  }
  
private:
  
  std::string m_Var;
};

struct VectorExpressionVariable : public ExpressionVariable
{
  cherryClassMacro(VectorExpressionVariable)
  
  typedef std::vector<ExpressionVariable::Pointer> VectorType;
  
  VectorExpressionVariable() {
    m_TypeInfo.Add("cherry::VectorExpressionVariable", typeid(this));
  }
  
  bool operator==(ExpressionVariable& var) {
    try {
      VectorExpressionVariable& that = dynamic_cast<VectorExpressionVariable&>(var);
      return this->m_Var == that.m_Var;
    }
    catch (std::bad_cast e)
    {
      return false;
    }
  }
  
  VectorType& GetVariable() {
    return m_Var;
  }
  
  int HashCode() {
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
  
  
private:
  
  VectorType m_Var;
};

struct BooleanExpressionVariable : public ExpressionVariable
{
  cherryClassMacro(BooleanExpressionVariable)
  
  BooleanExpressionVariable(bool b) : m_Var(b)
  {
    m_TypeInfo.Add("cherry::BooleanExpressionVariable", typeid(this));
  }
  
  bool operator==(ExpressionVariable& var) {
    try {
      BooleanExpressionVariable& that = dynamic_cast<BooleanExpressionVariable&>(var);
      return this->m_Var == that.m_Var;
    }
    catch (std::bad_cast e)
    {
      return false;
    }
  }
  
  void SetVariable(bool var) {
    this->m_Var = var;
  }
  
  bool& GetVariable() {
    return m_Var;
  }
  
  int HashCode() {
    return m_Var ? 1 : 0;
  }
 
  
private:
  
  bool m_Var;
};

struct FloatExpressionVariable : public ExpressionVariable
{
  cherryClassMacro(FloatExpressionVariable)
  
  FloatExpressionVariable(double d) : m_Var(d)
  {
    m_TypeInfo.Add("cherry::FloatExpressionVariable", typeid(this));
  }
  
  bool operator==(ExpressionVariable& var) {
    try {
      FloatExpressionVariable& that = dynamic_cast<FloatExpressionVariable&>(var);
      return this->m_Var == that.m_Var;
    }
    catch (std::bad_cast e)
    {
      return false;
    }
  }
  
  void SetVariable(double var) {
    this->m_Var = var;
  }
  
  int HashCode() {
    return (int)m_Var;
  }
  
  double& GetVariable() {
    return m_Var;
  }
  
  
private:
  
  double m_Var;
};

struct IntegerExpressionVariable : public ExpressionVariable
{
  cherryClassMacro(IntegerExpressionVariable)
  
  IntegerExpressionVariable(int i) : m_Var(i)
  {
    m_TypeInfo.Add("cherry::IntegerExpressionVariable", typeid(this));
  }
  
  bool operator==(ExpressionVariable& var) {
    try {
      IntegerExpressionVariable& that = dynamic_cast<IntegerExpressionVariable&>(var);
      return this->m_Var == that.m_Var;
    }
    catch (std::bad_cast e)
    {
      return false;
    }
  }
  
  void SetVariable(int var) {
    this->m_Var = var;
  }
  
  int& GetVariable() {
    return m_Var;
  }
  
  int HashCode() {
    return m_Var;
  }
  
  
private:
  
  int m_Var;
};

}

#endif /*CHERRYIEXPRESSIONVARIABLE_H_*/
