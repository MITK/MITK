/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#include <mitkContourModelSetMapper3D.h>

#include "mitkContourModelColorHelper.h"

#include <vtkCellArray.h>
#include <vtkPoints.h>
#include <vtkPolyLine.h>
#include <vtkProperty.h>

mitk::ContourModelSetMapper3D::ContourModelSetMapper3D()
{
}

mitk::ContourModelSetMapper3D::~ContourModelSetMapper3D()
{
}

const mitk::ContourModelSet *mitk::ContourModelSetMapper3D::GetInput(void)
{
  // convenient way to get the data from the dataNode
  return static_cast<const mitk::ContourModelSet *>(GetDataNode()->GetData());
}

vtkProp *mitk::ContourModelSetMapper3D::GetVtkProp(mitk::BaseRenderer *renderer)
{
  // return the actor corresponding to the renderer
  return m_LSH.GetLocalStorage(renderer)->m_Actor;
}

void mitk::ContourModelSetMapper3D::GenerateDataForRenderer(mitk::BaseRenderer *renderer)
{
  LocalStorage *localStorage = m_LSH.GetLocalStorage(renderer);

  auto *contourModelSet = dynamic_cast<ContourModelSet *>(this->GetDataNode()->GetData());

  if (contourModelSet != nullptr)
  {
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> cells = vtkSmartPointer<vtkCellArray>::New();
    vtkIdType baseIndex = 0;

    auto it = contourModelSet->Begin();
    auto end = contourModelSet->End();

    while (it != end)
    {
      ContourModel *contourModel = it->GetPointer();

      auto vertIt = contourModel->Begin();
      auto vertEnd = contourModel->End();

      while (vertIt != vertEnd)
      {
        points->InsertNextPoint((*vertIt)->Coordinates[0], (*vertIt)->Coordinates[1], (*vertIt)->Coordinates[2]);
        ++vertIt;
      }

      vtkSmartPointer<vtkPolyLine> line = vtkSmartPointer<vtkPolyLine>::New();
      vtkIdList *pointIds = line->GetPointIds();

      vtkIdType numPoints = contourModel->GetNumberOfVertices();
      pointIds->SetNumberOfIds(numPoints + 1);

      for (vtkIdType i = 0; i < numPoints; ++i)
        pointIds->SetId(i, baseIndex + i);

      pointIds->SetId(numPoints, baseIndex);

      cells->InsertNextCell(line);

      baseIndex += numPoints;

      ++it;
    }

    vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
    polyData->SetPoints(points);
    polyData->SetLines(cells);

    localStorage->m_Mapper->SetInputData(polyData);
  }

  this->ApplyContourProperties(renderer);
  this->ApplyContourModelSetProperties(renderer);
}

void mitk::ContourModelSetMapper3D::Update(mitk::BaseRenderer *renderer)
{
  bool visible = true;
  GetDataNode()->GetVisibility(visible, renderer, "visible");

  auto *data = static_cast<mitk::ContourModel *>(GetDataNode()->GetData());
  if (data == nullptr)
  {
    return;
  }

  // Calculate time step of the input data for the specified renderer (integer value)
  this->CalculateTimeStep(renderer);

  LocalStorage *localStorage = m_LSH.GetLocalStorage(renderer);

  if (this->GetTimestep() == TIMESTEP_INVALID)
  {
    return;
  }

  const DataNode *node = this->GetDataNode();
  data->UpdateOutputInformation();

  // check if something important has changed and we need to rerender
  if ((localStorage->m_LastUpdateTime < node->GetMTime()) // was the node modified?
      ||
      (localStorage->m_LastUpdateTime < data->GetPipelineMTime()) // Was the data modified?
      ||
      (localStorage->m_LastUpdateTime <
       renderer->GetCurrentWorldPlaneGeometryUpdateTime()) // was the geometry modified?
      ||
      (localStorage->m_LastUpdateTime < renderer->GetCurrentWorldPlaneGeometry()->GetMTime()) ||
      (localStorage->m_LastUpdateTime < node->GetPropertyList()->GetMTime()) // was a property modified?
      ||
      (localStorage->m_LastUpdateTime < node->GetPropertyList(renderer)->GetMTime()))
  {
    this->GenerateDataForRenderer(renderer);
  }

  // since we have checked that nothing important has changed, we can set
  // m_LastUpdateTime to the current time
  localStorage->m_LastUpdateTime.Modified();
}

void mitk::ContourModelSetMapper3D::ApplyContourModelSetProperties(BaseRenderer *renderer)
{
  LocalStorage *localStorage = m_LSH.GetLocalStorage(renderer);

  float lineWidth = 1;
  this->GetDataNode()->GetFloatProperty("contour.3D.width", lineWidth, renderer);

  localStorage->m_Actor->GetProperty()->SetLineWidth(lineWidth);
}

void mitk::ContourModelSetMapper3D::ApplyContourProperties(mitk::BaseRenderer *renderer)
{
  LocalStorage *localStorage = m_LSH.GetLocalStorage(renderer);

  const auto color = GetContourColor(this->GetDataNode(), renderer);

  localStorage->m_Actor->GetProperty()->SetColor(color.GetRed(), color.GetGreen(), color.GetBlue());
}

/*+++++++++++++++++++ LocalStorage part +++++++++++++++++++++++++*/

mitk::ContourModelSetMapper3D::LocalStorage *mitk::ContourModelSetMapper3D::GetLocalStorage(
  mitk::BaseRenderer *renderer)
{
  return m_LSH.GetLocalStorage(renderer);
}

mitk::ContourModelSetMapper3D::LocalStorage::LocalStorage()
{
  m_Mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  m_Actor = vtkSmartPointer<vtkActor>::New();
  m_Actor->SetMapper(m_Mapper);
}

void mitk::ContourModelSetMapper3D::SetDefaultProperties(mitk::DataNode *node,
                                                         mitk::BaseRenderer *renderer,
                                                         bool overwrite)
{
  node->AddProperty("color", ColorProperty::New(0.9, 1.0, 0.1), renderer, overwrite);
  node->AddProperty("contour.3D.width", mitk::FloatProperty::New(0.5), renderer, overwrite);

  Superclass::SetDefaultProperties(node, renderer, overwrite);
}
