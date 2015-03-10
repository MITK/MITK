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


#ifndef BOUNDINGOBJECTCUTANDCAST_H_HEADER_INCLUDED_C10B22CD
#define BOUNDINGOBJECTCUTANDCAST_H_HEADER_INCLUDED_C10B22CD

#include "mitkCommon.h"
#include "mitkBoundingObjectCutter.h"

#include "mitkBoundingObjectCutter.txx"

namespace mitk {

//##Documentation
//## @brief Cuts an Boundingobject out of an mitk Image
//##
//## Input Parameters are a mitk::BoundingObject and optionally an mitk::Image
//## if no mitk::Image is provided, the resulting image will have m_InsideValue as pixelvalue on inside pixel,
//## otherwise it will have the pixelvalue of the input image.
//## Pixel on the outside of the BoundingObject will have a pixelvalue of m_OutsideValue
//## \todo What Image resolution/spacing should be used, if no input image is given?
//## @ingroup Process
template <typename TPixel> class BoundingObjectCutAndCast : public BoundingObjectCutter
{
public:
  mitkClassMacro(BoundingObjectCutAndCast, BoundingObjectCutter);
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  typedef TPixel PixelType;

protected:
  BoundingObjectCutAndCast();
  virtual ~BoundingObjectCutAndCast();

  virtual const std::type_info& GetOutputPixelType();

  virtual void ComputeData(mitk::Image* input3D, int boTimeStep);
};
} // namespace mitk

#ifndef MITK_MANUAL_INSTANTIATION
#include "mitkBoundingObjectCutAndCast.txx"  // because it is a template
#endif

#endif /* BOUNDINGOBJECTCUTANDCAST_H_HEADER_INCLUDED_C10B22CD */
