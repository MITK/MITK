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
  worldPoint[0] = p[0] * m_Geometry3D->GetXAxis().GetNorm();
  worldPoint[1] = p[1] * m_Geometry3D->GetYAxis().GetNorm();
  worldPoint[2] = p[2] * m_Geometry3D->GetZAxis().GetNorm(); 

  // check of point is inside the ellipsoid
  ScalarType value =   pow(worldPoint[0], 2)/pow(m_Geometry3D->GetXAxis().GetNorm(), 2) 
                     + pow(worldPoint[1], 2)/pow(m_Geometry3D->GetYAxis().GetNorm(), 2)
                     + pow(worldPoint[2], 2)/pow(m_Geometry3D->GetZAxis().GetNorm(), 2);
  
  return (value <= 1);
}

void mitk::Ellipsoid::UpdateOutputInformation()
{  
  float bounds[6]={0,1,0,1,0,1};  
  // Easy Method: calculate a bounding box for a not-rotated-ellipsoid
  //bounds[0] = - m_Geometry3D->GetXAxis().GetNorm();
  //bounds[1] = + m_Geometry3D->GetXAxis().GetNorm();
  //bounds[2] = - m_Geometry3D->GetYAxis().GetNorm();
  //bounds[3] = + m_Geometry3D->GetYAxis().GetNorm();
  //bounds[4] = - m_Geometry3D->GetZAxis().GetNorm();
  //bounds[5] = + m_Geometry3D->GetZAxis().GetNorm();
  // Easy Method: use the largest diameter of the ellipsoid as length of each side of the bounding box
  //ScalarType maxRadius = m_Geometry3D->GetXAxis().GetNorm() > m_Geometry3D->GetYAxis().GetNorm() ? m_Geometry3D->GetXAxis().GetNorm() : m_Geometry3D->GetYAxis().GetNorm();
  //maxRadius = maxRadius > m_Geometry3D->GetZAxis().GetNorm() ? maxRadius : m_Geometry3D->GetZAxis().GetNorm();
  //bounds[0] = - maxRadius;
  //bounds[1] = + maxRadius;
  //bounds[2] = - maxRadius;
  //bounds[3] = + maxRadius;
  //bounds[4] = - maxRadius;
  //bounds[5] = + maxRadius;  

  ITKVector3D p; //= m_Geometry3D->GetXAxis() + m_Geometry3D->GetYAxis() + m_Geometry3D->GetZAxis();
  p[0] = fabs(m_Geometry3D->GetXAxis()[0]) + fabs(m_Geometry3D->GetYAxis()[0]) + fabs(m_Geometry3D->GetZAxis()[0]);
  p[1] = fabs(m_Geometry3D->GetXAxis()[1]) + fabs(m_Geometry3D->GetYAxis()[1]) + fabs(m_Geometry3D->GetZAxis()[1]);
  p[2] = fabs(m_Geometry3D->GetXAxis()[2]) + fabs(m_Geometry3D->GetYAxis()[2]) + fabs(m_Geometry3D->GetZAxis()[2]);
  bounds[0] = - p[0];
  bounds[1] = + p[0];
  bounds[2] = - p[1];
  bounds[3] = + p[1];
  bounds[4] = - p[2];
  bounds[5] = + p[2];
  m_Geometry3D->SetBoundingBox(bounds);
}
