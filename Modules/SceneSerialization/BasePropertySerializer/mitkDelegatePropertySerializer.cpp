/*=========================================================================
 
Program:   Medical Imaging & Stringeraction Toolkit
Module:    $RCSfile: mitkPropertyManager.cpp,v $
Language:  C++
Date:      $Date: 2009-07-07 15:56:37 +0200 (Di, 07. Jul 2009) $
Version:   $Revision: 1.12 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef mitkDelegatePropertySerializer_h_included
#define mitkDelegatePropertySerializer_h_included

#include "mitkBasePropertySerializer.h"

#include "mitkDelegateProperty.h"

namespace mitk
{

class SceneSerialization_EXPORT DelegatePropertySerializer : public BasePropertySerializer
{
  public:
    
    mitkClassMacro( DelegatePropertySerializer, BasePropertySerializer );
    itkNewMacro(Self);

    virtual TiXmlElement* Serialize()
    {
      if (const DelegateProperty* prop = dynamic_cast<const DelegateProperty*>(m_Property.GetPointer()))
      {
        TiXmlElement* element = new TiXmlElement("string");
        element->SetAttribute("value", prop->GetValue());
        return element;
      }
      else return NULL;
    }
  protected:
    DelegatePropertySerializer() {}
    virtual ~DelegatePropertySerializer() {}
};
} // namespace
// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(DelegatePropertySerializer);
#endif
