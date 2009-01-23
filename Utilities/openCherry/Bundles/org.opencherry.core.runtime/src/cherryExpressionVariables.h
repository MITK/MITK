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
#include <cherryObject.h>

#include "cherryRuntimeDll.h"

#include <typeinfo>
#include <string>
#include <deque>
#include <vector>

namespace cherry {

struct CHERRY_RUNTIME ExpressionVariable : public Object
{
  cherryObjectMacro(ExpressionVariable)

  typedef std::deque<std::string> TypeNames;
  typedef std::deque<const std::type_info*> TypeInfos;

  struct CHERRY_RUNTIME ExtTypeInfo {

    std::deque<std::string> m_TypeNames;
    std::deque<const std::type_info*> m_TypeInfos;

  };

  struct CHERRY_RUNTIME ExtTypeInfo_ : public ExtTypeInfo {
    void Add(const std::string& name, const std::type_info& typeinfo);
  };

  virtual bool operator==(const Object* var) const = 0;

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
  cherryObjectMacro(StringExpressionVariable)

  StringExpressionVariable(const std::string& str);

  bool operator==(const Object* var) const;

  void SetVariable(const std::string& var);

  std::string& GetVariable();

  int HashCode();

private:

  std::string m_Var;
};

struct CHERRY_RUNTIME VectorExpressionVariable : public ExpressionVariable
{
  cherryObjectMacro(VectorExpressionVariable)

  typedef std::vector<ExpressionVariable::Pointer> VectorType;

  VectorExpressionVariable();

  bool operator==(const Object* var) const;

  VectorType& GetVariable();

  int HashCode();


private:

  VectorType m_Var;
};

struct CHERRY_RUNTIME BooleanExpressionVariable : public ExpressionVariable
{
  cherryObjectMacro(BooleanExpressionVariable)

  BooleanExpressionVariable(bool b);

  bool operator==(const Object* var) const;

  void SetVariable(bool var);

  bool& GetVariable();

  int HashCode();


private:

  bool m_Var;
};

struct CHERRY_RUNTIME FloatExpressionVariable : public ExpressionVariable
{
  cherryObjectMacro(FloatExpressionVariable)

  FloatExpressionVariable(double d);

  bool operator==(const Object* var) const;

  void SetVariable(double var);

  int HashCode();

  double& GetVariable();


private:

  double m_Var;
};

struct CHERRY_RUNTIME IntegerExpressionVariable : public ExpressionVariable
{
  cherryObjectMacro(IntegerExpressionVariable)

  IntegerExpressionVariable(int i);

  bool operator==(const Object* var) const;

  void SetVariable(int var);

  int& GetVariable();

  int HashCode();


private:

  int m_Var;
};

}

#endif /*CHERRYIEXPRESSIONVARIABLE_H_*/
