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
    const auto timestep = this->GetTimestep();

    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> cells = vtkSmartPointer<vtkCellArray>::New();
    vtkIdType baseIndex = 0;

    auto it = contourModelSet->Begin();
    auto end = contourModelSet->End();

    while (it != end)
    {
      ContourModel *contourModel = it->GetPointer();

      // Fewer time steps than the set yields a negative count, a step that was
      // never filled yields zero. Neither makes a polyline, and a cell holding
      // a single vertex would index past the end of the points.
      const vtkIdType numPoints = contourModel->GetNumberOfVertices(timestep);

      if (numPoints < 1)
      {
        ++it;
        continue;
      }

      auto vertIt = contourModel->IteratorBegin(timestep);
      auto vertEnd = contourModel->IteratorEnd(timestep);

      while (vertIt != vertEnd)
      {
        points->InsertNextPoint((*vertIt)->Coordinates[0], (*vertIt)->Coordinates[1], (*vertIt)->Coordinates[2]);
        ++vertIt;
      }

      const bool isClosed = contourModel->IsClosed(timestep);

      vtkSmartPointer<vtkPolyLine> line = vtkSmartPointer<vtkPolyLine>::New();
      vtkIdList *pointIds = line->GetPointIds();

      pointIds->SetNumberOfIds(isClosed ? numPoints + 1 : numPoints);

      for (vtkIdType i = 0; i < numPoints; ++i)
        pointIds->SetId(i, baseIndex + i);

      // Back to the first vertex, which is where a closed contour ends.
      if (isClosed)
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

  // VtkPropRenderer::Update() walks every node regardless of visibility, so
  // without this a hidden contour keeps rebuilding its geometry.
  if (!visible)
    return;

  auto *data = GetDataNode()->GetData();
  if (data == nullptr)
  {
    return;
  }

  // Calculate time step of the input data for the specified renderer (integer value)
  this->CalculateTimeStep(renderer);

  LocalStorage *localStorage = m_LSH.GetLocalStorage(renderer);

  // Check if time step is valid
  const TimeGeometry *dataTimeGeometry = data->GetTimeGeometry();
  if ((dataTimeGeometry == nullptr) || (dataTimeGeometry->CountTimeSteps() == 0) ||
      (!dataTimeGeometry->IsValidTimePoint(renderer->GetTime())) || (this->GetTimestep() == TIMESTEP_INVALID))
  {
    // clear the rendered polydata
    localStorage->m_Mapper->SetInputData(vtkSmartPointer<vtkPolyData>::New());
    return;
  }

  const DataNode *node = this->GetDataNode();
  data->UpdateOutputInformation();

  // Rebuild the geometry only for what the geometry is made of. Note that a
  // property change also moves the node's own MTime, so that one must not be
  // part of this condition or every property change would rebuild.
  if ((localStorage->m_LastUpdateTime < data->GetMTime()) // was the data modified?
      ||
      (localStorage->m_LastUpdateTime < data->GetPipelineMTime()) // was the pipeline modified?
      ||
      (localStorage->m_LastUpdateTime <
       renderer->GetCurrentWorldPlaneGeometryUpdateTime()) // was the geometry modified?
      ||
      (localStorage->m_LastUpdateTime < renderer->GetCurrentWorldPlaneGeometry()->GetMTime()) ||
      (localStorage->m_LastUpdateTime < renderer->GetTimeStepUpdateTime())) // was the time step modified?
  {
    this->GenerateDataForRenderer(renderer);
    localStorage->m_LastUpdateTime.Modified();
  }

  // None of the properties of this mapper affect the generated geometry, so a
  // property change only has to be pushed to the actor.
  if ((localStorage->m_LastPropertyUpdateTime < node->GetPropertyList()->GetMTime()) ||
      (localStorage->m_LastPropertyUpdateTime < node->GetPropertyList(renderer)->GetMTime()))
  {
    this->ApplyContourProperties(renderer);
    this->ApplyContourModelSetProperties(renderer);
    localStorage->m_LastPropertyUpdateTime.Modified();
  }
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
