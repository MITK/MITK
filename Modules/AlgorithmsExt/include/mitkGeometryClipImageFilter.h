/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkGeometryClipImageFilter_h
#define mitkGeometryClipImageFilter_h

#include <MitkAlgorithmsExtExports.h>
#include <mitkCommon.h>
#include <mitkGeometryData.h>
#include <mitkImageTimeSelector.h>
#include <mitkImageToImageFilter.h>

namespace itk
{
  template <class TPixel, unsigned int VImageDimension>
  class ITK_EXPORT Image;
}

namespace mitk
{
  /**
   * \brief Filter for clipping an image with a PlaneGeometry.
   *
   * This filter clips an image along a plane defined by a PlaneGeometry.
   * The clipping geometry can be either a single PlaneGeometry or a
   * TimeGeometry containing PlaneGeometry instances for multiple time steps.
   * Pixels on one side of the plane are set to an outside value (or labeled),
   * while pixels on the other side retain their original values.
   *
   * The filter supports several modes:
   * - Simple clipping with a constant outside value
   * - Automatic outside value (minimum of the pixel type)
   * - Labeling both sides of the plane with distinct labels
   *
   * \sa PlaneGeometry
   * \sa HeightFieldSurfaceClipImageFilter
   * \ingroup Process
   */
  class MITKALGORITHMSEXT_EXPORT GeometryClipImageFilter : public ImageToImageFilter
  {
  public:
    mitkClassMacro(GeometryClipImageFilter, ImageToImageFilter);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

      /**
       * \brief Set the clipping geometry (must be a PlaneGeometry).
       * \param[in] aClippingGeometry The PlaneGeometry used for clipping.
       */
      void SetClippingGeometry(const mitk::BaseGeometry *aClippingGeometry);

    /**
     * \brief Set the clipping geometry as a TimeGeometry.
     *
     * The TimeGeometry must contain PlaneGeometry instances, one per time step.
     *
     * \param[in] aClippingGeometry The TimeGeometry containing PlaneGeometry instances.
     */
    void SetClippingGeometry(const mitk::TimeGeometry *aClippingGeometry);

    /**
     * \brief Get the clipping geometry for a single time step.
     * \return Const pointer to the clipping BaseGeometry.
     */
    const mitk::BaseGeometry *GetClippingGeometry() const;

    /**
     * \brief Get the clipping time geometry (if set).
     * \return Const pointer to the clipping TimeGeometry, or nullptr.
     */
    const mitk::TimeGeometry *GetClippingTimeGeometry() const;

    /**
     * \brief Get whether the part above the geometry is clipped (default: true).
     * \return True if the part above the geometry is clipped.
     */
    itkGetConstMacro(ClipPartAboveGeometry, bool);

    /**
     * \brief Set whether the part above or below the geometry shall be clipped (default: true).
     * \param[in] _arg True to clip above, false to clip below.
     */
    itkSetMacro(ClipPartAboveGeometry, bool);

    /** \brief Toggle ClipPartAboveGeometry on/off. */
    itkBooleanMacro(ClipPartAboveGeometry);

    /**
     * \brief Set the value for outside (clipped) pixels (default: 0).
     *
     * Only used when AutoOutsideValue is false.
     *
     * \param[in] _arg The outside pixel value.
     */
    itkSetMacro(OutsideValue, ScalarType);

    /**
     * \brief Get the value for outside (clipped) pixels.
     * \return The outside pixel value.
     */
    itkGetConstMacro(OutsideValue, ScalarType);

    /**
     * \brief Set whether to automatically use the pixel type minimum as outside value (default: false).
     * \param[in] _arg True to use automatic outside value.
     */
    itkSetMacro(AutoOutsideValue, bool);

    /**
     * \brief Get whether the outside value is automatically determined.
     * \return True if automatic outside value is enabled.
     */
    itkGetConstMacro(AutoOutsideValue, bool);

    /** \brief Toggle AutoOutsideValue on/off. */
    itkBooleanMacro(AutoOutsideValue);

    /**
     * \brief Set whether to auto-orient labels like vtkPolyDataNormals.
     * \param[in] _arg True to enable automatic label orientation.
     */
    itkSetMacro(AutoOrientLabels, bool);

    /**
     * \brief Get whether automatic label orientation is enabled.
     * \return True if automatic label orientation is enabled.
     */
    itkGetConstMacro(AutoOrientLabels, bool);

    /**
     * \brief Set whether both sides of the clipping geometry are labeled (default: false).
     *
     * When enabled, pixels above the geometry receive AboveGeometryLabel and
     * pixels below receive BelowGeometryLabel.
     *
     * \param[in] _arg True to label both sides.
     */
    itkSetMacro(LabelBothSides, bool);

    /**
     * \brief Get whether both sides labeling is enabled.
     * \return True if both sides are labeled.
     */
    itkGetConstMacro(LabelBothSides, bool);

    /** \brief Toggle LabelBothSides on/off. */
    itkBooleanMacro(LabelBothSides);

    /**
     * \brief Set the label value for voxels above the clipping geometry.
     *
     * Only used when LabelBothSides is true.
     *
     * \param[in] _arg The above-geometry label value.
     */
    itkSetMacro(AboveGeometryLabel, ScalarType);

    /**
     * \brief Get the label value for voxels above the clipping geometry.
     * \return The above-geometry label value.
     */
    itkGetConstMacro(AboveGeometryLabel, ScalarType);

    /**
     * \brief Set the label value for voxels below the clipping geometry.
     *
     * Only used when LabelBothSides is true.
     *
     * \param[in] _arg The below-geometry label value.
     */
    itkSetMacro(BelowGeometryLabel, ScalarType);

    /**
     * \brief Get the label value for voxels below the clipping geometry.
     * \return The below-geometry label value.
     */
    itkGetConstMacro(BelowGeometryLabel, ScalarType);

  protected:
    GeometryClipImageFilter();

    ~GeometryClipImageFilter() override;

    void GenerateInputRequestedRegion() override;

    void GenerateOutputInformation() override;

    void GenerateData() override;

    template <typename TPixel, unsigned int VImageDimension>
    void _InternalComputeClippedImage(itk::Image<TPixel, VImageDimension> *itkImage,
                                      mitk::GeometryClipImageFilter *geometryClipper,
                                      const mitk::PlaneGeometry *clippingPlaneGeometry);

    mitk::BaseGeometry::ConstPointer m_ClippingGeometry;
    mitk::GeometryData::Pointer m_ClippingGeometryData;
    mitk::TimeGeometry::ConstPointer m_TimeClippingGeometry;
    mitk::ImageTimeSelector::Pointer m_InputTimeSelector;
    mitk::ImageTimeSelector::Pointer m_OutputTimeSelector;

    /** \brief Defines whether the part above or below the geometry shall be clipped (default: true). */
    bool m_ClipPartAboveGeometry;

    /** \brief Value for outside pixels (default: 0).
     * Used only if m_AutoOutsideValue is false.
     */
    ScalarType m_OutsideValue;

    /** \brief If true, the minimum of the output pixel type is used as outside value (default: false). */
    bool m_AutoOutsideValue;

    /** \brief If true, all pixels above and below the geometry are labeled
     * with m_AboveGeometryLabel and m_BelowGeometryLabel.
     */
    bool m_LabelBothSides;

    /**
     * \brief Orient above like vtkPolyDataNormals does with AutoOrientNormals.
     */
    bool m_AutoOrientLabels;

    /** \brief Label value for pixels above the geometry when m_LabelBothSides is on. */
    ScalarType m_AboveGeometryLabel;

    /** \brief Label value for pixels below the geometry when m_LabelBothSides is on. */
    ScalarType m_BelowGeometryLabel;

    /** \brief Time when header was last initialized. */
    itk::TimeStamp m_TimeOfHeaderInitialization;
  };

} // namespace mitk

#endif
