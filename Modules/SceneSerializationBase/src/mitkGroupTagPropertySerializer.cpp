/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkGroupTagPropertySerializer_h_included
#define mitkGroupTagPropertySerializer_h_included

#include "mitkBasePropertySerializer.h"
#include "mitkGroupTagProperty.h"
#include <tinyxml2.h>

namespace mitk
{
  class GroupTagPropertySerializer : public BasePropertySerializer
  {
  public:
    mitkClassMacro(GroupTagPropertySerializer, BasePropertySerializer)
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    tinyxml2::XMLElement* Serialize(tinyxml2::XMLDocument& doc) override
    {
      if (dynamic_cast<const GroupTagProperty *>(m_Property.GetPointer()) != nullptr)
      {
        auto *element = doc.NewElement("GroupTag");
        return element;
      }
      else
        return nullptr;
    }

    BaseProperty::Pointer Deserialize(const tinyxml2::XMLElement *) override
    {
      return GroupTagProperty::New().GetPointer();
    }

  protected:
    GroupTagPropertySerializer() {}
    ~GroupTagPropertySerializer() override {}
  };

} // namespace

// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(GroupTagPropertySerializer);

#endif
