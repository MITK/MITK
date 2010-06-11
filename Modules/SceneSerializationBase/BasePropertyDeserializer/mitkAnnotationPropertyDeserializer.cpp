/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-10-08 16:58:56 +0200 (Do, 08. Okt 2009) $
Version:   $Revision: 1.12 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef mitkAnnotationPropertyDeserializer_h_included
#define mitkAnnotationPropertyDeserializer_h_included

#include "mitkBasePropertyDeserializer.h"

#include "mitkAnnotationProperty.h"

#include "SceneSerializationBaseExports.h"

namespace mitk
{

class SceneSerializationBase_EXPORT AnnotationPropertyDeserializer : public BasePropertyDeserializer
{
  public:
    mitkClassMacro( AnnotationPropertyDeserializer, BasePropertyDeserializer );
    itkNewMacro(Self);

    virtual BaseProperty::Pointer Deserialize(TiXmlElement* element)
    {
      if (!element) return NULL;
      const char* label( element->Attribute("label") );
      Point3D p;
      if ( element->QueryFloatAttribute( "x", &p[0] ) != TIXML_SUCCESS ) 
        return NULL;
      if ( element->QueryFloatAttribute( "y", &p[1] ) != TIXML_SUCCESS ) 
        return NULL;
      if ( element->QueryFloatAttribute( "z", &p[2] ) != TIXML_SUCCESS ) 
        return NULL;
      return AnnotationProperty::New(label, p).GetPointer();
    }
  protected:
    AnnotationPropertyDeserializer() {}
    virtual ~AnnotationPropertyDeserializer() {}
};
} // namespace
// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(AnnotationPropertyDeserializer);
#endif
