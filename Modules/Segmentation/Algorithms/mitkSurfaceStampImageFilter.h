/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSurfaceStampImageFilter_h
#define mitkSurfaceStampImageFilter_h

#include "MitkSegmentationExports.h"
#include "mitkCommon.h"
#include "mitkImageToImageFilter.h"
#include "mitkSurface.h"

#include <itkQuadEdgeMesh.h>

class vtkPolyData;

namespace mitk
{
  /**
   *
   * @brief Converts surface data to pixel data. Requires a surface and an
   * image, which header information defines the output image.
   *
   * The resulting image has the same dimension, size, and Geometry3D
   * as the input image. The image is cut using a vtkStencil.
   * The user can decide if he wants to keep the original values or create a
   * binary image by setting MakeBinaryOutputOn (default is \a false). If
   * set to \a true all voxels inside the surface are set to one and all
   * outside voxel are set to zero.
   *
   * NOTE: Since the reference input image is passed to the vtkStencil in
   * any case, the image needs to be initialized with pixel values greater than
   * the numerical minimum of the used pixel type (e.g. at least -127 for
   * unsigned char images, etc.) to produce a correct binary image
   * representation of the surface in MakeOutputBinary mode.
   *
   * @ingroup SurfaceFilters
   * @ingroup Process
   */
  class MITKSEGMENTATION_EXPORT SurfaceStampImageFilter : public ImageToImageFilter
  {
  public:
    mitkClassMacro(SurfaceStampImageFilter, ImageToImageFilter);
    itkNewMacro(Self);

    itkSetMacro(MakeOutputBinary, bool);
    itkGetMacro(MakeOutputBinary, bool);
    itkBooleanMacro(MakeOutputBinary);

    itkSetMacro(OverwriteBackground, bool);
    itkGetMacro(OverwriteBackground, bool);
    itkBooleanMacro(OverwriteBackground);

    itkGetConstMacro(BackgroundValue, float);
    itkSetMacro(BackgroundValue, float);

    itkGetConstMacro(ForegroundValue, float);
    itkSetMacro(ForegroundValue, float);

    void GenerateInputRequestedRegion() override;

    void GenerateOutputInformation() override;

    void GenerateData() override;

    void SetSurface(mitk::Surface *surface);

    typedef itk::QuadEdgeMesh<double, 3> MeshType;

  protected:
    SurfaceStampImageFilter();

    ~SurfaceStampImageFilter() override;

    void SurfaceStamp(int time = 0);

    template <typename TPixel>
    void SurfaceStampProcessing(itk::Image<TPixel, 3> *input, MeshType *mesh);

    void SurfaceStampBinaryOutputProcessing(MeshType *mesh);

    bool m_MakeOutputBinary;
    bool m_OverwriteBackground;
    float m_BackgroundValue;
    float m_ForegroundValue;

    mitk::Surface::Pointer m_Surface;
  };
} // namespace mitk

#endif
