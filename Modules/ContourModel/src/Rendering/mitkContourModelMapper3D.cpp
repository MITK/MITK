/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#include <mitkContourModelMapper3D.h>

#include "mitkContourModelColorHelper.h"

#include <vtkCellArray.h>
#include <vtkPoints.h>
#include <vtkProperty.h>

mitk::ContourModelMapper3D::ContourModelMapper3D()
{
}

mitk::ContourModelMapper3D::~ContourModelMapper3D()
{
}

const mitk::ContourModel *mitk::ContourModelMapper3D::GetInput(void)
{
  // convenient way to get the data from the dataNode
  return static_cast<const mitk::ContourModel *>(GetDataNode()->GetData());
}

vtkProp *mitk::ContourModelMapper3D::GetVtkProp(mitk::BaseRenderer *renderer)
{
  // return the actor corresponding to the renderer
  return m_LSH.GetLocalStorage(renderer)->m_Actor;
}

void mitk::ContourModelMapper3D::GenerateDataForRenderer(mitk::BaseRenderer *renderer)
{
  /* First convert the contourModel to vtkPolyData, then tube filter it and
   * set it input for our mapper
   */

  LocalStorage *localStorage = m_LSH.GetLocalStorage(renderer);

  auto *inputContour = static_cast<mitk::ContourModel *>(GetDataNode()->GetData());

  localStorage->m_OutlinePolyData = this->CreateVtkPolyDataFromContour(inputContour);

  this->ApplyContourProperties(renderer);

  // tube filter the polyData
  localStorage->m_TubeFilter->SetInputData(localStorage->m_OutlinePolyData);

  float lineWidth(1.0);
  if (this->GetDataNode()->GetFloatProperty("contour.3D.width", lineWidth, renderer))
  {
    localStorage->m_TubeFilter->SetRadius(lineWidth);
  }
  else
  {
    localStorage->m_TubeFilter->SetRadius(0.5);
  }
  localStorage->m_TubeFilter->CappingOn();
  localStorage->m_TubeFilter->SetNumberOfSides(10);
  localStorage->m_TubeFilter->Update();
  localStorage->m_Mapper->SetInputConnection(localStorage->m_TubeFilter->GetOutputPort());
}

void mitk::ContourModelMapper3D::Update(mitk::BaseRenderer *renderer)
{
  bool visible = true;
  GetDataNode()->GetVisibility(visible, renderer, "visible");

  // VtkPropRenderer::Update() walks every node regardless of visibility, so
  // without this a hidden contour keeps rebuilding its geometry.
  if (!visible)
    return;

  auto *data = static_cast<mitk::ContourModel *>(GetDataNode()->GetData());
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

  // "contour.3D.width" is the radius of the tube filter and therefore the one
  // property this mapper's geometry depends on.
  const auto *widthProperty = this->GetDataNode()->GetProperty("contour.3D.width", renderer);
  const bool widthModified = widthProperty != nullptr && localStorage->m_LastUpdateTime < widthProperty->GetMTime();

  // Rebuild the geometry only for what the geometry is made of. Note that a
  // property change also moves the node's own MTime, so that one must not be
  // part of this condition or every property change would rebuild.
  if (widthModified ||
      (localStorage->m_LastUpdateTime < data->GetMTime()) // was the data modified?
      ||
      (localStorage->m_LastUpdateTime < data->GetPipelineMTime()) // was the pipeline modified?
      ||
      (localStorage->m_LastUpdateTime < renderer->GetTimeStepUpdateTime())) // was the time step modified?
  {
    this->GenerateDataForRenderer(renderer);
    localStorage->m_LastUpdateTime.Modified();
  }

  // Every other property only has to be pushed to the actor.
  if ((localStorage->m_LastPropertyUpdateTime < node->GetPropertyList()->GetMTime()) ||
      (localStorage->m_LastPropertyUpdateTime < node->GetPropertyList(renderer)->GetMTime()))
  {
    this->ApplyContourProperties(renderer);
    localStorage->m_LastPropertyUpdateTime.Modified();
  }
}

vtkSmartPointer<vtkPolyData> mitk::ContourModelMapper3D::CreateVtkPolyDataFromContour(mitk::ContourModel *inputContour)
{
  const auto timestep = this->GetTimestep();

  // the points to draw
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  // the lines to connect the points
  vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New();
  // Create a polydata to store everything in
  vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();

  // iterate over the control points
  auto current = inputContour->IteratorBegin(timestep);
  auto next = inputContour->IteratorBegin(timestep);
  if (next != inputContour->IteratorEnd(timestep))
  {
    next++;

    auto end = inputContour->IteratorEnd(timestep);

    while (next != end)
    {
      mitk::ContourModel::VertexType *currentControlPoint = *current;
      mitk::ContourModel::VertexType *nextControlPoint = *next;

      if (!(currentControlPoint->Coordinates[0] == nextControlPoint->Coordinates[0] &&
            currentControlPoint->Coordinates[1] == nextControlPoint->Coordinates[1] &&
            currentControlPoint->Coordinates[2] == nextControlPoint->Coordinates[2]))
      {
        vtkIdType p1 = points->InsertNextPoint(currentControlPoint->Coordinates[0],
                                               currentControlPoint->Coordinates[1],
                                               currentControlPoint->Coordinates[2]);
        vtkIdType p2 = points->InsertNextPoint(
          nextControlPoint->Coordinates[0], nextControlPoint->Coordinates[1], nextControlPoint->Coordinates[2]);
        // add the line between both contorlPoints
        lines->InsertNextCell(2);
        lines->InsertCellPoint(p1);
        lines->InsertCellPoint(p2);
      }
      current++;
      next++;
    }

    if (inputContour->IsClosed(timestep))
    {
      // If the contour is closed add a line from the last to the first control point
      mitk::ContourModel::VertexType *firstControlPoint = *(inputContour->IteratorBegin(timestep));
      mitk::ContourModel::VertexType *lastControlPoint = *(--(inputContour->IteratorEnd(timestep)));
      if (lastControlPoint->Coordinates[0] != firstControlPoint->Coordinates[0] ||
          lastControlPoint->Coordinates[1] != firstControlPoint->Coordinates[1] ||
          lastControlPoint->Coordinates[2] != firstControlPoint->Coordinates[2])
      {
        vtkIdType p2 = points->InsertNextPoint(
          lastControlPoint->Coordinates[0], lastControlPoint->Coordinates[1], lastControlPoint->Coordinates[2]);
        vtkIdType p1 = points->InsertNextPoint(
          firstControlPoint->Coordinates[0], firstControlPoint->Coordinates[1], firstControlPoint->Coordinates[2]);

        // add the line to the cellArray
        lines->InsertNextCell(2);
        lines->InsertCellPoint(p1);
        lines->InsertCellPoint(p2);
      }
    }

    // Add the points to the dataset
    polyData->SetPoints(points);
    // Add the lines to the dataset
    polyData->SetLines(lines);
  }
  return polyData;
}

void mitk::ContourModelMapper3D::ApplyContourProperties(mitk::BaseRenderer *renderer)
{
  LocalStorage *localStorage = m_LSH.GetLocalStorage(renderer);

  const auto color = GetContourColor(this->GetDataNode(), renderer);

  localStorage->m_Actor->GetProperty()->SetColor(color.GetRed(), color.GetGreen(), color.GetBlue());
}

/*+++++++++++++++++++ LocalStorage part +++++++++++++++++++++++++*/

mitk::ContourModelMapper3D::LocalStorage *mitk::ContourModelMapper3D::GetLocalStorage(mitk::BaseRenderer *renderer)
{
  return m_LSH.GetLocalStorage(renderer);
}

mitk::ContourModelMapper3D::LocalStorage::LocalStorage()
{
  m_Mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  m_Actor = vtkSmartPointer<vtkActor>::New();
  m_OutlinePolyData = vtkSmartPointer<vtkPolyData>::New();
  m_TubeFilter = vtkSmartPointer<vtkTubeFilter>::New();

  // set the mapper for the actor
  m_Actor->SetMapper(m_Mapper);
}

void mitk::ContourModelMapper3D::SetDefaultProperties(mitk::DataNode *node,
                                                      mitk::BaseRenderer *renderer,
                                                      bool overwrite)
{
  node->AddProperty("color", ColorProperty::New(0.9, 1.0, 0.1), renderer, overwrite);
  node->AddProperty("contour.3D.width", mitk::FloatProperty::New(0.5), renderer, overwrite);

  Superclass::SetDefaultProperties(node, renderer, overwrite);
}
