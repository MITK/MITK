/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkImageToSurfaceFilter_h
#define mitkImageToSurfaceFilter_h

#include <MitkCoreExports.h>
#include <mitkCommon.h>
#include <mitkSurface.h>
#include <mitkSurfaceSource.h>
#include <vtkPolyData.h>

#include <mitkImage.h>
#include <vtkImageData.h>

#include <vtkMarchingCubes.h>
#include <vtkSmoothPolyDataFilter.h>

namespace mitk
{
  /**
   * \brief Converts pixel data to surface data by using a threshold.
   *
   * The mitkImageToSurfaceFilter is used to create a new surface out of an MITK image. The filter
   * uses a threshold to define the surface. It is based on the vtkMarchingCubes algorithm. By default
   * a vtkPolyData surface based on an input threshold for the input image will be created. Optionally
   * it is possible to reduce the number of triangles/polygons [SetDecimate(mitk::ImageToSurfaceFilter::DecimatePro) and
   * SetTargetReduction(float)]
   * or smooth the surface data [SetSmooth(true), SetSmoothIteration(int) and SetSmoothRelaxation(float)].
   *
   * The resulting VTK surface has the same size as the input image. The surface
   * can be generally smoothed by vtkDecimatePro to reduce complexity of triangles
   * and vtkSmoothPolyDataFilter to relax the mesh. It is also possible
   * to create time-sliced surfaces.
   *
   * \ingroup Process
   * \sa SurfaceSource
   * \sa Surface
   */

  class MITKCORE_EXPORT ImageToSurfaceFilter : public SurfaceSource
  {
  public:
    /**
     * \brief Enumeration of available polygon decimation strategies.
     *
     * Determines whether and how polygon reduction is applied to the created surface.
     */
    enum DecimationType
    {
      NoDecimation,
      DecimatePro,
      QuadricDecimation
    };

    mitkClassMacro(ImageToSurfaceFilter, SurfaceSource);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      /**
       * For each image time slice a surface will be created. This method is
       * called by Update().
       */

      void GenerateData() override;

    /**
    * Initializes the output information ( i.e. the geometry information ) of
    * the output of the filter
    */
    void GenerateOutputInformation() override;

    /**
     * Returns a const reference to the input image (e.g. the original input image that is used to create the surface)
     */
    const mitk::Image *GetInput(void);

    /**
     * Set the source image to create a surface for this filter class. As input every mitk
     * 3D or 3D+t image can be used.
     */
    using itk::ProcessObject::SetInput;
    virtual void SetInput(const mitk::Image *image);

    /**
     * \brief Set the number of iterations used to smooth the surface.
     *
     * Uses the vtkSmoothPolyDataFilter with a Laplacian filter. The higher the number
     * of iterations, the stronger the smoothing result.
     *
     * \param smoothIteration The number of smoothing iterations. Default is 50.
     *        The VTK documentation recommends small relaxation factors and large
     *        numbers of iterations.
     */
    void SetSmoothIteration(int smoothIteration);

    /**
     * \brief Set the relaxation factor for Laplacian smoothing.
     *
     * The VTK documentation recommends small relaxation factors
     * and large numbers of iterations.
     *
     * \param smoothRelaxation The relaxation factor. Default is 0.1.
     */
    void SetSmoothRelaxation(float smoothRelaxation);

    /**
     * \brief Set the threshold used for surface extraction via vtkMarchingCubes.
     *
     * All pixels in the input image with values higher than this threshold
     * will be considered in the surface. Default value is 1.
     */
    itkSetMacro(Threshold, ScalarType);

    /**
     * \brief Get the threshold used for vtkMarchingCubes surface extraction.
     */
    itkGetConstMacro(Threshold, ScalarType);

    /**
     * \brief Enable vtkSmoothPolyDataFilter for Laplacian smoothing.
     *
     * This filter will relax the surface. You can control the filter by
     * manipulating the number of iterations and the relaxation factor.
     */
    itkSetMacro(Smooth, bool);

    /** \brief Toggle surface smoothing on/off. */
    itkBooleanMacro(Smooth);

    /** \brief Get whether surface smoothing is enabled. */
    itkGetConstMacro(Smooth, bool);

    /**
     * \brief Get the current decimation mode.
     *
     * Possible values are NoDecimation, DecimatePro, or QuadricDecimation.
     */
    itkGetConstMacro(Decimate, DecimationType);

    /**
     * \brief Set the decimation mode to reduce the number of triangles in the mesh.
     *
     * For more detailed information see vtkDecimatePro and vtkQuadricDecimation.
     */
    itkSetMacro(Decimate, DecimationType);

    /**
     * \brief Set the desired target reduction of triangles in the range [0.0, 1.0].
     *
     * The destroyed triangles are relative to the total number of triangles.
     * For example, 0.9 will reduce the data set to 10% of its original size.
     */
    itkSetMacro(TargetReduction, float);

    /**
     * \brief Get the target reduction factor for the decimation filter.
     */
    itkGetConstMacro(TargetReduction, float);

    /**
     * \brief Transform a 3D point by a 4x4 matrix (affine transformation).
     *
     * \tparam T1 The matrix element type.
     * \tparam T2 The input point element type.
     * \tparam T3 The output point element type.
     */
    template <class T1, class T2, class T3>
    inline void mitkVtkLinearTransformPoint(T1 matrix[4][4], T2 in[3], T3 out[3])
    {
      T3 x = matrix[0][0] * in[0] + matrix[0][1] * in[1] + matrix[0][2] * in[2] + matrix[0][3];
      T3 y = matrix[1][0] * in[0] + matrix[1][1] * in[1] + matrix[1][2] * in[2] + matrix[1][3];
      T3 z = matrix[2][0] * in[0] + matrix[2][1] * in[1] + matrix[2][2] * in[2] + matrix[2][3];
      out[0] = x;
      out[1] = y;
      out[2] = z;
    }

  protected:
    ImageToSurfaceFilter();

    /** \brief Destructor. */
    ~ImageToSurfaceFilter() override;

    /**
     * \brief Create the surface for a single time step using vtkMarchingCubes.
     *
     * Optionally applies smoothing and decimation based on the current filter settings.
     *
     * \param time The time step to process (0 for single time step images).
     * \param vtkimage The VTK image data input.
     * \param surface The output surface to populate.
     * \param threshold The iso-surface threshold (can differ from the member threshold).
     */
    void CreateSurface(int time, vtkImageData *vtkimage, mitk::Surface *surface, const ScalarType threshold);

    /** \brief Flag whether the created surface shall be smoothed (default is false). */
    bool m_Smooth;

    /** \brief Decimation mode. Default is NoDecimation. */
    DecimationType m_Decimate;

    /** \brief Threshold for surface extraction. Default value is 1. */
    ScalarType m_Threshold;

    /** \brief The reduction factor for the decimation filter. Range [0.0, 1.0]. */
    float m_TargetReduction;

    /** \brief The number of iterations for the smoothing filter. Default is 50. */
    int m_SmoothIteration;

    /** \brief The relaxation factor for the smoothing filter. Default is 0.1. */
    float m_SmoothRelaxation;
  };

} // namespace mitk

#endif
