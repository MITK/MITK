/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkBoundingObjectCutAndCast_h
#define mitkBoundingObjectCutAndCast_h

#include "mitkBoundingObjectCutter.h"
#include "mitkCommon.h"

#include "mitkBoundingObjectCutter.txx"

namespace mitk
{
  //##Documentation
  //## @brief Cuts an Boundingobject out of an mitk Image
  //##
  //## Input Parameters are a mitk::BoundingObject and optionally an mitk::Image
  //## if no mitk::Image is provided, the resulting image will have m_InsideValue as pixelvalue on inside pixel,
  //## otherwise it will have the pixelvalue of the input image.
  //## Pixel on the outside of the BoundingObject will have a pixelvalue of m_OutsideValue
  //## \todo What Image resolution/spacing should be used, if no input image is given?
  //## @ingroup Process
  template <typename TPixel>
  class BoundingObjectCutAndCast : public BoundingObjectCutter
  {
  public:
    mitkClassMacro(BoundingObjectCutAndCast, BoundingObjectCutter);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      typedef TPixel PixelType;

  protected:
    BoundingObjectCutAndCast();
    virtual ~BoundingObjectCutAndCast();

    virtual const std::type_info &GetOutputPixelType();

    virtual void ComputeData(mitk::Image *input3D, int boTimeStep);
  };
} // namespace mitk

#ifndef MITK_MANUAL_INSTANTIATION
#include "mitkBoundingObjectCutAndCast.txx" // because it is a template
#endif

#endif
