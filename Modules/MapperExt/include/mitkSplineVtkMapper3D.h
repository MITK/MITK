/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSplineVtkMapper3D_h
#define mitkSplineVtkMapper3D_h

#include <MitkMapperExtExports.h>
#include <mitkPointSetVtkMapper3D.h>

class vtkActor;
class vtkAssembly;

namespace mitk
{
  /** \brief VTK-based mapper for rendering splines through point sets in 3D.
   *
   * Inherits from PointSetVtkMapper3D and renders a smooth spline curve
   * through the points of a PointSet. The spline is computed using VTK's
   * cardinal spline interpolation.
   *
   * Relevant DataNode properties:
   * - \b "line width": (FloatProperty) Width of the spline line.
   *
   * \sa PointSetVtkMapper3D
   * \ingroup Mapper
   */
  class MITKMAPPEREXT_EXPORT SplineVtkMapper3D : public PointSetVtkMapper3D
  {
  public:
    mitkClassMacro(SplineVtkMapper3D, PointSetVtkMapper3D);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

    /** \brief Get the VTK prop assembly containing the spline and points.
     *
     * \param[in] renderer The renderer context.
     * \return The VTK prop assembly.
     */
    vtkProp *GetVtkProp(mitk::BaseRenderer *renderer) override;

    /** \brief Update the VTK transform for the given renderer.
     *
     * \param[in] renderer The renderer to update.
     */
    void UpdateVtkTransform(mitk::BaseRenderer *renderer) override;

    /** \brief Check whether the spline data has been computed.
     *
     * \return \c true if the spline poly data is available.
     */
    bool SplinesAreAvailable();

    /** \brief Get the VTK poly data representing the spline curve.
     *
     * \return The spline poly data, or \c nullptr if not yet computed.
     */
    vtkPolyData *GetSplinesPolyData();

    /** \brief Get the VTK actor used to render the spline.
     *
     * \return The spline actor.
     */
    vtkActor *GetSplinesActor();

    /** \brief Recompute the spline from the current point set data. */
    virtual void UpdateSpline();

    /** \brief Set the number of interpolation points between each pair of input points.
     *
     * Higher values produce smoother splines. Default is typically 20.
     *
     * \param[in] _arg The spline resolution (number of subdivisions).
     */
    itkSetMacro(SplineResolution, unsigned int);

    /** \brief Get the current spline resolution.
     *
     * \return The number of interpolation points between input points.
     */
    itkGetMacro(SplineResolution, unsigned int);

  protected:
    SplineVtkMapper3D();

    ~SplineVtkMapper3D() override;

    void GenerateDataForRenderer(mitk::BaseRenderer *renderer) override;

    void ApplyAllProperties(BaseRenderer *renderer, vtkActor *actor) override;

    vtkActor *m_SplinesActor;

    vtkPropAssembly *m_SplineAssembly;

    bool m_SplinesAvailable;

    bool m_SplinesAddedToAssembly;

    unsigned int m_SplineResolution;

    itk::TimeStamp m_SplineUpdateTime;
  };

} // namespace mitk

#endif
