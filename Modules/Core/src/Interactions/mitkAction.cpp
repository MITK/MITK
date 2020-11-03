/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkAction.h"

namespace mitk
{
  /**
   * Default Constructor
   */
  Action::Action(int actionId) : m_ActionId(actionId), m_PropertiesList(nullptr) {}
  Action::~Action() {}
  /**
   * add a property
   */
  void Action::AddProperty(const char *propertyKey, BaseProperty *property)
  {
    if (m_PropertiesList.IsNull())
      m_PropertiesList = PropertyList::New();
    m_PropertiesList->SetProperty(propertyKey, property);
  }

  /**
   * return the actionId of this object
   */
  int Action::GetActionId() const { return m_ActionId; }
  /**
   * return the property with the given property key
   */
  mitk::BaseProperty *Action::GetProperty(const char *propertyKey) const
  {
    if (m_PropertiesList.IsNotNull())
      return m_PropertiesList->GetProperty(propertyKey);
    else
      return nullptr;
  }

} // namespace mitk
