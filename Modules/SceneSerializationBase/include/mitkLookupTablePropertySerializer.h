/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkLookupTablePropertySerializer_h_included
#define mitkLookupTablePropertySerializer_h_included

#include "mitkBasePropertySerializer.h"
#include <MitkSceneSerializationBaseExports.h>

namespace mitk
{
  /**
  \brief Base class for objects that serialize BaseData types.

  The name of sub-classes must be deduced from the class name of the object that should be serialized.
  The serialization assumes that

  \verbatim
  If the class derived from BaseData is called GreenData
  Then the serializer for this class must be called GreenDataSerializer
  \endverbatim
   */
  class MITKSCENESERIALIZATIONBASE_EXPORT LookupTablePropertySerializer : public BasePropertySerializer
  {
  public:
    mitkClassMacro(LookupTablePropertySerializer, BasePropertySerializer)
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    /**
      \brief Serializes given BaseData object.
      \return the filename of the newly created file.

      This should be overwritten by specific sub-classes.
      */
    tinyxml2::XMLElement *Serialize(tinyxml2::XMLDocument& doc) override;

    BaseProperty::Pointer Deserialize(const tinyxml2::XMLElement *element) override;

  protected:
    LookupTablePropertySerializer(){};
    ~LookupTablePropertySerializer() override{};
  };
}
// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(LookupTablePropertySerializer);

#endif
