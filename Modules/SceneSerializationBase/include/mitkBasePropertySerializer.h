/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkBasePropertySerializer_h
#define mitkBasePropertySerializer_h

#include "mitkSerializerMacros.h"
#include <MitkSceneSerializationBaseExports.h>

#include "mitkBaseProperty.h"
#include <itkObjectFactoryBase.h>

namespace tinyxml2
{
  class XMLDocument;
  class XMLElement;
}

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
  class MITKSCENESERIALIZATIONBASE_EXPORT BasePropertySerializer : public itk::Object
  {
  public:
    mitkClassMacroItkParent(BasePropertySerializer, itk::Object);

    itkSetConstObjectMacro(Property, BaseProperty);

    /**
      \brief Serializes given BaseProperty object.
      \return The filename of the newly created file.

      This should be overwritten by specific sub-classes.
    */
    virtual tinyxml2::XMLElement *Serialize(tinyxml2::XMLDocument& doc);

    /**
      \brief Deserializes given XML element.
      \return The deserialized Property.

      This should be overwritten by specific sub-classes.
    */

    virtual BaseProperty::Pointer Deserialize(const tinyxml2::XMLElement*);

  protected:
    BasePropertySerializer();
    ~BasePropertySerializer() override;

    BaseProperty::ConstPointer m_Property;
  };

} // namespace

#endif
