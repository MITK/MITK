#include "mitkGeometry3D.h"
#include "mitkPlaneGeometry.h"
#include "mitkOperation.h"
#include "mitkPointOperation.h"
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
  assert(m_BoundingBoxes[t].IsNotNull());

  return m_BoundingBoxes[t];
}

//##ModelId=3ED91D050299
void mitk::Geometry3D::SetBoundingBox(const mitk::BoundingBox* boundingBox,  int t)
{
  if(IsValidTime(t))
    m_BoundingBoxes[t]=boundingBox;
  else
    itkExceptionMacro("tried to set boundingbox for an invalid point of time (t:"<<t<<")");
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
    
    p[0]=bounds[1]; p[2]=bounds[3]; p[2]=bounds[5];
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

//New:

/*!
    /brief Returns the actual global transformation
    There are 4 cases:
    - the local transformation has not changed and the transformation of the base geometry has not changed
    - the local transformation has not changed but the transformation of the base geometry has changed
    - the local transformation has changed and the transformation of the base geometry has not changed
    - both the local and the base transformations have changed    
*/
vtkTransform* mitk::Geometry3D::GetTransform()
{
/*  bool baseGeometryChanged = (m_BaseTransformTimeStamp != m_BaseGeometry->GetTransform()->GetMTime());
  bool ownGeometryChanged =  (m_TransformTimeStamp != m_RelativeTransform->GetMTime());
  if (ownGeometryChanged)
    if (baseGeometryChanged)
    { // case 4
      // must update relative tranform, so that basetransform * relativetransform = own transform
      // and must update own transform somehow...
    }
    else
    { // case 3
      // must update relative tranform, so that basetransform * relativetransform = own transform

    }
  else  //own Geometry has not changed
    if (baseGeometryChanged)
    { // case 2
      // must update own transform, so that basetransform * relative transform = own transform
      m_Transform = m_BaseGeometry->GetTransform()
      return m_Transform;  
    }
    else
    { // case 1
      // nothing has changed, just return the transform
      return m_Transform;
    }
*/
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
  mitk::PointOperation *pointOp = dynamic_cast<mitk::PointOperation *>(operation);
	if (pointOp == NULL)
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
    mitk::ITKPoint3D newPos = pointOp->GetPoint();
    //std::cout << "Geometry: received move Operation: <" << newPos[0] << ", " << newPos[1] << ", " << newPos[2] << ">\n";    
    GetTransform()->GetMatrix()->SetElement(0, 3, newPos[0]);
    GetTransform()->GetMatrix()->SetElement(1, 3, newPos[1]);
    GetTransform()->GetMatrix()->SetElement(2, 3, newPos[2]);
    float pos[3];     
    GetTransform()->GetPosition(pos);
    std::cout << "Geometry: new Position: <" << pos[0] << ", " << pos[1] << ", " << pos[2] << ">\n";
    break;
  }		
	case OpSCALE:
  {
    mitk::ITKPoint3D newScale = pointOp->GetPoint();
    //std::cout << "Geometry: received scale Operation: <" << newScale[0] << ", " << newScale[1] << ", " << newScale[2] << ">\n";    
    double scale[3];     
    GetTransform()->GetScale(scale);
    //std::cout << "Geometry: old Scale: <" << scale[0] << ", " << scale[1] << ", " << scale[2] << ">\n";
    
    double e;
    e = GetTransform()->GetMatrix()->GetElement(0, 0);
    e = e / scale[0];
    e = e * newScale[0];
    GetTransform()->GetMatrix()->SetElement(0, 0, e);
    e = GetTransform()->GetMatrix()->GetElement(1, 1);
    e = e / scale[1];
    e = e * newScale[1];
    GetTransform()->GetMatrix()->SetElement(1, 1, e);
    e = GetTransform()->GetMatrix()->GetElement(2, 2);
    e = e / scale[2];
    e = e * newScale[2];
    GetTransform()->GetMatrix()->SetElement(2, 2, e);

    //GetTransform()->GetMatrix()->Modified();
    
    //GetTransform()->Scale(newScale[0], newScale[1], newScale[2]);     

    GetTransform()->GetScale(scale);
    
    std::cout << "Geometry: new Scale: <" << scale[0] << ", " << scale[1] << ", " << scale[2] << ">\n";
    break;
  }
  case OpROTATE:
  {
    break;  
  }
  default:
    NULL;
	}
}