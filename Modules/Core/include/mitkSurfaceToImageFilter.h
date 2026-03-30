/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSurfaceToImageFilter_h
#define mitkSurfaceToImageFilter_h

#include <MitkCoreExports.h>
#include <mitkCommon.h>
#include <mitkImageSource.h>
#include <mitkSurface.h>

class vtkPolyData;

namespace mitk
{
  /**
   * \brief Converts surface data to pixel data.
   *
   * Requires a surface and an image, whose header information defines the output image.
   * The resulting image has the same dimension, size, and Geometry3D
   * as the input image. The image is cut using a vtkStencil.
   * The user can decide whether to keep the original values or create a
   * binary image by setting MakeBinaryOutputOn (default is \a false). If
   * set to \a true, all voxels inside the surface are set to one and all
   * outside voxels are set to zero.
   *
   * \note Since the reference input image is passed to the vtkStencil in
   * any case, the image needs to be initialized with pixel values greater than
   * the numerical minimum of the used pixel type (e.g. at least -127 for
   * unsigned char images, etc.) to produce a correct binary image
   * representation of the surface in MakeOutputBinary mode.
   *
   * \ingroup SurfaceFilters
   * \ingroup Process
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

    /** \brief Generate the input requested region. */
    void GenerateInputRequestedRegion() override;

    /** \brief Generate output information based on the input image. */
    void GenerateOutputInformation() override;

    /** \brief Perform the surface-to-image conversion. */
    void GenerateData() override;

    /**
     * \brief Get the input surface.
     * \return Pointer to the input surface, or nullptr if none is set.
     */
    const mitk::Surface *GetInput(void);

    using itk::ProcessObject::SetInput;

    /**
     * \brief Set the input surface to be converted.
     *
     * \param[in] surface the surface to convert.
     */
    virtual void SetInput(const mitk::Surface *surface);

    /**
     * \brief Set the reference image that defines the output geometry.
     *
     * \param[in] source the reference image.
     */
    void SetImage(const mitk::Image *source);

    /**
     * \brief Get the reference image.
     * \return Pointer to the reference image, or nullptr if none is set.
     */
    const mitk::Image *GetImage(void);

  protected:
    SurfaceToImageFilter();

    ~SurfaceToImageFilter() override;

    /**
     * \brief Perform the 3D stencil operation for the given time step.
     *
     * \param[in] time the time step to process.
     */
    void Stencil3DImage(int time = 0);

    bool m_MakeOutputBinary;
    bool m_UShortBinaryPixelType;

    float m_BackgroundValue;
    double m_Tolerance;
  };

  /**
   * \brief Convert a surface into a binary mask image suitable for use as segmentation label content.
   *
   * \param[in] refImage pointer to an image that serves as template for the output geometry.
   * \param[in] surface pointer to the surface to be converted into a binary mask.
   * \return The generated binary mask image, or nullptr on failure.
   * \throw mitk::Exception if refImage or surface is nullptr.
   */
  MITKCORE_EXPORT mitk::Image::Pointer ConvertSurfaceToLabelMask(const mitk::Image* refImage, const mitk::Surface* surface);

} // namespace mitk

#endif
