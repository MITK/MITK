#include "mitkGeometry3D.h"
#include "mitkOperation.h"
#include "mitkRotationOperation.h"
#include "mitkPointOperation.h"
#include "mitkInteractionConst.h"
#include "mitkStatusBar.h"
#include <float.h>

#include <vtkTransform.h>
#include <vtkMatrix4x4.h>

// Standard Constructor for the new makro. sets the geometry to 3 dimensions
mitk::Geometry3D::Geometry3D() : m_ParametricBoundingBox(NULL), m_VtkUnitsToMMAffineTransform(NULL)
{
  Initialize();
}

//##ModelId=3E3456C50067
mitk::Geometry3D::~Geometry3D()
{

}

//##ModelId=3E3453C703AF
void mitk::Geometry3D::Initialize()
{
  float b[6] = {0,1,0,1,0,1};
  SetFloatBounds(b);

  Superclass::Initialize();

  TransferItkToVtkTransform();
 
  m_ParametricTransform = m_UnitsToMMAffineTransform;

  m_TimeBoundsInMS[0]=-ScalarTypeNumericTraits::max(); m_TimeBoundsInMS[1]=ScalarTypeNumericTraits::max();
}

void mitk::Geometry3D::TransferItkToVtkTransform()
{
 // copy m_UnitsToMMAffineTransform into m_VtkUnitsToMMAffineTransform 
  m_VtkUnitsToMMAffineTransform = vtkTransform::New();
  vtkMatrix4x4 * vtkmatrix = vtkMatrix4x4::New();
  vtkmatrix->Identity();
  int i,j;
  for(i=0;i<3;++i)
    for(j=0;j<3;++j)
      vtkmatrix->SetElement(i, j, m_UnitsToMMAffineTransform->GetMatrix().GetVnlMatrix().get(i, j));
  for(i=0;i<3;++i)
    vtkmatrix->SetElement(i, 3, m_UnitsToMMAffineTransform->GetOffset()[i]);
  m_VtkUnitsToMMAffineTransform->SetMatrix(vtkmatrix);
  vtkmatrix->Delete();
}

void mitk::Geometry3D::SetTimeBoundsInMS(const TimeBounds& timebounds)
{
  if(m_TimeBoundsInMS != timebounds)
  {
    m_TimeBoundsInMS = timebounds;
    Modified();
  }
}
  
void mitk::Geometry3D::SetFloatBounds(const float bounds[6])
{
  mitk::BoundingBox::BoundsArrayType b;
  b=bounds;
  SetBoundsArray(b, m_BoundingBox);
}

void mitk::Geometry3D::SetFloatParametricBounds(const float bounds[6])
{
  mitk::BoundingBox::BoundsArrayType b;
  b=bounds;
  SetBoundsArray(b, m_ParametricBoundingBox);
}

void mitk::Geometry3D::SetParametricBounds(const BoundingBox::BoundsArrayType& bounds)
{
  SetBoundsArray(bounds, m_ParametricBoundingBox);
}

void mitk::Geometry3D::MMToUnits(const mitk::Point3D &pt_mm, mitk::Point3D &pt_units) const
{
  pt_units = m_UnitsToMMAffineTransform->BackTransform(pt_mm);
}

//##ModelId=3DDE65DC0151
void mitk::Geometry3D::UnitsToMM(const mitk::Point3D &pt_units, mitk::Point3D &pt_mm) const
{
  pt_mm = m_ParametricTransform->TransformPoint(pt_units);
}

//##ModelId=3E3B986602CF
void mitk::Geometry3D::MMToUnits(const mitk::Vector3D &vec_mm, mitk::Vector3D &vec_units) const
{
  vec_units = m_UnitsToMMAffineTransform->BackTransform(vec_mm);
}

//##ModelId=3E3B987503A3
void mitk::Geometry3D::UnitsToMM(const mitk::Vector3D &vec_units, mitk::Vector3D &vec_mm) const
{
  vec_mm = m_ParametricTransform->TransformVector(vec_units);
}

void mitk::Geometry3D::SetUnitsToMMAffineTransform(mitk::AffineTransform3D* transform)
{
  if(m_UnitsToMMAffineTransform.GetPointer() != transform)
  {
    Superclass::SetUnitsToMMAffineTransform(transform);
    m_ParametricTransform = m_UnitsToMMAffineTransform;
    //m_VtkUnitsToMMAffineTransform = @FIXME connect vtk and itk transforms!!!
    Modified();
  }
}

/*
  \todo set User Matrix/transform also - clone it also?
*/
mitk::AffineGeometryFrame3D::Pointer mitk::Geometry3D::Clone() const
{
  Self::Pointer newGeometry = Self::New();
  newGeometry->Initialize();
  InitializeGeometry(newGeometry);
  return newGeometry.GetPointer();
}

void mitk::Geometry3D::InitializeGeometry(Geometry3D * newGeometry) const
{
  Superclass::InitializeGeometry(newGeometry);

  newGeometry->SetTimeBoundsInMS(m_TimeBoundsInMS);  

  newGeometry->GetVtkTransform()->SetMatrix(m_VtkUnitsToMMAffineTransform->GetMatrix());

  if(m_ParametricBoundingBox.IsNotNull())
    newGeometry->SetParametricBounds(m_ParametricBoundingBox->GetBounds());
}

vtkTransform* mitk::Geometry3D::GetVtkTransform()
{
  return m_VtkUnitsToMMAffineTransform;
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
		  mitk::StatusBar::DisplayText("received wrong type of operation!See mitkAffineInteractor.cpp", 10000);
		  return;
	  } 
    mitk::Point3D newPos = pointOp->GetPoint();
    ScalarType data[3];
    m_VtkUnitsToMMAffineTransform->GetPosition(data);
    m_VtkUnitsToMMAffineTransform->PostMultiply();
    m_VtkUnitsToMMAffineTransform->Translate(newPos[0], newPos[1], newPos[2]);    
    m_VtkUnitsToMMAffineTransform->PreMultiply();
    break;
  }
	case OpSCALE:
  {
    mitk::PointOperation *pointOp = dynamic_cast<mitk::PointOperation *>(operation);
	  if (pointOp == NULL)
	  {
		  mitk::StatusBar::DisplayText("received wrong type of operation!See mitkAffineInteractor.cpp", 10000);
		  return;
	  } 
    mitk::Point3D newScale = pointOp->GetPoint();
    ScalarType data[3];
    /* calculate new scale: newscale = oldscale * (oldscale + scaletoadd)/oldscale */
    data[0] = 1 + (newScale[0] / GetXAxis().GetNorm());
    data[1] = 1 + (newScale[1] / GetYAxis().GetNorm());
    data[2] = 1 + (newScale[2] / GetZAxis().GetNorm());  

    mitk::Point3D center = const_cast<mitk::BoundingBox*>(m_BoundingBox.GetPointer())->GetCenter();
    ScalarType pos[3];
    m_VtkUnitsToMMAffineTransform->GetPosition(pos);
    m_VtkUnitsToMMAffineTransform->PostMultiply();
    m_VtkUnitsToMMAffineTransform->Translate(-pos[0], -pos[1], -pos[2]);
    m_VtkUnitsToMMAffineTransform->Translate(-center[0], -center[1], -center[2]);   
    m_VtkUnitsToMMAffineTransform->PreMultiply();
    m_VtkUnitsToMMAffineTransform->Scale(data[0], data[1], data[2]);
    m_VtkUnitsToMMAffineTransform->PostMultiply();
    m_VtkUnitsToMMAffineTransform->Translate(+center[0], +center[1], +center[2]);   
    m_VtkUnitsToMMAffineTransform->Translate(pos[0], pos[1], pos[2]);
    m_VtkUnitsToMMAffineTransform->PreMultiply();
    break;
  }
  case OpROTATE:
  {
    mitk::RotationOperation *rotateOp = dynamic_cast<mitk::RotationOperation *>(operation);
    if (rotateOp == NULL)
    {
	    mitk::StatusBar::DisplayText("received wrong type of operation!See mitkAffineInteractor.cpp", 10000);
	    return;
    }
    Vector3D rotationVector = rotateOp->GetVectorOfRotation();
    Point3D center = rotateOp->GetCenterOfRotation();
    ScalarType angle = rotateOp->GetAngleOfRotation();
    angle = (angle < -360) ? -360 : angle;
    angle = (angle >  360) ?  360 : angle;
    m_VtkUnitsToMMAffineTransform->PostMultiply();
    m_VtkUnitsToMMAffineTransform->Translate(-center[0], -center[1], -center[2]);
    m_VtkUnitsToMMAffineTransform->RotateWXYZ(angle, rotationVector[0], rotationVector[1], rotationVector[2]);
    m_VtkUnitsToMMAffineTransform->Translate(center[0], center[1], center[2]);
    m_VtkUnitsToMMAffineTransform->PreMultiply();
    break;  
  }
  default:
    NULL;
	}
}

const mitk::Vector3D mitk::Geometry3D::GetXAxis()
{
  vtkMatrix4x4* m = GetVtkTransform()->GetMatrix();
  mitk::Vector3D v;
  v[0] = m->Element[0][0];
  v[1] = m->Element[1][0];
  v[2] = m->Element[2][0];
  return v;
}

const mitk::Vector3D mitk::Geometry3D::GetYAxis()
{
  vtkMatrix4x4* m = GetVtkTransform()->GetMatrix();
  mitk::Vector3D v;
  v[0] = m->Element[0][1];
  v[1] = m->Element[1][1];
  v[2] = m->Element[2][1];
  return v;
}
const mitk::Vector3D mitk::Geometry3D::GetZAxis()
{
  vtkMatrix4x4* m = GetVtkTransform()->GetMatrix();
  mitk::Vector3D v;
  v[0] = m->Element[0][2];
  v[1] = m->Element[1][2];
  v[2] = m->Element[2][2];
  return v;
}
