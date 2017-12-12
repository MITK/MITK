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

#include "mitkPeakImage.h"
#include "mitkImageDataItem.h"
#include "mitkImageCast.h"
#include <mitkImageToItk.h>
#include <vtkPolyLine.h>
#include <vtkPointData.h>
#include <mitkImagePixelWriteAccessor.h>
#include <mitkImagePixelReadAccessor.h>

mitk::PeakImage::PeakImage() : Image()
{
  m_PolyData = nullptr;
}

mitk::PeakImage::~PeakImage()
{

}

void mitk::PeakImage::ConstructPolydata()
{
  MITK_INFO << "PeakImage constructing polydata";
  if (this->GetDimensions()[3]%3!=0)
    mitkThrow() << "Fourth dimension needs to be a multiple of 3";

  typedef mitk::ImageToItk< ItkPeakImageType > CasterType;
  CasterType::Pointer caster = CasterType::New();
  caster->SetInput(this);
  caster->Update();
  ItkPeakImageType::ConstPointer itkImg = caster->GetOutput();

  int sz_x = this->GetDimensions()[0];
  int sz_y = this->GetDimensions()[1];
  int sz_z = this->GetDimensions()[2];
  unsigned int num_dirs = this->GetDimensions()[3]/3;

  double minSpacing = 1;
  ItkPeakImageType::SpacingType spacing = itkImg->GetSpacing();
  if(spacing[0]<spacing[1] && spacing[0]<spacing[2])
    minSpacing = spacing[0];
  else if (spacing[1] < spacing[2])
    minSpacing = spacing[1];
  else
    minSpacing = spacing[2];

  vtkSmartPointer<vtkCellArray> m_VtkCellArray = vtkSmartPointer<vtkCellArray>::New();
  vtkSmartPointer<vtkPoints>    m_VtkPoints = vtkSmartPointer<vtkPoints>::New();

#ifdef WIN32
#pragma omp parallel for
#else
#pragma omp parallel for collapse(3)
#endif
  for (int x=0; x<sz_x; ++x)
    for (int y=0; y<sz_y; ++y)
      for (int z=0; z<sz_z; ++z)
      {
        itk::Index<4> idx4; idx4[0] = x; idx4[1] = y; idx4[2] = z;

        for (unsigned int i=0; i<num_dirs; i++)
        {
          itk::ContinuousIndex<double, 4> center;
          center[0] = idx4[0];
          center[1] = idx4[1];
          center[2] = idx4[2];
          center[3] = 0;

          itk::Point<double, 4> worldCenter;
          itkImg->TransformContinuousIndexToPhysicalPoint( center, worldCenter );

          idx4[3] = 0;
          vnl_vector_fixed< float, 3 > dir;
          idx4[3] = i*3;
          dir[0] = itkImg->GetPixel(idx4);
          idx4[3] += 1;
          dir[1] = itkImg->GetPixel(idx4);
          idx4[3] += 1;
          dir[2] = itkImg->GetPixel(idx4);

          if (dir.magnitude()<0.001)
            continue;

#pragma omp critical
          {
            vtkSmartPointer<vtkPolyLine> container = vtkSmartPointer<vtkPolyLine>::New();
            // add direction to vector field (with spacing compensation)
            itk::Point<double> worldStart;
            worldStart[0] = worldCenter[0]-dir[0]/2*minSpacing;
            worldStart[1] = worldCenter[1]-dir[1]/2*minSpacing;
            worldStart[2] = worldCenter[2]-dir[2]/2*minSpacing;
            vtkIdType id = m_VtkPoints->InsertNextPoint(worldStart.GetDataPointer());
            container->GetPointIds()->InsertNextId(id);
            itk::Point<double> worldEnd;
            worldEnd[0] = worldCenter[0]+dir[0]/2*minSpacing;
            worldEnd[1] = worldCenter[1]+dir[1]/2*minSpacing;
            worldEnd[2] = worldCenter[2]+dir[2]/2*minSpacing;
            id = m_VtkPoints->InsertNextPoint(worldEnd.GetDataPointer());
            container->GetPointIds()->InsertNextId(id);
            m_VtkCellArray->InsertNextCell(container);
          }
        }
      }

  m_PolyData = vtkSmartPointer<vtkPolyData>::New();
  m_PolyData->SetPoints(m_VtkPoints);
  m_PolyData->SetLines(m_VtkCellArray);
  MITK_INFO << "PeakImage finished constructing polydata";

  ColorByOrientation();

  this->Modified();
}

void mitk::PeakImage::SetCustomColor(float r, float g, float b)
{
  vtkPoints* extrPoints = nullptr;
  extrPoints = m_PolyData->GetPoints();
  int numOfPoints = 0;
  if (extrPoints!=nullptr)
    numOfPoints = extrPoints->GetNumberOfPoints();

  int componentSize = 4;

  vtkSmartPointer<vtkUnsignedCharArray> colors = vtkSmartPointer<vtkUnsignedCharArray>::New();
  colors->Allocate(numOfPoints * componentSize);
  colors->SetNumberOfComponents(componentSize);
  colors->SetName("FIBER_COLORS");

  unsigned char rgba[4] = {0,0,0,0};
  for(long i=0; i<m_PolyData->GetNumberOfPoints(); ++i)
  {
    rgba[0] = (unsigned char) r;
    rgba[1] = (unsigned char) g;
    rgba[2] = (unsigned char) b;
    rgba[3] = (unsigned char) 255;
    colors->InsertTypedTuple(i, rgba);
  }

  m_PolyData->GetPointData()->AddArray(colors);
  this->Modified();
}

void mitk::PeakImage::ColorByOrientation()
{
  //===== FOR WRITING A TEST ========================
  //  colorT size == tupelComponents * tupelElements
  //  compare color results
  //  to cover this code 100% also PolyData needed, where colorarray already exists
  //  + one fiber with exactly 1 point
  //  + one fiber with 0 points
  //=================================================

  vtkPoints* extrPoints = nullptr;
  extrPoints = m_PolyData->GetPoints();
  int numOfPoints = 0;
  if (extrPoints!=nullptr)
    numOfPoints = extrPoints->GetNumberOfPoints();

  //colors and alpha value for each single point, RGBA = 4 components
  unsigned char rgba[4] = {0,0,0,0};
  int componentSize = 4;

  vtkSmartPointer<vtkUnsignedCharArray> colors = vtkSmartPointer<vtkUnsignedCharArray>::New();
  colors->Allocate(numOfPoints * componentSize);
  colors->SetNumberOfComponents(componentSize);
  colors->SetName("FIBER_COLORS");

  int num_peaks = m_PolyData->GetNumberOfLines();
  if (num_peaks < 1)
    return;

  vtkCellArray* fiberList = m_PolyData->GetLines();
  fiberList->InitTraversal();
  for (int fi=0; fi<num_peaks; ++fi)
  {
    vtkIdType* idList; // contains the point id's of the line
    vtkIdType num_points; // number of points for current line
    fiberList->GetNextCell(num_points, idList);

    if (num_points == 2)
    {
      vnl_vector_fixed< double, 3 > currentPntvtk(extrPoints->GetPoint(idList[0])[0], extrPoints->GetPoint(idList[0])[1],extrPoints->GetPoint(idList[0])[2]);
      vnl_vector_fixed< double, 3 > nextPntvtk(extrPoints->GetPoint(idList[1])[0], extrPoints->GetPoint(idList[1])[1], extrPoints->GetPoint(idList[1])[2]);

      vnl_vector_fixed< double, 3 > diff1;
      diff1 = currentPntvtk - nextPntvtk;
      diff1.normalize();

      rgba[0] = (unsigned char) (255.0 * std::fabs(diff1[0]));
      rgba[1] = (unsigned char) (255.0 * std::fabs(diff1[1]));
      rgba[2] = (unsigned char) (255.0 * std::fabs(diff1[2]));
      rgba[3] = (unsigned char) (255.0);
      colors->InsertTypedTuple(idList[0], rgba);
      colors->InsertTypedTuple(idList[1], rgba);
    }
    else
    {
      MITK_INFO << "Peak with point number!=2 detected!" ;
      continue;
    }
  }

  m_PolyData->GetPointData()->AddArray(colors);
  this->Modified();
}
