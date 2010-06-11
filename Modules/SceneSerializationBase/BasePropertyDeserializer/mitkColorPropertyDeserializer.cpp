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

#ifndef mitkColorPropertyDeserializer_h_included
#define mitkColorPropertyDeserializer_h_included

#include "mitkBasePropertyDeserializer.h"

#include "mitkColorProperty.h"

#include "SceneSerializationBaseExports.h"

namespace mitk
{

class SceneSerializationBase_EXPORT ColorPropertyDeserializer : public BasePropertyDeserializer
{
  public:
    
    mitkClassMacro( ColorPropertyDeserializer, BasePropertyDeserializer );
    itkNewMacro(Self);

    virtual BaseProperty::Pointer Deserialize(TiXmlElement* element)
    {
      if (!element) return NULL;

      Color c;
      if ( element->QueryFloatAttribute( "r", &c[0] ) != TIXML_SUCCESS ) return NULL;
      if ( element->QueryFloatAttribute( "g", &c[1] ) != TIXML_SUCCESS ) return NULL;
      if ( element->QueryFloatAttribute( "b", &c[2] ) != TIXML_SUCCESS ) return NULL;

      return ColorProperty::New( c ).GetPointer();
    }

  protected:

    ColorPropertyDeserializer() {}
    virtual ~ColorPropertyDeserializer() {}
};

} // namespace

// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(ColorPropertyDeserializer);

#endif

