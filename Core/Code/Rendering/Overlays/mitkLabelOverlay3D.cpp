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

#include "mitkLabelOverlay3D.h"
#include <vtkTextProperty.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkPointData.h>


mitk::LabelOverlay3D::LabelOverlay3D()
{
}


mitk::LabelOverlay3D::~LabelOverlay3D()
{
}

mitk::LabelOverlay3D::LocalStorage::~LocalStorage()
{
}

mitk::LabelOverlay3D::LocalStorage::LocalStorage()
{

  m_Points = vtkSmartPointer<vtkPolyData>::New();

  // Add label array.
  m_Labels = vtkSmartPointer<vtkStringArray>::New();
  m_Labels->SetNumberOfValues(0);
  m_Labels->SetName("labels");

  // Add priority array.
  m_Sizes = vtkSmartPointer<vtkIntArray>::New();
  m_Sizes->SetNumberOfValues(0);
  m_Sizes->SetName("sizes");

  m_Points->GetPointData()->AddArray(m_Sizes);
  m_Points->GetPointData()->AddArray(m_Labels);

  m_PointSetToLabelHierarchyFilter = vtkSmartPointer<vtkPointSetToLabelHierarchy>::New();
  m_PointSetToLabelHierarchyFilter->SetInput(m_Points);
  m_PointSetToLabelHierarchyFilter->SetLabelArrayName("labels");
  m_PointSetToLabelHierarchyFilter->SetPriorityArrayName("sizes");
  m_PointSetToLabelHierarchyFilter->Update();

  m_LabelMapper = vtkSmartPointer<vtkLabelPlacementMapper>::New();
  m_LabelMapper->SetInputConnection(m_PointSetToLabelHierarchyFilter->GetOutputPort());

  m_LabelsActor = vtkSmartPointer<vtkActor2D>::New();
  m_LabelsActor->SetMapper(m_LabelMapper);
}

void mitk::LabelOverlay3D::UpdateVtkOverlay(mitk::BaseRenderer *renderer)
{
  if(m_LabelCoordinates.IsNull())
  {
    MITK_WARN << "No pointset defined to print labels!";
    return;
  }
  LocalStorage* ls = this->m_LSH.GetLocalStorage(renderer);

  vtkSmartPointer<vtkPoints> points =
    vtkSmartPointer<vtkPoints>::New();

  ls->m_Labels->SetNumberOfValues(m_LabelCoordinates->GetSize());
  ls->m_Sizes->SetNumberOfValues(m_LabelCoordinates->GetSize());

  for(int i=0 ; i<m_LabelCoordinates->GetSize() ; i++)
  {
    mitk::Point3D coordinate = m_LabelCoordinates->GetPoint(i);
    points->InsertNextPoint(coordinate[0]+GetOffsetVector(renderer)[0], coordinate[1]+GetOffsetVector(renderer)[1], coordinate[2]+GetOffsetVector(renderer)[2]);
    if(m_LabelVector.size()>i)
      ls->m_Labels->SetValue(i,m_LabelVector[i]);
    else ls->m_Labels->SetValue(i,"");

    if(m_PriorityVector.size()>i)
      ls->m_Sizes->SetValue(i,m_PriorityVector[i]);
    else ls->m_Sizes->SetValue(i,1);
  }

  ls->m_Points->SetPoints(points);
  ls->m_PointSetToLabelHierarchyFilter->Update();
  ls->m_LabelMapper->Update();

  float color[3] = {1,1,1};
  float opacity = 1.0;
  GetColor(color,renderer);
  GetOpacity(opacity,renderer);
  ls->m_LabelsActor->GetProperty()->SetColor(color[0], color[1], color[2]);
  ls->m_LabelsActor->GetProperty()->SetOpacity(opacity);
}


vtkSmartPointer<vtkProp> mitk::LabelOverlay3D::GetVtkProp(BaseRenderer *renderer)
{
  LocalStorage* ls = this->m_LSH.GetLocalStorage(renderer);
  return ls->m_LabelsActor;
}


void mitk::LabelOverlay3D::SetLabelVector(std::vector<const char*> LabelVector)
{
  m_LabelVector = LabelVector;
}

void mitk::LabelOverlay3D::SetPriorityVector(std::vector<int> PriorityVector)
{
  m_PriorityVector = PriorityVector;
}

void mitk::LabelOverlay3D::SetLabelCoordinates(mitk::PointSet::Pointer LabelCoordinates)
{
  m_LabelCoordinates = LabelCoordinates;
}
