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


#ifndef _MITK_SPLINE_VTK_MAPPER_3D__H
#define _MITK_SPLINE_VTK_MAPPER_3D__H

#include "mitkPointSetVtkMapper3D.h"
#include "MitkMapperExtExports.h"

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

    mitkClassMacro( SplineVtkMapper3D, PointSetVtkMapper3D );

    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    virtual vtkProp* GetVtkProp(mitk::BaseRenderer *renderer) override;
    virtual void UpdateVtkTransform(mitk::BaseRenderer *renderer) override;

    bool SplinesAreAvailable();

    vtkPolyData* GetSplinesPolyData();

    vtkActor* GetSplinesActor();

    virtual void UpdateSpline();

    itkSetMacro( SplineResolution, unsigned int );

    itkGetMacro( SplineResolution, unsigned int );

protected:

    SplineVtkMapper3D();

    virtual ~SplineVtkMapper3D();

    virtual void GenerateDataForRenderer(mitk::BaseRenderer * renderer) override;

    virtual void ApplyAllProperties(BaseRenderer *renderer, vtkActor *actor) override;

    vtkActor* m_SplinesActor;

    vtkPropAssembly* m_SplineAssembly;

    bool m_SplinesAvailable;

    bool m_SplinesAddedToAssembly;

    unsigned int m_SplineResolution;

    itk::TimeStamp m_SplineUpdateTime;
};


} //namespace mitk


#endif
