/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkMaskAndCutRoiImageFilter_h
#define mitkMaskAndCutRoiImageFilter_h

#include <MitkAlgorithmsExtExports.h>
#include <mitkImageToImageFilter.h>

#include <itkRegionOfInterestImageFilter.h>
#include <mitkAutoCropImageFilter.h>
#include <mitkBoundingObject.h>
#include <mitkDataNode.h>
#include <mitkMaskImageFilter.h>

namespace mitk
{
  /**
   * \brief Cuts and masks a region of interest (ROI) out of an image.
   *
   * This filter performs a three-step operation:
   * 1. The ROI mask image is auto-cropped to its minimal bounding region.
   * 2. The corresponding subvolume is extracted from the input image.
   * 3. The subvolume is masked with the ROI so that pixels outside the ROI
   *    are replaced by an outside value (-32765).
   *
   * The ROI can be provided as a mitk::Image (binary mask) or as a
   * mitk::BoundingObject, which is internally converted to a binary mask.
   *
   * \sa AutoCropImageFilter
   * \sa MaskImageFilter
   * \sa BoundingObjectToSegmentationFilter
   * \ingroup Process
   */
  class MITKALGORITHMSEXT_EXPORT MaskAndCutRoiImageFilter : public ImageToImageFilter
  {
    typedef itk::Image<short, 3> ItkImageType;
    typedef itk::Image<unsigned char, 3> ItkMaskType;
    typedef itk::ImageRegion<3> RegionType;
    typedef itk::RegionOfInterestImageFilter<ItkImageType, ItkImageType> ROIFilterType;

  public:
    mitkClassMacro(MaskAndCutRoiImageFilter, ImageToImageFilter);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /**
     * \brief Set the region of interest.
     *
     * The ROI can be either a mitk::Image (used directly as a binary mask) or a
     * mitk::BoundingObject (internally converted to a binary mask using
     * BoundingObjectToSegmentationFilter).
     *
     * \param[in] roi The region of interest as a BaseData (Image or BoundingObject).
     */
    void SetRegionOfInterest(mitk::BaseData *roi);

    /**
     * \brief Get the output image.
     * \return Smart pointer to the masked and cut output image.
     */
    mitk::Image::Pointer GetOutput();

  protected:
    MaskAndCutRoiImageFilter();
    ~MaskAndCutRoiImageFilter() override;

    void GenerateData() override;

    ROIFilterType::Pointer m_RoiFilter;
    mitk::AutoCropImageFilter::Pointer m_CropFilter;
    mitk::MaskImageFilter::Pointer m_MaskFilter;

    // needed for temporary fix
    mitk::Image::Pointer m_outputImage;

  }; // class

} // namespace

#endif
