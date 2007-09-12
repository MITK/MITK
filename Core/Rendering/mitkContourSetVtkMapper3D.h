/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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


#ifndef MITK_CONTOUR_SET_VTK_MAPPER_3D_H
#define MITK_CONTOUR_SET_VTK_MAPPER_3D_H

#include "mitkCommon.h"
#include "mitkBaseVtkMapper3D.h"
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
class ContourSetVtkMapper3D : public BaseVtkMapper3D
{
  public:

    mitkClassMacro(ContourSetVtkMapper3D, BaseVtkMapper3D);

    itkNewMacro(Self);

    virtual const mitk::ContourSet* GetInput();

  protected:
   
    ContourSetVtkMapper3D();
   
    virtual ~ContourSetVtkMapper3D();

    virtual void GenerateData(mitk::BaseRenderer* renderer);

    vtkPolyDataMapper* m_PointVtkPolyDataMapper;
    vtkPolyDataMapper* m_VtkPolyDataMapper;
    vtkTubeFilter*     m_TubeFilter;

		vtkPolyData *m_ContourSet;
    vtkActor *m_Actor;
  
};

} // namespace mitk

#endif // MITK_CONTOUR_VTK_MAPPER_3D_H
