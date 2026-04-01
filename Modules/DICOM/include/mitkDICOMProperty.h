/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDICOMProperty_h
#define mitkDICOMProperty_h

#include <mitkDICOMImageBlockDescriptor.h>
#include <mitkTemporoSpatialStringProperty.h>
#include <mitkDICOMTagPath.h>

#include <MitkDICOMExports.h>

namespace mitk
{

  /**
   * \ingroup DICOMModule
   * \brief Type alias for DICOM property values, stored as TemporoSpatialStringProperty.
   * \sa TemporoSpatialStringProperty
   */
  typedef TemporoSpatialStringProperty DICOMProperty;

  /**
   * \brief Functor to convert collected DICOM tag values into DICOMProperty instances.
   *
   * Generation functor for DICOMFileReader classes. Given a lookup table of per-slice tag values,
   * creates a single property: either a StringProperty (if all values are identical) or a
   * TemporoSpatialStringProperty.
   *
   * \param[in] cacheLookupTable The lookup table mapping slice indices to cached DICOM values.
   * \return A smart pointer to the created BaseProperty.
   * \sa DICOMCachedValueLookupTable, DICOMImageBlockDescriptor::SetTagLookupTableToPropertyFunctor
   */
  MITKDICOM_EXPORT mitk::BaseProperty::Pointer GetDICOMPropertyForDICOMValuesFunctor(const DICOMCachedValueLookupTable& cacheLookupTable);


  class PropertyList;
  class BaseData;

  /**
   * \brief Search a PropertyList for properties matching a DICOM tag path.
   *
   * \param[in] list The PropertyList to search.
   * \param[in] path The DICOMTagPath to match against property names.
   * \return A map of matching property names to BaseProperty smart pointers.
   * \sa DICOMTagPathToPropertyRegEx
   */
  MITKDICOM_EXPORT std::map< std::string, BaseProperty::Pointer> GetPropertyByDICOMTagPath(const PropertyList* list, const DICOMTagPath& path);

  /**
   * \brief Search a BaseData's properties for those matching a DICOM tag path.
   *
   * \param[in] data The BaseData whose property list is searched.
   * \param[in] path The DICOMTagPath to match against property names.
   * \return A map of matching property names to BaseProperty smart pointers.
   * \sa DICOMTagPathToPropertyRegEx
   */
  MITKDICOM_EXPORT std::map< std::string, BaseProperty::Pointer> GetPropertyByDICOMTagPath(const BaseData* data, const DICOMTagPath& path);

  /**
   * \brief Convert a DICOM property string to a numeric value.
   *
   * Helper function that converts the content of a DICOM property string
   * into the given return type. The function makes the following assumptions:
   * 1. dcmValueString encodes exactly one number.
   * 2. The value is encoded compliant to locale "C".
   *
   * \tparam TNumericReturnType The numeric type to convert to (e.g. double, int).
   * \param[in] dcmValueString The DICOM value string to convert.
   * \return The converted numeric value.
   * \throw mitk::Exception if dcmValueString cannot be converted to the return type.
   * \pre dcmValueString must be convertible into the return type.
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

  /**
   * \brief Convert a numeric value to a DICOM property string.
   *
   * Helper function that converts a numeric value into a string representation
   * suitable for storing in a DICOM property, using locale "C".
   *
   * \tparam TNumericType The numeric type to convert from (e.g. double, int).
   * \param[in] value The numeric value to convert.
   * \return The string representation of the value.
   * \throw mitk::Exception if the value cannot be converted to a string.
   * \pre value must be convertible to a string.
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
