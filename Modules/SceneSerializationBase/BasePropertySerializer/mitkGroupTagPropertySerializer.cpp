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

#include "SceneSerializationBaseExports.h"

namespace mitk
{

class SceneSerializationBase_EXPORT GroupTagPropertySerializer : public BasePropertySerializer
{
  public:

    mitkClassMacro( GroupTagPropertySerializer, BasePropertySerializer );
    itkNewMacro(Self);

    virtual TiXmlElement* Serialize()
    {
      if (dynamic_cast<const GroupTagProperty*>(m_Property.GetPointer()) != NULL)
      {
        TiXmlElement* element = new TiXmlElement("GroupTag");
        return element;
      }
      else return NULL;
    }

    virtual BaseProperty::Pointer Deserialize(TiXmlElement*)
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

