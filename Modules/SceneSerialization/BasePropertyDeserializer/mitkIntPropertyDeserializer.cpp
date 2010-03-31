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

#ifndef mitkIntPropertyDeserializer_h_included
#define mitkIntPropertyDeserializer_h_included

#include "mitkBasePropertyDeserializer.h"

#include "mitkProperties.h"

namespace mitk
{

class SceneSerialization_EXPORT IntPropertyDeserializer : public BasePropertyDeserializer
{
  public:
    
    mitkClassMacro( IntPropertyDeserializer, BasePropertyDeserializer );
    itkNewMacro(Self);

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

    IntPropertyDeserializer() {}
    virtual ~IntPropertyDeserializer() {}
};

} // namespace

// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(IntPropertyDeserializer);

#endif

