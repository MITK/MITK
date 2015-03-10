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

#ifndef __mitkAutoCropImageFilter_h_
#define __mitkAutoCropImageFilter_h_

#include "mitkCommon.h"
#include "MitkAlgorithmsExtExports.h"
#include "mitkSubImageSelector.h"
#include "mitkImageTimeSelector.h"

#include <itkImageRegion.h>
#include <itkImage.h>

namespace mitk {

/**
 *
 * @brief Shrink the image borders to a minimum considering a background color.
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
 * @ingroup Process
 *
 * @author Thomas Boettger; revised by Tobias Schwarz and Daniel Stein; Division of Medical
 * and Biological Informatics
 *
 */

class MITKALGORITHMSEXT_EXPORT AutoCropImageFilter : public SubImageSelector
{
public:

  typedef itk::ImageRegion<3> RegionType;

  mitkClassMacro(AutoCropImageFilter, SubImageSelector);

  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  itkGetConstMacro(BackgroundValue,float);
  itkSetMacro(BackgroundValue,float);

  itkGetConstMacro(MarginFactor,float);
  itkSetMacro(MarginFactor,float);

  itkGetMacro(CroppingRegion, RegionType);

  // Use this method to manually set a region
  void SetCroppingRegion(RegionType overrideRegion);

  virtual const PixelType GetOutputPixelType();

protected:

  // default constructor
  AutoCropImageFilter();

  // default destructor
  virtual ~AutoCropImageFilter();

  // This method calculates the actual smallest box
  void ComputeNewImageBounds();

  // Crops the image using the itk::RegionOfInterestImageFilter and creates the new output image
  template < typename TPixel, unsigned int VImageDimension>
  void ITKCrop3DImage( itk::Image< TPixel, VImageDimension >* inputItkImage, unsigned int timestep );

  // Here, the output image is initialized by the input and the newly calculated region
  virtual void GenerateOutputInformation();

  // Purposely not implemented
  virtual void GenerateInputRequestedRegion();

  // Crops the image on all time steps
  virtual void GenerateData();

  float m_BackgroundValue;

  RegionType m_CroppingRegion;

  float m_MarginFactor;

  typedef itk::Image<float,3>    ImageType;
  typedef ImageType::Pointer      ImagePointer;

  RegionType::SizeType m_RegionSize;
  RegionType::IndexType m_RegionIndex;

  mitk::ImageTimeSelector::Pointer m_TimeSelector;

  mitk::SlicedData::RegionType m_InputRequestedRegion;
  itk::TimeStamp m_TimeOfHeaderInitialization;

  bool m_OverrideCroppingRegion;

};

} // namespace mitk

#endif
