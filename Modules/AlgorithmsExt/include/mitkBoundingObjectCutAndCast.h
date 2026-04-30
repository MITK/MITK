/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkBoundingObjectCutAndCast_h
#define mitkBoundingObjectCutAndCast_h

#include <mitkBoundingObjectCutter.h>
#include <mitkCommon.h>

#include <mitkBoundingObjectCutter.tpp>

namespace mitk
{
  /**
   * \brief Cuts a BoundingObject region out of an image and casts the result to a specific pixel type.
   *
   * This is a templated extension of BoundingObjectCutter that additionally casts
   * the output image to the pixel type specified by the template parameter TPixel.
   * Input parameters are a mitk::BoundingObject and optionally a mitk::Image.
   * If no image is provided, the resulting image will have m_InsideValue as the pixel
   * value for inside pixels. If an image is provided, inside pixels retain their
   * original values. Pixels outside the BoundingObject are set to m_OutsideValue.
   *
   * \tparam TPixel The desired output pixel type (e.g., short, float, unsigned char).
   *
   * \sa BoundingObjectCutter
   * \sa BoundingObject
   * \ingroup Process
   */
  template <typename TPixel>
  class BoundingObjectCutAndCast : public BoundingObjectCutter
  {
  public:
    mitkClassMacro(BoundingObjectCutAndCast, BoundingObjectCutter);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      /** \brief The output pixel type for the cast operation. */
      typedef TPixel PixelType;

  protected:
    BoundingObjectCutAndCast();
    virtual ~BoundingObjectCutAndCast();

    virtual const std::type_info &GetOutputPixelType();

    virtual void ComputeData(mitk::Image *input3D, int boTimeStep);
  };
} // namespace mitk

#ifndef MITK_MANUAL_INSTANTIATION
#include <mitkBoundingObjectCutAndCast.tpp> // because it is a template
#endif

#endif
