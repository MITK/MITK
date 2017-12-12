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

#include "mitkLabelAnnotation3D.h"
#include <mitkPointSet.h>
#include <vtkActor2D.h>
#include <vtkIntArray.h>
#include <vtkLabelPlacementMapper.h>
#include <vtkPointData.h>
#include <vtkPointSetToLabelHierarchy.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty2D.h>
#include <vtkStringArray.h>
#include <vtkTextProperty.h>

mitk::LabelAnnotation3D::LabelAnnotation3D() : m_PointSetModifiedObserverTag(0)
{
}

mitk::LabelAnnotation3D::~LabelAnnotation3D()
{
  if (m_LabelCoordinates.IsNotNull())
    m_LabelCoordinates->RemoveObserver(m_PointSetModifiedObserverTag);
  for (BaseRenderer *renderer : m_LSH.GetRegisteredBaseRenderer())
  {
    if (renderer)
    {
      this->RemoveFromBaseRenderer(renderer);
    }
  }
}

mitk::LabelAnnotation3D::LocalStorage::~LocalStorage()
{
}

mitk::LabelAnnotation3D::LocalStorage::LocalStorage()
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
  m_PointSetToLabelHierarchyFilter->SetInputData(m_Points);
  m_PointSetToLabelHierarchyFilter->SetLabelArrayName("labels");
  m_PointSetToLabelHierarchyFilter->SetPriorityArrayName("sizes");
  m_PointSetToLabelHierarchyFilter->Update();

  m_LabelMapper = vtkSmartPointer<vtkLabelPlacementMapper>::New();
  m_LabelMapper->SetInputConnection(m_PointSetToLabelHierarchyFilter->GetOutputPort());

  m_LabelsActor = vtkSmartPointer<vtkActor2D>::New();
  m_LabelsActor->SetMapper(m_LabelMapper);
}

void mitk::LabelAnnotation3D::UpdateVtkAnnotation(mitk::BaseRenderer *renderer)
{
  if (m_LabelCoordinates.IsNull())
  {
    MITK_WARN << "No pointset defined to print labels!";
    return;
  }
  LocalStorage *ls = this->m_LSH.GetLocalStorage(renderer);

  if (ls->IsGenerateDataRequired(renderer, this))
  {
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();

    auto pointsetsize = (size_t)m_LabelCoordinates->GetSize();
    ls->m_Labels->SetNumberOfValues(pointsetsize);
    ls->m_Sizes->SetNumberOfValues(pointsetsize);

    for (size_t i = 0; i < pointsetsize; i++)
    {
      mitk::Point3D coordinate = m_LabelCoordinates->GetPoint(i);
      points->InsertNextPoint(coordinate[0] + GetOffsetVector()[0],
                              coordinate[1] + GetOffsetVector()[1],
                              coordinate[2] + GetOffsetVector()[2]);
      if (m_LabelVector.size() > i)
        ls->m_Labels->SetValue(i, m_LabelVector[i]);
      else
        ls->m_Labels->SetValue(i, "");

      if (m_PriorityVector.size() > i)
        ls->m_Sizes->SetValue(i, m_PriorityVector[i]);
      else
        ls->m_Sizes->SetValue(i, 1);
    }

    ls->m_Points->SetPoints(points);
    ls->m_PointSetToLabelHierarchyFilter->Update();
    ls->m_LabelMapper->Update();

    float color[3] = {1, 1, 1};
    float opacity = 1.0;
    GetColor(color);
    GetOpacity(opacity);
    ls->m_LabelsActor->GetProperty()->SetColor(color[0], color[1], color[2]);
    ls->m_LabelsActor->GetProperty()->SetOpacity(opacity);
    ls->UpdateGenerateDataTime();
  }
}

vtkProp *mitk::LabelAnnotation3D::GetVtkProp(BaseRenderer *renderer) const
{
  LocalStorage *ls = this->m_LSH.GetLocalStorage(renderer);
  return ls->m_LabelsActor;
}

void mitk::LabelAnnotation3D::SetLabelVector(const std::vector<std::string> &LabelVector)
{
  m_LabelVector = LabelVector;
  this->Modified();
}

void mitk::LabelAnnotation3D::SetPriorityVector(const std::vector<int> &PriorityVector)
{
  m_PriorityVector = PriorityVector;
  this->Modified();
}

void mitk::LabelAnnotation3D::SetLabelCoordinates(mitk::PointSet::Pointer LabelCoordinates)
{
  if (m_LabelCoordinates.IsNotNull())
  {
    m_LabelCoordinates->RemoveObserver(m_PointSetModifiedObserverTag);
    m_PointSetModifiedObserverTag = 0;
    m_LabelCoordinates = nullptr;
  }
  if (LabelCoordinates.IsNull())
  {
    return;
  }
  m_LabelCoordinates = LabelCoordinates;
  itk::MemberCommand<mitk::LabelAnnotation3D>::Pointer _PropertyListModifiedCommand =
    itk::MemberCommand<mitk::LabelAnnotation3D>::New();
  _PropertyListModifiedCommand->SetCallbackFunction(this, &mitk::LabelAnnotation3D::PointSetModified);
  m_PointSetModifiedObserverTag = m_LabelCoordinates->AddObserver(itk::ModifiedEvent(), _PropertyListModifiedCommand);
  this->Modified();
}

void mitk::LabelAnnotation3D::PointSetModified(const itk::Object * /*caller*/, const itk::EventObject &)
{
  this->Modified();
}
