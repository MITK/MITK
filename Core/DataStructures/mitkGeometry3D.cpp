/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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
mitk::Geometry3D::Geometry3D() : m_ParametricBoundingBox(NULL), m_VtkIndexToWorldTransform(NULL), m_FrameOfReferenceID(0)
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

  m_IndexToObjectTransform = TransformType::New();
  m_ObjectToNodeTransform = TransformType::New();

  if(m_IndexToWorldTransform.IsNull())
    m_IndexToWorldTransform = TransformType::New();
  else
    m_IndexToWorldTransform->SetIdentity();

  if(m_VtkIndexToWorldTransform==NULL)
    m_VtkIndexToWorldTransform = vtkTransform::New();
  m_VtkIndexToWorldTransform->Identity();
 
  m_ParametricTransform = m_IndexToWorldTransform;

  m_TimeBoundsInMS[0]=-ScalarTypeNumericTraits::max(); m_TimeBoundsInMS[1]=ScalarTypeNumericTraits::max();

  m_FrameOfReferenceID = 0;
}

void mitk::Geometry3D::TransferItkToVtkTransform()
{
 // copy m_IndexToWorldTransform into m_VtkIndexToWorldTransform 
  if(m_VtkIndexToWorldTransform==NULL)
    m_VtkIndexToWorldTransform = vtkTransform::New();
  vtkMatrix4x4 * vtkmatrix = vtkMatrix4x4::New();
  int i,j;
  for(i=0;i<3;++i)
    for(j=0;j<3;++j)
      vtkmatrix->SetElement(i, j, m_IndexToWorldTransform->GetMatrix().GetVnlMatrix().get(i, j));
  for(i=0;i<3;++i)
    vtkmatrix->SetElement(i, 3, m_IndexToWorldTransform->GetOffset()[i]);
  m_VtkIndexToWorldTransform->SetMatrix(vtkmatrix);
  m_VtkIndexToWorldTransform->Modified();
  vtkmatrix->Delete();
}

void mitk::Geometry3D::TransferVtkToITKTransform()
{
  vtkMatrix4x4* vtkmatrix = m_VtkIndexToWorldTransform->GetMatrix();
  itk::Matrix<mitk::ScalarType,3,3>::InternalMatrixType& vlnMatrix = const_cast<itk::Matrix<mitk::ScalarType,3,3>::InternalMatrixType&>(m_IndexToWorldTransform->GetMatrix().GetVnlMatrix());  

  for ( int i=0; i < 3; ++i)
    for( int j=0; j < 3; ++j )
      vlnMatrix.put( i, j, vtkmatrix->GetElement( i, j ) );      

  itk::AffineTransform<mitk::ScalarType>::OffsetType offset;
  offset[0] = vtkmatrix->GetElement( 0, 3 );
  offset[1] = vtkmatrix->GetElement( 1, 3 );
  offset[2] = vtkmatrix->GetElement( 2, 3 );
  m_IndexToWorldTransform->SetOffset( offset );
  m_IndexToWorldTransform->Modified();
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
  pt_units = m_IndexToWorldTransform->BackTransform(pt_mm);
}

//##ModelId=3DDE65DC0151
void mitk::Geometry3D::UnitsToMM(const mitk::Point3D &pt_units, mitk::Point3D &pt_mm) const
{
  pt_mm = m_ParametricTransform->TransformPoint(pt_units);
}

//##ModelId=3E3B986602CF
void mitk::Geometry3D::MMToUnits(const mitk::Vector3D &vec_mm, mitk::Vector3D &vec_units) const
{
  vec_units = m_IndexToWorldTransform->BackTransform(vec_mm);
}

//##ModelId=3E3B987503A3
void mitk::Geometry3D::UnitsToMM(const mitk::Vector3D &vec_units, mitk::Vector3D &vec_mm) const
{
  vec_mm = m_ParametricTransform->TransformVector(vec_units);
}

void mitk::Geometry3D::SetIndexToWorldTransform(mitk::AffineTransform3D* transform)
{
  if(m_IndexToWorldTransform.GetPointer() != transform)
  {
    Superclass::SetIndexToWorldTransform(transform);
    m_ParametricTransform = m_IndexToWorldTransform;    
    TransferItkToVtkTransform();
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

  newGeometry->GetVtkTransform()->SetMatrix(m_VtkIndexToWorldTransform->GetMatrix());
	//newGeometry->TransferVtkToITKTransform(); //MH


  if(m_ParametricBoundingBox.IsNotNull())
    newGeometry->SetParametricBounds(m_ParametricBoundingBox->GetBounds());

  newGeometry->SetFrameOfReferenceID(GetFrameOfReferenceID());
}

vtkTransform* mitk::Geometry3D::GetVtkTransform()
{
  return m_VtkIndexToWorldTransform;
}

mitk::BoundingBox::Pointer mitk::Geometry3D::CalculateBoundingBoxRelativeToTransform(const mitk::AffineTransform3D* transform)
{
  mitk::BoundingBox::PointsContainer::Pointer pointscontainer=mitk::BoundingBox::PointsContainer::New();

  mitk::BoundingBox::PointIdentifier pointid=0;

  unsigned char i;
  for(i=0; i<8; ++i)
    pointscontainer->InsertElement( pointid++, transform->BackTransformPoint( GetCornerPoint(i) ));

  mitk::BoundingBox::Pointer result = mitk::BoundingBox::New();
  result->SetPoints(pointscontainer);
  result->ComputeBoundingBox();

  return result;
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
    m_VtkIndexToWorldTransform->GetPosition(data);
    m_VtkIndexToWorldTransform->PostMultiply();
    m_VtkIndexToWorldTransform->Translate(newPos[0], newPos[1], newPos[2]);    
    m_VtkIndexToWorldTransform->PreMultiply();
    TransferVtkToITKTransform();
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
    m_VtkIndexToWorldTransform->GetPosition(pos);
    m_VtkIndexToWorldTransform->PostMultiply();
    m_VtkIndexToWorldTransform->Translate(-pos[0], -pos[1], -pos[2]);
    m_VtkIndexToWorldTransform->Translate(-center[0], -center[1], -center[2]);   
    m_VtkIndexToWorldTransform->PreMultiply();
    m_VtkIndexToWorldTransform->Scale(data[0], data[1], data[2]);
    m_VtkIndexToWorldTransform->PostMultiply();
    m_VtkIndexToWorldTransform->Translate(+center[0], +center[1], +center[2]);   
    m_VtkIndexToWorldTransform->Translate(pos[0], pos[1], pos[2]);
    m_VtkIndexToWorldTransform->PreMultiply();
    TransferVtkToITKTransform();
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
    m_VtkIndexToWorldTransform->PostMultiply();
    m_VtkIndexToWorldTransform->Translate(-center[0], -center[1], -center[2]);
    m_VtkIndexToWorldTransform->RotateWXYZ(angle, rotationVector[0], rotationVector[1], rotationVector[2]);
    m_VtkIndexToWorldTransform->Translate(center[0], center[1], center[2]);
    m_VtkIndexToWorldTransform->PreMultiply();
    TransferVtkToITKTransform();
    break;  
  }
  default:
    break;
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
