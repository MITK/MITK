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
    ExtractSliceFilter2();
    ~ExtractSliceFilter2() override;

    void AllocateOutputs() override;
    void BeforeThreadedGenerateData() override;
    void ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread, itk::ThreadIdType threadId) override;
    void VerifyInputInformation() override;

    struct Impl;
    Impl* m_Impl;
  };
}

#endif
