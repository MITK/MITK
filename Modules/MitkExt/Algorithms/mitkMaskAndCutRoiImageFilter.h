/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/
#ifndef MITKMASKANDCUTROIIMAGEFILTER_H
#define MITKMASKANDCUTROIIMAGEFILTER_H

#include "MitkExtExports.h"
#include "mitkImageToImageFilter.h"

#include "mitkBoundingObject.h"
#include "mitkDataNode.h"
#include "mitkAutoCropImageFilter.h"
#include "itkRegionOfInterestImageFilter.h"
#include "mitkMaskImageFilter.h"

namespace mitk
{
  /**
  \brief Cuts a region of interest (ROI) out of an image

  In the first step, this filter reduces the image region of the given ROI to a minimum. Using this region, a subvolume ist cut out of the given input image.
  The ROI is then used to mask the subvolume. Pixel inside the ROI will have their original value, pixel outside will be replaced by m_OutsideValue

  */
  class MitkExt_EXPORT MaskAndCutRoiImageFilter : public ImageToImageFilter
  {
      typedef itk::Image<short, 3> ItkImageType;
      typedef itk::Image<unsigned char, 3> ItkMaskType;
      typedef itk::ImageRegion<3> RegionType;
      typedef itk::RegionOfInterestImageFilter<ItkImageType, ItkImageType> ROIFilterType;

  public:
    mitkClassMacro(MaskAndCutRoiImageFilter, ImageToImageFilter);
    itkNewMacro(MaskAndCutRoiImageFilter);

    itkGetMacro(MaxValue, mitk::ScalarType);
    itkGetMacro(MinValue, mitk::ScalarType);

    void SetRegionOfInterest(mitk::BaseData* roi);
    //void SetRegionOfInterest(Image::Pointer image);
    //void SetRegionOfInterest(BoundingObject::Pointer boundingObject);
    //void SetRegionOfInterestByNode(mitk::DataNode::Pointer node);

    //temporary fix for bug #
    mitk::Image::Pointer GetOutput();

  protected:
    MaskAndCutRoiImageFilter();
    ~MaskAndCutRoiImageFilter();

    void GenerateData();

    ROIFilterType::Pointer m_RoiFilter;
    mitk::AutoCropImageFilter::Pointer m_CropFilter;
    mitk::MaskImageFilter::Pointer m_MaskFilter;

    //needed for temporary fix
    mitk::Image::Pointer m_outputImage;

    mitk::ScalarType m_MaxValue;
    mitk::ScalarType m_MinValue;

  };//class

}//namespace

#endif
