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


#include "mitkExtrudedContour.h"
#include "mitkVector.h"
#include "mitkBaseProcess.h"

#include <itkSmartPointerForwardReference.txx>

#include <vtkLinearExtrusionFilter.h>
#include <vtkPolyData.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkPlanes.h>
#include <vtkClipPolyData.h>

#include <vtkPolygon.h>
#include <vtkFloatArray.h>
#include <vtkDoubleArray.h>
#include <vtkPlane.h>


//vtkButterflySubdivisionFilter * subdivs;
#include <vtkSampleFunction.h>
#include <vtkTriangleFilter.h>
#include <vtkLinearSubdivisionFilter.h>
#include <vtkImageData.h>
#include <vtkCubeSource.h>

mitk::ExtrudedContour::ExtrudedContour() 
  : m_Contour(NULL), m_ClippingGeometry(NULL), m_AutomaticVectorGeneration(false)
{
  GetTimeSlicedGeometry()->Initialize(1);

  FillVector3D(m_Vector, 0.0, 0.0, 1.0);
  m_RightVector.Fill(0.0);

  m_ExtrusionFilter = vtkLinearExtrusionFilter::New();

  m_ExtrusionFilter->CappingOff();
  m_ExtrusionFilter->SetExtrusionTypeToVectorExtrusion();

#if ((VTK_MAJOR_VERSION > 4) || ((VTK_MAJOR_VERSION==4) && (VTK_MINOR_VERSION>=4) ))
  double vtkvector[3]={0,0,1};
#else
  float vtkvector[3]={0,0,1};
#endif
  // set extrusion vector
  m_ExtrusionFilter->SetVector(vtkvector);

  m_TriangleFilter = vtkTriangleFilter::New();
  m_TriangleFilter->SetInput(m_ExtrusionFilter->GetOutput());

  m_SubdivisionFilter = vtkLinearSubdivisionFilter::New();
  m_SubdivisionFilter->SetInput(m_TriangleFilter->GetOutput());
  m_SubdivisionFilter->SetNumberOfSubdivisions(4);

  m_ClippingBox = vtkPlanes::New();
  m_ClipPolyDataFilter = vtkClipPolyData::New();
  m_ClipPolyDataFilter->SetInput(m_SubdivisionFilter->GetOutput());
  m_ClipPolyDataFilter->SetClipFunction(m_ClippingBox);
  m_ClipPolyDataFilter->InsideOutOn();

  m_Polygon = vtkPolygon::New();
  
  m_ProjectionPlane = mitk::PlaneGeometry::New();
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
#if ((VTK_MAJOR_VERSION > 4) || ((VTK_MAJOR_VERSION==4) && (VTK_MINOR_VERSION>=4) ))
  static double polygonNormal[3]={0.0,0.0,1.0};
#else
  static float polygonNormal[3]={0.0,0.0,1.0};
#endif


  // project point onto plane
  float xt[3];
  itk2vtk(worldPoint, xt);

  xt[0] = worldPoint[0]-m_Origin[0];
  xt[1] = worldPoint[1]-m_Origin[1];
  xt[2] = worldPoint[2]-m_Origin[2];

  float dist=xt[0]*m_Normal[0]+xt[1]*m_Normal[1]+xt[2]*m_Normal[2];
  xt[0] -= dist*m_Normal[0];
  xt[1] -= dist*m_Normal[1];
  xt[2] -= dist*m_Normal[2];

#if ((VTK_MAJOR_VERSION > 4) || ((VTK_MAJOR_VERSION==4) && (VTK_MINOR_VERSION>=4) ))
    double x[3];
#else
   float x[3];
#endif

  x[0] = xt[0]*m_Right[0]+xt[1]*m_Right[1]+xt[2]*m_Right[2];
  x[1] = xt[0]*m_Down[0] +xt[1]*m_Down[1] +xt[2]*m_Down[2];
  x[2] = 0;

#if ((VTK_MAJOR_VERSION > 4) || ((VTK_MAJOR_VERSION==4) && (VTK_MINOR_VERSION>=4) ))
  // determine whether it's in the selection loop and then evaluate point
  // in polygon only if absolutely necessary.
  if ( x[0] >= this->m_ProjectedContourBounds[0] && x[0] <= this->m_ProjectedContourBounds[1] &&
    x[1] >= this->m_ProjectedContourBounds[2] && x[1] <= this->m_ProjectedContourBounds[3] &&
    this->m_Polygon->PointInPolygon(x, m_Polygon->Points->GetNumberOfPoints(),
    ((vtkDoubleArray *)this->m_Polygon->Points->GetData())->GetPointer(0), 
    (double*)const_cast<mitk::ExtrudedContour*>(this)->m_ProjectedContourBounds, polygonNormal) == 1 )
    return true;
  else
    return false;
#else
  // determine whether it's in the selection loop and then evaluate point
  // in polygon only if absolutely necessary.
  if ( x[0] >= this->m_ProjectedContourBounds[0] && x[0] <= this->m_ProjectedContourBounds[1] &&
    x[1] >= this->m_ProjectedContourBounds[2] && x[1] <= this->m_ProjectedContourBounds[3] &&
    this->m_Polygon->PointInPolygon(x, m_Polygon->Points->GetNumberOfPoints(),
    ((vtkFloatArray *)this->m_Polygon->Points->GetData())->GetPointer(0), 
    const_cast<mitk::ExtrudedContour*>(this)->m_ProjectedContourBounds, polygonNormal) == 1 )
    return true;
  else
    return false;
#endif

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
  {
    BuildGeometry();
    BuildSurface();
  }
  //if ( ( m_CalculateBoundingBox ) && ( m_PolyDataSeries.size() > 0 ) )
  //  CalculateBoundingBox();
}

void mitk::ExtrudedContour::BuildSurface()
{
  if(m_Contour.IsNull())
  {
    SetVtkPolyData(NULL);
    return;
  }

  // set extrusion contour
  vtkPolyData *polyData = vtkPolyData::New();
  vtkCellArray *polys   = vtkCellArray::New();

  polys->InsertNextCell(m_Polygon->GetPointIds());
  polyData->SetPoints(m_Polygon->GetPoints());

  //float vtkpoint[3];
  //unsigned int i, numPts = m_Polygon->GetNumberOfPoints();
  //for(i=0; i<numPts; ++i)
  //{
  //  float * vtkpoint = this->m_Polygon->Points->GetPoint(i);

  //  pointids[i]=loopPoints->InsertNextPoint(vtkpoint);
  //}
  //polys->InsertNextCell( i, pointids );
  //delete [] pointids;

  //polyData->SetPoints( loopPoints );
  polyData->SetPolys( polys );
  polys->Delete();

  m_ExtrusionFilter->SetInput(polyData);

  polyData->Delete();

  // set extrusion scale factor
  m_ExtrusionFilter->SetScaleFactor(GetGeometry()->GetExtentInMM(2));
  SetVtkPolyData(m_SubdivisionFilter->GetOutput());
  //if(m_ClippingGeometry.IsNull())
  //{
  //  SetVtkPolyData(m_SubdivisionFilter->GetOutput());
  //}
  //else
  //{
  //  m_ClipPolyDataFilter->SetInput(m_SubdivisionFilter->GetOutput());
  //  mitk::BoundingBox::BoundsArrayType bounds=m_ClippingGeometry->GetBounds();

  //  m_ClippingBox->SetBounds(bounds[0], bounds[1], bounds[2], bounds[3], bounds[4], bounds[5]);
  //  m_ClippingBox->SetTransform(GetGeometry()->GetVtkTransform());
  //  m_ClipPolyDataFilter->SetClipFunction(m_ClippingBox);
  //  m_ClipPolyDataFilter->SetValue(0);
  //  SetVtkPolyData(m_ClipPolyDataFilter->GetOutput());
  //}

  m_LastCalculateExtrusionTime.Modified();
}

void mitk::ExtrudedContour::BuildGeometry()
{
  if(m_Contour.IsNull())
    return;

//  Initialize(1);

  Vector3D nullvector; nullvector.Fill(0.0);
  float xProj[3];
  unsigned int i;
  unsigned int numPts = 20; //m_Contour->GetNumberOfPoints();
  mitk::Contour::PathPointer path = m_Contour->GetContourPath();
  mitk::Contour::PathType::InputType cstart = path->StartOfInput();
  mitk::Contour::PathType::InputType cend   = path->EndOfInput();
  mitk::Contour::PathType::InputType cstep  = (cend-cstart)/numPts;
  mitk::Contour::PathType::InputType ccur;

  // Part I: guarantee/calculate legal vectors

  m_Vector.Normalize();
  itk2vtk(m_Vector, m_Normal);
  // check m_Vector
  if(mitk::Equal(m_Vector, nullvector) || m_AutomaticVectorGeneration)
  {
    if ( m_AutomaticVectorGeneration == false)
      itkWarningMacro("Extrusion vector is 0 ("<< m_Vector << "); trying to use normal of polygon");

    vtkPoints *loopPoints = vtkPoints::New();
    //mitk::Contour::PointsContainerIterator pointsIt = m_Contour->GetPoints()->Begin();
#if ((VTK_MAJOR_VERSION > 4) || ((VTK_MAJOR_VERSION==4) && (VTK_MINOR_VERSION>=4) ))
    double vtkpoint[3];
#else
    float vtkpoint[3];
#endif
    unsigned int i=0;
    for(i=0, ccur=cstart; i<numPts; ++i, ccur+=cstep)
    {
      itk2vtk(path->Evaluate(ccur), vtkpoint);
      loopPoints->InsertNextPoint(vtkpoint);
    }

    // Make sure points define a loop with a m_Normal
    vtkPolygon::ComputeNormal(loopPoints, m_Normal);
    loopPoints->Delete();

    vtk2itk(m_Normal, m_Vector);
    if(mitk::Equal(m_Vector, nullvector))
    {
      itkExceptionMacro("Cannot calculate normal of polygon");
    }
  }
  // check m_RightVector
  if((mitk::Equal(m_RightVector, nullvector)) || (mitk::Equal(m_RightVector*m_Vector, 0.0)==false))
  {
    if(mitk::Equal(m_RightVector, nullvector))
    {
      itkDebugMacro("Right vector is 0. Calculating.");
    }
    else
    {
      itkWarningMacro("Right vector ("<<m_RightVector<<") not perpendicular to extrusion vector "<<m_Vector<<": "<<m_RightVector*m_Vector);
    }
    // calculate a legal m_RightVector
    if( mitk::Equal( m_Vector[1], 0.0f ) == false )
    {
      FillVector3D( m_RightVector, 1.0f, -m_Vector[0]/m_Vector[1], 0.0f ); 
      m_RightVector.Normalize();
    }
    else
    {
      FillVector3D( m_RightVector, 0.0f, 1.0f, 0.0f );
    }
  }
 
  // calculate down-vector
  VnlVector rightDV = m_RightVector.Get_vnl_vector(); rightDV.normalize(); vnl2vtk(rightDV, m_Right);
  VnlVector downDV  = vnl_cross_3d( m_Vector.Get_vnl_vector(), rightDV ); downDV.normalize();  vnl2vtk(downDV,  m_Down);
  
  // Part II: calculate plane as base for extrusion, project the contour
  // on this plane and store as polygon for IsInside test and BoundingBox calculation

  // initialize m_ProjectionPlane, yet with origin at 0
  m_ProjectionPlane->InitializeStandardPlane(rightDV, downDV);

  // create vtkPolygon from contour and simultaneously determine 2D bounds of 
  // contour projected on m_ProjectionPlane
  //mitk::Contour::PointsContainerIterator pointsIt = m_Contour->GetPoints()->Begin();
  m_Polygon->Points->Reset();
  m_Polygon->Points->SetNumberOfPoints(numPts);
  m_Polygon->PointIds->Reset();
  m_Polygon->PointIds->SetNumberOfIds(numPts);
  mitk::Point2D pt2d;
  mitk::Point3D pt3d;
  mitk::Point2D min, max;
  min.Fill(ScalarTypeNumericTraits::max());
  max.Fill(ScalarTypeNumericTraits::min());
  xProj[2]=0.0;
  for(i=0, ccur=cstart; i<numPts; ++i, ccur+=cstep)
  {
    pt3d.CastFrom(path->Evaluate(ccur));
    m_ProjectionPlane->Map(pt3d, pt2d);
    xProj[0]=pt2d[0];
    if(pt2d[0]<min[0]) min[0]=pt2d[0];
    if(pt2d[0]>max[0]) max[0]=pt2d[0];
    xProj[1]=pt2d[1];
    if(pt2d[1]<min[1]) min[1]=pt2d[1];
    if(pt2d[1]>max[1]) max[1]=pt2d[1];
    m_Polygon->Points->SetPoint(i, xProj);
    m_Polygon->PointIds->SetId(i, i);
  }
  // shift parametric origin to (0,0)
  for(i=0; i<numPts; ++i)
  {
#if ((VTK_MAJOR_VERSION > 4) || ((VTK_MAJOR_VERSION==4) && (VTK_MINOR_VERSION>=4) ))
    double * pt = this->m_Polygon->Points->GetPoint(i);
#else
    float * pt = this->m_Polygon->Points->GetPoint(i);
#endif
    pt[0]-=min[0]; pt[1]-=min[1];
    itkDebugMacro( << i << ": (" << pt[0] << "," << pt[1] << "," << pt[2] << ")" );
  }
  this->m_Polygon->GetBounds(m_ProjectedContourBounds);
  //m_ProjectedContourBounds[4]=-1.0; m_ProjectedContourBounds[5]=1.0;

  // calculate origin (except translation along the normal) and bounds 
  // of m_ProjectionPlane:
  //  origin is composed of the minimum x-/y-coordinates of the polygon,
  //  bounds from the extent of the polygon, both after projecting on the plane
  mitk::Point3D origin;
  m_ProjectionPlane->Map(min, origin);
  ScalarType bounds[6]={0, max[0]-min[0], 0, max[1]-min[1], 0, 1};
  m_ProjectionPlane->SetBounds(bounds);
  m_ProjectionPlane->SetOrigin(origin);

  // Part III: initialize geometry
  if(m_ClippingGeometry.IsNotNull())
  {
    ScalarType min_dist=ScalarTypeNumericTraits::max(), max_dist=ScalarTypeNumericTraits::min(), dist;
    unsigned char i;
    for(i=0; i<8; ++i)
    {
      dist = m_ProjectionPlane->SignedDistance( m_ClippingGeometry->GetCornerPoint(i) );
      if(dist<min_dist) min_dist=dist;
      if(dist>max_dist) max_dist=dist;
    }
    //incorporate translation along the normal into origin
    origin = origin+m_Vector*min_dist;
    m_ProjectionPlane->SetOrigin(origin);
    bounds[5]=max_dist-min_dist;
  }
  else
    bounds[5]=20;

  itk2vtk(origin, m_Origin);

  mitk::TimeSlicedGeometry::Pointer timeGeometry = this->GetTimeSlicedGeometry();
  mitk::Geometry3D::Pointer g3d = timeGeometry->GetGeometry3D( 0 );
  assert( g3d.IsNotNull() );
  g3d->SetBounds(bounds);
  g3d->SetIndexToWorldTransform(m_ProjectionPlane->GetIndexToWorldTransform());
  g3d->TransferItkToVtkTransform();
  timeGeometry->InitializeEvenlyTimed(g3d, 1);
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
