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


#ifndef MITKMESHVTKMAPPER3D_H_HEADER_INCLUDED
#define MITKMESHVTKMAPPER3D_H_HEADER_INCLUDED

#include "mitkCommon.h"
#include "mitkBaseVtkMapper3D.h"
#include "mitkMesh.h"
#include "mitkBaseRenderer.h"

#include <vtkSphereSource.h>
#include <vtkAppendPolyData.h>
#include <vtkPolyData.h>
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkCellArray.h>
#include <vtkFloatArray.h>
#include <vtkTubeFilter.h>
#include <vtkVectorText.h>
#include <vtkTextSource.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>


class vtkActor;
class vtkAssembly;
class vtkFollower;
class vtkPolyDataMapper;
class vtkPropAssembly;

namespace mitk {

//##Documentation
//## @brief Vtk-based mapper for PointList
//## @ingroup Mapper
class MeshVtkMapper3D : public BaseVtkMapper3D
{
  public:

    mitkClassMacro(MeshVtkMapper3D, BaseVtkMapper3D);

    itkNewMacro(Self);

    virtual const mitk::Mesh* GetInput();

    virtual vtkProp* GetProp();

  protected:
    MeshVtkMapper3D();

    virtual ~MeshVtkMapper3D();

    virtual void GenerateData(mitk::BaseRenderer* renderer);

    vtkPropAssembly* m_PropAssemply;

    vtkActor *m_SpheresActor;
    vtkActor *m_ContourActor;
    vtkPolyDataMapper* m_ContourMapper;
    vtkPolyDataMapper* m_SpheresMapper;

    vtkPolyDataMapper* m_TextVtkPolyDataMapper;

		vtkAppendPolyData *m_Spheres;
		vtkAppendPolyData *m_vtkTextList;
		vtkPolyData *m_Contour;
    vtkTubeFilter * m_tubefilter;
};

} // namespace mitk

#endif /* MITKMESHVTKMAPPER3D_H_HEADER_INCLUDED*/
