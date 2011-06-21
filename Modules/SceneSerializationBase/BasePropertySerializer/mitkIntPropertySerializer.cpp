/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 1.12 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef mitkIntPropertySerializer_h_included
#define mitkIntPropertySerializer_h_included

#include "mitkBasePropertySerializer.h"

#include "mitkProperties.h"

#include "SceneSerializationBaseExports.h"

namespace mitk
{

class SceneSerializationBase_EXPORT IntPropertySerializer : public BasePropertySerializer
{
  public:
    
    mitkClassMacro( IntPropertySerializer, BasePropertySerializer );
    itkNewMacro(Self);

    virtual TiXmlElement* Serialize()
    {
      if (const IntProperty* prop = dynamic_cast<const IntProperty*>(m_Property.GetPointer()))
      {
        TiXmlElement* element = new TiXmlElement("int");
        element->SetAttribute("value", prop->GetValue());
        return element;
      }
      else return NULL;
    }

    virtual BaseProperty::Pointer Deserialize(TiXmlElement* element)
    {
      if (!element) return NULL;

      int integer;
      if ( element->QueryIntAttribute( "value", &integer ) == TIXML_SUCCESS )
      {
        return IntProperty::New(integer).GetPointer();
      }
      else
      {
        return NULL;
      }
    }

  protected:

    IntPropertySerializer() {}
    virtual ~IntPropertySerializer() {}
};

} // namespace

// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(IntPropertySerializer);

#endif

