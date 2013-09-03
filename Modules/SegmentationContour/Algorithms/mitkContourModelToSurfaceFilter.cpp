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
#include <vtkPolygon.h>

mitk::ContourModelToSurfaceFilter::ContourModelToSurfaceFilter()
{
  this->SetNthOutput(0,mitk::Surface::New().GetPointer());
}
mitk::ContourModelToSurfaceFilter::~ContourModelToSurfaceFilter()
{

}


void mitk::ContourModelToSurfaceFilter::GenerateOutputInformation()
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

  unsigned int numberOfTimeSteps = inputContour->GetTimeSteps();
  surface->Expand(numberOfTimeSteps);

  for(unsigned int currentTimeStep = 0; currentTimeStep < numberOfTimeSteps; currentTimeStep++)
  {

    /* First of all convert the control points of the contourModel to vtk points
    * and add lines in between them
    */
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New(); //the points to draw
    vtkSmartPointer<vtkCellArray> polygons = vtkSmartPointer<vtkCellArray>::New();

    //iterate over all control points
    mitk::ContourModel::VertexIterator current = inputContour->IteratorBegin(currentTimeStep);

    mitk::ContourModel::VertexIterator end = inputContour->IteratorEnd(currentTimeStep);

    vtkSmartPointer<vtkPolygon> polygon = vtkSmartPointer<vtkPolygon>::New();
    polygon->GetPointIds()->SetNumberOfIds(inputContour->GetNumberOfVertices(currentTimeStep));

    int j(0);
    while(current != end)
    {
      mitk::ContourModel::VertexType* currentPoint = *current;

      vtkIdType id = points->InsertNextPoint(currentPoint->Coordinates[0], currentPoint->Coordinates[1], currentPoint->Coordinates[2]);

      polygon->GetPointIds()->SetId(j,id);

      current++;
      j++;
    }

    polygons->InsertNextCell(polygon);


    // Create a polydata to store everything in
    vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
    // Add the points to the dataset
    polyData->SetPoints(points);
    polyData->SetPolys(polygons);
    polyData->BuildLinks();

    surface->SetVtkPolyData(polyData, currentTimeStep);
  }
}
