/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkExtractSliceFilter2_h
#define mitkExtractSliceFilter2_h

#include <mitkImageToImageFilter.h>
#include <mitkPlaneGeometry.h>
#include <MitkCoreExports.h>

namespace mitk
{
  /**
   * \brief Extract an arbitrarily oriented 2D image from a 3D image.
   *
   * Use ExtractSliceFilter2::SetOutputGeometry to specify the orientation of
   * the 2D output image.
   *
   * If a pixel of the 2D output image is not located within the bounds of the
   * 3D input image, it is set to the lowest possible pixel value.
   *
   * Cubic interpolation is considerably slow on the first update for a newly
   * set input image. Subsequent filter updates with cubic interpolation are
   * faster by several orders of magnitude as long as the input image was
   * neither changed nor modified.
   *
   * This filter is completely based on ITK compared to the VTK-based
   * mitk::ExtractSliceFilter. It is more robust, easy to use, and produces
   * an mitk::Image with valid geometry. Generally it is not as fast as
   * mitk::ExtractSliceFilter, though.
   *
   * \pre The input image must be 3-dimensional, initialized, and have a valid volume set.
   * \pre The output geometry must be set and must be an image geometry.
   *
   * \sa ExtractSliceFilter
   * \ingroup Algorithms
   */
  class MITKCORE_EXPORT ExtractSliceFilter2 final : public ImageToImageFilter
  {
  public:
    /**
     * \brief Interpolation methods for slice extraction.
     *
     * \li NearestNeighbor -- Fastest, no smoothing.
     * \li Linear -- Moderate speed, linear interpolation.
     * \li Cubic -- Slowest on first use, B-spline interpolation (order 2).
     */
    enum Interpolator
    {
      NearestNeighbor,
      Linear,
      Cubic
    };

    mitkClassMacro(ExtractSliceFilter2, ImageToImageFilter);

    itkFactorylessNewMacro(Self);

    /**
     * \brief Set the input image.
     *
     * Resets the internal interpolation function cache when the input changes.
     *
     * \param[in] image  The 3D input image.
     */
    void SetInput(const InputImageType* image) override;

    /**
     * \brief Set the input image at a given index.
     *
     * Only index 0 is supported; any other index will throw.
     *
     * \param[in] index  Must be 0.
     * \param[in] image  The 3D input image.
     * \throw mitk::Exception if index is not 0.
     */
    void SetInput(unsigned int index, const InputImageType* image) override;

    /**
     * \brief Get the output geometry defining the orientation of the extracted 2D slice.
     *
     * \return The output PlaneGeometry, or nullptr if not set.
     */
    const PlaneGeometry* GetOutputGeometry() const;

    /**
     * \brief Set the output geometry defining the orientation of the extracted 2D slice.
     *
     * \param[in] outputGeometry  The PlaneGeometry specifying the slice orientation.
     */
    void SetOutputGeometry(PlaneGeometry::Pointer outputGeometry);

    /**
     * \brief Get the current interpolation method.
     *
     * \return The current Interpolator enum value.
     */
    Interpolator GetInterpolator() const;

    /**
     * \brief Set the interpolation method.
     *
     * Changing the interpolator invalidates the cached interpolation function.
     *
     * \param[in] interpolator  The Interpolator enum value to use.
     */
    void SetInterpolator(Interpolator interpolator);

  private:
    using Superclass::SetInput;

    /** \brief Constructor. Initializes with NearestNeighbor interpolation. */
    ExtractSliceFilter2();

    /** \brief Destructor. */
    ~ExtractSliceFilter2() override;

    /** \brief Allocate the output image based on input pixel type and output geometry. */
    void AllocateOutputs() override;

    /** \brief Execute the slice extraction. */
    void GenerateData() override;

    /**
     * \brief Verify that input image and output geometry are valid.
     *
     * \throw mitk::Exception if input or output geometry is invalid.
     */
    void VerifyInputInformation() const override;

    struct Impl;
    Impl* m_Impl;
  };
}

#endif
