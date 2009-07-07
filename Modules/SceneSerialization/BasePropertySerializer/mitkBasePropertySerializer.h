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

#ifndef mitkSerializeBaseProperty_h_included
#define mitkSerializeBaseProperty_h_included

#include "SceneSerializationExports.h"
#include "mitkSerializerMacros.h"

#include <itkObjectFactoryBase.h>
#include "mitkBaseProperty.h"

#include <tinyxml.h>

namespace mitk
{

/**
  \brief Base class for objects that serialize BaseProperty types.

  The name of sub-classes must be deduced from the class name of the object that should be serialized.
  The serialization assumes that 

  \verbatim
  If the class derived from BaseProperty is called GreenProperty
  Then the serializer for this class must be called GreenPropertySerializer
  \endverbatim
*/
class SceneSerialization_EXPORT BasePropertySerializer : public itk::Object
{
  public:
    
    mitkClassMacro( BasePropertySerializer, itk::Object );

    itkSetConstObjectMacro(Property, BaseProperty);

    /**
      \brief Serializes given BaseProperty object.
      \return the filename of the newly created file.
      
      This should be overwritten by specific sub-classes. 
      */
    virtual TiXmlElement* Serialize();

  protected:

    BasePropertySerializer();
    virtual ~BasePropertySerializer();
    
    BaseProperty::ConstPointer m_Property;
};

} // namespace

#endif

