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

#include "mitkLabelSetSurfaceStamp.h"
#include <itkBilateralImageFilter.h>
#include "mitkImageAccessByItk.h"
#include "mitkImageCast.h"

#include <mitkSurface.h>
#include <mitkLabelSetImage.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkCell.h>
#include <itkQuadEdgeMesh.h>
#include <itkTriangleMeshToBinaryImageFilter.h>
#include <mitkLabelSetImage.h>

mitk::LabelSetSurfaceStamp::LabelSetSurfaceStamp()
{
  this->SetNumberOfIndexedInputs(1);
  this->SetNumberOfRequiredInputs(1);
}

mitk::LabelSetSurfaceStamp::~LabelSetSurfaceStamp()
{
}

void mitk::LabelSetSurfaceStamp::GenerateData()
{
  this->SetOutput(0,this->GetInput(0));

  mitk::Image::Pointer inputImage = this->GetInput(0);

  if (m_Surface.IsNull())
  {
    MITK_ERROR << "Input surface is NULL.";
    return;
  }
  if ( (inputImage->GetDimension() > 4) || (inputImage->GetDimension() < 2) )
  {
    MITK_ERROR << "mitk::LabelSetSurfaceStamp:GenerateData works only with 2D, 2D+t, 3D, 3D+t and 4D images, sorry.";
    return;
  }

  // This is necessary because only these dimensions are supported from the corresponding filter.
  switch(inputImage->GetDimension())
  {
  case 2:
    {
      //AccessFixedDimensionByItk_1(inputImage, ItkImageProcessing, 2, m_Surface); break;
    }
  case 3:
    {
      AccessFixedDimensionByItk_1(inputImage, ItkImageProcessing, 3, m_Surface); break;
    }
  case 4:
    {
      AccessFixedDimensionByItk_1(inputImage, ItkImageProcessing, 4, m_Surface); break;
    }
  default: break;
  }
}

template<typename TPixel, unsigned int VImageDimension>
void mitk::LabelSetSurfaceStamp::ItkImageProcessing( itk::Image<TPixel,VImageDimension>* itkImage, mitk::Surface::Pointer surface )
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  mitk::LabelSetImage::Pointer LabelSetInputImage = dynamic_cast<LabelSetImage *>(GetInput());
  try
  {
    vtkPolyData *polydata = surface->GetVtkPolyData();

    vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
    transform->SetMatrix(surface->GetGeometry()->GetVtkTransform()->GetMatrix());
    transform->Update();

    vtkSmartPointer<vtkTransformPolyDataFilter> transformer = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
    transformer->SetInputData(polydata);
    transformer->SetTransform(transform);
    transformer->Update();

    typedef double Coord;
    typedef itk::QuadEdgeMesh< Coord, VImageDimension > MeshType;

    MeshType::Pointer mesh = MeshType::New();
    mesh->SetCellsAllocationMethod(MeshType::CellsAllocatedDynamicallyCellByCell);
    int numberOfPoints = polydata->GetNumberOfPoints();
    mesh->GetPoints()->Reserve(numberOfPoints);

    vtkPoints* points = polydata->GetPoints();

    MeshType::PointType point;
    for (int i = 0; i < numberOfPoints; i++)
    {
      double* aux = points->GetPoint(i);
      point[0] = aux[0];
      point[1] = aux[1];
      point[2] = aux[2];
      mesh->SetPoint(i, point);
    }

    // Load the polygons into the itk::Mesh
    typedef MeshType::CellAutoPointer     CellAutoPointerType;
    typedef MeshType::CellType            CellType;
    typedef itk::TriangleCell< CellType > TriangleCellType;
    typedef MeshType::PointIdentifier     PointIdentifierType;
    typedef MeshType::CellIdentifier      CellIdentifierType;

    // Read the number of polygons
    CellIdentifierType numberOfPolygons = 0;
    numberOfPolygons = polydata->GetNumberOfPolys();

    PointIdentifierType numberOfCellPoints = 3;

    for (CellIdentifierType i = 0; i < numberOfPolygons; i++)
    {
      vtkIdList *cellIds;
      vtkCell *vcell = polydata->GetCell(i);
      cellIds = vcell->GetPointIds();

      CellAutoPointerType cell;
      auto   triangleCell = new TriangleCellType;
      PointIdentifierType k;
      for (k = 0; k < numberOfCellPoints; k++)
      {
        triangleCell->SetPointId(k, cellIds->GetId(k));
      }

      cell.TakeOwnership(triangleCell);
      mesh->SetCell(i, cell);
    }

    typedef itk::TriangleMeshToBinaryImageFilter<MeshType, ImageType> TriangleMeshToBinaryImageFilterType;

    TriangleMeshToBinaryImageFilterType::Pointer filter = TriangleMeshToBinaryImageFilterType::New();
    filter->SetInput(mesh);
    filter->SetInfoImage(itkImage);
    filter->SetInsideValue(1);
    filter->SetOutsideValue(0);
    filter->Update();

    //GoTrough
    auto resultImage = filter->GetOutput();
    resultImage->DisconnectPipeline();

    typedef itk::ImageRegionConstIterator< ImageType > SourceIteratorType;
    typedef itk::ImageRegionIterator< ImageType > TargetIteratorType;

    SourceIteratorType sourceIter(resultImage, resultImage->GetLargestPossibleRegion());
    sourceIter.GoToBegin();

    TargetIteratorType targetIter(itkImage, itkImage->GetLargestPossibleRegion());
    targetIter.GoToBegin();

    int activeLabel = (LabelSetInputImage->GetActiveLabel(LabelSetInputImage->GetActiveLayer()))->GetValue();

    while (!sourceIter.IsAtEnd())
    {
      int sourceValue = static_cast<int>(sourceIter.Get());
      int targetValue = static_cast<int>(targetIter.Get());

      if ((sourceValue != 0) && (m_ForceOverwrite || !LabelSetInputImage->GetLabel(targetValue)->GetLocked())) // skip exterior and locked labels
      {
        targetIter.Set(activeLabel);
      }
      ++sourceIter;
      ++targetIter;
    }
  }
  catch (itk::ExceptionObject& e)
  {
    mitkThrow() << e.GetDescription();
  }
  this->Modified();
}

void mitk::LabelSetSurfaceStamp::GenerateOutputInformation()
{
  mitk::Image::Pointer inputImage  = (mitk::Image*) this->GetInput();
  mitk::Image::Pointer output = this->GetOutput();
  itkDebugMacro(<<"GenerateOutputInformation()");
  if(inputImage.IsNull()) return;
}