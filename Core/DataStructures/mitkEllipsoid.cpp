#include "mitkEllipsoid.h"
#include "vtkTransform.h"
#include "mitkVector.h"
#include "vtkSphereSource.h"

mitk::Ellipsoid::Ellipsoid()
{
  m_Geometry3D->Initialize();
  vtkSphereSource* sphere = vtkSphereSource::New();
  sphere->SetRadius(1.0);
  sphere->SetThetaResolution(20);
  sphere->SetPhiResolution(20);
  SetVtkPolyData(sphere->GetOutput());
  sphere->Delete();
}

mitk::Ellipsoid::~Ellipsoid()
{
}

bool mitk::Ellipsoid::IsInside(ITKPoint3D worldPoint)
{
  // transform point from world to object coordinates
  ScalarType p[4];
  p[0] = worldPoint[0];
  p[1] = worldPoint[1];
  p[2] = worldPoint[2];
  p[3] = 1;
  m_Geometry3D->GetVtkTransform()->GetInverse()->TransformPoint(p, p);
	mitk::ITKPoint3D itkPoint;

  // reapply scaling
  p[0] = p[0] * m_Geometry3D->GetXAxis().GetNorm();
  p[1] = p[1] * m_Geometry3D->GetYAxis().GetNorm();
  p[2] = p[2] * m_Geometry3D->GetZAxis().GetNorm(); 

  // check of point is inside the ellipsoid
  return (   pow(p[0], 2)/pow(m_Geometry3D->GetXAxis().GetNorm(), 2) 
           + pow(p[1], 2)/pow(m_Geometry3D->GetYAxis().GetNorm(), 2)
           + pow(p[2], 2)/pow(m_Geometry3D->GetZAxis().GetNorm(), 2)
          <= 1);
}
