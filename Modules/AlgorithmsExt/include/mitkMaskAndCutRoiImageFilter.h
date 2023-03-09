/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkMaskAndCutRoiImageFilter_h
#define mitkMaskAndCutRoiImageFilter_h

#include "MitkAlgorithmsExtExports.h"
#include "mitkImageToImageFilter.h"

#include "itkRegionOfInterestImageFilter.h"
#include "mitkAutoCropImageFilter.h"
#include "mitkBoundingObject.h"
#include "mitkDataNode.h"
#include "mitkMaskImageFilter.h"

namespace mitk
{
  /**
  \brief Cuts a region of interest (ROI) out of an image

  In the first step, this filter reduces the image region of the given ROI to a minimum. Using this region, a subvolume
  ist cut out of the given input image.
  The ROI is then used to mask the subvolume. Pixel inside the ROI will have their original value, pixel outside will be
  replaced by m_OutsideValue

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
    itkGetMacro(MaxValue, mitk::ScalarType);
    itkGetMacro(MinValue, mitk::ScalarType);

    void SetRegionOfInterest(mitk::BaseData *roi);
    // void SetRegionOfInterest(Image::Pointer image);
    // void SetRegionOfInterest(BoundingObject::Pointer boundingObject);
    // void SetRegionOfInterestByNode(mitk::DataNode::Pointer node);

    // temporary fix for bug #
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

    mitk::ScalarType m_MaxValue;
    mitk::ScalarType m_MinValue;

  }; // class

} // namespace

#endif
