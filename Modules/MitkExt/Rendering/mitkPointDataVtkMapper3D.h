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

#ifndef _MITK_POINT_DATA_VTK_MAPPER_3D__H_
#define _MITK_POINT_DATA_VTK_MAPPER_3D__H_

#include <mitkBaseVtkMapper3D.h>
#include "mitkPointData.h"
#include "vtkCellArray.h"
#include "vtkFloatArray.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"


class vtkActor;
class vtkPolyDataMapper;
class vtkSphereSource;
class vtkPolyData;
class vtkPoints;
class vtkCellArray;

namespace mitk
{

class MITKEXT_CORE_EXPORT PointDataVtkMapper3D : public BaseVtkMapper3D
{
public:

    mitkClassMacro( PointDataVtkMapper3D, BaseVtkMapper3D );

    itkNewMacro( Self );

    itkSetMacro( Radius, float);

    itkGetMacro( Radius, float);
    
    virtual vtkProp* GetVtkProp(mitk::BaseRenderer* renderer);

protected:

    PointDataVtkMapper3D();

    virtual ~PointDataVtkMapper3D();

    virtual void GenerateData();

    vtkActor* m_PointActor;

    vtkPolyDataMapper* m_PointMapper;

    vtkPolyData* m_PolyData;
    vtkPoints* m_Points;
    vtkCellArray* m_CellArray;

    float m_Radius;
};


} //namespace mitk


#endif


