/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef mitkMultiLabelSegmentationMappingHelper_h
#define mitkMultiLabelSegmentationMappingHelper_h

#include <mapRegistrationBase.h>
#include <mitkLabelSetImage.h>
#include <mitkGeometry3D.h>

#include <mitkMAPRegistrationWrapper.h>

#include <MitkMatchPointRegistrationExports.h>

namespace mitk
{

  /**
   * \brief Namespace providing helper functions for mapping multi-label segmentations using MatchPoint registrations.
   *
   * Each label is mapped individually using nearest-neighbor interpolation to preserve label integrity.
   *
   * \sa mitk::ImageMappingHelper, mitk::MAPRegistrationWrapper, mitk::LabelSetImage
   */
  namespace MultiLabelSegmentationMappingHelper
  {
    /** \brief MatchPoint registration base type. */
    typedef ::map::core::RegistrationBase RegistrationType;
    /** \brief MITK wrapper type for MatchPoint registrations. */
    typedef ::mitk::MAPRegistrationWrapper MITKRegistrationType;

    /** \brief Geometry type used for the result image grid specification. */
    typedef ::mitk::BaseGeometry ResultGeometryType;

    /**
     * \brief Maps a multi-label segmentation image using a MatchPoint registration.
     *
     * \param[in] input Multi-label segmentation to be mapped.
     * \param[in] registration Pointer to the MatchPoint registration to use.
     * \param[in] throwOnOutOfInputAreaError If true, throws when the input does not cover the full result region.
     * \param[in] resultGeometry Geometry defining the output grid. If nullptr, the input geometry is used.
     * \param[in] throwOnMappingError If true, throws when the registration does not cover the result region.
     * \param[in] errorValue Label value used for voxels where mapping fails (when not throwing).
     * \return Smart pointer to the resulting mapped multi-label segmentation.
     * \pre \p input must be valid.
     * \pre \p registration must be valid.
     * \pre Dimensionality of the registration must match the input image.
     */
    MITKMATCHPOINTREGISTRATION_EXPORT MultiLabelSegmentation::Pointer map(const MultiLabelSegmentation* input, const RegistrationType* registration,
      bool throwOnOutOfInputAreaError = false, const ResultGeometryType* resultGeometry = nullptr,
      bool throwOnMappingError = true, const MultiLabelSegmentation::LabelValueType& errorValue = 0);

    /**
     * \brief Maps a multi-label segmentation using a MITK registration wrapper.
     * \overload
     *
     * \param[in] input Multi-label segmentation to be mapped.
     * \param[in] registration Pointer to the MITK registration wrapper to use.
     * \param[in] throwOnOutOfInputAreaError If true, throws when the input does not cover the full result region.
     * \param[in] resultGeometry Geometry defining the output grid, or nullptr to use the input geometry.
     * \param[in] throwOnMappingError If true, throws when the registration does not cover the result region.
     * \param[in] errorValue Label value used for voxels where mapping fails.
     * \return Smart pointer to the resulting mapped multi-label segmentation.
     */
    MITKMATCHPOINTREGISTRATION_EXPORT MultiLabelSegmentation::Pointer map(const MultiLabelSegmentation* input, const MITKRegistrationType* registration,
      bool throwOnOutOfInputAreaError = false, const ResultGeometryType* resultGeometry = nullptr,
      bool throwOnMappingError = true, const MultiLabelSegmentation::LabelValueType& errorValue = 0);

  }

}

#endif
