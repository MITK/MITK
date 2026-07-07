/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkMaskImageFilter_h
#define mitkMaskImageFilter_h

#include <MitkAlgorithmsExtExports.h>
#include <mitkCommon.h>
#include <mitkImageToImageFilter.h>

#include <itkImage.h>

namespace mitk
{
  /**
   * \brief Masks an image with a binary mask image.
   *
   * This filter applies a binary mask to an input image. Pixels where the mask
   * is non-zero retain their original values, while pixels where the mask is zero
   * are set to an outside value. By default, the outside value is the minimum
   * of the output pixel type, but it can be overridden to a user-defined value.
   *
   * The filter works on 3D images and processes each time step independently.
   * Both the input image and the mask must have compatible dimensions.
   *
   * \sa MaskAndCutRoiImageFilter
   * \ingroup Process
   */
  class MITKALGORITHMSEXT_EXPORT MaskImageFilter : public ImageToImageFilter
  {
  public:
    mitkClassMacro(MaskImageFilter, ImageToImageFilter);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

    /**
     * \brief Set the binary mask image.
     *
     * The mask is stored as the second input (index 1) of this filter.
     *
     * \param[in] mask The binary mask image.
     */
    void SetMask(const mitk::Image *mask);

    /**
     * \brief Get the mask image (const version).
     * \return Const pointer to the mask image.
     */
    const Image *GetMask() const;

    /**
     * \brief Get the mask image (non-const version).
     * \return Pointer to the mask image.
     */
    Image* GetMask();

    /**
     * \brief Set the value used for masked-out (outside) pixels.
     *
     * Only effective when OverrideOutsideValue is true. Default is 0.
     *
     * \param[in] _arg The outside pixel value.
     */
    itkSetMacro(OutsideValue, mitk::ScalarType);

    /**
     * \brief Get the value used for masked-out (outside) pixels.
     * \return The outside pixel value.
     */
    itkGetMacro(OutsideValue, mitk::ScalarType);

    /**
     * \brief Set whether to use a user-defined outside value.
     *
     * If false (default), the minimum of the output pixel type is used.
     * If true, the value set via SetOutsideValue is used.
     *
     * \param[in] _arg True to use the user-defined outside value.
     */
    itkSetMacro(OverrideOutsideValue, bool);

    /**
     * \brief Get whether the user-defined outside value is used.
     * \return True if OverrideOutsideValue is enabled.
     */
    itkGetMacro(OverrideOutsideValue, bool);

    /** \brief Toggle OverrideOutsideValue on/off. */
    itkBooleanMacro(OverrideOutsideValue);

  protected:
    MaskImageFilter();

    ~MaskImageFilter() override;

    void GenerateInputRequestedRegion() override;

    void GenerateOutputInformation() override;

    void GenerateData() override;

    template <typename TPixel1, unsigned int VImageDimension1, typename TPixel2, unsigned int VImageDimension2>
    void InternalComputeMask(itk::Image<TPixel1, VImageDimension1>* itkInput, itk::Image<TPixel2, VImageDimension2>* itkMask);

    /** \brief Time when header was last initialized. */
    itk::TimeStamp m_TimeOfHeaderInitialization;

    mitk::ScalarType m_OutsideValue;
    TimeStepType m_CurrentOutputTS;
    bool m_OverrideOutsideValue;
  };

} // namespace mitk

#endif
