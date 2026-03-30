/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef mitkImageMappingHelper_h
#define mitkImageMappingHelper_h

#include <mapImageMappingTask.h>
#include <mapRegistrationBase.h>
#include <mitkImage.h>
#include <mitkGeometry3D.h>

#include <mitkMAPRegistrationWrapper.h>

#include <MitkMatchPointRegistrationExports.h>

namespace mitk
{
  /**
   * \brief Enumeration of interpolation strategies for image mapping operations.
   *
   * \sa mitk::ImageMappingHelper::map
   */
  struct ImageMappingInterpolator
  {
    enum Type
    {
      UserDefined = 0,     /**< \brief User may specify a custom interpolator. */
      NearestNeighbor = 1, /**< \brief Nearest neighbor interpolation. */
      Linear = 2,          /**< \brief Linear interpolation. */
      BSpline_3 = 3,       /**< \brief 3rd order B-spline interpolation. */
      WSinc_Hamming = 4,   /**< \brief Windowed sinc interpolation with Hamming window. */
      WSinc_Welch = 5      /**< \brief Windowed sinc interpolation with Welch window. */
    };
  };

  /**
   * \brief Namespace providing helper functions for mapping (resampling) MITK images using MatchPoint registrations.
   *
   * \sa mitk::MAPRegistrationWrapper, mitk::ImageMappingInterpolator
   */
  namespace ImageMappingHelper
  {
    /** \brief MatchPoint registration base type. */
    typedef ::map::core::RegistrationBase RegistrationType;
    /** \brief MITK wrapper type for MatchPoint registrations. */
    typedef ::mitk::MAPRegistrationWrapper MITKRegistrationType;

    /** \brief Geometry type used for the result image grid specification. */
    typedef ::mitk::BaseGeometry ResultImageGeometryType;
    /** \brief Input image type for mapping operations. */
    typedef ::mitk::Image InputImageType;
    /** \brief Result image type returned by mapping operations. */
    typedef ::mitk::Image ResultImageType;

    /**
     * \brief Creates a TimeGeometry for the mapping result based on the input data and optional result geometry.
     *
     * \param[in] input The input data whose time geometry serves as the template.
     * \param[in] resultGeometry Optional pointer to the spatial geometry for the result. If nullptr, the input geometry is used.
     * \return A smart pointer to the created TimeGeometry.
     */
    MITKMATCHPOINTREGISTRATION_EXPORT TimeGeometry::Pointer CreateResultTimeGeometry(const mitk::BaseData* input,
      const mitk::ImageMappingHelper::ResultImageGeometryType* resultGeometry);


    /**
     * \brief Maps (resamples) an input image using a MatchPoint registration.
     *
     * \param[in] input Image that should be mapped.
     * \param[in] registration Pointer to the MatchPoint registration to use for mapping.
     * \param[in] throwOnOutOfInputAreaError If true, throws an exception when the input image does
     *            not cover the entire requested result region. If false, uses \p paddingValue.
     * \param[in] paddingValue Value to use for voxels outside the input image area (when not throwing).
     * \param[in] resultGeometry Pointer to the geometry defining the output grid. If nullptr,
     *            the input image geometry is used.
     * \param[in] throwOnMappingError If true, throws an exception when the registration does not
     *            cover the requested region. If false, uses \p errorValue.
     * \param[in] errorValue Value to use for voxels where mapping fails (when not throwing).
     * \param[in] interpolatorType The interpolation strategy to use.
     * \return Smart pointer to the resulting mapped image.
     * \pre \p input must be valid.
     * \pre \p registration must be valid.
     * \pre Dimensionality of the registration must match the input image.
     */
    MITKMATCHPOINTREGISTRATION_EXPORT ResultImageType::Pointer map(const InputImageType* input, const RegistrationType* registration,
      bool throwOnOutOfInputAreaError = false, const double& paddingValue = 0,
      const ResultImageGeometryType* resultGeometry = nullptr,
      bool throwOnMappingError = true, const double& errorValue = 0, mitk::ImageMappingInterpolator::Type interpolatorType = mitk::ImageMappingInterpolator::Linear);

    /**
     * \brief Maps (resamples) an input image using a MITK registration wrapper.
     * \overload
     *
     * \param[in] input Image that should be mapped.
     * \param[in] registration Pointer to the MITK registration wrapper to use for mapping.
     * \param[in] throwOnOutOfInputAreaError If true, throws on out-of-input-area errors.
     * \param[in] paddingValue Padding value for out-of-input-area voxels.
     * \param[in] resultGeometry Pointer to the output grid geometry, or nullptr to use the input geometry.
     * \param[in] throwOnMappingError If true, throws on mapping errors.
     * \param[in] errorValue Value for voxels where mapping fails.
     * \param[in] interpolatorType The interpolation strategy to use.
     * \return Smart pointer to the resulting mapped image.
     * \pre \p input must be valid.
     * \pre \p registration must be valid.
     * \pre Dimensionality of the registration must match the input image.
     */
    MITKMATCHPOINTREGISTRATION_EXPORT ResultImageType::Pointer map(const InputImageType* input, const MITKRegistrationType* registration,
      bool throwOnOutOfInputAreaError = false, const double& paddingValue = 0,
      const ResultImageGeometryType* resultGeometry = nullptr,
      bool throwOnMappingError = true, const double& errorValue = 0, mitk::ImageMappingInterpolator::Type interpolatorType = mitk::ImageMappingInterpolator::Linear);

    /**
     * \brief Generates a super-sampled geometry by scaling the spacing of an existing geometry.
     *
     * \param[in] inputGeometry The geometry to super-sample.
     * \param[in] xScaling Scaling factor for the X-axis spacing.
     * \param[in] yScaling Scaling factor for the Y-axis spacing.
     * \param[in] zScaling Scaling factor for the Z-axis spacing.
     * \return Smart pointer to the new geometry with adjusted spacing and size.
     */
    MITKMATCHPOINTREGISTRATION_EXPORT ResultImageGeometryType::Pointer GenerateSuperSampledGeometry(const ResultImageGeometryType* inputGeometry,
      double xScaling, double yScaling, double zScaling);

    /**
     * \brief Refines the geometry of an image by applying an affine registration to its Geometry3D.
     *
     * Instead of resampling the image voxels, this method clones the input image and
     * transforms its geometry using the registration's direct mapping kernel. This only
     * works if the kernel can be decomposed into an affine matrix transformation.
     *
     * \param[in] input The input image whose geometry should be refined.
     * \param[in] registration The MatchPoint registration to apply.
     * \param[in] throwOnError If true, throws an exception when the registration cannot
     *            be decomposed into an affine transform. If false, returns nullptr instead.
     * \return Smart pointer to the result image with refined geometry, or nullptr on failure
     *         (when \p throwOnError is false).
     * \pre \p input must point to a valid instance.
     * \pre \p registration must point to a valid instance.
     */
    MITKMATCHPOINTREGISTRATION_EXPORT ResultImageType::Pointer refineGeometry(const InputImageType* input, const RegistrationType* registration, bool throwOnError = true);
    /** \overload */
    MITKMATCHPOINTREGISTRATION_EXPORT ResultImageType::Pointer refineGeometry(const InputImageType* input, const MITKRegistrationType* registration, bool throwOnError = true);

    /**
     * \brief Checks whether a registration can be used for geometry refinement.
     *
     * \param[in] registration The registration to check.
     * \return True if the registration's direct kernel can be decomposed into an affine matrix.
     */
    MITKMATCHPOINTREGISTRATION_EXPORT bool canRefineGeometry(const RegistrationType* registration);
    /** \overload */
    MITKMATCHPOINTREGISTRATION_EXPORT bool canRefineGeometry(const MITKRegistrationType* registration);

  }

}

#endif
