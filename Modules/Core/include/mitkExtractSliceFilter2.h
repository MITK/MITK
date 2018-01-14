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
    enum class Interpolator
    {
      NearestNeighbor,
      Linear,
      BSpline
    };

    mitkClassMacro(ExtractSliceFilter2, ImageToImageFilter)
    itkFactorylessNewMacro(Self)

    const PlaneGeometry* GetOutputGeometry() const;
    void SetOutputGeometry(PlaneGeometry::Pointer outputGeometry);

    Interpolator GetInterpolator() const;
    void SetInterpolator(Interpolator interpolator);

  private:
    ExtractSliceFilter2();
    ~ExtractSliceFilter2() override;

    void GenerateData() override;
    void GenerateInputRequestedRegion() override;
    void VerifyInputInformation() override;

    struct Impl;
    Impl* m_Impl;
  };
}

#endif
