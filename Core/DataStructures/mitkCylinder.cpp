#include "mitkCylinder.h"
#include "vtkTransform.h"
#include "mitkVector.h"
#include "vtkCylinderSource.h"

mitk::Cylinder::Cylinder()
{
  m_Geometry3D->Initialize();
  vtkCylinderSource* cylinder = vtkCylinderSource::New();
  cylinder->SetRadius(1.0);
  cylinder->SetHeight(2.0);
  cylinder->SetCenter(0.0, 0.0, 0.0);
  cylinder->SetResolution(20);
  cylinder->CappingOn();
  SetVtkPolyData(cylinder->GetOutput());
  cylinder->Delete();
}

mitk::Cylinder::~Cylinder()
{

}

bool mitk::Cylinder::IsInside(ITKPoint3D worldPoint)
{
  // transform point from world to object coordinates
  ScalarType p[4];
  p[0] = worldPoint[0];
  p[1] = worldPoint[1];
  p[2] = worldPoint[2];
  p[3] = 1;
  m_Geometry3D->GetVtkTransform()->GetInverse()->TransformPoint(p, p);
	mitk::ITKPoint3D itkPoint;

  mitk::ScalarType v =  pow(p[0], 2) + pow(p[2], 2);
  bool retval = (v <= 1) && (p[1] >= -1) && (p[1] <= 1);
  return retval; 
}

mitk::ScalarType mitk::Cylinder::GetVolume()
{
  return   m_Geometry3D->GetXAxis().GetNorm() * m_Geometry3D->GetZAxis().GetNorm() 
         * 2 * m_Geometry3D->GetYAxis().GetNorm() * vnl_math::pi;
}
