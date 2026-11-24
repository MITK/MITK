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

#include <string>

namespace mitk
{
  namespace test
  {
    /**
     * \brief Compare a property's value, as GetValueAsString() gives it, to an expected string,
     *        exactly.
     *
     * For a TemporoSpatialStringProperty that is its first stored value, which is also how
     * SourceImageRelationRule reads the purpose of a relation. Returns false for a null property.
     * Used by the DICOM SEG round-trip tests so the same assertion shape works regardless of
     * which side of MITK's persistence layer the property travelled through.
     */
    inline bool PropertyScalarValueEquals(const mitk::BaseProperty *prop,
                                          const std::string &expected)
    {
      return prop != nullptr && prop->GetValueAsString() == expected;
    }
  }
}

#endif
