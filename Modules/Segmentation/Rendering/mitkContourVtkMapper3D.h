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

#ifndef MITK_CONTOUR_VTK_MAPPER_3D_H
#define MITK_CONTOUR_VTK_MAPPER_3D_H

#include "SegmentationExports.h"
#include "mitkVtkMapper3D.h"

#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

class vtkPolyDataMapper;
class vtkAppendPolyData;
class vtkActor;
class vtkTubeFilter;

namespace mitk {

class BaseRenderer;
class Contour;

/**
 @brief Vtk-based mapper for mitk::Contour
 @ingroup Mapper
 */
class Segmentation_EXPORT ContourVtkMapper3D : public VtkMapper3D
{
public:
  mitkClassMacro(ContourVtkMapper3D, VtkMapper3D);

  itkNewMacro(Self);

  virtual const mitk::Contour* GetInput();

  virtual vtkProp* GetVtkProp(mitk::BaseRenderer* renderer);

protected:
  ContourVtkMapper3D();
  virtual ~ContourVtkMapper3D();

  virtual void GenerateDataForRenderer(mitk::BaseRenderer* renderer);

  vtkSmartPointer<vtkPolyDataMapper> m_VtkPolyDataMapper;
  vtkSmartPointer<vtkTubeFilter>     m_TubeFilter;

  vtkSmartPointer<vtkAppendPolyData> m_VtkPointList;
  vtkSmartPointer<vtkPolyData> m_Contour;
  vtkSmartPointer<vtkActor> m_Actor;
};

} // namespace mitk

#endif // MITK_CONTOUR_VTK_MAPPER_3D_H

