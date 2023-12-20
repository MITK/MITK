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

    itk::SmartPointer<BaseProperty> CreateInstance(const std::string& className) override;

  protected:
    void InternalRegisterProperty(const BaseProperty* property) override;

  private:
    using MapType = std::map<std::string, BaseProperty::ConstPointer>;
    MapType m_Map;
  };
}

#endif
