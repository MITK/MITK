/*=========================================================================
 
Program:   Medical Imaging & Stringeraction Toolkit
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

#ifndef mitkStringPropertyDeserializer_h_included
#define mitkStringPropertyDeserializer_h_included

#include "mitkBasePropertyDeserializer.h"

#include "mitkStringProperty.h"

namespace mitk
{

class SceneSerialization_EXPORT StringPropertyDeserializer : public BasePropertyDeserializer
{
  public:
    
    mitkClassMacro( StringPropertyDeserializer, BasePropertyDeserializer );
    itkNewMacro(Self);

    virtual BaseProperty::Pointer Deserialize(TiXmlElement* element)
    {
      if (!element) return NULL;
      const char* s( element->Attribute("value") );
      return StringProperty::New( std::string(s?s:"") ).GetPointer();
    }

  protected:

    StringPropertyDeserializer() {}
    virtual ~StringPropertyDeserializer() {}
};

} // namespace

// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(StringPropertyDeserializer);

#endif

