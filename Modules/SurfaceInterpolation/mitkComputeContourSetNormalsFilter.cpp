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

#include "mitkComputeContourSetNormalsFilter.h"

#include "mitkImagePixelReadAccessor.h"


mitk::ComputeContourSetNormalsFilter::ComputeContourSetNormalsFilter()
  : m_SegmentationBinaryImage(NULL)
  , m_MaxSpacing(5)
  , m_NegativeNormalCounter(0)
  , m_PositiveNormalCounter(0)
  , m_UseProgressBar(false)
  , m_ProgressStepSize(1)
{
  mitk::Surface::Pointer output = mitk::Surface::New();
  this->SetNthOutput(0, output.GetPointer());
}

mitk::ComputeContourSetNormalsFilter::~ComputeContourSetNormalsFilter()
{
}

void mitk::ComputeContourSetNormalsFilter::GenerateData()
{
  unsigned int numberOfInputs = this->GetNumberOfIndexedInputs();
  this->CreateOutputsForAllInputs(numberOfInputs);

  //Iterating over each input
  for(unsigned int i = 0; i < numberOfInputs; i++)
  {
    //Getting the inputs polydata and polygons
    Surface* currentSurface = const_cast<Surface*>( this->GetInput(i) );
    vtkPolyData* polyData = currentSurface->GetVtkPolyData();

    vtkSmartPointer<vtkCellArray> existingPolys = polyData->GetPolys();

    vtkSmartPointer<vtkPoints> existingPoints = polyData->GetPoints();

    existingPolys->InitTraversal();

    vtkIdType* cell (NULL);
    vtkIdType cellSize (0);

    //The array that contains all the vertex normals of the current polygon
    vtkSmartPointer<vtkDoubleArray> normals = vtkSmartPointer<vtkDoubleArray>::New();
    normals->SetNumberOfComponents(3);
    normals->SetNumberOfTuples(polyData->GetNumberOfPoints());

    //If the current contour is an inner contour then the direction is -1
    //A contour lies inside another one if the pixel values in the direction of the normal is 1
    m_NegativeNormalCounter = 0;
    m_PositiveNormalCounter = 0;
    vtkIdType offSet (0);

    //Iterating over each polygon
    for( existingPolys->InitTraversal(); existingPolys->GetNextCell(cellSize, cell);)
    {
      if(cellSize < 3)continue;

      //First we calculate the current polygon's normal
      double polygonNormal[3] = {0.0};

      double p1[3];
      double p2[3];

      double v1[3];
      double v2[3];

      existingPoints->GetPoint(cell[0], p1);
      unsigned int index = cellSize*0.5;
      existingPoints->GetPoint(cell[index], p2);

      v1[0] = p2[0]-p1[0];
      v1[1] = p2[1]-p1[1];
      v1[2] = p2[2]-p1[2];

      for (vtkIdType k = 2; k < cellSize; k++)
      {
        index = cellSize*0.25;
        existingPoints->GetPoint(cell[index], p1);
        index = cellSize*0.75;
        existingPoints->GetPoint(cell[index], p2);

        v2[0] = p2[0]-p1[0];
        v2[1] = p2[1]-p1[1];
        v2[2] = p2[2]-p1[2];

        vtkMath::Cross(v1,v2,polygonNormal);
        if (vtkMath::Norm(polygonNormal) != 0)
          break;
      }

      vtkMath::Normalize(polygonNormal);

      //Now we start computing the normal for each vertex

      double vertexNormalTemp[3];
      existingPoints->GetPoint(cell[0], p1);
      existingPoints->GetPoint(cell[1], p2);

      v1[0] = p2[0]-p1[0];
      v1[1] = p2[1]-p1[1];
      v1[2] = p2[2]-p1[2];

      vtkMath::Cross(v1,polygonNormal,vertexNormalTemp);

      vtkMath::Normalize(vertexNormalTemp);

      double vertexNormal[3];

      for (vtkIdType j = 0; j < cellSize-2; j++)
      {
        existingPoints->GetPoint(cell[j+1], p1);
        existingPoints->GetPoint(cell[j+2], p2);

        v1[0] = p2[0]-p1[0];
        v1[1] = p2[1]-p1[1];
        v1[2] = p2[2]-p1[2];

        vtkMath::Cross(v1,polygonNormal,vertexNormal);

        vtkMath::Normalize(vertexNormal);

        double finalNormal[3];

        finalNormal[0] = (vertexNormal[0] + vertexNormalTemp[0])*0.5;
        finalNormal[1] = (vertexNormal[1] + vertexNormalTemp[1])*0.5;
        finalNormal[2] = (vertexNormal[2] + vertexNormalTemp[2])*0.5;

        //Here we determine the direction of the normal
        if (m_SegmentationBinaryImage)
        {
          Point3D worldCoord;
          worldCoord[0] = p1[0]+finalNormal[0]*m_MaxSpacing;
          worldCoord[1] = p1[1]+finalNormal[1]*m_MaxSpacing;
          worldCoord[2] = p1[2]+finalNormal[2]*m_MaxSpacing;

          double val = 0.0;

          mitk::ImagePixelReadAccessor<unsigned char> readAccess(m_SegmentationBinaryImage);
          itk::Index<3> idx;
          m_SegmentationBinaryImage->GetGeometry()->WorldToIndex(worldCoord, idx);
          try
          {
            val = readAccess.GetPixelByIndexSafe(idx);
          }
          catch (mitk::Exception e)
          {
            // If value is outside the image's region ignore it
          }

          if (val == 0.0)
          {
              ++m_PositiveNormalCounter;
          }
          else
          {
              ++m_NegativeNormalCounter;
          }
        }

        vertexNormalTemp[0] = vertexNormal[0];
        vertexNormalTemp[1] = vertexNormal[1];
        vertexNormalTemp[2] = vertexNormal[2];

        vtkIdType id = cell[j+1];
        normals->SetTuple(id,finalNormal);
      }

      existingPoints->GetPoint(cell[0], p1);
      existingPoints->GetPoint(cell[1], p2);

      v1[0] = p2[0]-p1[0];
      v1[1] = p2[1]-p1[1];
      v1[2] = p2[2]-p1[2];

      vtkMath::Cross(v1,polygonNormal,vertexNormal);

      vtkMath::Normalize(vertexNormal);

      vertexNormal[0] = (vertexNormal[0] + vertexNormalTemp[0])*0.5;
      vertexNormal[1] = (vertexNormal[1] + vertexNormalTemp[1])*0.5;
      vertexNormal[2] = (vertexNormal[2] + vertexNormalTemp[2])*0.5;

      vtkIdType id = cell[0];
      normals->SetTuple(id,vertexNormal);
      id = cell[cellSize-1];
      normals->SetTuple(id,vertexNormal);

      if(m_NegativeNormalCounter > m_PositiveNormalCounter)
      {
          for(vtkIdType n = 0; n < cellSize; n++)
          {
              double normal[3];
              normals->GetTuple(offSet+n, normal);
              normal[0] = (-1)*normal[0];
              normal[1] = (-1)*normal[1];
              normal[2] = (-1)*normal[2];
              normals->SetTuple(offSet+n, normal);
          }
      }

      m_NegativeNormalCounter = 0;
      m_PositiveNormalCounter = 0;
      offSet += cellSize;


    }//end for all cells

    Surface::Pointer surface = this->GetOutput(i);
    surface->GetVtkPolyData()->GetCellData()->SetNormals(normals);
  }//end for all inputs

  //Setting progressbar
  if (this->m_UseProgressBar)
    mitk::ProgressBar::GetInstance()->Progress(this->m_ProgressStepSize);
}


mitk::Surface::Pointer mitk::ComputeContourSetNormalsFilter::GetNormalsAsSurface()
{
  //Just for debugging:
  vtkSmartPointer<vtkPolyData> newPolyData = vtkSmartPointer<vtkPolyData>::New();
  vtkSmartPointer<vtkCellArray> newLines = vtkSmartPointer<vtkCellArray>::New();
  vtkSmartPointer<vtkPoints> newPoints = vtkSmartPointer<vtkPoints>::New();
  unsigned int idCounter (0);
  //Debug end

  for (unsigned int i = 0; i < this->GetNumberOfIndexedOutputs(); i++)
  {
    Surface* currentSurface = const_cast<Surface*>( this->GetOutput(i) );
    vtkPolyData* polyData = currentSurface->GetVtkPolyData();

    vtkSmartPointer<vtkDoubleArray> currentCellNormals = vtkDoubleArray::SafeDownCast(polyData->GetCellData()->GetNormals());

    vtkSmartPointer<vtkCellArray> existingPolys = polyData->GetPolys();

    vtkSmartPointer<vtkPoints> existingPoints = polyData->GetPoints();

    existingPolys->InitTraversal();

    vtkIdType* cell (NULL);
    vtkIdType cellSize (0);

    for( existingPolys->InitTraversal(); existingPolys->GetNextCell(cellSize, cell);)
    {
      for ( vtkIdType j = 0; j < cellSize; j++ )
      {
        double currentNormal[3];
        currentCellNormals->GetTuple(cell[j], currentNormal);
        vtkSmartPointer<vtkLine> line = vtkSmartPointer<vtkLine>::New();
        line->GetPointIds()->SetNumberOfIds(2);
        double newPoint[3];
        double p0[3];
        existingPoints->GetPoint(cell[j], p0);
        newPoint[0] = p0[0] + currentNormal[0];
        newPoint[1] = p0[1] + currentNormal[1];
        newPoint[2] = p0[2] + currentNormal[2];

        line->GetPointIds()->SetId(0, idCounter);
        newPoints->InsertPoint(idCounter, p0);
        idCounter++;

        line->GetPointIds()->SetId(1, idCounter);
        newPoints->InsertPoint(idCounter, newPoint);
        idCounter++;

        newLines->InsertNextCell(line);
      }//end for all points
    }//end for all cells
  }//end for all outputs

  newPolyData->SetPoints(newPoints);
  newPolyData->SetLines(newLines);
  newPolyData->BuildCells();


  mitk::Surface::Pointer surface = mitk::Surface::New();
  surface->SetVtkPolyData(newPolyData);

  return surface;

}

void mitk::ComputeContourSetNormalsFilter::SetMaxSpacing(double maxSpacing)
{
    m_MaxSpacing = maxSpacing;
}

void mitk::ComputeContourSetNormalsFilter::GenerateOutputInformation()
{
  Superclass::GenerateOutputInformation();
}

void mitk::ComputeContourSetNormalsFilter::Reset()
{
  for (unsigned int i = 0; i < this->GetNumberOfIndexedInputs(); i++)
  {
    this->PopBackInput();
  }
  this->SetNumberOfIndexedInputs(0);
  this->SetNumberOfIndexedOutputs(0);

  mitk::Surface::Pointer output = mitk::Surface::New();
  this->SetNthOutput(0, output.GetPointer());
}

void mitk::ComputeContourSetNormalsFilter::SetUseProgressBar(bool status)
{
  this->m_UseProgressBar = status;
}

void mitk::ComputeContourSetNormalsFilter::SetProgressStepSize(unsigned int stepSize)
{
  this->m_ProgressStepSize = stepSize;
}
