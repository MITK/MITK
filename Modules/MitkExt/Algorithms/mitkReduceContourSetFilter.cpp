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

mitk::ReduceContourSetFilter::ReduceContourSetFilter()
{
  m_MaxSegmentLenght = 0;
  m_StepSize = 10;
  m_Tolerance = -1;
  m_ReductionType = DOUGLAS_PEUCKER;
  m_MaxSpacing = -1;
  m_MinSpacing = -1;
}

mitk::ReduceContourSetFilter::~ReduceContourSetFilter()
{
}

void mitk::ReduceContourSetFilter::GenerateData()
{
  unsigned int numberOfInputs = this->GetNumberOfInputs();
  unsigned int numberOfOutputs (0);
  this->CreateOutputsForAllInputs(numberOfInputs);

  vtkSmartPointer<vtkPolyData> newPolyData;
  vtkSmartPointer<vtkCellArray> newPolygons;
  vtkSmartPointer<vtkPoints> newPoints;

  //For the purpose of evaluation
  //unsigned int numberOfPointsBefore (0);
  //unsigned int numberOfPointsAfter (0);

  for(unsigned int i = 0; i < numberOfInputs; i++)
  {
    mitk::Surface* currentSurface = const_cast<mitk::Surface*>( this->GetInput(i) );
    vtkSmartPointer<vtkPolyData> polyData = currentSurface->GetVtkPolyData();

    newPolyData = vtkPolyData::New();
    newPolygons = vtkCellArray::New();
    newPoints = vtkPoints::New();

    vtkSmartPointer<vtkCellArray> existingPolys = polyData->GetPolys();

    vtkSmartPointer<vtkPoints> existingPoints = polyData->GetPoints();

    existingPolys->InitTraversal();

    vtkIdType* cell (NULL);
    vtkIdType cellSize (0);

    for( existingPolys->InitTraversal(); existingPolys->GetNextCell(cellSize, cell);)
    {
      bool incorporatePolygon = this->CheckForIntersection(cell,cellSize,existingPoints, /*numberOfIntersections, intersectionPoints, */i);
      if ( !incorporatePolygon ) continue;

      vtkSmartPointer<vtkPolygon> newPolygon = vtkPolygon::New();

      if(m_ReductionType == NTH_POINT)
      {
        this->ReduceNumberOfPointsByNthPoint(cellSize, cell, existingPoints, newPolygon, newPoints);
        if (newPolygon->GetPointIds()->GetNumberOfIds() != 0)
        {
          newPolygons->InsertNextCell(newPolygon);
        }
      }
      else if (m_ReductionType == DOUGLAS_PEUCKER)
      {
        this->ReduceNumberOfPointsByDouglasPeucker(cellSize, cell, existingPoints, newPolygon, newPoints);
        if (newPolygon->GetPointIds()->GetNumberOfIds() > 3)
        {
          newPolygons->InsertNextCell(newPolygon);
        }
      }

      //Again for evaluation
      //numberOfPointsBefore += cellSize;
      //numberOfPointsAfter += newPolygon->GetPointIds()->GetNumberOfIds();

    }

    if (newPolygons->GetNumberOfCells() != 0)
    {
      newPolyData->SetPolys(newPolygons);
      newPolyData->SetPoints(newPoints);
      newPolyData->BuildLinks();

      Surface::Pointer surface = this->GetOutput(numberOfOutputs);
      surface->SetVtkPolyData(newPolyData);
      numberOfOutputs++;
    }

  }
  //MITK_INFO<<"Points before: "<<numberOfPointsBefore<<" ##### Points after: "<<numberOfPointsAfter;
  this->SetNumberOfOutputs(numberOfOutputs);
}

void mitk::ReduceContourSetFilter::ReduceNumberOfPointsByNthPoint (vtkIdType cellSize, vtkIdType* cell, vtkPoints* points, vtkPolygon* reducedPolygon, vtkPoints* reducedPoints)
{

  unsigned int newNumberOfPoints (0);
  unsigned int mod = cellSize%m_StepSize;

  if(mod == 0)
  {
    newNumberOfPoints = cellSize/m_StepSize;
  }
  else
  {
    newNumberOfPoints = ( (cellSize-mod)/m_StepSize )+1;
  }

  if (newNumberOfPoints <= 3)
  {
    return;
  }
  reducedPolygon->GetPointIds()->SetNumberOfIds(newNumberOfPoints);
  reducedPolygon->GetPoints()->SetNumberOfPoints(newNumberOfPoints);

  for (unsigned int i = 0; i < cellSize; i++)
  {
    if (i%m_StepSize == 0)
    {
      double point[3];
      points->GetPoint(cell[i], point);
      vtkIdType id = reducedPoints->InsertNextPoint(point);
      reducedPolygon->GetPointIds()->SetId(i/m_StepSize, id);
    }

  }
  vtkIdType id =  cell[0];
  double point[3];
  points->GetPoint(id, point);
  id = reducedPoints->InsertNextPoint(point);
  reducedPolygon->GetPointIds()->SetId(newNumberOfPoints-1, id);

}

void mitk::ReduceContourSetFilter::ReduceNumberOfPointsByDouglasPeucker(vtkIdType cellSize, vtkIdType* cell, vtkPoints* points,
                                                                        vtkPolygon* reducedPolygon, vtkPoints* reducedPoints)
{
  //If the cell is too small to obtain a reduced polygon with the given stepsize return
  if (cellSize <= m_StepSize*3)return;

  /*
  What we do now is (see the Douglas Peucker Algorithm):

  1. Divide the current contour in two line segments (start - middle; middle - end), put them into the stack

  2. Fetch first line segment and create the following vectors:

  - v1 = (start;end)
  - v2 = (start;currentPoint) -> for each point of the current line segment!

  3. Calculate the distance from the currentPoint to v1:

  a. Determine the length of the orthogonal projection of v2 to v1 by:

  l = v2 * (normalized v1)

  b. There a three possibilities for the distance then:

  d = sqrt(lenght(v2)^2 - l^2) if l > 0 and l < length(v1)
  d = lenght(v2-v1) if l > 0 and l > lenght(v1)
  d = length(v2) if l < 0 because v2 is then pointing in a different direction than v1

  4. Memorize the point with the biggest distance and create two new line segments with it at the end of the iteration
  and put it into the stack

  5. If the distance value D <= m_Tolerance, then add the start and end index and the corresponding points to the reduced ones
  */

  //First of all set tolerance if none is specified
  if(m_Tolerance < 0)
  {
      if(m_MaxSpacing > 0)
      {
          m_Tolerance = m_MinSpacing;
      }
      else
      {
          m_Tolerance = 1.5;
      }
  }

  std::stack<LineSegment> lineSegments;

  //1. Divide in line segments

  LineSegment ls2;
  ls2.StartIndex = cell[cellSize/2];
  ls2.EndIndex = cell[cellSize-1];
  lineSegments.push(ls2);

  LineSegment ls1;
  ls1.StartIndex = cell[0];
  ls1.EndIndex = cell[cellSize/2];
  lineSegments.push(ls1);

  LineSegment currentSegment;
  double v1[3];
  double v2[3];
  double tempV[3];
  double lenghtV1;

  double currentMaxDistance (0);
  vtkIdType currentMaxDistanceIndex (0);

  double l;
  double d;

  vtkIdType pointId (0);
  //Add the start index to the reduced points. From now on just the end indices will be added
  pointId = reducedPoints->InsertNextPoint(points->GetPoint(cell[0]));
  reducedPolygon->GetPointIds()->InsertNextId(pointId);

  while (!lineSegments.empty())
  {
    currentSegment = lineSegments.top();
    lineSegments.pop();

    //2. Create vectors

    points->GetPoint(currentSegment.EndIndex, tempV);
    points->GetPoint(currentSegment.StartIndex, v1);

    v1[0] = tempV[0]-v1[0];
    v1[1] = tempV[1]-v1[1];
    v1[2] = tempV[2]-v1[2];

    lenghtV1 = vtkMath::Norm(v1);

    vtkMath::Normalize(v1);
    int range = currentSegment.EndIndex - currentSegment.StartIndex;
    for (int i = 1; i < abs(range); ++i)
    {
      points->GetPoint(currentSegment.StartIndex+i, tempV);
      points->GetPoint(currentSegment.StartIndex, v2);

      v2[0] = tempV[0]-v2[0];
      v2[1] = tempV[1]-v2[1];
      v2[2] = tempV[2]-v2[2];

      //3. Calculate the distance

      l = vtkMath::Dot(v2, v1);

      d =  vtkMath::Norm(v2);

      if (l > 0 && l < lenghtV1)
      {
        d = sqrt((d*d-l*l));
      }
      else if (l > 0 && l > lenghtV1)
      {
        tempV[0] = lenghtV1*v1[0] - v2[0];
        tempV[1] = lenghtV1*v1[1] - v2[1];
        tempV[2] = lenghtV1*v1[2] - v2[2];
        d = vtkMath::Norm(tempV);
      }

      //4. Memorize maximum distance
      if (d > currentMaxDistance)
      {
        currentMaxDistance = d;
        currentMaxDistanceIndex = currentSegment.StartIndex+i;
      }

    }

    //4. & 5.
    if (currentMaxDistance <= m_Tolerance)
    {

      //double temp[3];
      int segmentLenght = currentSegment.EndIndex - currentSegment.StartIndex;
      if (segmentLenght > m_MaxSegmentLenght)
      {
        m_MaxSegmentLenght = segmentLenght;
      }

      //MITK_INFO<<"Lenght: "<<abs(segmentLenght);
      if (abs(segmentLenght) > 20)
      {
          unsigned int divisions = abs(segmentLenght)%20;
        //MITK_INFO<<"Divisions: "<<divisions<<" Start: "<<currentSegment.StartIndex<<" End: "<<currentSegment.EndIndex<<endl;
        for (unsigned int i = 0; i<divisions; ++i)
        {
            if(currentSegment.EndIndex > (currentSegment.StartIndex + 20*i))
            {
                //MITK_INFO<<"Inserting Index: "<<(currentSegment.StartIndex + 20*i);
                pointId = reducedPoints->InsertNextPoint(points->GetPoint(currentSegment.StartIndex + 20*i));
                reducedPolygon->GetPointIds()->InsertNextId(pointId);
            }
        }
      }
      pointId = reducedPoints->InsertNextPoint(points->GetPoint(currentSegment.EndIndex));
      reducedPolygon->GetPointIds()->InsertNextId(pointId);
    }
    else
    {
      ls2.StartIndex = currentMaxDistanceIndex;
      ls2.EndIndex = currentSegment.EndIndex;
      lineSegments.push(ls2);

      ls1.StartIndex = currentSegment.StartIndex;
      ls1.EndIndex = currentMaxDistanceIndex;
      lineSegments.push(ls1);
    }
    currentMaxDistance = 0;

  }

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

void mitk::ReduceContourSetFilter::Reset()
{
  for (unsigned int i = 0; i < this->GetNumberOfInputs(); i++)
  {
    this->PopBackInput();
  }
  this->SetNumberOfInputs(0);
  this->SetNumberOfOutputs(0);
}
