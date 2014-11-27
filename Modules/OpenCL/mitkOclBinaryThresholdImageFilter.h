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

#ifndef _MITKOCLBINARYTHRESHOLDIMAGEFILTER_H_
#define _MITKOCLBINARYTHRESHOLDIMAGEFILTER_H_

#include "mitkOclImageToImageFilter.h"
#include <itkObject.h>

namespace mitk
{
class OclImageToImageFilter;

/** Documentation
  *
  * \brief The OclBinaryThresholdImageFilter computes a binary segmentation based on given
  threshold values.

  *
  * The filter requires two threshold values ( the upper and the lower threshold ) and two image values ( inside and outside ). The resulting voxel of the segmentation image is assigned the inside value 1 if the image value is between the given thresholds and the outside value otherwise.
  */
class MitkOpenCL_EXPORT OclBinaryThresholdImageFilter : public OclImageToImageFilter, public itk::Object
{

public:
  mitkClassMacro(OclBinaryThresholdImageFilter, OclImageToImageFilter);
  itkNewMacro(Self);

  /**
  * @brief SetInput Set the input image. Only 3D images are supported for now.
  * @param image a 3D image.
  * @throw mitk::Exception if the dimesion is not 3.
  */
  void SetInput(Image::Pointer image);

  /** Update the filter */
  void Update();

  /** Set the lower threshold
    @param thr Threshold value
    */
  void SetLowerThreshold( int lowerThreshold )
  {
    this->m_LowerThreshold = lowerThreshold;
  }

  /** Set the upper threshold
    @param thr Threshold value
    */
  void SetUpperThreshold( int upperThreshold )
  {
    this->m_UpperThreshold = upperThreshold;
  }

  /** Set the outside value

    @param val The outside value
    */
  void SetOutsideValue( int outsideValue )
  {
    this->m_OutsideValue = outsideValue;
  }

  /** Set the inside value
    @param val The inside value
    */
  void SetInsideValue( int insideValue )
  {
    this->m_InsideValue = insideValue;
  }

protected:

  /** Constructor */
  OclBinaryThresholdImageFilter();

  /** Destructor */
  virtual ~OclBinaryThresholdImageFilter();

  /** Initialize the filter */
  bool Initialize();

  void Execute();

  mitk::PixelType GetOutputType()
  {
    return mitk::MakeScalarPixelType<unsigned char>();
  }

  int GetBytesPerElem()
  {
    return sizeof(unsigned char);
  }

  virtual us::Module* GetModule();

private:
  /** The OpenCL kernel for the filter */
  cl_kernel m_ckBinaryThreshold;

  int m_LowerThreshold;

  int m_UpperThreshold;

  int m_InsideValue;

  int m_OutsideValue;
};
}


#endif
