/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef MITK_CONTOUR_VTK_MAPPER_3D_H
#define MITK_CONTOUR_VTK_MAPPER_3D_H

#include "MitkExtExports.h"
#include "mitkBaseVtkMapper3D.h"

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
class MitkExt_EXPORT ContourVtkMapper3D : public BaseVtkMapper3D
{
public:
  mitkClassMacro(ContourVtkMapper3D, BaseVtkMapper3D);

  itkNewMacro(Self);

  virtual const mitk::Contour* GetInput();

  virtual vtkProp* GetVtkProp(mitk::BaseRenderer* renderer);

protected:
  ContourVtkMapper3D();
  virtual ~ContourVtkMapper3D();

  virtual void GenerateData(mitk::BaseRenderer* renderer);

  vtkSmartPointer<vtkPolyDataMapper> m_VtkPolyDataMapper;
  vtkSmartPointer<vtkTubeFilter>     m_TubeFilter;

  vtkSmartPointer<vtkAppendPolyData> m_VtkPointList;
  vtkSmartPointer<vtkPolyData> m_Contour;
  vtkSmartPointer<vtkActor> m_Actor;
};

} // namespace mitk

#endif // MITK_CONTOUR_VTK_MAPPER_3D_H

