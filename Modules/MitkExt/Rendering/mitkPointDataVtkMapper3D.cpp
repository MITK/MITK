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

#include "mitkPointDataVtkMapper3D.h"

#include <vtkActor.h>
#include <vtkPolyDataMapper.h>
#include <vtkSphereSource.h>
#include <vtkProperty.h>

mitk::PointDataVtkMapper3D::PointDataVtkMapper3D()
{
  int i;
    float x[6][3] = {{105,0,0},{-105,0,0},{0,105,0},{0,-105,0},{0,0,105},{0,0,-105}};
  vtkIdType pts[3][2] = {{0,1},{2,3},{4,5}};
  m_PointActor = vtkActor::New();
  m_PointMapper = vtkPolyDataMapper::New();

  m_PolyData = vtkPolyData::New();
  m_Points = vtkPoints::New();
  m_CellArray = vtkCellArray::New();

  for (i=0;i<6;i++) m_Points->InsertPoint(i,x[i]);
  for (i=0;i<3;i++) m_CellArray->InsertNextCell(2,pts[i]);

  m_PolyData->SetPoints(m_Points);
  m_PolyData->SetLines(m_CellArray);

  m_PointMapper->SetInput(m_PolyData);
  m_PointActor->SetMapper( m_PointMapper );
  m_PointActor->GetProperty()->SetColor(0.0,0.0,1.0);

}

mitk::PointDataVtkMapper3D::~PointDataVtkMapper3D()
{
  m_Points->Delete();
  m_CellArray->Delete();
  m_PolyData->Delete();
  m_PointMapper->Delete();
}

vtkProp* mitk::PointDataVtkMapper3D::GetVtkProp(mitk::BaseRenderer*  /*renderer*/)
{
  return m_PointActor;

}

void mitk::PointDataVtkMapper3D::GenerateData()
{
    // Get the input data object ( a mitk::PointData )
    mitk::PointData::Pointer input = dynamic_cast< mitk::PointData* >( this->GetData() );

  mitk::Point3D point = input->GetPoint3D();
//  MITK_INFO<<point[0]<<" "<<point[1]<<" "<<point[2]<<std::endl;

  m_PointActor->SetPosition( point[0], point[1], point[2] );

  m_PolyData->Update();
  
  SetVtkMapperImmediateModeRendering(m_PointMapper);

}
