/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDICOMPMPropertyHelper_h
#define mitkDICOMPMPropertyHelper_h

#include <mitkDICOMTag.h>
#include <mitkBaseData.h>

#include <MitkDICOMQIExports.h>

namespace mitk
{
  /** \brief Helper for setting DICOM Parametric Map (PM) properties on derived images.
   *
   * Provides utility methods to populate DICOM PM-specific properties on
   * BaseData objects, such as modality and SOP class information, plus
   * the legacy "referenceFiles" source-file lookup the PM writer relies
   * on to locate the source DICOM files at emission time.
   *
   * \sa DICOMQIPropertyHelper
   */
  class MITKDICOMQI_EXPORT DICOMPMPropertyHelper
  {
  public:
    /** \brief Set DICOM Parametric Map properties on a derived image.
     *
     * Populates the derived image's property list with PM-specific DICOM
     * tags (Modality "PM", Series Description, Content Creator Name) and,
     * when \p sourceDICOMImage is non-null, copies the source's "files"
     * lookup table to the derived image as "referenceFiles" so the PM
     * writer can locate the source DICOM files at write time.
     *
     * The referenceFiles copy lives here, not in
     * DICOMQIPropertyHelper::DeriveDICOMSourceProperties, because only
     * the PM writer reads it; the DICOM SEG writer has moved to the
     * SegSourceImageRelationRule architecture and would carry the
     * property as dead weight.
     *
     * \param[in] sourceDICOMImage The source DICOM image to copy
     *   referenceFiles from. May be null; the PM-specific tags are
     *   stamped regardless.
     * \param[in,out] derivedDICOMImage The image to set PM properties on.
     *
     * \pre \p derivedDICOMImage is not null.
     */
    static void DeriveDICOMPMProperties(const BaseData *sourceDICOMImage,
                                        BaseData *derivedDICOMImage);
  };
} // namespace mitk
#endif
