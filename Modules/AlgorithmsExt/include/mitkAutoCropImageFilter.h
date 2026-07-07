/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkAutoCropImageFilter_h
#define mitkAutoCropImageFilter_h

#include <MitkAlgorithmsExtExports.h>
#include <mitkCommon.h>
#include <mitkImageTimeSelector.h>
#include <mitkSubImageSelector.h>

#include <itkImage.h>
#include <itkImageRegion.h>

namespace mitk
{
  /**
   *
   * \brief Shrink the image borders to a minimum considering a background color.
   *
   * This filter determines the smallest bounding box of all pixels different
   * from the background, and returns an output image which has been cropped to this size.
   * The box calculated this way is not the smallest possible box, but the box with the
   * smallest sides perpendicular to the world coordinate system.
   *
   * The filter works on 3D and 4D image data. For the 4D case, the smallest box is
   * calculated with side lengths as the maximum of single side lengths from all time steps.
   *
   * 2D images are not supported, and will never be.
   *
   * It is also possible to set the region to be cropped manually using the
   * SetCroppingRegion() method.
   *
   * A margin can be set to enlarge the cropped region with a constant factor in all
   * directions around the smallest possible.
   *
   *
   * \ingroup Process
   *
   * \author Thomas Boettger; revised by Tobias Schwarz and Daniel Stein
   *
   */

  class MITKALGORITHMSEXT_EXPORT AutoCropImageFilter : public SubImageSelector
  {
  public:
    /** \brief 3D image region type used for cropping bounds. */
    typedef itk::ImageRegion<3> RegionType;

    mitkClassMacro(AutoCropImageFilter, SubImageSelector);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

    /**
     * \brief Get the background value used to determine non-relevant pixels.
     * \return The current background pixel value (default: 0).
     */
    itkGetConstMacro(BackgroundValue, float);

    /**
     * \brief Set the background value. Pixels with this value are considered background.
     * \param[in] _arg The background pixel value.
     */
    itkSetMacro(BackgroundValue, float);

    /**
     * \brief Get the margin factor for enlarging the cropping region.
     * \return The margin factor (default: 0).
     */
    itkGetConstMacro(MarginFactor, float);

    /**
     * \brief Set the margin factor to enlarge the cropping region.
     *
     * The cropping region is enlarged by this factor in all directions
     * around the tightest bounding box of non-background pixels.
     *
     * \param[in] _arg The margin factor.
     */
    itkSetMacro(MarginFactor, float);

    /**
     * \brief Get the computed or manually set cropping region.
     * \return The cropping region as an itk::ImageRegion<3>.
     */
    itkGetMacro(CroppingRegion, RegionType);

    /**
     * \brief Manually set the cropping region, overriding automatic computation.
     *
     * When this method is called, the filter will use the provided region
     * instead of computing the smallest bounding box from the image content.
     *
     * \param[in] overrideRegion The region to use for cropping.
     */
    void SetCroppingRegion(RegionType overrideRegion);

    /**
     * \brief Returns the pixel type of the output image.
     * \return The output pixel type.
     */
    virtual const PixelType GetOutputPixelType();

  protected:
    // default constructor
    AutoCropImageFilter();

    // default destructor
    ~AutoCropImageFilter() override;

    // This method calculates the actual smallest box
    void ComputeNewImageBounds();

    // Crops the image using the itk::RegionOfInterestImageFilter and creates the new output image
    template <typename TPixel, unsigned int VImageDimension>
    void ITKCrop3DImage(itk::Image<TPixel, VImageDimension> *inputItkImage, unsigned int timestep);

    // Here, the output image is initialized by the input and the newly calculated region
    void GenerateOutputInformation() override;

    // Purposely not implemented
    void GenerateInputRequestedRegion() override;

    // Crops the image on all time steps
    void GenerateData() override;

    float m_BackgroundValue;

    RegionType m_CroppingRegion;

    float m_MarginFactor;

    typedef itk::Image<float, 3> ImageType;
    typedef ImageType::Pointer ImagePointer;

    RegionType::SizeType m_RegionSize;
    RegionType::IndexType m_RegionIndex;

    mitk::ImageTimeSelector::Pointer m_TimeSelector;

    mitk::SlicedData::RegionType m_InputRequestedRegion;
    itk::TimeStamp m_TimeOfHeaderInitialization;

    bool m_OverrideCroppingRegion;
  };

} // namespace mitk

#endif
