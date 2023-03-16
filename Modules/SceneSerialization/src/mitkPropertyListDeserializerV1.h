/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPropertyListDeserializerV1_h
#define mitkPropertyListDeserializerV1_h

#include "mitkPropertyListDeserializer.h"

namespace mitk
{
  /**
    \brief Deserializes a mitk::PropertyList
  */
  class PropertyListDeserializerV1 : public PropertyListDeserializer
  {
  public:
    mitkClassMacro(PropertyListDeserializerV1, PropertyListDeserializer);
    itkFactorylessNewMacro(Self) // is this needed? should never be instantiated, only subclasses should
      itkCloneMacro(Self);

      /**
        \brief Reads a propertylist from file. Get result via GetOutput()
        \return success of deserialization
        */
      bool Deserialize() override;

  protected:
    PropertyListDeserializerV1();
    ~PropertyListDeserializerV1() override;
  };

} // namespace

#endif
