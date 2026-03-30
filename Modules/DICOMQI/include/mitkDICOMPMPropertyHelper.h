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
   * BaseData objects, such as modality and SOP class information.
   *
   * \sa DICOMQIPropertyHelper
   */
  class MITKDICOMQI_EXPORT DICOMPMPropertyHelper
  {
  public:
    /** \brief Set DICOM Parametric Map properties on a derived image.
     *
     * Populates the derived image's property list with PM-specific DICOM
     * tags such as Modality ("RWV") and SOP Class UID.
     *
     * \param[in,out] derivedDICOMImage The image to set PM properties on.
     *
     * \pre \p derivedDICOMImage is not null.
     */
    static void DeriveDICOMPMProperties(BaseData *derivedDICOMImage);

  };
} // namespace mitk
#endif
