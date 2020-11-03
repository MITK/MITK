/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDICOMProperty_h
#define mitkDICOMProperty_h

#include "mitkDICOMImageBlockDescriptor.h"
#include "mitkTemporoSpatialStringProperty.h"
#include "mitkDICOMTagPath.h"

#include "MitkDICOMExports.h"

namespace mitk
{

  typedef TemporoSpatialStringProperty DICOMProperty;

  /** Generation functor for DICOMFileReader classes to convert the collected tag values into DICOMProperty instances. */
  MITKDICOM_EXPORT mitk::BaseProperty::Pointer GetDICOMPropertyForDICOMValuesFunctor(const DICOMCachedValueLookupTable& cacheLookupTable);


  class PropertyList;
  class BaseData;

  /** Helper function that searches for all properties in a given property list that matches the passed path.
   * The result will be the matching properties in a map*/
  MITKDICOM_EXPORT std::map< std::string, BaseProperty::Pointer> GetPropertyByDICOMTagPath(const PropertyList* list, const DICOMTagPath& path);
  /** Helper function that searches for all properties in a given base data that matches the passed path.
  * The result will be the matching properties in a map*/
  MITKDICOM_EXPORT std::map< std::string, BaseProperty::Pointer> GetPropertyByDICOMTagPath(const BaseData* data, const DICOMTagPath& path);

  /**Helper function that can be used to convert the content of a DICOM property
     into the given return type. The function makes the following assumptions:
     1. dcmValueString does only encode one number.
     2. The value is encoded compliant to locale "C".
     @pre dcmValueString must be convertibel into the return type.
     If this is not the case an exception will be thrown.
     */
  template<typename TNumericReturnType>
  TNumericReturnType ConvertDICOMStrToValue(const std::string& dcmValueString)
  {
    std::istringstream iss(dcmValueString);
    iss.imbue(std::locale("C"));
    TNumericReturnType d;
    if (!(iss >> d) || !(iss.eof()))
    {
      mitkThrow() << "Cannot convert string to value type. Type: " << typeid(TNumericReturnType).name() << "; String: " << dcmValueString;
    }

    return d;
  };

  /**Helper function that can be used to convert a numeric value into content of a DICOM property.
     @pre value must be convertibel into a string.
     If this is not the case an exception will be thrown.
     */
  template<typename TNumericType>
  std::string ConvertValueToDICOMStr(const TNumericType value)
  {
    std::ostringstream oss;
    oss.imbue(std::locale("C"));
    if (!(oss << value))
    {
      mitkThrow() << "Cannot convert value type to dicom string. Type: " << typeid(TNumericType).name() << "; value: " << value;
    }

    return oss.str();
  };


}

#endif
