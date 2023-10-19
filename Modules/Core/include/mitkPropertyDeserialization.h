/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPropertyDeserialization_h
#define mitkPropertyDeserialization_h

#include <mitkIPropertyDeserialization.h>
#include <map>

namespace mitk
{
  class PropertyDeserialization : public IPropertyDeserialization
  {
  public:
    PropertyDeserialization();
    virtual ~PropertyDeserialization();

    PropertyDeserialization(const PropertyDeserialization&) = delete;
    PropertyDeserialization& operator=(const PropertyDeserialization&) = delete;

    void RegisterProperty(const BaseProperty* property) override;
    itk::SmartPointer<BaseProperty> CreateInstance(const std::string& className) override;

  private:
    using MapType = std::map<std::string, itk::SmartPointer<BaseProperty>>;
    MapType m_Map;
  };
}

#endif
