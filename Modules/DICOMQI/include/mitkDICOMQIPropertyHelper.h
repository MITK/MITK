/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDICOMQIPropertyHelper_h
#define mitkDICOMQIPropertyHelper_h

#include <mitkDICOMTag.h>
#include <mitkBaseData.h>

#include <MitkDICOMQIExports.h>

namespace mitk
{
  /** \brief Helper for propagating DICOM properties from source to derived images.
   *
   * Copies relevant DICOM tag properties (patient, study, series information)
   * from a source DICOM image to a derived image, ensuring proper DICOM
   * provenance tracking.
   *
   * \sa DICOMPMPropertyHelper
   */
  class MITKDICOMQI_EXPORT DICOMQIPropertyHelper
  {
  public:
    /** \brief Copy DICOM source properties from a source image to a derived image.
     *
     * Transfers patient, study, and series-level DICOM tag properties from the
     * source image's property list to the derived image. Tags that are missing
     * in the source are set to a default empty string.
     *
     * \param[in]     sourceDICOMImage  The original DICOM image to read properties from.
     * \param[in,out] derivedDICOMImage The derived image to set properties on.
     *
     * \pre Both \p sourceDICOMImage and \p derivedDICOMImage are not null.
     */
    static void DeriveDICOMSourceProperties(const BaseData *sourceDICOMImage, BaseData *derivedDICOMImage);

  private:
    static void AdoptReferenceDICOMProperty(PropertyList *referencedPropertyList,
      PropertyList *propertyList,
      const DICOMTag &tag,
      const std::string &defaultString = "");
    //-------------
  };
}
#endif
