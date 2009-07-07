/*=========================================================================
 
Program:   Medical Imaging & Stringeraction Toolkit
Module:    $RCSfile: mitkPropertyManager.cpp,v $
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

#ifndef mitkStringPropertySerializer_h_included
#define mitkStringPropertySerializer_h_included

#include "mitkBasePropertySerializer.h"

#include "mitkStringProperty.h"

namespace mitk
{

class SceneSerialization_EXPORT StringPropertySerializer : public BasePropertySerializer
{
  public:
    
    mitkClassMacro( StringPropertySerializer, BasePropertySerializer );
    itkNewMacro(Self);

    virtual TiXmlElement* Serialize()
    {
      if (const StringProperty* prop = dynamic_cast<const StringProperty*>(m_Property.GetPointer()))
      {
        TiXmlElement* element = new TiXmlElement("string");
        element->SetAttribute("value", prop->GetValue());
        return element;
      }
      else return NULL;
    }

  protected:

    StringPropertySerializer() {}
    virtual ~StringPropertySerializer() {}
};

} // namespace

// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(StringPropertySerializer);

#endif

