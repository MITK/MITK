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

#ifndef mitkColormapPropertySerializer_h_included
#define mitkColormapPropertySerializer_h_included

#include "mitkBasePropertySerializer.h"

#include "mitkColormapProperty.h"

#include "SceneSerializationBaseExports.h"

namespace mitk
{

class SceneSerializationBase_EXPORT ColormapPropertySerializer : public BasePropertySerializer
{
  public:

    mitkClassMacro( ColormapPropertySerializer, BasePropertySerializer );
    itkNewMacro(Self);

    virtual TiXmlElement* Serialize()
    {
      if (const ColormapProperty* prop = dynamic_cast<const ColormapProperty*>(m_Property.GetPointer()))
      {
        TiXmlElement* element = new TiXmlElement("colormap");
        int value = prop->GetColormap();
        element->SetAttribute("value", value);
        return element;
      }
      else return NULL;
    }

    virtual BaseProperty::Pointer Deserialize(TiXmlElement* element)
    {
      if (!element) return NULL;

      int value;
      if ( element->QueryIntAttribute( "value", &value ) != TIXML_SUCCESS ) return NULL;

      return ColormapProperty::New( value ).GetPointer();
    }

  protected:

    ColormapPropertySerializer() {}
    virtual ~ColormapPropertySerializer() {}
};

} // namespace

// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(ColormapPropertySerializer);

#endif

