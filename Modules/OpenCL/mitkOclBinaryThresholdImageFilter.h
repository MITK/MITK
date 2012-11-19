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

namespace mitk
{
class OclImageToImageFilter;

/** Documentation
  *
  * \brief The OclBinaryThresholdImageFilter computes an binary segmentation based on given
  threshold values

  *
  * The filter requires two threshold values ( the upper and the lower threshold ) and two values ( inside and outside ). The resulting
  voxel of the segmentation image is assigned the inside value 1 if the image value is between the given thresholds and the outside value otherwise.
  */
class MitkOcl_EXPORT OclBinaryThresholdImageFilter : public OclImageToImageFilter
{
  typedef OclFilter Superclass;

public:
  /** Update the filter */
  void Update();

  int GetBytesPerElement();

  /** Constructor */
  OclBinaryThresholdImageFilter();

  /** Destructor */
  virtual ~OclBinaryThresholdImageFilter();

  /** Set the lower threshold

    @param thr Threshold value
    */
  void SetLowerThreshold( int thr )
  {
    this->m_lowerThr = thr;
  }

  /** Set the upper threshold

    @param thr Threshold value
    */
  void SetUpperThreshold( int thr )
  {
    this->m_upperThr = thr;
  }

  /** Set the outside value

    @param val The outside value
    */
  void SetOutsideValue( int val)
  {
    this->m_outsideVal = val;
  }

  /** Set the inside value

    @param val The inside value
    */
  void SetInsideValue( int val)
  {
    this->m_insideVal = val;
  }

protected:
  /** Initialize the filter */
  bool Initialize();

  void Execute();

  mitk::PixelType GetOutputType()
  {
    return mitk::MakeScalarPixelType<short>();
  }

  int GetBytesPerElem()
  {
    return sizeof(short);
  }

private:
  /** The OpenCL kernel for the filter */
  cl_kernel m_ckBinaryThreshold;

  int m_lowerThr;

  int m_upperThr;

  int m_insideVal;

  int m_outsideVal;
};
}


#endif
