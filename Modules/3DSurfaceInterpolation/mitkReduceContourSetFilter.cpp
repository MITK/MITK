/*=========================================================================
Program: Medical Imaging & Interaction Toolkit
Module: $RCSfile$
Language: C++
Date: $Date: $
Version: $Revision: $
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE. See the above copyright notices for more information.
=========================================================================*/

#include "mitkReduceContourSetFilter.h"
#include "mitkSurface.h"
#include "vtkPolygon.h"
#include "vtkPoints.h"
#include "vtkCellArray.h"
#include "vtkMath.h"

mitk::ReduceContourSetFilter::ReduceContourSetFilter()
{
}

mitk::ReduceContourSetFilter::~ReduceContourSetFilter()
{
}

void mitk::ReduceContourSetFilter::GenerateData()
{
  unsigned int numberOfInputs = this->GetNumberOfInputs();
  this->CreateOutputsForAllInputs(numberOfInputs);

  for(unsigned int i = 0; i < numberOfInputs; i++)
  {
    Surface* currentSurface = const_cast<Surface*>( this->GetInput(i) );
    vtkPolyData* polyData = currentSurface->GetVtkPolyData();
    vtkSmartPointer<vtkPolyData> newPolyData = this->ReduceNumberOfPoints(polyData, i);
    Surface::Pointer surface = this->GetOutput(i);
    surface->SetVtkPolyData(newPolyData);


  }
}

vtkPolyData* mitk::ReduceContourSetFilter::ReduceNumberOfPoints (vtkPolyData* polyData, unsigned int currentInputIndex)
{
  vtkSmartPointer<vtkPolyData> newPolyData = vtkPolyData::New();
  vtkSmartPointer<vtkCellArray> newPolygons = vtkCellArray::New();
  vtkSmartPointer<vtkPoints> newPoints = vtkPoints::New();

  vtkSmartPointer<vtkCellArray> existingPolys = polyData->GetPolys();

  vtkSmartPointer<vtkPoints> existingPoints = polyData->GetPoints();

  existingPolys->InitTraversal();

  vtkIdType* cell (NULL);
  vtkIdType cellSize (0);  

  unsigned int newPointIndex (0);

  for( existingPolys->InitTraversal(); existingPolys->GetNextCell(cellSize, cell);)
  {

    //TODO implement functionality for detection intersection point so that these points won't be eliminated
    vtkIdType numberOfIntersections (0);
    vtkIdType* intersectionPoints (0);
      bool incorporatePolygon = this->CheckForIntersection(cell,cellSize,existingPoints, /*numberOfIntersections, intersectionPoints, */currentInputIndex);
    if ( !incorporatePolygon ) continue;
 
    vtkSmartPointer<vtkPolygon> newPolygon = vtkPolygon::New();
    unsigned int newNumberOfPoints (0);
    unsigned int mod = cellSize%20;

    if(mod == 0)
    {
      newNumberOfPoints = cellSize/20 + numberOfIntersections;
    }
    else
    {
      newNumberOfPoints = ( (cellSize-mod)/20 )+1 + numberOfIntersections;
    }

    if (newNumberOfPoints <= 2) continue;
    newPolygon->GetPointIds()->SetNumberOfIds(newNumberOfPoints);
    newPolygon->GetPoints()->SetNumberOfPoints(newNumberOfPoints);

    for (unsigned int i = 0; i < cellSize; i++)
    {
        //TODO implement functionality for detection intersection point so that these points won't be eliminated
        /*bool isIntersectionPoint = false;
        for (unsigned int j = 0; j < numberOfIntersections; j++)
        {
            if (cell[i] == intersectionPoints[j]) {
                isIntersectionPoint = true;
                break;
            }
        }*/
      if (i%20 == 0 /*|| isIntersectionPoint*/)
      {
        double point[3];
        existingPoints->GetPoint(cell[i], point);
        newPoints->InsertPoint(newPointIndex, point);
        newPolygon->GetPointIds()->SetId(i/20, newPointIndex);
        newPointIndex++;
      }

    }
    vtkIdType id =  cell[0];
    double point[3];
    existingPoints->GetPoint(id, point);
    newPoints->InsertPoint(newPointIndex, point);
    newPolygon->GetPointIds()->SetId(newNumberOfPoints-1, newPointIndex);
    newPointIndex++;

    newPolygons->InsertNextCell(newPolygon);
  }

  //TODO Hier überprüfen ob Punkte gesetzt sind evtl. sogar in der View nach Anzahl Punkten schauen
  newPolyData->SetPoints( newPoints );
  newPolyData->SetPolys( newPolygons );
  newPolyData->BuildLinks();

  return newPolyData;
}

bool mitk::ReduceContourSetFilter::CheckForIntersection (vtkIdType* currentCell, vtkIdType currentCellSize, vtkPoints* currentPoints,/* vtkIdType numberOfIntersections, vtkIdType* intersectionPoints,*/ unsigned int currentInputIndex)
{
  /*
    If we check the current cell for intersections then we have to consider three possibilies:
      1. There is another cell among all the other input surfaces which intersects the current polygon:
         - That means we have to save the intersection points because these points should not be eliminated
      2. There current polygon exists just because of an intersection of another polygon with the current plane defined by the current polygon
         - That means the current polygon should not be incorporated and all of its points should be eliminated
      3. There is no intersection
         - That mean we can just reduce the current polygons points without considering any intersections
  */

  //TODO implement functionality for detection intersection point so that these points won't be eliminated
  //std::vector<vtkIdType> intersectionPointsTemp;

  for (unsigned int i = 0; i < this->GetNumberOfInputs(); i++)
  {
    //Don't check for intersection with the polygon itself
    if (i == currentInputIndex) continue;

    //Get the next polydata to check for intersection
    vtkSmartPointer<vtkPolyData> poly = const_cast<Surface*>( this->GetInput(i) )->GetVtkPolyData();
    vtkSmartPointer<vtkCellArray> polygonArray = poly->GetPolys();
    polygonArray->InitTraversal();
    vtkIdType anotherInputPolygonSize (0);
    vtkIdType* anotherInputPolygonIDs(NULL);

    /*
    The procedure is:
    - Create the equation of the plane, defined by the points of next input
    - Calculate the distance of each point of the current polygon to the plane
    - If the maximum distance is not bigger than 1.5 of the maximum spacing AND the minimal distance is not bigger
      than 0.5 of the minimum spacing then the current contour is an intersection contour
    */
    
    for( polygonArray->InitTraversal(); polygonArray->GetNextCell(anotherInputPolygonSize, anotherInputPolygonIDs);)
    {
      //Choosing three plane points to calculate the plane vectors
      double p1[3];
      double p2[3];
      double p3[3];

      //The plane vectors
      double v1[3];
      double v2[3];
      //The plane normal
      double normal[3];

      //Create first Vector
      poly->GetPoint(anotherInputPolygonIDs[0], p1);
      poly->GetPoint(anotherInputPolygonIDs[1], p2);

      v1[0] = p2[0]-p1[0];
      v1[1] = p2[1]-p1[1];
      v1[2] = p2[2]-p1[2];

      //Find 3rd point for 2nd vector (The angle between the two plane vectors should be bigger than 30 degrees)
        
      double maxDistance (0);
      double minDistance (10000);
        
      for (unsigned int j = 2; j < anotherInputPolygonSize; j++)
      {
        poly->GetPoint(anotherInputPolygonIDs[j], p3);

        v2[0] = p3[0]-p1[0];
        v2[1] = p3[1]-p1[1];
        v2[2] = p3[2]-p1[2];

        //Calculate the angle between the two vector for the current point
        double dotV1V2 = vtkMath::Dot(v1,v2);
        double absV1 = sqrt(vtkMath::Dot(v1,v1));
        double absV2 = sqrt(vtkMath::Dot(v2,v2));
        double cosV1V2 = dotV1V2/(absV1*absV2);

        double arccos = acos(cosV1V2);
        double degree = vtkMath::DegreesFromRadians(arccos);

        //If angle is bigger than 30 degrees break
        if (degree > 30) break;

      }//for (to find 3rd point)

      //Calculate normal of the plane by taking the cross product of the two vectors
      vtkMath::Cross(v1,v2,normal);
      vtkMath::Normalize(normal);

      //Determine position of the plane
      double lambda = vtkMath::Dot(normal, p1);

      /*
      Calculate the distance to the plane for each point of the current polygon
      If the distance is zero then save the currentPoint as intersection point
      */
      for (unsigned int k = 0; k < currentCellSize; k++)
      {
        double currentPoint[3];
        currentPoints->GetPoint(currentCell[k], currentPoint);

        double tempPoint[3];
        tempPoint[0] = normal[0]*currentPoint[0];
        tempPoint[1] = normal[1]*currentPoint[1];
        tempPoint[2] = normal[2]*currentPoint[2];

        double temp = tempPoint[0]+tempPoint[1]+tempPoint[2]-lambda;
        double distance = fabs(temp);

        if (distance > maxDistance)
        {
          maxDistance = distance;
        }
        if (distance < minDistance)
        {
          minDistance = distance;
        }
      }//for (to calculate distance and intersections with currentPolygon)
        
      if (maxDistance < 1.5*m_MaxSpacing && minDistance < 0.5*m_MinSpacing)
      {
        return false;
      }
        
      //Because we are considering the plane defined by the acual input polygon only one iteration is sufficient
      //We do not need to consider each cell of the plane
      break;
    }//for (to traverse through all cells of actualInputPolyData)

  }//for (to iterate through all inputs)
  
  return true;
}

void mitk::ReduceContourSetFilter::GenerateOutputInformation()
{
  Superclass::GenerateOutputInformation();
}
