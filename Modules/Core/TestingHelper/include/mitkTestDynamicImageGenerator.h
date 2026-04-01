/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkTestDynamicImageGenerator_h
#define mitkTestDynamicImageGenerator_h

#include <itkImage.h>
#include <itkImageRegionIterator.h>

#include <mitkImage.h>
#include <mitkImagePixelReadAccessor.h>

#include <MitkTestingHelperExports.h>

namespace mitk
{
  typedef itk::Image<int> TestImageType;
  typedef itk::Image<unsigned char> TestMaskType;

  /**
   * \brief Generate a test ITK image with deterministic intensity values.
   * \param factor Multiplicative factor applied to voxel values.
   * \return A 3D ITK integer image suitable for unit tests.
   */
  TestImageType::Pointer MITKTESTINGHELPER_EXPORT GenerateTestImage(int factor = 1);

  /**
   * \brief Generate a binary test mask as an ITK image.
   * \return A 3D unsigned-char ITK image with a deterministic mask pattern.
   */
  TestMaskType::Pointer MITKTESTINGHELPER_EXPORT GenerateTestMask();

  /**
   * \brief Generate a single MITK image frame for a specific time point.
   * \param timePoint The time point value for this frame.
   * \return A MITK Image representing a single frame of a dynamic image.
   */
  Image::Pointer MITKTESTINGHELPER_EXPORT GenerateTestFrame(double timePoint);

  /**
   * \brief Generate a test mask as a MITK Image.
   * \return A MITK Image containing a binary mask pattern for testing.
   */
  Image::Pointer MITKTESTINGHELPER_EXPORT GenerateTestMaskMITK();

  /**
   * \brief Generate a 4D dynamic test image as a MITK Image.
   * \return A MITK Image with multiple time steps for dynamic image testing.
   */
  Image::Pointer MITKTESTINGHELPER_EXPORT GenerateDynamicTestImageMITK();
}

#endif
