/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
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

#ifndef mitkBasePropertyDeserializer_h_included
#define mitkBasePropertyDeserializer_h_included

#include "SceneSerializationExports.h"
#include "mitkSerializerMacros.h"

#include <itkObjectFactoryBase.h>
#include "mitkBaseProperty.h"

#include <tinyxml.h>

namespace mitk
{

/**
  \brief Base class for objects that deserialize BaseProperty types.

  The name of sub-classes must be deduced from the class name of the object that should be constructed from its serialization.
  The serialization assumes that 

  \verbatim
  IF the class derived from BaseProperty is called GreenProperty
  THEN the deserializer for this class must be called GreenPropertyDeserializer
  \endverbatim
*/
class SceneSerialization_EXPORT BasePropertyDeserializer : public itk::Object
{
  public:
    
    mitkClassMacro( BasePropertyDeserializer, itk::Object );

    virtual BaseProperty::Pointer Deserialize(TiXmlElement*);

  protected:

    BasePropertyDeserializer();
    virtual ~BasePropertyDeserializer();
};

} // namespace

#endif

