/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryExpression.h"
#include <Poco/Hash.h>


namespace berry {

const uint Expression::HASH_CODE_NOT_COMPUTED = 0;
const uint Expression::HASH_FACTOR = 89;
const QString Expression::ATT_VALUE= "value";

const Expression::Pointer Expression::TRUE_EVAL(new TRUE_EVALExpression());
const Expression::Pointer Expression::FALSE_EVAL(new FALSE_EVALExpression());

Expression::Expression()
{
  fHashCode = HASH_CODE_NOT_COMPUTED;
}

Expression::~Expression()
{

}

bool Expression::Equals(const QList<Expression::Pointer>& leftArray,
                        const QList<Expression::Pointer>& rightArray)
{
  return (leftArray == rightArray);
}

bool Expression::Equals(const QList<Object::Pointer>& leftArray,
                        const QList<Object::Pointer>& rightArray)
{
  return (leftArray == rightArray);
}

uint
Expression::HashCode(Expression::Pointer object)
{
  return object != 0 ? object->HashCode() : 0;
}

uint
Expression::HashCode(const QList<Expression::Pointer>& array)
{
  if (array.size() == 0)
  {
    return 0;
  }
  uint hashCode = qHash("QList<Expression::Pointer>");
  for (int i= 0; i < array.size(); i++)
  {
    hashCode = hashCode * HASH_FACTOR + HashCode(array[i]);
  }
  return hashCode;
}

uint
Expression::HashCode(const QList<Object::Pointer>& array)
{
  if (array.isEmpty())
  {
    return 0;
  }
  int hashCode = (int) Poco::hash("std::vector<Object::Pointer>");
  for (int i= 0; i < array.size(); i++)
  {
    hashCode = hashCode + (int) array[i]->HashCode();
  }
  return hashCode;
}

const ExpressionInfo*
Expression::ComputeExpressionInfo() const
{
  auto   result= new ExpressionInfo();
  this->CollectExpressionInfo(result);
  return result;
}

void
Expression::CollectExpressionInfo(ExpressionInfo* info) const
{
  info->AddMisBehavingExpressionType(typeid(this));
}

uint
Expression::ComputeHashCode() const
{
  return qHash(this);
}

uint Expression::HashCode() const
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

QString Expression::ToString() const
{
  return typeid(this).name();
}

}  // namespace berry
