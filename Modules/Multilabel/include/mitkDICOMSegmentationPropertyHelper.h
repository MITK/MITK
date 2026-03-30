/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDICOMSegmentationPropertyHelper_h
#define mitkDICOMSegmentationPropertyHelper_h

#include <mitkDICOMTag.h>
#include <mitkLabel.h>
#include <mitkLabelSetImage.h>

#include <MitkMultilabelExports.h>

namespace mitk
{
  /**
   * \brief Helper class for setting standard DICOM Segmentation properties on a MultiLabelSegmentation.
   *
   * Provides a static method that populates a MultiLabelSegmentation's property list with
   * required DICOM tags for SEG modality, including Modality (SEG), Series Description,
   * Content Creator Name, Clinical Trial Series ID, Time Point ID, and Coordinating
   * Center Name.
   *
   * \sa DICOMSegmentationConstants, MultiLabelSegmentation
   */
  class MITKMULTILABEL_EXPORT DICOMSegmentationPropertyHelper
  {
  public:
    /**
     * \brief Populates the segmentation image with standard DICOM Segmentation properties.
     *
     * Sets properties such as Modality="SEG", Series Description="MITK Segmentation",
     * Content Creator Name="MITK", and related clinical trial identifiers.
     *
     * \param[in,out] dicomSegImage The segmentation image to populate with DICOM properties.
     * \pre dicomSegImage must point to a valid MultiLabelSegmentation instance.
     */
    static void DeriveDICOMSegmentationProperties(MultiLabelSegmentation* dicomSegImage);
  };
}
#endif
