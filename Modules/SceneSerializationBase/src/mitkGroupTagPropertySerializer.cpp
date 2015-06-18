/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef mitkGroupTagPropertySerializer_h_included
#define mitkGroupTagPropertySerializer_h_included

#include "mitkBasePropertySerializer.h"

#include "mitkGroupTagProperty.h"

namespace mitk
{

class GroupTagPropertySerializer : public BasePropertySerializer
{
  public:

    mitkClassMacro( GroupTagPropertySerializer, BasePropertySerializer );
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    virtual TiXmlElement* Serialize() override
    {
      if (dynamic_cast<const GroupTagProperty*>(m_Property.GetPointer()) != nullptr)
      {
        auto  element = new TiXmlElement("GroupTag");
        return element;
      }
      else return nullptr;
    }

    virtual BaseProperty::Pointer Deserialize(TiXmlElement*) override
    {
      //if (!element)
      //  return NULL;
      return GroupTagProperty::New().GetPointer();
    }

  protected:

    GroupTagPropertySerializer() {}
    virtual ~GroupTagPropertySerializer() {}
};

} // namespace

// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(GroupTagPropertySerializer);

#endif

