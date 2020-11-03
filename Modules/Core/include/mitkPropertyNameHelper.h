/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPropertyNameHelper_h
#define mitkPropertyNameHelper_h

#include <mitkPropertyList.h>
#include <string>

#include "MitkCoreExports.h"
namespace mitk
{
  /** returns the correct property name for a simple DICOM tag. The tag is defined
   * by the passed group and element number (both in hex). */
  std::string MITKCORE_EXPORT GeneratePropertyNameForDICOMTag(unsigned int group, unsigned int element);

  /** Simple helper function that can be used on most occasions where deprecated naming styles
   * are used and it should be switched to the standardized naming (see GeneratePropertyNameForDICOMTag()),
   * but keeping backwards compatibility.
   * @remark It assumes that the needed property value is a string.
   * @remark Only use this function, if you want/need to keep backwards compatibility. In other cases you should
   * use e.g. GeneratePropertyNameForDICOMTag() directly
   *@param group searched DICOM group number as hex integer.
   *@param element searched DICOM element number as hex integer.
   *@param backwardsCompatiblePropertyName Old string that was used for the property before.
   *@param propertyList List of properties that should be searched.
   *@param propertyValue [out] Value of the found property. Only valid if function returns true.
   *@return indicating if the property was found and the variable propertyValue contains a valid value.*/

  bool MITKCORE_EXPORT GetBackwardsCompatibleDICOMProperty(unsigned int group,
                                                           unsigned int element,
                                                           std::string const &backwardsCompatiblePropertyName,
                                                           PropertyList const *propertyList,
                                                           std::string &propertyValue);
}

#endif
