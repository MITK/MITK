#include "mitkCuboid.h"
#include "vtkTransform.h"
#include "mitkVector.h"
#include "vtkCubeSource.h"

mitk::Cuboid::Cuboid()
{
  m_Geometry3D->Initialize();
  vtkCubeSource* cube = vtkCubeSource::New();
  cube->SetXLength(2.0);
  cube->SetYLength(2.0);
  cube->SetZLength(2.0);
  SetVtkPolyData(cube->GetOutput());
  cube->Delete();
}

mitk::Cuboid::~Cuboid()
{

}

bool mitk::Cuboid::IsInside(ITKPoint3D worldPoint)
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
  //worldPoint[0] = p[0] * m_Geometry3D->GetXAxis().GetNorm();
  //worldPoint[1] = p[1] * m_Geometry3D->GetYAxis().GetNorm();
  //worldPoint[2] = p[2] * m_Geometry3D->GetZAxis().GetNorm(); 

  // check of point is inside the cuboid    
  //bool retval =(p[0] >= -m_Geometry3D->GetXAxis().GetNorm()) && (p[0] <= m_Geometry3D->GetXAxis().GetNorm()) 
  //          && (p[1] >= -m_Geometry3D->GetYAxis().GetNorm()) && (p[1] <= m_Geometry3D->GetYAxis().GetNorm()) 
  //          && (p[2] >= -m_Geometry3D->GetZAxis().GetNorm()) && (p[2] <= m_Geometry3D->GetZAxis().GetNorm());

  bool retval =(p[0] >= -1) && (p[0] <= 1) 
            && (p[1] >= -1) && (p[1] <= 1) 
            && (p[2] >= -1) && (p[2] <= 1);
  
  return retval;
  
}

void mitk::Cuboid::UpdateOutputInformation()
{  
  float bounds[6]={0,1,0,1,0,1};  
  
  // calculate vector from origin (in the center of the cuboid) to each corner
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

  
  // Easy Method: calculate a bounding box for a not-rotated-cuboid
  //bounds[0] = - m_Geometry3D->GetXAxis().GetNorm();
  //bounds[1] = + m_Geometry3D->GetXAxis().GetNorm();
  //bounds[2] = - m_Geometry3D->GetYAxis().GetNorm();
  //bounds[3] = + m_Geometry3D->GetYAxis().GetNorm();
  //bounds[4] = - m_Geometry3D->GetZAxis().GetNorm();
  //bounds[5] = + m_Geometry3D->GetZAxis().GetNorm();
  // Easy Method: use the diagonal as length of each side of the bounding box
  //ScalarType maxRadius = (m_Geometry3D->GetXAxis() + m_Geometry3D->GetYAxis() + m_Geometry3D->GetZAxis()).GetNorm();
  //bounds[0] = - maxRadius;
  //bounds[1] = + maxRadius;
  //bounds[2] = - maxRadius;
  //bounds[3] = + maxRadius;
  //bounds[4] = - maxRadius;
  //bounds[5] = + maxRadius;

  m_Geometry3D->SetBoundingBox(bounds);
}
