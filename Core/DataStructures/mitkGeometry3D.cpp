#include "mitkGeometry3D.h"
#include "mitkPlaneGeometry.h"
#include "mitkOperation.h"
#include "mitkAffineTransformationOperation.h"
#include "mitkInteractionConst.h"
#include "mitkStatusBar.h"

#include <vecmath.h>
#include <vtkTransform.h>

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
  //New
  m_Transform = vtkTransform::New();
  m_BaseGeometry = NULL; // there is no base geometry, this one is independend (until SetBaseGeometry() is called)
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

  switch (operation->GetOperationType())
	{
	case OpNOTHING:
		break;
	case OpMOVE:
  {
    //Save actual scale, position and orientation
    double orientation[3];
    GetTransform()->GetOrientation(orientation);
    double scale[3];
    GetTransform()->GetScale(scale);
    double position[3];
    GetTransform()->GetPosition(position);

    mitk::ITKPoint3D newPos = affineOp->GetPoint();

    //GetTransform()->Translate((newPos[0] - position[0]) / scale[0], (newPos[1] - position[1]) / scale[1], (newPos[2] - position[2]) / scale[0]);  // set new position
    GetTransform()->Identity();               // reset transformation
    GetTransform()->PreMultiply();
    GetTransform()->Translate(newPos[0], newPos[1], newPos[2]);  // set new position
    GetTransform()->RotateZ(orientation[2]);
    GetTransform()->RotateX(orientation[0]);  // restore orientation
    GetTransform()->RotateY(orientation[1]);
    GetTransform()->Scale(scale);             // restore scale factors

    //GetTransform()->GetPosition(position);
    //std::cout << "Geometry: new Position: <" << position[0] << ", " << position[1] << ", " << position[2] << ">\n";
    //GetTransform()->GetScale(scale);
    //std::cout << "Geometry: new Scale: <" << scale[0] << ", " << scale[1] << ", " << scale[2] << ">\n";
    //GetTransform()->GetOrientation(orientation);
    //std::cout << "Geometry: new Orientation: <" << orientation[0] << ", " << orientation[1] << ", " << orientation[2] << ">\n";
    break;
  }
	case OpSCALE:
  {
    //Save actual scale, position and orientation
    double scale[3];
    GetTransform()->GetScale(scale);
    double position[3];
    GetTransform()->GetPosition(position);
    double orientation[3];
    GetTransform()->GetOrientation(orientation);

    mitk::ITKPoint3D newScale = affineOp->GetPoint();

    //GetTransform()->Scale(newScale[0] / scale[0], newScale[1] / scale[1], newScale[2] / scale[2]); // set new scale factors

    GetTransform()->Identity();               // reset transformation
    GetTransform()->PreMultiply();
    GetTransform()->Translate(position);      // restore position
    GetTransform()->RotateZ(orientation[2]);
    GetTransform()->RotateX(orientation[0]);  // restore orientation
    GetTransform()->RotateY(orientation[1]);
    GetTransform()->Scale(newScale[0], newScale[1], newScale[2]);             // new scale factors

    //GetTransform()->GetScale(scale);
    //std::cout << "Geometry: new Scale: <" << scale[0] << ", " << scale[1] << ", " << scale[2] << ">\n";
    break;
  }
  case OpROTATE:
  {
    //Save actual scale, position and orientation
    double scale[3];
    GetTransform()->GetScale(scale);
    double position[3];
    GetTransform()->GetPosition(position);
    double orientation[3];
    GetTransform()->GetOrientation(orientation);

    mitk::ITKPoint3D newRotation = affineOp->GetPoint();

    GetTransform()->Identity();               // reset transformation
    GetTransform()->PreMultiply();
    GetTransform()->Translate(position);      // restore position
    GetTransform()->RotateWXYZ(affineOp->GetAngle(), newRotation[0], newRotation[1], newRotation[2]);
    GetTransform()->RotateZ(orientation[2]);
    GetTransform()->RotateX(orientation[0]);  // restore orientation
    GetTransform()->RotateY(orientation[1]);
    GetTransform()->Scale(scale);             // restore scale factors

    GetTransform()->GetPosition(position);
    std::cout << "Geometry: new Position: <" << position[0] << ", " << position[1] << ", " << position[2] << ">\n";
    GetTransform()->GetScale(scale);
    std::cout << "Geometry: new Scale: <" << scale[0] << ", " << scale[1] << ", " << scale[2] << ">\n";
    GetTransform()->GetOrientation(orientation);
    std::cout << "Geometry: new Orientation: <" << orientation[0] << ", " << orientation[1] << ", " << orientation[2] << ">\n";
    break;
  }
  default:
    NULL;
	}
}