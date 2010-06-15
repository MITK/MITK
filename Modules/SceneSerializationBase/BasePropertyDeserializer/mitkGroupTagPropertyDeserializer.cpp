/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-10-08 17:07:40 +0200 (Do, 08. Okt 2009) $
Version:   $Revision: 1.12 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef mitkGroupTagPropertyDeserializer_h_included
#define mitkGroupTagPropertyDeserializer_h_included

#include "mitkBasePropertyDeserializer.h"

#include "mitkGroupTagProperty.h"

#include "SceneSerializationBaseExports.h"

namespace mitk
{

class SceneSerializationBase_EXPORT GroupTagPropertyDeserializer : public BasePropertyDeserializer
{
  public:
    
    mitkClassMacro( GroupTagPropertyDeserializer, BasePropertyDeserializer );
    itkNewMacro(Self);

    virtual BaseProperty::Pointer Deserialize(TiXmlElement*)
    {
      //if (!element) 
      //  return NULL;
      return GroupTagProperty::New().GetPointer();
    }

  protected:
    GroupTagPropertyDeserializer() {}
    virtual ~GroupTagPropertyDeserializer() {}
};

} // namespace

// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(GroupTagPropertyDeserializer);

#endif

