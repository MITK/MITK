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

#ifndef mitkFloatPropertyDeserializer_h_included
#define mitkFloatPropertyDeserializer_h_included

#include "mitkBasePropertyDeserializer.h"

#include "mitkProperties.h"

#include "SceneSerializationBaseExports.h"

namespace mitk
{

class SceneSerializationBase_EXPORT FloatPropertyDeserializer : public BasePropertyDeserializer
{
  public:
    
    mitkClassMacro( FloatPropertyDeserializer, BasePropertyDeserializer );
    itkNewMacro(Self);

    virtual BaseProperty::Pointer Deserialize(TiXmlElement* element)
    {
      if (!element) return NULL;

      float f;
      if ( element->QueryFloatAttribute( "value", &f ) == TIXML_SUCCESS )
      {
        return FloatProperty::New(f).GetPointer();
      }
      else
      {
        return NULL;
      }
    }

  protected:

    FloatPropertyDeserializer() {}
    virtual ~FloatPropertyDeserializer() {}
};

} // namespace

// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(FloatPropertyDeserializer);

#endif

