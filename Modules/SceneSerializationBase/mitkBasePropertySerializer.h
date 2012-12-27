/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef mitkSerializeBaseProperty_h_included
#define mitkSerializeBaseProperty_h_included

#include "SceneSerializationBaseExports.h"
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
class SceneSerializationBase_EXPORT BasePropertySerializer : public itk::Object
{
  public:

    mitkClassMacro( BasePropertySerializer, itk::Object );

    itkSetConstObjectMacro(Property, BaseProperty);

    /**
      \brief Serializes given BaseProperty object.
      \return The filename of the newly created file.

      This should be overwritten by specific sub-classes.
    */
    virtual TiXmlElement* Serialize();

    /**
      \brief Deserializes given TiXmlElement.
      \return The deserialized Property.

      This should be overwritten by specific sub-classes.
    */

    virtual BaseProperty::Pointer Deserialize(TiXmlElement*);

  protected:

    BasePropertySerializer();
    virtual ~BasePropertySerializer();

    BaseProperty::ConstPointer m_Property;
};

} // namespace

#endif

