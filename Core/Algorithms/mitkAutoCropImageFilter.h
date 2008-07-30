/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef __mitkAutoCropImageFilter_h_
#define __mitkAutoCropImageFilter_h_

#include "mitkCommon.h"
#include "mitkSubImageSelector.h"
#include "mitkImageTimeSelector.h"

#include <itkImageRegion.h>
#include <itkCropImageFilter.h>

namespace mitk {

/** 
 *
 * @brief Shrink the image borders to a minimum considering a background color.
 * 
 * This filter determines the bounding box of all pixels different.
 * from the background, crops the input image and returns it.
 *
 * @ingroup Process
 *
 * @todo clean up, this is messy. bug #524
 * @todo not able to handle 3D+t images. bug #1281
 * 
 */
class MITK_CORE_EXPORT AutoCropImageFilter : public SubImageSelector
{
public:

  typedef itk::ImageRegion<3> RegionType;

  mitkClassMacro(AutoCropImageFilter, SubImageSelector);

  itkNewMacro(Self);  

  itkGetConstMacro(BackgroundValue,float);
  itkSetMacro(BackgroundValue,float);

  itkGetConstMacro(MarginFactor,float);
  itkSetMacro(MarginFactor,float);
  
  void SetCroppingRegion(RegionType overrideRegion);

  virtual const std::type_info& GetOutputPixelType();

protected:

  void ComputeNewImageBounds();

  template < typename TPixel, unsigned int VImageDimension> 
  void ITKCrop3DImage( itk::Image< TPixel, VImageDimension >* inputItkImage, unsigned int timestep );


protected:

  AutoCropImageFilter();

  virtual ~AutoCropImageFilter();

  virtual void GenerateOutputInformation();

  virtual void GenerateInputRequestedRegion();

  virtual void GenerateData();

  float m_BackgroundValue;

  RegionType m_CroppingRegion;

  float m_MarginFactor;

  typedef itk::Image<float,3>    ImageType;
  typedef ImageType::Pointer      ImagePointer;
  typedef itk::CropImageFilter<ImageType,ImageType> CropFilterType;

  ImageType::RegionType::SizeType m_RegionSize;
  ImageType::RegionType::IndexType m_RegionIndex;

  CropFilterType::SizeType m_LowerBounds;
  CropFilterType::SizeType m_UpperBounds;

  mitk::ImageTimeSelector::Pointer m_TimeSelector;

  mitk::SlicedData::RegionType m_InputRequestedRegion;
  itk::TimeStamp m_TimeOfHeaderInitialization;

  bool m_OverrideCroppingRegion;

};

} // namespace mitk

#endif


