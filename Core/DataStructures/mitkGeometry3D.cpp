#include "mitkGeometry3D.h"
#include "mitkPlaneGeometry.h"
#include "mitkOperation.h"
//#include "mitkAffineTransformationOperation.h"
#include "mitkRotationOperation.h"
#include "mitkPointOperation.h"
#include "mitkInteractionConst.h"
#include "mitkStatusBar.h"
#include <float.h>

#include <vecmath.h>
#include <vtkTransform.h>
#include <vtkMatrix4x4.h>

//void printmatrix(vtkMatrix4x4* m)
//{
//  std::cout << "matrix: " << m->Element[0][0] << ",\t" << m->Element[0][1] << ",\t" << m->Element[0][2] << ",\t" << m->Element[0][3] << std::endl;
//  std::cout << "        " << m->Element[1][0] << ",\t" << m->Element[1][1] << ",\t" << m->Element[1][2] << ",\t" << m->Element[0][3] << std::endl;
//  std::cout << "        " << m->Element[2][0] << ",\t" << m->Element[2][1] << ",\t" << m->Element[2][2] << ",\t" << m->Element[0][3] << std::endl;
//  std::cout << "        " << m->Element[3][0] << ",\t" << m->Element[3][1] << ",\t" << m->Element[3][2] << ",\t" << m->Element[3][3] << std::endl;
//}

//##ModelId=3ED91D050305
void mitk::Geometry3D::SetBoundingBox(const float bounds[6])
{
  mitk::BoundingBox::Pointer boundingBox=mitk::BoundingBox::New();
  
  mitk::BoundingBox::PointsContainer::Pointer pointscontainer=mitk::BoundingBox::PointsContainer::New();
  mitk::ScalarType nullpoint[]={0,0,0};
  mitk::BoundingBox::PointType p(nullpoint);
  
  mitk::BoundingBox::PointIdentifier pointid=0;
  
  //add the upper-left-front and lower-right-back edge points of the plane geometry to the bounding box calculator.
  p[0]=bounds[0]; p[1]=bounds[2]; p[2]=bounds[4];
  pointscontainer->InsertElement(pointid++, p);
 
  p[0]=bounds[1]; p[1]=bounds[3]; p[2]=bounds[5];
  pointscontainer->InsertElement(pointid++, p);

  boundingBox->SetPoints(pointscontainer);
  
  boundingBox->ComputeBoundingBox();
  
  SetBoundingBox(boundingBox);
}

//##ModelId=3DCBF5E9037F
const mitk::TimeBounds& mitk::Geometry3D::GetTimeBoundsInMS() const
{
  return m_TimeBoundsInMS;
}

void mitk::Geometry3D::SetTimeBoundsInMS(const mitk::TimeBounds& timebounds)
{
  m_TimeBoundsInMS = timebounds;
}

//##ModelId=3DE763C500C4
/*!
\todo use parameter t or removed it!!!
\deprecated New method returns a vtkTransform
*/

//##ModelId=3DDE65D1028A
void mitk::Geometry3D::MMToUnits(const mitk::Point3D &pt_mm, mitk::Point3D &pt_units) const
{
  m_TransformMMToUnits.transform(pt_mm, &pt_units);
}

//##ModelId=3DDE65DC0151
void mitk::Geometry3D::UnitsToMM(const mitk::Point3D &pt_units, mitk::Point3D &pt_mm) const
{
  m_TransformUnitsToMM.transform(pt_units, &pt_mm);
}

//##ModelId=3E3B986602CF
void mitk::Geometry3D::MMToUnits(const mitk::Vector3D &vec_mm, mitk::Vector3D &vec_units) const
{
  m_TransformMMToUnits.transform(vec_mm, &vec_units);
}

//##ModelId=3E3B987503A3
void mitk::Geometry3D::UnitsToMM(const mitk::Vector3D &vec_units, mitk::Vector3D &vec_mm) const
{
  m_TransformUnitsToMM.transform(vec_units, &vec_mm);
}

//##ModelId=3E3453C703AF
void mitk::Geometry3D::Initialize()
{
  //initialize m_TransformOfOrigin and m_Spacing (and m_TransformUnitsToMM/m_TransformMMToUnits).
  m_TransformOfOrigin.setIdentity();
}

// Standard Constructor for the new makro. sets the geometry to 3 dimensions
mitk::Geometry3D::Geometry3D() : m_BoundingBox(NULL)
{
  m_TimeBoundsInMS[0]=-ScalarTypeNumericTraits::max(); m_TimeBoundsInMS[1]=ScalarTypeNumericTraits::max();

  m_TransformMMToUnits.setIdentity();
  m_TransformUnitsToMM.setIdentity();

  m_BaseGeometry = NULL; // there is no base geometry, this one is independend (until SetBaseGeometry() is called)
  
  m_Transform = vtkTransform::New();  
  //m_Transform->PostMultiply();  
}

//##ModelId=3E3456C50067
mitk::Geometry3D::~Geometry3D()
{

}

void mitk::Geometry3D::SetBaseGeometry(mitk::Geometry3D* base)
{
  m_Transform->SetInput(base->GetVtkTransform());
  m_BaseGeometry = base;
}

void mitk::Geometry3D::SetMasterTransform(const vtkTransform * transform)
{

}

/*
  \todo set User Matrix/transform also - clone it also?
*/
mitk::Geometry3D::Pointer mitk::Geometry3D::Clone() const
{
  mitk::Geometry3D::Pointer newGeometry = Geometry3D::New();
  newGeometry->Initialize();

  newGeometry->SetBoundingBox(m_BoundingBox);
  newGeometry->SetTimeBoundsInMS(m_TimeBoundsInMS);  
  newGeometry->GetVtkTransform()->SetMatrix(m_Transform->GetMatrix());
  return newGeometry;  
}

vtkTransform* mitk::Geometry3D::GetVtkTransform()
{
  return m_Transform;
}

void mitk::Geometry3D::ExecuteOperation(Operation* operation)
{  
  switch (operation->GetOperationType())
	{
	case OpNOTHING:
		break;
	case OpMOVE:
  {
    mitk::PointOperation *pointOp = dynamic_cast<mitk::PointOperation *>(operation);
	  if (pointOp == NULL)
	  {
		  mitk::StatusBar::DisplayText("Recieved wrong type of operation!See mitkAffineInteractor.cpp", 10000);
		  return;
	  } 
    mitk::ITKPoint3D newPos = pointOp->GetPoint();
    ScalarType data[3];
    m_Transform->GetPosition(data);
    m_Transform->PostMultiply();
    m_Transform->Translate(newPos[0], newPos[1], newPos[2]);    
    m_Transform->PreMultiply();
    break;
  }
	case OpSCALE:
  {
    mitk::PointOperation *pointOp = dynamic_cast<mitk::PointOperation *>(operation);
	  if (pointOp == NULL)
	  {
		  mitk::StatusBar::DisplayText("Recieved wrong type of operation!See mitkAffineInteractor.cpp", 10000);
		  return;
	  } 
    mitk::ITKPoint3D newScale = pointOp->GetPoint();
    ScalarType data[3];
    data[0] = 1 + (newScale[0] / GetXAxis().GetNorm());
    data[1] = 1 + (newScale[1] / GetYAxis().GetNorm());
    data[2] = 1 + (newScale[2] / GetZAxis().GetNorm());  
    
    ScalarType pos[3];
    m_Transform->GetPosition(pos);
    m_Transform->PostMultiply();
    m_Transform->Translate(-pos[0], -pos[1], -pos[2]);
    m_Transform->PreMultiply();
    m_Transform->Scale(data[0], data[1], data[2]);
    m_Transform->PostMultiply();
    m_Transform->Translate(pos[0], pos[1], pos[2]);
    m_Transform->PreMultiply();
    break;
  }
  case OpROTATE:
  {
    mitk::RotationOperation *rotateOp = dynamic_cast<mitk::RotationOperation *>(operation);
    if (rotateOp == NULL)
    {
	    mitk::StatusBar::DisplayText("Recieved wrong type of operation!See mitkAffineInteractor.cpp", 10000);
	    return;
    }
    ITKVector3D rotationVector = rotateOp->GetVectorOfRotation();
    ITKPoint3D center = rotateOp->GetCenterOfRotation();
    ScalarType angle = rotateOp->GetAngleOfRotation();
    //ScalarType pos[3];
    //m_Transform->GetPosition(pos);
    m_Transform->PostMultiply();
    m_Transform->Translate(-center[0], -center[1], -center[2]);
    m_Transform->RotateWXYZ(angle, rotationVector[0], rotationVector[1], rotationVector[2]);
    m_Transform->Translate(center[0], center[1], center[2]);
    m_Transform->PreMultiply();
    break;  
  }
  default:
    NULL;
	}
}

const mitk::ITKVector3D mitk::Geometry3D::GetXAxis()
{
  vtkMatrix4x4* m = GetVtkTransform()->GetMatrix();
  mitk::ITKVector3D v;
  v[0] = m->Element[0][0];
  v[1] = m->Element[1][0];
  v[2] = m->Element[2][0];
  //v.Normalize();  
  return v;
}

const mitk::ITKVector3D mitk::Geometry3D::GetYAxis()
{
  vtkMatrix4x4* m = GetVtkTransform()->GetMatrix();
  mitk::ITKVector3D v;
  v[0] = m->Element[0][1];
  v[1] = m->Element[1][1];
  v[2] = m->Element[2][1];
  //v.Normalize();
  return v;
}
const mitk::ITKVector3D mitk::Geometry3D::GetZAxis()
{
  vtkMatrix4x4* m = GetVtkTransform()->GetMatrix();
  mitk::ITKVector3D v;
  v[0] = m->Element[0][2];
  v[1] = m->Element[1][2];
  v[2] = m->Element[2][2];
  //v.Normalize();
  return v;
}
