/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "berryExpressionInfo.h"

#include <QString>

namespace berry {

ExpressionInfo::ExpressionInfo()
  : fHasDefaultVariableAccess(false)
  , fHasSystemPropertyAccess(false)
{
}

bool
ExpressionInfo::HasDefaultVariableAccess() const
{
    return fHasDefaultVariableAccess;
  }

void
ExpressionInfo::MarkDefaultVariableAccessed()
{
    fHasDefaultVariableAccess= true;
  }

bool
ExpressionInfo::HasSystemPropertyAccess() const
{
    return fHasSystemPropertyAccess;
  }

void
ExpressionInfo::MarkSystemPropertyAccessed()
{
    fHasSystemPropertyAccess= true;
  }

QSet<QString> ExpressionInfo::GetAccessedVariableNames() const
{
  return fAccessedVariableNames;
}

void
ExpressionInfo::AddVariableNameAccess(const QString &name)
{
    fAccessedVariableNames.insert(name);
}

QSet<QString>
ExpressionInfo::GetAccessedPropertyNames() const
{
  return fAccessedPropertyNames;
}

void
ExpressionInfo::AddAccessedPropertyName(const QString &name)
{
  fAccessedPropertyNames.insert(name);
}

QSet<QString> ExpressionInfo::GetMisbehavingExpressionTypes() const
{
  return fMisbehavingExpressionTypes;
}

void
ExpressionInfo::AddMisBehavingExpressionType(const std::type_info& clazz)
{
  fMisbehavingExpressionTypes.insert(QString(clazz.name()));
}

void
ExpressionInfo::Merge(ExpressionInfo* other)
{
  this->MergeDefaultVariableAccess(other);
  this->MergeSystemPropertyAccess(other);

  this->MergeAccessedVariableNames(other);
  this->MergeAccessedPropertyNames(other);
  this->MergeMisbehavingExpressionTypes(other);
}

void
ExpressionInfo::MergeExceptDefaultVariable(ExpressionInfo* other)
{
  this->MergeSystemPropertyAccess(other);

  this->MergeAccessedVariableNames(other);
  this->MergeAccessedPropertyNames(other);
  this->MergeMisbehavingExpressionTypes(other);
}

void
ExpressionInfo::MergeDefaultVariableAccess(ExpressionInfo* other)
{
  fHasDefaultVariableAccess= fHasDefaultVariableAccess || other->fHasDefaultVariableAccess;
}

void
ExpressionInfo::MergeSystemPropertyAccess(ExpressionInfo* other)
{
  fHasSystemPropertyAccess= fHasSystemPropertyAccess || other->fHasSystemPropertyAccess;
}

void
ExpressionInfo::MergeAccessedVariableNames(ExpressionInfo* other)
{
  if (fAccessedVariableNames.size() == 0) {
    fAccessedVariableNames= other->fAccessedVariableNames;
  }
  else
  {
    fAccessedVariableNames.unite(other->fAccessedVariableNames);
  }
}

void
ExpressionInfo::MergeAccessedPropertyNames(ExpressionInfo* other)
{
  if (fAccessedPropertyNames.size() == 0)
  {
    fAccessedPropertyNames = other->fAccessedPropertyNames;
  }
  else
  {
    fAccessedPropertyNames.unite(other->fAccessedPropertyNames);
  }
}

void
ExpressionInfo::MergeMisbehavingExpressionTypes(ExpressionInfo* other)
{
  if (fMisbehavingExpressionTypes.size() == 0)
  {
    fMisbehavingExpressionTypes= other->fMisbehavingExpressionTypes;
  }
  else
  {
    fMisbehavingExpressionTypes.unite(other->fMisbehavingExpressionTypes);
  }
}

}  // namespace berry
