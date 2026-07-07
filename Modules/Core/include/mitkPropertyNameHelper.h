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

#include <MitkCoreExports.h>
namespace mitk
{
  /** \brief Returns the correct property name for a simple DICOM tag.
   *
   * The tag is defined by the passed group and element number (both in hex).
   * The resulting name has the format "DICOM.GGGG.EEEE".
   *
   * \param group the DICOM group number as a hex integer.
   * \param element the DICOM element number as a hex integer.
   * \return The standardized property name string for the DICOM tag.
   *
   * \sa GetDICOMPropertyValue
   */
  std::string MITKCORE_EXPORT GeneratePropertyNameForDICOMTag(unsigned int group, unsigned int element);

  /** \brief Retrieve a DICOM tag's value as a string from a property list.
   *
   * Looks up the property named by GeneratePropertyNameForDICOMTag() and
   * returns its value via the generic BaseProperty::GetValueAsString().
   * Unlike PropertyList::GetStringProperty(), this resolves any property
   * type, including the TemporoSpatialStringProperty that DICOM-tag
   * properties use - GetStringProperty() only matches a plain StringProperty
   * and would silently miss those.
   *
   * \remark For a TemporoSpatialStringProperty the returned value is the one
   * at the first time step / slice. Use this helper only for DICOM tags that
   * are assumed uniform across all slices and time steps. If you need to keep
   * backwards compatibility with the old (pre-standardized) property naming
   * style, use GetBackwardsCompatibleDICOMPropertyValue() instead.
   *
   * \param group searched DICOM group number as hex integer.
   * \param element searched DICOM element number as hex integer.
   * \param propertyList list of properties that should be searched.
   * \param propertyValue [out] value of the found property. Empty when the
   * function returns false.
   * \return True if the property was found and carries a non-empty value.
   *
   * \sa GeneratePropertyNameForDICOMTag, GetBackwardsCompatibleDICOMPropertyValue
   */
  bool MITKCORE_EXPORT GetDICOMPropertyValue(unsigned int group,
                                             unsigned int element,
                                             PropertyList const *propertyList,
                                             std::string &propertyValue);

  /** \brief Retrieve a DICOM property value as a string, with backwards-compatible naming.
   *
   * Like GetDICOMPropertyValue(), but if the standardized property (see
   * GeneratePropertyNameForDICOMTag()) is absent, it falls back to a
   * deprecated/old property name. Use this only where backwards compatibility
   * with the old naming style is needed; otherwise prefer GetDICOMPropertyValue().
   *
   * \note Renamed from GetBackwardsCompatibleDICOMProperty (the old name
   * misleadingly suggested it returned a property object rather than a value).
   * This is a breaking API change; downstream call sites must be updated.
   *
   * \param group searched DICOM group number as hex integer.
   * \param element searched DICOM element number as hex integer.
   * \param backwardsCompatiblePropertyName old string that was used for the property before.
   * \param propertyList list of properties that should be searched.
   * \param propertyValue [out] value of the found property. Only valid if function returns true.
   * \return True if the property was found and \p propertyValue contains a valid value.
   *
   * \sa GeneratePropertyNameForDICOMTag, GetDICOMPropertyValue
   */
  bool MITKCORE_EXPORT GetBackwardsCompatibleDICOMPropertyValue(unsigned int group,
                                                                unsigned int element,
                                                                std::string const &backwardsCompatiblePropertyName,
                                                                PropertyList const *propertyList,
                                                                std::string &propertyValue);
}

#endif
