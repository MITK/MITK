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

#include "cherryExpression.h"


namespace cherry {

const int Expression::HASH_CODE_NOT_COMPUTED = -1;
const intptr_t Expression::HASH_FACTOR = 89;
const std::string Expression::ATT_VALUE= "value"; //$NON-NLS-1$

const Expression::Pointer Expression::TRUE(new TrueExpression());
const Expression::Pointer Expression::FALSE(new FalseExpression());

Expression::Expression()
{
  fHashCode = HASH_CODE_NOT_COMPUTED;
}

Expression::~Expression()
{
  
}

bool Expression::Equals(std::vector<Expression::Pointer>& leftArray,
    std::vector<Expression::Pointer>& rightArray)
{
  if (leftArray == rightArray)
  {
    return true;
  }

  if (leftArray.size() != rightArray.size())
  {
    return false;
  }

  for (unsigned int i= 0; i < leftArray.size(); ++i)
  {
    Expression::Pointer left= leftArray[i];
    Expression::Pointer right= rightArray[i];
    const bool equal = (left.IsNull()) ? (right.IsNull()) : (*left == *right);
    if (!equal)
    {
      return false;
    }
  }

  return true;
}

bool Expression::Equals(std::vector<ExpressionVariable::Pointer>& leftArray,
    std::vector<ExpressionVariable::Pointer>& rightArray)
{
  if (leftArray == rightArray)
  {
    return true;
  }

  if (leftArray.size() != rightArray.size())
  {
    return false;
  }

  for (unsigned int i= 0; i < leftArray.size(); ++i)
  {
    ExpressionVariable::Pointer left= leftArray[i];
    ExpressionVariable::Pointer right= rightArray[i];
    const bool equal = (left.IsNull()) ? (right.IsNull()) : (*left == *right);
    if (!equal)
    {
      return false;
    }
  }

  return true;
}

int 
Expression::HashCode(Expression* object)
{
  return object != 0 ? object->HashCode() : 0;
}

int
Expression::HashCode(std::vector<Expression::Pointer>& array)
{
  if (array.size() == 0) {
    return 0;
  }
  int hashCode = Poco::Hash<std::string>()("std::vector<Expression::Pointer>");
  for (unsigned int i= 0; i < array.size(); i++) {
    hashCode = hashCode * HASH_FACTOR + HashCode(array[i]);
  }
  return hashCode;
}

int
Expression::HashCode(std::vector<ExpressionVariable::Pointer>& array)
{
  if (array.size() == 0) {
    return 0;
  }
  int hashCode = Poco::Hash<std::string>()("std::vector<ExpressionVariable::Pointer>");
  for (unsigned int i= 0; i < array.size(); i++) {
    hashCode = hashCode + array[i]->HashCode();
  }
  return hashCode;
}

const ExpressionInfo* 
Expression::ComputeExpressionInfo() 
{
  ExpressionInfo* result= new ExpressionInfo();
  this->CollectExpressionInfo(result);
  return result;
}

void 
Expression::CollectExpressionInfo(ExpressionInfo* info) 
{
  info->AddMisBehavingExpressionType(typeid(this));
}

intptr_t
Expression::ComputeHashCode() 
{
  return reinterpret_cast<intptr_t>(this);
}

int 
Expression::HashCode() 
{
  if (fHashCode != HASH_CODE_NOT_COMPUTED)
    return fHashCode;
  fHashCode= this->ComputeHashCode();
  if (fHashCode == HASH_CODE_NOT_COMPUTED)
    fHashCode++;
  return fHashCode;
}

bool 
Expression::operator==(Expression& object)
{
  return this->HashCode() == object.HashCode();
}

bool Expression::operator==(Expression* object)
{
  return this->operator==(*object);
}

std::string Expression::ToString()
{
  return typeid(this).name();
}

}  // namespace cherry
