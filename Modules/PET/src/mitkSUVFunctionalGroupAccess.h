/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSUVFunctionalGroupAccess_h
#define mitkSUVFunctionalGroupAccess_h

#include <mitkBaseProperty.h>
#include <mitkDICOMProperty.h>
#include <mitkDICOMTagPath.h>
#include <mitkIPropertyProvider.h>

#include <string>

namespace mitk::SUVFunctionalGroupAccess
{
  /** Attribute of a single-item functional-group macro, addressed the way the
   *  reader publishes it: relative to the functional-group item, with the
   *  macro's one item named explicitly. A lookup answers from its first match,
   *  so a wildcard item would only be safe by luck. */
  inline DICOMTagPath MacroAttribute(unsigned int macroGroup, unsigned int macroElement,
                                     unsigned int group, unsigned int element)
  {
    DICOMTagPath path;
    path.AddSelection(macroGroup, macroElement, 0);
    return path.AddElement(group, element);
  }

  /** Resolve a path once, so a caller iterating slots does not repeat the
   *  property-key scan per slot. Null when nothing matches. */
  inline BaseProperty::ConstPointer FirstMatch(const IPropertyProvider* provider, const DICOMTagPath& path)
  {
    const auto matches = GetPropertyByDICOMTagPath(provider, path);
    return matches.empty() ? BaseProperty::ConstPointer() : matches.begin()->second;
  }

  /** The value of \p property at one slot, without a close-match fallback, so
   *  an absent slot yields an empty string rather than a neighbouring frame's
   *  value. A property that is not slice-resolved carries one value for the
   *  whole image and answers with it at every slot. A null property yields an
   *  empty string. */
  inline std::string ValueAt(const BaseProperty* property, TimeStepType t, DICOMProperty::IndexValueType z)
  {
    if (nullptr == property)
    {
      return {};
    }
    const auto* sliced = dynamic_cast<const DICOMProperty*>(property);
    if (nullptr != sliced)
    {
      return sliced->GetValue(t, z, false, false);
    }
    return property->GetValueAsString();
  }
}

#endif
