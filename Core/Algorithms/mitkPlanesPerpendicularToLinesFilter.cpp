/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkPlanesPerpendicularToLinesFilter.h"

#include <vnl/vnl_cross.h>
#include <vnl/vnl_quaternion.h>
#include <vnl/vnl_quaternion.txx>

mitk::PlanesPerpendicularToLinesFilter::PlanesPerpendicularToLinesFilter()
  : m_Plane(NULL), m_UseAllPoints(false), m_CreatedGeometries(NULL), normal(3), targetRight(3)
{
  m_CreatedGeometries = mitk::SlicedGeometry3D::New();
}

mitk::PlanesPerpendicularToLinesFilter::~PlanesPerpendicularToLinesFilter()
{
}

void mitk::PlanesPerpendicularToLinesFilter::GenerateOutputInformation()
{
  mitk::Mesh::ConstPointer input  = this->GetInput();
  mitk::GeometryData::Pointer output = this->GetOutput();

  itkDebugMacro(<<"GenerateOutputInformation()");

  if(input.IsNull()) return;

  output->SetGeometry(m_CreatedGeometries);
}

void mitk::PlanesPerpendicularToLinesFilter::CreatePlane(const mitk::Point3D& curr)
{
  int j;
  for(j=0;j<3;++j)
    normal[j] = last[j]-curr[j]; //@todo globally define normal direction of display xxx
  normal.normalize();

  down = vnl_cross_3d(normal, targetRight);
  down.normalize();
  right = vnl_cross_3d(down, normal);
  right.normalize();

  itk2vtk(last.Get_vnl_vector()-right*halfWidthInMM-down*halfHeightInMM, origin);
  right  *= targetSpacing[0];
  down   *= targetSpacing[1];
  normal *= targetSpacing[2];

  mitk::Matrix3D matrix;
  matrix.GetVnlMatrix().set_column(0, right);
  matrix.GetVnlMatrix().set_column(1, down);
  matrix.GetVnlMatrix().set_column(2, normal);

  PlaneGeometry::Pointer plane = PlaneGeometry::New();
  plane->GetIndexToWorldTransform()->SetMatrix(matrix);
  plane->SetOrigin(origin);
  plane->SetBounds(bounds);

  planes.push_back(plane);

  last = curr;
}

void mitk::PlanesPerpendicularToLinesFilter::GenerateData()
{
  mitk::Mesh::ConstPointer input  = this->GetInput();
  mitk::GeometryData::Pointer output = this->GetOutput();

  if(m_Plane.IsNotNull())
  {
    targetRight   = m_Plane->GetMatrixColumn(0);
    targetSpacing = m_Plane->GetSpacing();
    bounds = m_Plane->GetBoundingBox()->GetBounds();
    halfWidthInMM  = m_Plane->GetExtentInMM(0)*0.5;
    halfHeightInMM = m_Plane->GetExtentInMM(1)*0.5;
  }
  else
  {
    FillVector3D(targetRight, 1.0, 0.0, 0.0);
    targetSpacing.Fill(1.0);
    halfWidthInMM=halfHeightInMM=100.0;
    ScalarType stdBounds[6] = {0.0, 2.0*halfWidthInMM, 0.0, 2.0*halfHeightInMM, 0.0, 0.0};
    bounds = stdBounds;
  }

  if(m_UseAllPoints==false)
  {
    int i, size;
    //iterate through all cells and build planes
    Mesh::CellIterator cellIt, cellEnd;
    cellEnd = input->GetMesh()->GetCells()->End();
    for( cellIt = input->GetMesh()->GetCells()->Begin(); cellIt != cellEnd; ++cellIt )
    {
      Mesh::CellType& cell = *cellIt->Value();

      Mesh::PointIdIterator ptIt, ptEnd;
      ptEnd = cell.PointIdsEnd();

      size=cell.GetNumberOfPoints();
      if(size<=1)
        continue;

      ptIt = cell.PointIdsBegin();
      last = input->GetPoint(*ptIt);
      ++ptIt;
      for(i=1;i<size;++i, ++ptIt)
      {
        CreatePlane(input->GetPoint(*ptIt));
      }
    }
  }
  else //m_UseAllPoints==true
  {
    //iterate through all points and build planes
    mitk::PointSet::PointsConstIterator it, pend = input->GetPointSet()->GetPoints()->End();
    it=input->GetPointSet()->GetPoints()->Begin();
    last = it.Value();
    ++it;
    for(;it!=pend;++it)
    {
        CreatePlane(it.Value());
    }
  }

  if(planes.size()>0)
  {
    //initialize sliced-geometry for the number of created planes
    m_CreatedGeometries->Initialize(planes.size()+1);

    //set last plane at last point with same normal as the one before the last
    PlaneGeometry::Pointer plane = static_cast<PlaneGeometry*>((*planes.rbegin())->Clone().GetPointer());
    itk2vtk(last.Get_vnl_vector()-right*halfWidthInMM-down*halfHeightInMM, origin);
    plane->SetOrigin(origin);
    m_CreatedGeometries->SetGeometry2D(plane, planes.size());

    //add all planes to sliced-geometry
    int s;
    for(s=0; planes.empty()==false; planes.pop_front(), ++s)
      m_CreatedGeometries->SetGeometry2D(planes.front(), s);

    m_CreatedGeometries->SetEvenlySpaced(false);

    if(m_FrameGeometry.IsNotNull())
    {
      m_CreatedGeometries->SetIndexToWorldTransform(m_FrameGeometry->GetIndexToWorldTransform());
      m_CreatedGeometries->SetBounds(m_FrameGeometry->GetBounds());
    }
  }

  output->SetGeometry(m_CreatedGeometries);
}

void mitk::PlanesPerpendicularToLinesFilter::SetPlane(const mitk::PlaneGeometry* aPlane)
{
  if(aPlane!=NULL)
  {
    m_Plane = static_cast<mitk::PlaneGeometry*>(aPlane->Clone().GetPointer());
  }
  else
  {
    if(m_Plane.IsNull())
      return;
    m_Plane=NULL;
  }
  Modified();
}

const mitk::Mesh *mitk::PlanesPerpendicularToLinesFilter::GetInput(void)
{
  if (this->GetNumberOfInputs() < 1)
  {
    return 0;
  }

  return static_cast<const mitk::Mesh * >
    (this->ProcessObject::GetInput(0) );
}

void mitk::PlanesPerpendicularToLinesFilter::SetInput(const mitk::Mesh *input)
{
  // Process object is not const-correct so the const_cast is required here
  this->ProcessObject::SetNthInput(0, 
    const_cast< mitk::Mesh * >( input ) );
}

void mitk::PlanesPerpendicularToLinesFilter::SetFrameGeometry(const mitk::Geometry3D* frameGeometry)
{
  if((frameGeometry != NULL) && (frameGeometry->IsValid()))
  {
    m_FrameGeometry = static_cast<mitk::Geometry3D*>(frameGeometry->Clone().GetPointer());
  }
  else
  {
    m_FrameGeometry = NULL;
  }
}
