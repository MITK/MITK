/*=========================================================================
 
Program:   Medical Imaging & TransferFunctioneraction Toolkit
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

#ifndef mitkTransferFunctionPropertyDeserializer_h_included
#define mitkTransferFunctionPropertyDeserializer_h_included

#include "mitkBasePropertyDeserializer.h"

#include "mitkTransferFunctionProperty.h"

namespace mitk
{

class SceneSerialization_EXPORT TransferFunctionPropertyDeserializer : public BasePropertyDeserializer
{
  public:
    
    mitkClassMacro( TransferFunctionPropertyDeserializer, BasePropertyDeserializer );
    itkNewMacro(Self);

    virtual BaseProperty::Pointer Deserialize(TiXmlElement* element)
    {
      if (!element) return NULL;
      
      return NULL;
    }

  protected:

    TransferFunctionPropertyDeserializer() {}
    virtual ~TransferFunctionPropertyDeserializer() {}
};

} // namespace

// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(TransferFunctionPropertyDeserializer);

#endif

