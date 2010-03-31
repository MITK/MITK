/*=========================================================================
 
Program:   Medical Imaging & Enumerationeraction Toolkit
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

#ifndef mitkEnumerationPropertySerializer_h_included
#define mitkEnumerationPropertySerializer_h_included

#include "mitkBasePropertySerializer.h"

#include "mitkEnumerationProperty.h"

namespace mitk
{

class SceneSerialization_EXPORT EnumerationPropertySerializer : public BasePropertySerializer
{
  public:
    
    mitkClassMacro( EnumerationPropertySerializer, BasePropertySerializer );

    virtual TiXmlElement* Serialize()
    {
      if (const EnumerationProperty* prop = dynamic_cast<const EnumerationProperty*>(m_Property.GetPointer()))
      {
        TiXmlElement* element = new TiXmlElement("enum");
        element->SetAttribute("value", prop->GetValueAsString());
        return element;
      }
      else return NULL;
    }

  protected:

    EnumerationPropertySerializer() {}
    virtual ~EnumerationPropertySerializer() {}
};

} // namespace

// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(EnumerationPropertySerializer);

#endif

