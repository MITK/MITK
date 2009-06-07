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
#include <Poco/Hash.h>


namespace cherry {

const std::size_t Expression::HASH_CODE_NOT_COMPUTED = 0;
const std::size_t Expression::HASH_FACTOR = 89;
const std::string Expression::ATT_VALUE= "value"; //$NON-NLS-1$

const Expression::Pointer Expression::TRUE_EVAL(new TRUE_EVALExpression());
const Expression::Pointer Expression::FALSE_EVAL(new FALSE_EVALExpression());

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
    const bool equal = (left.IsNull()) ? (right.IsNull()) : (left == right);
    if (!equal)
    {
      return false;
    }
  }

  return true;
}

bool Expression::Equals(std::vector<Object::Pointer>& leftArray,
    std::vector<Object::Pointer>& rightArray)
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
    Object::Pointer left= leftArray[i];
    Object::Pointer right= rightArray[i];
    const bool equal = (left.IsNull()) ? (right.IsNull()) : (left == right);
    if (!equal)
    {
      return false;
    }
  }

  return true;
}

std::size_t
Expression::HashCode(Expression::Pointer object)
{
  return object != 0 ? object->HashCode() : 0;
}

std::size_t
Expression::HashCode(std::vector<Expression::Pointer>& array)
{
  if (array.size() == 0) {
    return 0;
  }
  std::size_t hashCode = Poco::hash("std::vector<Expression::Pointer>");
  for (unsigned int i= 0; i < array.size(); i++) {
    hashCode = hashCode * HASH_FACTOR + HashCode(array[i]);
  }
  return hashCode;
}

std::size_t
Expression::HashCode(std::vector<Object::Pointer>& array)
{
  if (array.size() == 0) {
    return 0;
  }
  int hashCode = Poco::hash("std::vector<Object::Pointer>");
  for (unsigned int i= 0; i < array.size(); i++) {
    hashCode = hashCode + array[i]->HashCode();
  }
  return hashCode;
}

const ExpressionInfo*
Expression::ComputeExpressionInfo() const
{
  ExpressionInfo* result= new ExpressionInfo();
  this->CollectExpressionInfo(result);
  return result;
}

void
Expression::CollectExpressionInfo(ExpressionInfo* info) const
{
  info->AddMisBehavingExpressionType(typeid(this));
}

std::size_t
Expression::ComputeHashCode() const
{
  return reinterpret_cast<std::size_t>(this);
}

std::size_t
Expression::HashCode() const
{
  if (fHashCode != HASH_CODE_NOT_COMPUTED)
    return fHashCode;
  fHashCode= this->ComputeHashCode();
  if (fHashCode == HASH_CODE_NOT_COMPUTED)
    fHashCode++;
  return fHashCode;
}

bool
Expression::operator==(const Object* object) const
{
  if (const Expression* other = dynamic_cast<const Expression*>(object))
    return this->HashCode() == other->HashCode();

  return false;
}

std::string Expression::ToString()
{
  return typeid(this).name();
}

}  // namespace cherry
