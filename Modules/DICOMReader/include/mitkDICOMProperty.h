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

#ifndef mitkDICOMProperty_h
#define mitkDICOMProperty_h

#include "mitkDICOMImageBlockDescriptor.h"
#include "mitkTemporoSpatialStringProperty.h"
#include "mitkDICOMTagPath.h"

#include "MitkDICOMReaderExports.h"

namespace mitk
{

  typedef TemporoSpatialStringProperty DICOMProperty;

  /** Generation functor for DICOMFileReader classes to convert the collected tag values into DICOMProperty instances. */
  MITKDICOMREADER_EXPORT mitk::BaseProperty::Pointer GetDICOMPropertyForDICOMValuesFunctor(const DICOMCachedValueLookupTable& cacheLookupTable);


  class PropertyList;
  class BaseData;

  /** Helper function that searches for all properties in a given property list that matches the passed path.
   * The result will be the matching properties in a map*/
  MITKDICOMREADER_EXPORT std::map< std::string, BaseProperty::Pointer> GetPropertyByDICOMTagPath(const PropertyList* list, const DICOMTagPath& path);
  /** Helper function that searches for all properties in a given base data that matches the passed path.
  * The result will be the matching properties in a map*/
  MITKDICOMREADER_EXPORT std::map< std::string, BaseProperty::Pointer> GetPropertyByDICOMTagPath(const BaseData* data, const DICOMTagPath& path);

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


}

#endif
