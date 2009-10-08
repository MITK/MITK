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

#ifndef mitkPropertyListDeserializerV1_h_included
#define mitkPropertyListDeserializerV1_h_included

#include "SceneSerializationExports.h"

#include "mitkPropertyList.h"

#include <itkObjectFactoryBase.h>

namespace mitk
{

/**
  \brief Deserializes a mitk::PropertyList
*/
class SceneSerialization_EXPORT PropertyListDeserializerV1 : public PropertyListDeserializer
{
  public:
    
    mitkClassMacro( PropertyListDeserializerV1, PropertyListDeserializer );
    itkNewMacro(Self); // is this needed? should never be instantiated, only subclasses should

    /**
      \brief Reads a propertylist from file
      \return success of deserialization
      */
    virtual bool Deserialize();

    virtual PropertyList::Pointer GetOutput();

  protected:

    PropertyListDeserializerV1();
    virtual ~PropertyListDeserializerV1();
};

} // namespace

#endif

