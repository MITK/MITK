/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkHeightFieldSurfaceClipImageFilter_h
#define mitkHeightFieldSurfaceClipImageFilter_h

#include <MitkAlgorithmsExtExports.h>
#include <mitkCommon.h>
#include <mitkGeometry3D.h>
#include <mitkImageTimeSelector.h>
#include <mitkImageToImageFilter.h>
#include <mitkSurface.h>

namespace itk
{
  template <class TPixel, unsigned int VImageDimension>
  class ITK_EXPORT Image;
}

namespace mitk
{
  /**
   * \brief Filter for clipping an image with an height-field represented by
   * an mitk::Surface.
   *
   * The filter assumes that the surface represents a height field as seen from
   * the plane defined by (1.0/0.0/0.0), (0.0/1.0/0.0). All voxels lying on one
   * side of the height field will be clipped, i.e., set to 0. In addition, any
   * voxels which are out of bounds when projected on this plane will be clipped
   * as well.
   *
   * \ingroup Process
   */
  class MITKALGORITHMSEXT_EXPORT HeightFieldSurfaceClipImageFilter : public ImageToImageFilter
  {
  public:
    /** \brief List of Surface pointers used for multi-plane clipping. */
    typedef std::vector<mitk::Surface *> ClippingPlaneList;

    mitkClassMacro(HeightFieldSurfaceClipImageFilter, ImageToImageFilter);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

      /**
       * \brief Set the surface defining a height field as a triangle mesh.
       * \param[in] clippingSurface The surface to use for clipping.
       */
      void SetClippingSurface(Surface *clippingSurface);

    /**
     * \brief Set multiple surfaces for multi-plane clipping.
     * \param[in] planeList A vector of Surface pointers to use for clipping.
     */
    void SetClippingSurfaces(ClippingPlaneList planeList);

    /**
     * \brief Get the clipping surface.
     * \return Const pointer to the clipping Surface.
     */
    const Surface *GetClippingSurface() const;

    /**
     * \brief Clipping mode enumeration.
     *
     * Defines how clipped voxels are modified:
     * - CLIPPING_MODE_CONSTANT: Replace clipped voxels with a constant value.
     * - CLIPPING_MODE_MULTIPLYBYFACTOR: Multiply clipped voxels by a factor.
     * - CLIPPING_MODE_MULTIPLANE: Use multi-plane labeling.
     */
    enum
    {
      CLIPPING_MODE_CONSTANT = 0,
      CLIPPING_MODE_MULTIPLYBYFACTOR,
      CLIPPING_MODE_MULTIPLANE
    };

    /**
     * \brief Set the clipping mode.
     * \param[in] mode One of the CLIPPING_MODE_* constants.
     */
    void SetClippingMode(int mode);

    /**
     * \brief Get the current clipping mode.
     * \return The current clipping mode constant.
     */
    int GetClippingMode();

    /** \brief Set clipping mode to CLIPPING_MODE_CONSTANT. */
    void SetClippingModeToConstant();

    /** \brief Set clipping mode to CLIPPING_MODE_MULTIPLYBYFACTOR. */
    void SetClippingModeToMultiplyByFactor();

    /** \brief Set clipping mode to CLIPPING_MODE_MULTIPLANE. */
    void SetClippingModeToMultiPlaneValue();

    /**
     * \brief Set the constant gray-value for clipping in CONSTANT mode.
     * \param[in] _arg The constant value to fill clipped voxels with.
     */
    itkSetMacro(ClippingConstant, ScalarType);

    /**
     * \brief Get the constant gray-value for clipping in CONSTANT mode.
     * \return The clipping constant value.
     */
    itkGetConstMacro(ClippingConstant, ScalarType);

    /**
     * \brief Set the multiplication factor for clipping in MULTIPLYBYFACTOR mode.
     * \param[in] _arg The factor to multiply clipped voxels by.
     */
    itkSetMacro(MultiplicationFactor, ScalarType);

    /**
     * \brief Get the multiplication factor for clipping in MULTIPLYBYFACTOR mode.
     * \return The multiplication factor.
     */
    itkGetConstMacro(MultiplicationFactor, ScalarType);

    /**
     * \brief Set the x-resolution of the height-field sampling (default: 256).
     * \param[in] _arg The x-resolution in samples.
     */
    itkSetMacro(HeightFieldResolutionX, unsigned int);

    /**
     * \brief Get the x-resolution of the height-field sampling.
     * \return The x-resolution in samples.
     */
    itkGetConstMacro(HeightFieldResolutionX, unsigned int);

    /**
     * \brief Set the y-resolution of the height-field sampling (default: 256).
     * \param[in] _arg The y-resolution in samples.
     */
    itkSetMacro(HeightFieldResolutionY, unsigned int);

    /**
     * \brief Get the y-resolution of the height-field sampling.
     * \return The y-resolution in samples.
     */
    itkGetConstMacro(HeightFieldResolutionY, unsigned int);

  protected:
    HeightFieldSurfaceClipImageFilter();

    ~HeightFieldSurfaceClipImageFilter() override;

    void GenerateInputRequestedRegion() override;

    void GenerateOutputInformation() override;

    void GenerateData() override;

    template <typename TPixel, unsigned int VImageDimension>
    void _InternalComputeClippedImage(itk::Image<TPixel, VImageDimension> *itkImage,
                                      HeightFieldSurfaceClipImageFilter *clipImageFilter,
                                      vtkPolyData *clippingPolyData,
                                      AffineTransform3D *imageToPlaneTransform);

    ImageTimeSelector::Pointer m_InputTimeSelector;
    ImageTimeSelector::Pointer m_OutputTimeSelector;

    int m_ClippingMode;

    ScalarType m_ClippingConstant;
    ScalarType m_MultiplicationFactor;
    unsigned m_MultiPlaneValue;

    unsigned int m_HeightFieldResolutionX;
    unsigned int m_HeightFieldResolutionY;

    double m_MaxHeight;

    /** \brief Time when Header was last initialized */
    itk::TimeStamp m_TimeOfHeaderInitialization;
  };

} // namespace mitk

#endif
