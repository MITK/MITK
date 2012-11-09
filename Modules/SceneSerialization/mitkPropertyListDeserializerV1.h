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

#ifndef mitkPropertyListDeserializerV1_h_included
#define mitkPropertyListDeserializerV1_h_included

#include "mitkPropertyListDeserializer.h"

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
      \brief Reads a propertylist from file. Get result via GetOutput()
      \return success of deserialization
      */
    virtual bool Deserialize();

  protected:

    PropertyListDeserializerV1();
    virtual ~PropertyListDeserializerV1();
};

} // namespace

#endif

