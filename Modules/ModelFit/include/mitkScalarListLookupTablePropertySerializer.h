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

        virtual TiXmlElement* Serialize();
        virtual BaseProperty::Pointer Deserialize(TiXmlElement* element);

    protected:
        ScalarListLookupTablePropertySerializer() {}
        virtual ~ScalarListLookupTablePropertySerializer() {}
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

#endif // mitkScalarListLookupTablePropertySerializer_h
