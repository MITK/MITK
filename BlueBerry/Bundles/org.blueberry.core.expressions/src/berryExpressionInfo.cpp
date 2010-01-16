/*=========================================================================

Program:   BlueBerry Platform
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

#include "berryExpressionInfo.h"

#include <algorithm>

namespace berry {

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

std::set<std::string>
ExpressionInfo::GetAccessedVariableNames() const
{
  return fAccessedVariableNames;
}

void
ExpressionInfo::AddVariableNameAccess(const std::string& name)
{
    fAccessedVariableNames.insert(name);
}

std::set<std::string>
ExpressionInfo::GetAccessedPropertyNames() const
{
  return fAccessedPropertyNames;
}

void
ExpressionInfo::AddAccessedPropertyName(const std::string& name)
{
  fAccessedPropertyNames.insert(name);
}

std::set<std::string>
ExpressionInfo::GetMisbehavingExpressionTypes() const
{
  return fMisbehavingExpressionTypes;
}

void
ExpressionInfo::AddMisBehavingExpressionType(const std::type_info& clazz)
{
  fMisbehavingExpressionTypes.insert(clazz.name());
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
    for (std::set<std::string>::iterator iter = other->fAccessedVariableNames.begin(); iter != other->fAccessedVariableNames.end(); ++iter)
    {
      fAccessedVariableNames.insert(*iter);
    }
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
    for (std::set<std::string>::iterator iter = other->fAccessedPropertyNames.begin(); iter != other->fAccessedPropertyNames.end(); ++iter)
    {
      fAccessedPropertyNames.insert(*iter);
    }
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
    for (std::set<std::string>::iterator iter= other->fMisbehavingExpressionTypes.begin(); iter != other->fMisbehavingExpressionTypes.end(); ++iter)
    {
      fMisbehavingExpressionTypes.insert(*iter);
    }
  }
}

}  // namespace berry
