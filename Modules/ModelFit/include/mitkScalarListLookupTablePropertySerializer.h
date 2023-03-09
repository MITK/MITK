/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkScalarListLookupTablePropertySerializer_h
#define mitkScalarListLookupTablePropertySerializer_h

#include "mitkBasePropertySerializer.h"

#include "mitkBaseProperty.h"

#include "MitkModelFitExports.h"

namespace mitk
{
    /**
     *  @brief  Serializer for the ScalarListLookupTableProperty so it can be written and read from
     *          file.
     */
    class ScalarListLookupTablePropertySerializer : public BasePropertySerializer
    {
    public:
        mitkClassMacro(ScalarListLookupTablePropertySerializer, BasePropertySerializer);
        itkNewMacro(Self);

        tinyxml2::XMLElement* Serialize(tinyxml2::XMLDocument& doc) override;
        BaseProperty::Pointer Deserialize(const tinyxml2::XMLElement* element) override;

    protected:
        ScalarListLookupTablePropertySerializer() {}
        ~ScalarListLookupTablePropertySerializer() override {}
    };

  namespace PropertyPersistenceSerialization
  {
    /** Serialization of a ScalarListLookupTableProperty into a XML string.*/
    MITKMODELFIT_EXPORT ::std::string serializeScalarListLookupTablePropertyToXML(const mitk::BaseProperty *prop);
  }

  namespace PropertyPersistenceDeserialization
  {
    /**Deserialize a passed XML string into a ScalarListLookupTableProperty.*/
    MITKMODELFIT_EXPORT mitk::BaseProperty::Pointer deserializeXMLToScalarListLookupTableProperty(const std::string &value);
  }
}

#endif
