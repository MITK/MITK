/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSplineVtkMapper3D_h
#define mitkSplineVtkMapper3D_h

#include "MitkMapperExtExports.h"
#include "mitkPointSetVtkMapper3D.h"

class vtkActor;
class vtkAssembly;

namespace mitk
{
  /**
  * @brief Vtk-based mapper for Splines.
  *
  * The mapper inherits from mitk::PointSetVTKMapper3D and renders a spline in 3D, using a mitk pointset as input.
  *
  *
  * Properties that can be set for splines which affect the mitk::SplineVtkMapper3D are:
  *
  *   - \b "line width": (FloatProperty) width of the spline
  *
  * There might be still some other, deprecated properties. These will not be documented anymore.
  * Please check the source if you really need them.
  *
  * @ingroup Mapper
  */
  class MITKMAPPEREXT_EXPORT SplineVtkMapper3D : public PointSetVtkMapper3D
  {
  public:
    mitkClassMacro(SplineVtkMapper3D, PointSetVtkMapper3D);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

      vtkProp *GetVtkProp(mitk::BaseRenderer *renderer) override;
    void UpdateVtkTransform(mitk::BaseRenderer *renderer) override;

    bool SplinesAreAvailable();

    vtkPolyData *GetSplinesPolyData();

    vtkActor *GetSplinesActor();

    virtual void UpdateSpline();

    itkSetMacro(SplineResolution, unsigned int);

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
