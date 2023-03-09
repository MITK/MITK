/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSurfaceToImageFilter_h
#define mitkSurfaceToImageFilter_h

#include "MitkCoreExports.h"
#include "mitkCommon.h"
#include "mitkImageSource.h"
#include "mitkSurface.h"

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
  class MITKCORE_EXPORT SurfaceToImageFilter : public ImageSource
  {
  public:
    mitkClassMacro(SurfaceToImageFilter, ImageSource);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);
    itkSetMacro(MakeOutputBinary, bool);
    itkGetMacro(MakeOutputBinary, bool);
    itkBooleanMacro(MakeOutputBinary);

    itkSetMacro(UShortBinaryPixelType, bool);
    itkGetMacro(UShortBinaryPixelType, bool);
    itkBooleanMacro(UShortBinaryPixelType);

    itkGetConstMacro(BackgroundValue, float);
    itkSetMacro(BackgroundValue, float);

    itkGetConstMacro(Tolerance, double);
    itkSetMacro(Tolerance, double);

    void GenerateInputRequestedRegion() override;

    void GenerateOutputInformation() override;

    void GenerateData() override;

    const mitk::Surface *GetInput(void);

    using itk::ProcessObject::SetInput;
    virtual void SetInput(const mitk::Surface *surface);

    void SetImage(const mitk::Image *source);

    const mitk::Image *GetImage(void);

  protected:
    SurfaceToImageFilter();

    ~SurfaceToImageFilter() override;

    void Stencil3DImage(int time = 0);

    bool m_MakeOutputBinary;
    bool m_UShortBinaryPixelType;

    float m_BackgroundValue;
    double m_Tolerance;
  };

} // namespace mitk

#endif
