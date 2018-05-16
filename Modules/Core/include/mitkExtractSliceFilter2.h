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

#ifndef mitkExtractSliceFilter2_h
#define mitkExtractSliceFilter2_h

#include <mitkImageToImageFilter.h>
#include <mitkPlaneGeometry.h>
#include <MitkCoreExports.h>

namespace mitk
{
  /** \brief Extract an arbitrarily oriented 2-d image from a 3-d image.
   *
   * Use ExtractSliceFilter2::SetOutputGeometry to specify the orientation of
   * the 2-d output image.
   *
   * If a pixel of the 2-d output image isn't located within the bounds of the
   * 3-d input image, it is set to the lowest possible pixel value.
   *
   * Cubic interpolation is considerably slow on the first update for a newly
   * set input image. Subsequent filter updates with cubic interpolation are
   * faster by several orders of magnitude as long as the input image was
   * neither changed nor modified.
   *
   * This filter is completely based on ITK compared to the VTK-based
   * mitk::ExtractSliceFilter. It is more robust, easy to use, and produces
   * an mitk::Image with valid geometry. Generally it is not as fast as
   * mitk::ExtractSliceFilter, though.
   */
  class MITKCORE_EXPORT ExtractSliceFilter2 final : public ImageToImageFilter
  {
  public:
    enum Interpolator
    {
      NearestNeighbor,
      Linear,
      Cubic
    };

    mitkClassMacro(ExtractSliceFilter2, ImageToImageFilter)
    itkFactorylessNewMacro(Self)

    void SetInput(const InputImageType* image) override;
    void SetInput(unsigned int index, const InputImageType* image) override;

    const PlaneGeometry* GetOutputGeometry() const;
    void SetOutputGeometry(PlaneGeometry::Pointer outputGeometry);

    Interpolator GetInterpolator() const;
    void SetInterpolator(Interpolator interpolator);

  private:
    using Superclass::SetInput;

    ExtractSliceFilter2();
    ~ExtractSliceFilter2() override;

    void AllocateOutputs() override;
    // void BeforeThreadedGenerateData() override;
    // void ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread, itk::ThreadIdType threadId) override;
    void GenerateData() override;
    void VerifyInputInformation() override;

    struct Impl;
    Impl* m_Impl;
  };
}

#endif
