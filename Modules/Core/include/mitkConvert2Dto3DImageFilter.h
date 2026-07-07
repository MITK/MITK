/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkConvert2Dto3DImageFilter_h
#define mitkConvert2Dto3DImageFilter_h

// MITK
#include <mitkImageToImageFilter.h>
#include <itkImage.h>
#include <mitkImage.h>

namespace mitk
{
  /**
   * \brief Converts a 2D MITK image to a 3D MITK image with a single slice.
   *
   * A new 3D MITK image is created and all pixel and geometry information from
   * the given 2D input image is copied. The resulting 3D image has exactly one slice.
   *
   * This filter is useful before saving a 2D image that carries 3D geometry
   * information. By converting it to 3D with one slice, common formats (e.g. NRRD)
   * can store a full 3x3 transformation matrix.
   *
   * \ingroup Geometry
   * \sa ImageToImageFilter
   */
  class MITKCORE_EXPORT Convert2Dto3DImageFilter : public ImageToImageFilter
  {
  public:
    mitkClassMacro(Convert2Dto3DImageFilter, ImageToImageFilter);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

  protected:
    /** \brief Default constructor. */
    Convert2Dto3DImageFilter();
    /** \brief Destructor. */
    ~Convert2Dto3DImageFilter() override;

    /**
     * \brief Generate the 3D output image from the 2D input.
     *
     * Called during the pipeline update process. Copies all pixel data and
     * geometry from the 2D input image into a new 3D image with one slice.
     */
    void GenerateData() override;

    /**
     * \brief ITK-level conversion from a 2D ITK image to a 3D MITK image.
     * \tparam TPixel           The pixel type.
     * \tparam VImageDimension  The image dimension (expected to be 2).
     */
    template <typename TPixel, unsigned int VImageDimension>
    void ItkConvert2DTo3D(const itk::Image<TPixel, VImageDimension> *itkImage, mitk::Image::Pointer &mitkImage);
  };
} // END mitk namespace
#endif
