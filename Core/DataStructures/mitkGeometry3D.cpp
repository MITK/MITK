#include "mitkGeometry3D.h"
#include "mitkPlaneGeometry.h"
#include "mitkOperation.h"
#include "mitkAffineTransformationOperation.h"
#include "mitkInteractionConst.h"
#include "mitkStatusBar.h"

#include <vecmath.h>
#include <vtkTransform.h>
#include <vtkMatrix4x4.h>

#ifdef MBI_INTERNAL
extern "C"
{
#include "ipDicom.h"
}
#endif

//##ModelId=3DCBF5D40253
mitk::BoundingBox::ConstPointer mitk::Geometry3D::GetBoundingBox(int t) const
{
  assert(m_BoundingBoxes.size()>0);
  assert(m_BoundingBoxes[t].IsNotNull());

  return m_BoundingBoxes[t];
}

//##ModelId=3ED91D050299
void mitk::Geometry3D::SetBoundingBox(const mitk::BoundingBox* boundingBox,  int t)
{
  if(IsValidTime(t))
    m_BoundingBoxes[t]=boundingBox;
  else
    itkExceptionMacro("tried to set boundingbox for an invalid point of time (t:"<<t<<"). Was Initialize called at all?");
}

//##ModelId=3ED91D050305
void mitk::Geometry3D::SetBoundingBox(const float bounds[6],  int t)
{
  if(IsValidTime(t))
  {
    mitk::BoundingBox::Pointer boundingBox=mitk::BoundingBox::New();

    mitk::BoundingBox::PointsContainer::Pointer pointscontainer=mitk::BoundingBox::PointsContainer::New();
    mitk::ScalarType nullpoint[]={0,0,0};
    mitk::BoundingBox::PointType p(nullpoint);

    mitk::BoundingBox::PointIdentifier pointid=0;

    p[0]=bounds[0]; p[1]=bounds[2]; p[2]=bounds[4];
    pointscontainer->InsertElement(pointid++, p);

    p[0]=bounds[1]; p[1]=bounds[3]; p[2]=bounds[5];
    pointscontainer->InsertElement(pointid++, p);

    boundingBox->SetPoints(pointscontainer);

    boundingBox->ComputeBoundingBox();

    m_BoundingBoxes[t]=boundingBox;
  }
  else
    itkExceptionMacro("tried to set boundingbox for an invalid point of time (t:"<<t<<")");
}

//##ModelId=3DCBF5E9037F
/*!
\todo use parameter t or removed it!!!
*/
double mitk::Geometry3D::GetTime(int t) const
{
  itkExceptionMacro("GetTime not yet supported.");
  return 0;
}

//##ModelId=3DE763C500C4
/*!
\todo use parameter t or removed it!!!
\deprecated New method returns a vtkTransform
*/

//##ModelId=3DDE65D1028A
/*!
\todo use parameter t or removed it!!!
*/
void mitk::Geometry3D::MMToUnits(const mitk::Point3D &pt_mm, mitk::Point3D &pt_units, float t) const
{
  m_TransformMMToUnits.transform(pt_mm, &pt_units);
}

//##ModelId=3DDE65DC0151
/*!
\todo use parameter t or removed it!!!
*/
void mitk::Geometry3D::UnitsToMM(const mitk::Point3D &pt_units, mitk::Point3D &pt_mm, float t) const
{
  m_TransformUnitsToMM.transform(pt_units, &pt_mm);
}

//##ModelId=3E3B986602CF
/*!
\todo use parameter t or removed it!!!
*/
void mitk::Geometry3D::MMToUnits(const mitk::Vector3D &vec_mm, mitk::Vector3D &vec_units, float t) const
{
  m_TransformMMToUnits.transform(vec_mm, &vec_units);
}

//##ModelId=3E3B987503A3
/*!
\todo use parameter t or removed it!!!
*/
void mitk::Geometry3D::UnitsToMM(const mitk::Vector3D &vec_units, mitk::Vector3D &vec_mm, float t) const
{
  m_TransformUnitsToMM.transform(vec_units, &vec_mm);
}

//##ModelId=3E3453C703AF
void mitk::Geometry3D::Initialize(unsigned int timeSteps)
{
  m_TimeSteps = timeSteps;

  //initialize m_TransformOfOrigin and m_Spacing (and m_TransformUnitsToMM/m_TransformMMToUnits).
  m_TransformOfOrigin.setIdentity();

  //initialize bounding box array
  int num=m_TimeSteps;
  BoundingBox::ConstPointer bnull=NULL;
  m_BoundingBoxes.reserve(m_TimeSteps);
  m_BoundingBoxes.assign(num, bnull);

}

// Standard Constructor for the new makro. sets the geometry to 3 dimensions
mitk::Geometry3D::Geometry3D() : m_TimeSteps(0)
{
  m_TransformMMToUnits.setIdentity();
  m_TransformUnitsToMM.setIdentity();

  m_BaseGeometry = NULL; // there is no base geometry, this one is independend (until SetBaseGeometry() is called)
  
  m_Transform = vtkTransform::New();  
  m_Transform->PostMultiply();

  m_Position[0] = 0.0;
  m_Position[1] = 0.0;
  m_Position[2] = 0.0;

  m_Orientation[0] = 0.0;
  m_Orientation[1] = 0.0;
  m_Orientation[2] = 0.0;

  m_Scale[0] = 50.0;
  m_Scale[1] = 50.0;
  m_Scale[2] = 50.0;
}

//##ModelId=3E3456C50067
mitk::Geometry3D::~Geometry3D()
{

}

//##ModelId=3E3BE1F8000C
bool mitk::Geometry3D::IsValidTime(int t) const
{
  return (t>=0) && (t< (int)m_TimeSteps);
}


vtkTransform* mitk::Geometry3D::GetTransform()
{
  return m_Transform;
}

void mitk::Geometry3D::SetBaseGeometry(mitk::Geometry3D* base)
{
  m_Transform->SetInput(base->GetTransform());
  m_BaseGeometry = base;
}

void mitk::Geometry3D::SetMasterTransform(const vtkTransform * transform)
{

}

mitk::Geometry3D::Pointer mitk::Geometry3D::Clone()
{
  mitk::Geometry3D::Pointer newGeometry = Geometry3D::New();
  newGeometry->Initialize(m_TimeSteps);
  newGeometry->GetTransform()->SetMatrix(m_Transform->GetMatrix());
  //newGeometry->GetRelativeTransform()->SetMatrix(m_RelativeTransform->GetMatrix());
  newGeometry->SetPosition(m_Position);
  newGeometry->SetOrientation(m_Orientation);
  newGeometry->SetScale(m_Scale);
  
  return newGeometry;  
}

void mitk::Geometry3D::ExecuteOperation(Operation* operation)
{  
  mitk::AffineTransformationOperation *affineOp = dynamic_cast<mitk::AffineTransformationOperation *>(operation);
	if (affineOp == NULL)
	{
		(StatusBar::GetInstance())->DisplayText("Recieved wrong type of operation!See mitkAffineInteractor.cpp", 10000);
		return;
	}
  ScalarType angle = 0.0;
  mitk::ITKPoint3D rotationVector;
 
  switch (operation->GetOperationType())
	{
	case OpNOTHING:
		break;
	case OpMOVE:
  {
    mitk::ITKPoint3D newPos = affineOp->GetPoint();
    m_Position[0] = newPos[0];
    m_Position[1] = newPos[1];
    m_Position[2] = newPos[2];
    break;
  }
	case OpSCALE:
  {
    mitk::ITKPoint3D newScale = affineOp->GetPoint();    
    m_Scale[0] = newScale[0];
    m_Scale[1] = newScale[1];
    m_Scale[2] = newScale[2];
    break;
  }
  case OpROTATE:
  {
    rotationVector = affineOp->GetPoint();
    angle = affineOp->GetAngle();
    break;  
  }
  default:
    NULL;
	}
  GetTransform()->GetOrientation(m_Orientation);  // save orientation
  
  GetTransform()->Identity();

  // scale
  GetTransform()->Scale(m_Scale[0], m_Scale[1], m_Scale[2]);

  // rotate
  GetTransform()->RotateY(m_Orientation[1]);
  GetTransform()->RotateX(m_Orientation[0]);
  GetTransform()->RotateZ(m_Orientation[2]);
  if (angle != 0.0)
  {
    GetTransform()->RotateWXYZ(angle, rotationVector[0], rotationVector[1], rotationVector[2]);    
  }

  // move back from origin and translate  
  GetTransform()->Translate(m_Position[0], m_Position[1], m_Position[2]);
}

const mitk::ITKVector3D mitk::Geometry3D::GetXAxis()
{
  vtkMatrix4x4* m = GetTransform()->GetMatrix();
  mitk::ITKVector3D v;
  v[0] = m->Element[0][0];
  v[1] = m->Element[1][0];
  v[2] = m->Element[2][0];
  v.Normalize();  
  return v;
}

const mitk::ITKVector3D mitk::Geometry3D::GetYAxis()
{
  vtkMatrix4x4* m = GetTransform()->GetMatrix();
  mitk::ITKVector3D v;
  v[0] = m->Element[0][1];
  v[1] = m->Element[1][1];
  v[2] = m->Element[2][1];
  v.Normalize();
  return v;
}
const mitk::ITKVector3D mitk::Geometry3D::GetZAxis()
{
  vtkMatrix4x4* m = GetTransform()->GetMatrix();
  mitk::ITKVector3D v;
  v[0] = m->Element[0][2];
  v[1] = m->Element[1][2];
  v[2] = m->Element[2][2];
  v.Normalize();
  return v;
}

//printmatrix(vtkMatrix4x4* m)
//{
//  std::cout << "matrix: " << m->Element[0][0] << ", " << m->Element[0][1] << ", " << m->Element[0][2] << ", " << m->Element[0][3] << std::endl;
//  std::cout << "        " << m->Element[1][0] << ", " << m->Element[1][1] << ", " << m->Element[1][2] << ", " << m->Element[0][3] << std::endl;
//  std::cout << "        " << m->Element[2][0] << ", " << m->Element[2][1] << ", " << m->Element[2][2] << ", " << m->Element[0][3] << std::endl;
//  std::cout << "        " << m->Element[3][0] << ", " << m->Element[3][1] << ", " << m->Element[3][2] << ", " << m->Element[3][3] << std::endl;
//}
