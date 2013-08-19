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
#include <mitkContourModelToSurfaceFilter.h>
#include <mitkSurface.h>
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>

mitk::ContourModelToSurfaceFilter::ContourModelToSurfaceFilter()
{

}
mitk::ContourModelToSurfaceFilter::~ContourModelToSurfaceFilter()
{

}

void mitk::ContourModelToSurfaceFilter::SetInput ( const mitk::ContourModelToSurfaceFilter::InputType* input )
{
  this->SetInput( 0, input );
}

void mitk::ContourModelToSurfaceFilter::SetInput ( unsigned int idx, const mitk::ContourModelToSurfaceFilter::InputType* input )
{
  if ( idx + 1 > this->GetNumberOfInputs() )
  {
    this->SetNumberOfRequiredInputs(idx + 1);
  }
  if ( input != static_cast<InputType*> ( this->ProcessObject::GetInput ( idx ) ) )
  {
    this->ProcessObject::SetNthInput ( idx, const_cast<InputType*> ( input ) );
    this->Modified();
  }
}



const mitk::ContourModelToSurfaceFilter::InputType* mitk::ContourModelToSurfaceFilter::GetInput( void )
{
  if (this->GetNumberOfInputs() < 1)
    return NULL;
  return static_cast<const mitk::ContourModelToSurfaceFilter::InputType*>(this->ProcessObject::GetInput(0));
}



const mitk::ContourModelToSurfaceFilter::InputType* mitk::ContourModelToSurfaceFilter::GetInput( unsigned int idx )
{
  if (this->GetNumberOfInputs() < 1)
    return NULL;
  return static_cast<const mitk::ContourModelToSurfaceFilter::InputType*>(this->ProcessObject::GetInput(idx));
}



void mitk::ContourModelToSurfaceFilter::GenerateData()
{
  mitk::Surface* surface = this->GetOutput();
  mitk::ContourModel* inputContour = (mitk::ContourModel*)GetInput();

  unsigned int timestep = inputContour->GetTimeSteps();

  /* First of all convert the control points of the contourModel to vtk points
   * and add lines in between them
   */
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New(); //the points to draw
  vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New(); //the lines to connect the points

  //iterate over all control points
  mitk::ContourModel::VertexIterator current = inputContour->IteratorBegin(timestep);
  mitk::ContourModel::VertexIterator next = inputContour->IteratorBegin(timestep);
  next++;

  mitk::ContourModel::VertexIterator end = inputContour->IteratorEnd(timestep);

  while(next != end)
  {
    mitk::ContourModel::VertexType* currentControlPoint = *current;
    mitk::ContourModel::VertexType* nextControlPoint = *next;

    vtkIdType p1 = points->InsertNextPoint(currentControlPoint->Coordinates[0], currentControlPoint->Coordinates[1], currentControlPoint->Coordinates[2]);
    vtkIdType p2 = points->InsertNextPoint(nextControlPoint->Coordinates[0], nextControlPoint->Coordinates[1], nextControlPoint->Coordinates[2]);
    //add the line between both contorlPoints
    lines->InsertNextCell(2);
    lines->InsertCellPoint(p1);
    lines->InsertCellPoint(p2);

    current++;
    next++;
  }

  /* If the contour is closed an additional line has to be created between the very first point
   * and the last point
   */
  if(inputContour->IsClosed(timestep))
  {
    //add a line from the last to the first control point
    mitk::ContourModel::VertexType* firstControlPoint = *(inputContour->IteratorBegin(timestep));
    mitk::ContourModel::VertexType* lastControlPoint = *(--(inputContour->IteratorEnd(timestep)));
    vtkIdType p2 = points->InsertNextPoint(lastControlPoint->Coordinates[0], lastControlPoint->Coordinates[1], lastControlPoint->Coordinates[2]);
    vtkIdType p1 = points->InsertNextPoint(firstControlPoint->Coordinates[0], firstControlPoint->Coordinates[1], firstControlPoint->Coordinates[2]);

    //add the line between both contorlPoints
    lines->InsertNextCell(2);
    lines->InsertCellPoint(p1);
    lines->InsertCellPoint(p2);
  }

  // Create a polydata to store everything in
  vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
  // Add the points to the dataset
  polyData->SetPoints(points);
  // Add the lines to the dataset
  polyData->SetLines(lines);

  surface->SetVtkPolyData(polyData);
}
