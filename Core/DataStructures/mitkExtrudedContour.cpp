/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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


#include "mitkExtrudedContour.h"
#include "vtkTransform.h"
#include "mitkVector.h"

#include <vtkLinearExtrusionFilter.h>
#include <vtkPolyData.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkPlanes.h>
#include <vtkClipPolyData.h>

#include <vtkPolygon.h>
#include <vtkFloatArray.h>
#include <vtkPlane.h>


//vtkButterflySubdivisionFilter * subdivs;
#include <vtkSampleFunction.h>
#include <vtkTriangleFilter.h>
#include <vtkDecimatePro.h>
#include <vtkLinearSubdivisionFilter.h>
#include <vtkImageData.h>
#include <vtkCubeSource.h>

mitk::ExtrudedContour::ExtrudedContour() 
  : m_Contour(NULL), m_ClippingGeometry(NULL)
{
  m_Geometry3D->Initialize();

  FillVector3D(m_Vector, 0.0, 0.0, 1.0);

  m_ExtrusionFilter = vtkLinearExtrusionFilter::New();

  m_ExtrusionFilter->CappingOff();
  m_ExtrusionFilter->SetExtrusionTypeToVectorExtrusion();

  m_TriangleFilter = vtkTriangleFilter::New();
  m_TriangleFilter->SetInput(m_ExtrusionFilter->GetOutput());

  m_Decimate = vtkDecimatePro::New();
  m_Decimate->SetInput(m_TriangleFilter->GetOutput());

  m_SubdivisionFilter = vtkLinearSubdivisionFilter::New();
  m_SubdivisionFilter->SetInput(m_Decimate->GetOutput());
  m_SubdivisionFilter->SetNumberOfSubdivisions(3);

  m_ClippingBox = vtkPlanes::New();
  m_ClipPolyDataFilter = vtkClipPolyData::New();
  m_ClipPolyDataFilter->SetInput(m_SubdivisionFilter->GetOutput());
  m_ClipPolyDataFilter->SetClipFunction(m_ClippingBox);
  m_ClipPolyDataFilter->InsideOutOn();

  m_Polygon = vtkPolygon::New();
}

mitk::ExtrudedContour::~ExtrudedContour()
{
  m_ClipPolyDataFilter->Delete();
  m_ClippingBox->Delete();
  m_SubdivisionFilter->Delete();
  m_TriangleFilter->Delete();
  m_ExtrusionFilter->Delete();
  m_Polygon->Delete();
}

bool mitk::ExtrudedContour::IsInside(const Point3D& worldPoint) const
{
  // project point onto plane
  float x[3];
  itk2vtk(worldPoint, x);
  vtkPlane::ProjectPoint(x, const_cast<mitk::ExtrudedContour*>(this)->m_Origin, const_cast<mitk::ExtrudedContour*>(this)->m_Normal, x);

  // determine whether it's in the selection loop and then evaluate point
  // in polygon only if absolutely necessary.
  if ( x[0] >= this->m_ProjectedContourBounds[0] && x[0] <= this->m_ProjectedContourBounds[1] &&
    x[1] >= this->m_ProjectedContourBounds[2] && x[1] <= this->m_ProjectedContourBounds[3] &&
    x[2] >= this->m_ProjectedContourBounds[4] && x[2] <= this->m_ProjectedContourBounds[5] &&
    this->m_Polygon->PointInPolygon(x, m_Polygon->Points->GetNumberOfPoints(),
    ((vtkFloatArray *)this->m_Polygon->Points->GetData())->GetPointer(0), 
    const_cast<mitk::ExtrudedContour*>(this)->m_ProjectedContourBounds,const_cast<mitk::ExtrudedContour*>(this)->m_Normal) == 1 )
    return true;
  else
    return false;
}

mitk::ScalarType mitk::ExtrudedContour::GetVolume()
{
  return -1.0;
}

void mitk::ExtrudedContour::UpdateOutputInformation()
{
  if ( this->GetSource() )
  {
    this->GetSource()->UpdateOutputInformation();
  }
  if(GetMTime() > m_LastCalculateExtrusionTime)
    CalculateExtrusion();
  if ( ( m_CalculateBoundingBox ) && ( m_PolyDataSeries.size() > 0 ) )
    CalculateBoundingBox();
}

void mitk::ExtrudedContour::CalculateExtrusion()
{
  if(m_Contour.IsNull())
  {
    SetVtkPolyData(NULL);
    return;
  }

  Point3D center;
  if(m_ClippingGeometry.IsNotNull())
    center = m_ClippingGeometry->GetCenter();
  else
    center.Fill(0);
  
  // set extrusion vector
  float vtkvector[3];
  itk2vtk(m_Vector, vtkvector);
  m_ExtrusionFilter->SetVector(vtkvector);

  // set extrusion contour
  vtkPolyData *polyData = vtkPolyData::New();
  vtkPoints *loopPoints = vtkPoints::New();
  vtkCellArray *polys = vtkCellArray::New();

  mitk::Contour::PointsContainerIterator pointsIt = m_Contour->GetPoints()->Begin(), end = m_Contour->GetPoints()->End();
  float vtkpoint[3];
  int* pointids = new int[m_Contour->GetNumberOfPoints()] ;
  int i=0;
  Vector3D diff;
  ScalarType scalardiff, min_dist=ScalarTypeNumericTraits::max();
  while ( pointsIt != end )
  {
    itk2vtk(pointsIt.Value(), vtkpoint);
    pointids[i]=loopPoints->InsertNextPoint(vtkpoint);

    diff=center-pointsIt.Value();
    scalardiff=diff.GetSquaredNorm();
    if(scalardiff < min_dist) min_dist = scalardiff;

    ++pointsIt; ++i;
  }
  polys->InsertNextCell( i, pointids );
  delete [] pointids;

  polyData->SetPoints( loopPoints );
  polyData->SetPolys( polys );
  polys->Delete();

  m_ExtrusionFilter->SetInput(polyData);

  polyData->Delete();

  // set extrusion scale factor
  if(m_ClippingGeometry.IsNull())
  {
    m_ExtrusionFilter->SetScaleFactor(10.0);
    SetVtkPolyData(m_SubdivisionFilter->GetOutput());
  }
  else
  {
    min_dist = sqrt(min_dist)+m_ClippingGeometry->GetDiagonalLength();
    m_ExtrusionFilter->SetScaleFactor(min_dist);

    vtkCubeSource* cube =vtkCubeSource::New();
cube->SetBounds(10,50,10,50,-10,50);
vtkTriangleFilter *triangle = vtkTriangleFilter::New();
triangle->SetInput(cube->GetOutput());
vtkLinearSubdivisionFilter  * subdivs=vtkLinearSubdivisionFilter::New();
subdivs->SetInput(triangle->GetOutput());
subdivs->SetNumberOfSubdivisions(2);
m_ClipPolyDataFilter->SetInput(m_SubdivisionFilter->GetOutput());
    mitk::BoundingBox::BoundsArrayType bounds=m_ClippingGeometry->GetBounds();
    m_ClippingBox->SetBounds(bounds[0], bounds[1], bounds[2], bounds[3], bounds[4], bounds[5]);
//    m_ClippingBox->SetTransform(m_ClippingGeometry->GetVtkTransform());
    m_ClipPolyDataFilter->SetClipFunction(m_ClippingBox);
    m_ClipPolyDataFilter->SetValue(0);
    SetVtkPolyData(m_ClipPolyDataFilter->GetOutput());
    //vtkSampleFunction *sample = vtkSampleFunction::New();
    //sample->SetImplicitFunction(m_ClippingBox);
    //sample->SetSampleDimensions(bounds[1]+20, bounds[3]+20, bounds[5]+20);
    //sample->SetModelBounds(-10, bounds[1]+10, -10, bounds[3]+10, -10, bounds[5]+10);
    //sample->SetOutputScalarTypeToFloat ();
    //sample->Update();
    //mitk::Image::Pointer image=mitk::Image::New();
    //image->Initialize(sample->GetOutput());
    //image->SetVolume(sample->GetOutput()->GetScalarPointer());
    //ipPicPut("C:\\clip.pic", image->GetPic());
    //sample->Delete();
  }

  //calculate polygon for IsInside test
  int numPts;
  float x[3], xProj[3];

  itk2vtk(m_Vector, m_Normal);

  numPts = loopPoints->GetNumberOfPoints();
  this->m_Polygon->Points->SetNumberOfPoints(numPts);

  //if ( this->AutomaticNormalGeneration )
  //  {
  //  // Make sure points define a loop with a m_Normal
  //  vtkPolygon::ComputeNormal(this->loopPoints, this->Normal);
  //  if ( this->Normal[0] == 0.0 && this->Normal[1] == 0.0 && 
  //       this->Normal[2] == 0.0 )
  //    {
  //    vtkErrorMacro(<<"Cannot determine inside/outside of loop");
  //    }
  //  }

  // Determine origin point by taking average
  mitk::FillVector3D(m_Origin, 0.0, 0.0, 0.0);
  for (i=0; i<numPts; i++)
  {
    loopPoints->GetPoint(i, x);
    this->m_Origin[0] += x[0]; this->m_Origin[1] += x[1]; this->m_Origin[2] += x[2];
  }
  this->m_Origin[0] /= numPts; this->m_Origin[1] /= numPts; this->m_Origin[2] /= numPts;

  // Project points onto plane generating new coordinates
  for (i=0; i<numPts; i++)
  {
    loopPoints->GetPoint(i, x);
    vtkPlane::ProjectPoint(x, this->m_Origin, m_Normal, xProj);
    this->m_Polygon->Points->SetPoint(i, xProj);
  }

  this->m_Polygon->GetBounds(this->m_ProjectedContourBounds);

  loopPoints->Delete();

  m_LastCalculateExtrusionTime.Modified();
}

unsigned long mitk::ExtrudedContour::GetMTime() const
{
  unsigned long latestTime = Superclass::GetMTime();
  if(m_Contour.IsNotNull())
	{
    unsigned long localTime;
    localTime = m_Contour->GetMTime();
    if(localTime > latestTime) latestTime = localTime;
	}
  return latestTime;
}
