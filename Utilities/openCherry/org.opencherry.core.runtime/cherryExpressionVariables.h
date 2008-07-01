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

namespace cherry {

struct CHERRY_RUNTIME ExpressionVariable : public Object
{
  cherryClassMacro(ExpressionVariable)
  
  typedef std::deque<std::string> TypeNames;
  typedef std::deque<const std::type_info*> TypeInfos;
  
  struct CHERRY_RUNTIME ExtTypeInfo {

    std::deque<std::string> m_TypeNames;
    std::deque<const std::type_info*> m_TypeInfos;
    
  };
  
  struct CHERRY_RUNTIME ExtTypeInfo_ : public ExtTypeInfo {
    void Add(const std::string& name, const std::type_info& typeinfo);
  };
  
  virtual bool operator==(ExpressionVariable& var) = 0;
  
  virtual bool operator==(ExpressionVariable* var);
  
  virtual int HashCode() = 0;
  
  ExpressionVariable();
  
  const ExtTypeInfo& GetExtTypeInfo() const;
  
  virtual std::string GetClassName() const;
  
  virtual std::string ToString() const;
  
protected:
  ExtTypeInfo_ m_TypeInfo;
  
};

struct CHERRY_RUNTIME StringExpressionVariable : public ExpressionVariable
{
  cherryClassMacro(StringExpressionVariable)
  
  StringExpressionVariable(const std::string& str);
  
  bool operator==(ExpressionVariable& var);
  
  void SetVariable(const std::string& var);
  
  std::string& GetVariable();
  
  int HashCode();
  
private:
  
  std::string m_Var;
};

struct CHERRY_RUNTIME VectorExpressionVariable : public ExpressionVariable
{
  cherryClassMacro(VectorExpressionVariable)
  
  typedef std::vector<ExpressionVariable::Pointer> VectorType;
  
  VectorExpressionVariable();
  
  bool operator==(ExpressionVariable& var);
  
  VectorType& GetVariable();
  
  int HashCode();
  
  
private:
  
  VectorType m_Var;
};

struct CHERRY_RUNTIME BooleanExpressionVariable : public ExpressionVariable
{
  cherryClassMacro(BooleanExpressionVariable)
  
  BooleanExpressionVariable(bool b);
  
  bool operator==(ExpressionVariable& var);
  
  void SetVariable(bool var);
  
  bool& GetVariable();
  
  int HashCode();
 
  
private:
  
  bool m_Var;
};

struct CHERRY_RUNTIME FloatExpressionVariable : public ExpressionVariable
{
  cherryClassMacro(FloatExpressionVariable)
  
  FloatExpressionVariable(double d);
  
  bool operator==(ExpressionVariable& var);
  
  void SetVariable(double var);
  
  int HashCode();
  
  double& GetVariable();
  
  
private:
  
  double m_Var;
};

struct CHERRY_RUNTIME IntegerExpressionVariable : public ExpressionVariable
{
  cherryClassMacro(IntegerExpressionVariable)
  
  IntegerExpressionVariable(int i);
  
  bool operator==(ExpressionVariable& var);
  
  void SetVariable(int var);
  
  int& GetVariable();
  
  int HashCode();
  
  
private:
  
  int m_Var;
};

}

#endif /*CHERRYIEXPRESSIONVARIABLE_H_*/
