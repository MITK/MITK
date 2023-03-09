/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkContourSetVtkMapper3D_h
#define mitkContourSetVtkMapper3D_h

#include "mitkBaseRenderer.h"
#include "mitkCommon.h"
#include "mitkContourSet.h"
#include "mitkVtkMapper.h"
#include <MitkSegmentationExports.h>

#include <vtkPolyData.h>

class vtkPolyDataMapper;
class vtkAppendPolyData;
class vtkActor;
class vtkTubeFilter;

namespace mitk
{
  //##Documentation
  //## @brief Vtk-based mapper for mitk::Contour
  //## @ingroup Mapper
  class MITKSEGMENTATION_EXPORT ContourSetVtkMapper3D : public VtkMapper
  {
  public:
    mitkClassMacro(ContourSetVtkMapper3D, VtkMapper);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

      virtual const mitk::ContourSet *GetInput();

    vtkProp *GetVtkProp(mitk::BaseRenderer *renderer) override;

  protected:
    ContourSetVtkMapper3D();

    ~ContourSetVtkMapper3D() override;

    void GenerateDataForRenderer(mitk::BaseRenderer *renderer) override;

    vtkPolyDataMapper *m_VtkPolyDataMapper;
    vtkTubeFilter *m_TubeFilter;

    vtkPolyData *m_ContourSet;
    vtkActor *m_Actor;
  };

} // namespace mitk

#endif
