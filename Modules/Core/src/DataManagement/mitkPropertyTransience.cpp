/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkPropertyTransience.h>

#include <utility>

mitk::PropertyTransience::PropertyTransience()
{
}

mitk::PropertyTransience::~PropertyTransience()
{
}

void mitk::PropertyTransience::InternalAddTransient(const std::string &propertyName,
                                                    std::function<bool(const BaseData *)> dataTypeMatcher)
{
  m_Matchers.emplace_back(propertyName, std::move(dataTypeMatcher));
}

bool mitk::PropertyTransience::IsTransient(const BaseData *data, const std::string &propertyName) const
{
  for (const auto &matcher : m_Matchers)
  {
    if (matcher.first == propertyName && matcher.second(data))
      return true;
  }

  return false;
}

mitk::IPropertyTransience *mitk::CreateTestInstancePropertyTransience()
{
  return new PropertyTransience();
}
