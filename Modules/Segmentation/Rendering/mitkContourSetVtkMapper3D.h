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


#ifndef MITK_CONTOUR_SET_VTK_MAPPER_3D_H
#define MITK_CONTOUR_SET_VTK_MAPPER_3D_H

#include "mitkCommon.h"
#include <MitkSegmentationExports.h>
#include "mitkVtkMapper.h"
#include "mitkContourSet.h"
#include "mitkBaseRenderer.h"

#include <vtkPolyData.h>

class vtkPolyDataMapper;
class vtkAppendPolyData;
class vtkActor;
class vtkTubeFilter;

namespace mitk {

//##Documentation
//## @brief Vtk-based mapper for mitk::Contour
//## @ingroup Mapper
class MITKSEGMENTATION_EXPORT ContourSetVtkMapper3D : public VtkMapper
{
  public:

    mitkClassMacro(ContourSetVtkMapper3D, VtkMapper);

    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    virtual const mitk::ContourSet* GetInput();

    virtual vtkProp* GetVtkProp(mitk::BaseRenderer* renderer) override;

  protected:

    ContourSetVtkMapper3D();

    virtual ~ContourSetVtkMapper3D();

    virtual void GenerateDataForRenderer(mitk::BaseRenderer* renderer) override;

    vtkPolyDataMapper* m_VtkPolyDataMapper;
    vtkTubeFilter*     m_TubeFilter;

    vtkPolyData *m_ContourSet;
    vtkActor *m_Actor;

};

} // namespace mitk

#endif // MITK_CONTOUR_VTK_MAPPER_3D_H
