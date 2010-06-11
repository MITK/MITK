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

#ifndef mitkPoint3iPropertyDeserializer_h_included
#define mitkPoint3iPropertyDeserializer_h_included

#include "mitkBasePropertyDeserializer.h"
#include "mitkProperties.h"

#include "SceneSerializationBaseExports.h"

namespace mitk
{
class SceneSerializationBase_EXPORT Point3iPropertyDeserializer : public BasePropertyDeserializer
{
  public:
    mitkClassMacro( Point3iPropertyDeserializer, BasePropertyDeserializer );
    itkNewMacro(Self);
    virtual BaseProperty::Pointer Deserialize(TiXmlElement* element)
    {
      if (!element) return NULL;

      Point3I v;
      if ( element->QueryIntAttribute( "x", &v[0] ) != TIXML_SUCCESS ) return NULL;
      if ( element->QueryIntAttribute( "y", &v[1] ) != TIXML_SUCCESS ) return NULL;
      if ( element->QueryIntAttribute( "z", &v[2] ) != TIXML_SUCCESS ) return NULL;
     return Point3iProperty::New( v ).GetPointer();
    }
  protected:
    Point3iPropertyDeserializer() {}
    virtual ~Point3iPropertyDeserializer() {}
};
} // namespace
// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(Point3iPropertyDeserializer);
#endif
