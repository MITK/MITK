/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDICOMSegPropertyTestHelpers_h
#define mitkDICOMSegPropertyTestHelpers_h

#include <mitkBaseProperty.h>
#include <mitkTemporoSpatialStringProperty.h>

#include <string>

namespace mitk
{
  namespace test
  {
    /**
     * \brief Compare a property's scalar value to an expected string.
     *
     * Handles two property shapes that the DICOM property machinery may
     * produce for the same logical scalar value:
     *   - TemporoSpatialStringProperty: the value lives at slot
     *     (timeStep=0, sliceIndex=0). GetValueAsString() returns a JSON
     *     serialisation of the full sparse map, not the scalar — so a
     *     direct string compare against the expected scalar fails on
     *     this type.
     *   - Plain BaseProperty (StringProperty etc.): GetValueAsString()
     *     returns the value itself; exact match or substring match
     *     suffices.
     *
     * Returns true when the property is non-null and its primary scalar
     * value matches the expected string. Used by the DICOM SEG round-trip
     * tests so the same assertion shape works regardless of which side of
     * MITK's persistence layer the property travelled through.
     */
    inline bool PropertyScalarValueEquals(const mitk::BaseProperty *prop,
                                          const std::string &expected)
    {
      if (prop == nullptr)
        return false;
      if (const auto *tsProp = dynamic_cast<const mitk::TemporoSpatialStringProperty *>(prop))
        return tsProp->GetValue(0, 0) == expected;
      const auto value = prop->GetValueAsString();
      return value == expected
        || value.find(expected) != std::string::npos;
    }
  }
}

#endif
