/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include <sstream>
#include <iomanip>

#include "mitkBaseGeometry.h"
#include "mitkVector.h"
#include "mitkMatrixConvert.h"
#include <vtkMatrixToLinearTransform.h>
#include <vtkMatrix4x4.h>
#include "mitkRotationOperation.h"
#include "mitkRestorePlanePositionOperation.h"
#include "mitkApplyTransformMatrixOperation.h"
#include "mitkPointOperation.h"
#include "mitkInteractionConst.h"
#include "mitkModifiedLock.h"

mitk::BaseGeometry::BaseGeometry(): Superclass(), mitk::OperationActor(),
  m_FrameOfReferenceID(0), m_IndexToWorldTransformLastModified(0), m_ImageGeometry(false), m_ModifiedLockFlag(false), m_ModifiedCalledFlag(false)
{
  m_VtkMatrix = vtkMatrix4x4::New();
  m_VtkIndexToWorldTransform = vtkMatrixToLinearTransform::New();
  m_VtkIndexToWorldTransform->SetInput(m_VtkMatrix);
  Initialize();
}

mitk::BaseGeometry::BaseGeometry(const BaseGeometry& other): Superclass(), mitk::OperationActor(), //m_TimeBounds(other.m_TimeBounds),
  m_FrameOfReferenceID(other.m_FrameOfReferenceID), m_IndexToWorldTransformLastModified(other.m_IndexToWorldTransformLastModified), m_Origin(other.m_Origin),
  m_ImageGeometry(other.m_ImageGeometry), m_ModifiedLockFlag(false), m_ModifiedCalledFlag(false)
{
  m_VtkMatrix = vtkMatrix4x4::New();
  m_VtkIndexToWorldTransform = vtkMatrixToLinearTransform::New();
  m_VtkIndexToWorldTransform->SetInput(m_VtkMatrix);
  other.InitializeGeometry(this);
}

mitk::BaseGeometry::~BaseGeometry()
{
  m_VtkMatrix->Delete();
  m_VtkIndexToWorldTransform->Delete();
}

const mitk::Point3D& mitk::BaseGeometry::GetOrigin() const
{
  return m_Origin;
}

void mitk::BaseGeometry::SetOrigin(const Point3D & origin)
{
  mitk::ModifiedLock lock(this);

  if(origin!=GetOrigin())
  {
    m_Origin = origin;
    m_IndexToWorldTransform->SetOffset(m_Origin.GetVectorFromOrigin());
    Modified();
    TransferItkToVtkTransform();
  }
}

void mitk::BaseGeometry::TransferItkToVtkTransform()
{
  mitk::ModifiedLock lock(this);
  // copy m_IndexToWorldTransform into m_VtkIndexToWorldTransform

  TransferItkTransformToVtkMatrix(m_IndexToWorldTransform.GetPointer(), m_VtkMatrix);
  m_VtkIndexToWorldTransform->Modified();
}

static void CopySpacingFromTransform(mitk::AffineTransform3D* transform, mitk::Vector3D& spacing)
{
  mitk::AffineTransform3D::MatrixType::InternalMatrixType vnlmatrix;
  vnlmatrix = transform->GetMatrix().GetVnlMatrix();

  spacing[0]=vnlmatrix.get_column(0).magnitude();
  spacing[1]=vnlmatrix.get_column(1).magnitude();
  spacing[2]=vnlmatrix.get_column(2).magnitude();
}

void mitk::BaseGeometry::Initialize()
{
  float b[6] = {0,1,0,1,0,1};
  SetFloatBounds(b);

  if(m_IndexToWorldTransform.IsNull())
    m_IndexToWorldTransform = TransformType::New();
  else
    m_IndexToWorldTransform->SetIdentity();
  CopySpacingFromTransform(m_IndexToWorldTransform, m_Spacing);
  vtk2itk(m_IndexToWorldTransform->GetOffset(), m_Origin);

  m_VtkMatrix->Identity();

  //m_TimeBounds[0]=ScalarTypeNumericTraits::NonpositiveMin(); m_TimeBounds[1]=ScalarTypeNumericTraits::max();

  m_FrameOfReferenceID = 0;

  m_ImageGeometry = false;

  this->PostInitialize();
}

void mitk::BaseGeometry::PostInitializeGeometry(BaseGeometry * newGeometry) const
{
  newGeometry->m_ImageGeometry = m_ImageGeometry;
}

void mitk::BaseGeometry::SetFloatBounds(const float bounds[6])
{
  mitk::BoundingBox::BoundsArrayType b;
  const float *input = bounds;
  int i=0;
  for(mitk::BoundingBox::BoundsArrayType::Iterator it = b.Begin(); i < 6 ;++i) *it++ = (mitk::ScalarType)*input++;
  SetBounds(b);
}

void mitk::BaseGeometry::SetFloatBounds(const double bounds[6])
{
  mitk::BoundingBox::BoundsArrayType b;
  const double *input = bounds;
  int i=0;
  for(mitk::BoundingBox::BoundsArrayType::Iterator it = b.Begin(); i < 6 ;++i) *it++ = (mitk::ScalarType)*input++;
  SetBounds(b);
}

/** Initialize the geometry */
void
  mitk::BaseGeometry::InitializeGeometry(BaseGeometry* newGeometry) const
{
  newGeometry->SetBounds(m_BoundingBox->GetBounds());
  // we have to create a new transform!!
  //newGeometry->SetTimeBounds(m_TimeBounds);
  newGeometry->SetFrameOfReferenceID(GetFrameOfReferenceID());

  if(m_IndexToWorldTransform)
  {
    TransformType::Pointer indexToWorldTransform = m_IndexToWorldTransform->Clone();
    newGeometry->SetIndexToWorldTransform(indexToWorldTransform);
  }

  this->PostInitializeGeometry(newGeometry);
}
void mitk::BaseGeometry::PostInitialize()
{
}

/** Set the bounds */
void mitk::BaseGeometry::SetBounds(const BoundsArrayType& bounds)
{
  mitk::ModifiedLock lock(this);

  PreSetBounds(bounds);

  m_BoundingBox = BoundingBoxType::New();

  BoundingBoxType::PointsContainer::Pointer pointscontainer =
    BoundingBoxType::PointsContainer::New();
  BoundingBoxType::PointType p;
  BoundingBoxType::PointIdentifier pointid;

  for(pointid=0; pointid<2;++pointid)
  {
    unsigned int i;
    for(i=0; i<m_NDimensions; ++i)
    {
      p[i] = bounds[2*i+pointid];
    }
    pointscontainer->InsertElement(pointid, p);
  }

  m_BoundingBox->SetPoints(pointscontainer);
  m_BoundingBox->ComputeBoundingBox();
  this->Modified();
}

void mitk::BaseGeometry::PreSetBounds(const BoundsArrayType& /*bounds*/){};

void mitk::BaseGeometry::SetIndexToWorldTransform(mitk::AffineTransform3D* transform)
{
  mitk::ModifiedLock lock(this);

  PreSetIndexToWorldTransform(transform);

  m_IndexToWorldTransform = transform;
  CopySpacingFromTransform(m_IndexToWorldTransform, m_Spacing);
  vtk2itk(m_IndexToWorldTransform->GetOffset(), m_Origin);
  TransferItkToVtkTransform();
  Modified();

  PostSetIndexToWorldTransform(transform);

}

void mitk::BaseGeometry::PreSetIndexToWorldTransform(mitk::AffineTransform3D* /*transform*/)
{}
void mitk::BaseGeometry::PostSetIndexToWorldTransform(mitk::AffineTransform3D* /*transform*/)
{}

const  mitk::BaseGeometry::BoundsArrayType  mitk::BaseGeometry::GetBounds() const
{
  assert(m_BoundingBox.IsNotNull());
  return m_BoundingBox->GetBounds();
}

bool mitk::BaseGeometry::IsValid() const
{
  return true;
}

void mitk::BaseGeometry::SetSpacing(const mitk::Vector3D& aSpacing, bool enforceSetSpacing )
{
  PreSetSpacing(aSpacing);
  _SetSpacing(aSpacing, enforceSetSpacing);
}

void mitk::BaseGeometry::PreSetSpacing(const mitk::Vector3D& /*aSpacing*/)
{}
void mitk::BaseGeometry::_SetSpacing(const mitk::Vector3D& aSpacing, bool enforceSetSpacing){
  if(mitk::Equal(m_Spacing, aSpacing) == false || enforceSetSpacing)
  {
    assert(aSpacing[0]>0 && aSpacing[1]>0 && aSpacing[2]>0);

    m_Spacing = aSpacing;

    AffineTransform3D::MatrixType::InternalMatrixType vnlmatrix;

    vnlmatrix = m_IndexToWorldTransform->GetMatrix().GetVnlMatrix();

    mitk::VnlVector col;
    col = vnlmatrix.get_column(0); col.normalize(); col*=aSpacing[0]; vnlmatrix.set_column(0, col);
    col = vnlmatrix.get_column(1); col.normalize(); col*=aSpacing[1]; vnlmatrix.set_column(1, col);
    col = vnlmatrix.get_column(2); col.normalize(); col*=aSpacing[2]; vnlmatrix.set_column(2, col);

    Matrix3D matrix;
    matrix = vnlmatrix;

    AffineTransform3D::Pointer transform = AffineTransform3D::New();
    transform->SetMatrix(matrix);
    transform->SetOffset(m_IndexToWorldTransform->GetOffset());

    SetIndexToWorldTransform(transform.GetPointer());
  }
}

mitk::Vector3D mitk::BaseGeometry::GetAxisVector(unsigned int direction) const
{
  Vector3D frontToBack;
  frontToBack.SetVnlVector(m_IndexToWorldTransform->GetMatrix().GetVnlMatrix().get_column(direction));
  frontToBack *= GetExtent(direction);
  return frontToBack;
}

mitk::ScalarType mitk::BaseGeometry::GetExtent(unsigned int direction) const
{
  assert(m_BoundingBox.IsNotNull());
  if (direction>=m_NDimensions)
    mitkThrow() << "Direction is too big. This geometry is for 3D Data";
  BoundsArrayType bounds = m_BoundingBox->GetBounds();
  return bounds[direction*2+1]-bounds[direction*2];
}

bool mitk::BaseGeometry::Is2DConvertable()
{
  bool isConvertableWithoutLoss = true;
  do
  {
    if (this->GetSpacing()[2] != 1)
    {
      isConvertableWithoutLoss = false;
      break;
    }
    if (this->GetOrigin()[2] != 0)
    {
      isConvertableWithoutLoss = false;
      break;
    }
    mitk::Vector3D col0, col1, col2;
    col0.SetVnlVector(this->GetIndexToWorldTransform()->GetMatrix().GetVnlMatrix().get_column(0));
    col1.SetVnlVector(this->GetIndexToWorldTransform()->GetMatrix().GetVnlMatrix().get_column(1));
    col2.SetVnlVector(this->GetIndexToWorldTransform()->GetMatrix().GetVnlMatrix().get_column(2));

    if ((col0[2] != 0) || (col1[2] != 0) || (col2[0] != 0) || (col2[1] != 0) || (col2[2] != 1))
    {
      isConvertableWithoutLoss = false;
      break;
    }
  } while (0);

  return isConvertableWithoutLoss;
}

mitk::Point3D mitk::BaseGeometry::GetCenter() const
{
  assert(m_BoundingBox.IsNotNull());
  return m_IndexToWorldTransform->TransformPoint(m_BoundingBox->GetCenter());
}

double mitk::BaseGeometry::GetDiagonalLength2() const
{
  Vector3D diagonalvector = GetCornerPoint()-GetCornerPoint(false, false, false);
  return diagonalvector.GetSquaredNorm();
}

//##Documentation
//## @brief Get the length of the diagonal of the bounding-box in mm
//##
double mitk::BaseGeometry::GetDiagonalLength() const
{
  return sqrt(GetDiagonalLength2());
}

mitk::Point3D mitk::BaseGeometry::GetCornerPoint(int id) const
{
  assert(id >= 0);
  assert(this->IsBoundingBoxNull()==false);

  BoundingBox::BoundsArrayType bounds = this->GetBoundingBox()->GetBounds();

  Point3D cornerpoint;
  switch(id)
  {
  case 0: FillVector3D(cornerpoint, bounds[0],bounds[2],bounds[4]); break;
  case 1: FillVector3D(cornerpoint, bounds[0],bounds[2],bounds[5]); break;
  case 2: FillVector3D(cornerpoint, bounds[0],bounds[3],bounds[4]); break;
  case 3: FillVector3D(cornerpoint, bounds[0],bounds[3],bounds[5]); break;
  case 4: FillVector3D(cornerpoint, bounds[1],bounds[2],bounds[4]); break;
  case 5: FillVector3D(cornerpoint, bounds[1],bounds[2],bounds[5]); break;
  case 6: FillVector3D(cornerpoint, bounds[1],bounds[3],bounds[4]); break;
  case 7: FillVector3D(cornerpoint, bounds[1],bounds[3],bounds[5]); break;
  default:
    {
      itkExceptionMacro(<<"A cube only has 8 corners. These are labeled 0-7.");
    }
  }
  if(m_ImageGeometry)
  {
    // Here i have to adjust the 0.5 offset manually, because the cornerpoint is the corner of the
    // bounding box. The bounding box itself is no image, so it is corner-based
    FillVector3D(cornerpoint, cornerpoint[0]-0.5, cornerpoint[1]-0.5, cornerpoint[2]-0.5);
  }
  return this->GetIndexToWorldTransform()->TransformPoint(cornerpoint);
}

mitk::Point3D mitk::BaseGeometry::GetCornerPoint(bool xFront, bool yFront, bool zFront) const
{
  assert(this->IsBoundingBoxNull()==false);
  BoundingBox::BoundsArrayType bounds = this->GetBoundingBox()->GetBounds();

  Point3D cornerpoint;
  cornerpoint[0] = (xFront ? bounds[0] : bounds[1]);
  cornerpoint[1] = (yFront ? bounds[2] : bounds[3]);
  cornerpoint[2] = (zFront ? bounds[4] : bounds[5]);
  if(m_ImageGeometry)
  {
    // Here i have to adjust the 0.5 offset manually, because the cornerpoint is the corner of the
    // bounding box. The bounding box itself is no image, so it is corner-based
    FillVector3D(cornerpoint, cornerpoint[0]-0.5, cornerpoint[1]-0.5, cornerpoint[2]-0.5);
  }

  return this->GetIndexToWorldTransform()->TransformPoint(cornerpoint);
}

mitk::ScalarType mitk::BaseGeometry::GetExtentInMM(int direction) const
{
  return m_IndexToWorldTransform->GetMatrix().GetVnlMatrix().get_column(direction).magnitude()*GetExtent(direction);
}

void mitk::BaseGeometry::SetExtentInMM(int direction, ScalarType extentInMM)
{
  mitk::ModifiedLock lock(this);

  ScalarType len = GetExtentInMM(direction);
  if(fabs(len - extentInMM)>=mitk::eps)
  {
    AffineTransform3D::MatrixType::InternalMatrixType vnlmatrix;
    vnlmatrix = m_IndexToWorldTransform->GetMatrix().GetVnlMatrix();
    if(len>extentInMM)
      vnlmatrix.set_column(direction, vnlmatrix.get_column(direction)/len*extentInMM);
    else
      vnlmatrix.set_column(direction, vnlmatrix.get_column(direction)*extentInMM/len);
    Matrix3D matrix;
    matrix = vnlmatrix;
    m_IndexToWorldTransform->SetMatrix(matrix);
    TransferItkToVtkTransform();
    Modified();
  }
  PostSetExtentInMM(direction,extentInMM);
}

void mitk::BaseGeometry::PostSetExtentInMM(int /*direction*/, ScalarType /*extentInMM*/){};

bool mitk::BaseGeometry::IsInside(const mitk::Point3D& p) const
{
  mitk::Point3D index;
  WorldToIndex(p, index);
  return IsIndexInside(index);
}

bool mitk::BaseGeometry::IsIndexInside(const mitk::Point3D& index) const
{
  bool inside = false;
  //if it is an image geometry, we need to convert the index to discrete values
  //this is done by applying the rounding function also used in WorldToIndex (see line 323)
  if (m_ImageGeometry)
  {
    mitk::Point3D discretIndex;
    discretIndex[0]=itk::Math::RoundHalfIntegerUp<mitk::ScalarType>( index[0] );
    discretIndex[1]=itk::Math::RoundHalfIntegerUp<mitk::ScalarType>( index[1] );
    discretIndex[2]=itk::Math::RoundHalfIntegerUp<mitk::ScalarType>( index[2] );

    inside = this->GetBoundingBox()->IsInside(discretIndex);
    //we have to check if the index is at the upper border of each dimension,
    // because the boundingbox is not centerbased
    if (inside)
    {
      const BoundingBox::BoundsArrayType& bounds = this->GetBoundingBox()->GetBounds();
      if((discretIndex[0] == bounds[1]) ||
        (discretIndex[1] == bounds[3]) ||
        (discretIndex[2] == bounds[5]))
        inside = false;
    }
  }
  else
    inside = this->GetBoundingBox()->IsInside(index);

  return inside;
}

void mitk::BaseGeometry::WorldToIndex(const mitk::Point3D &pt_mm, mitk::Point3D &pt_units) const
{
  BackTransform(pt_mm, pt_units);
}

void mitk::BaseGeometry::WorldToIndex( const mitk::Vector3D &vec_mm, mitk::Vector3D &vec_units) const
{
  BackTransform( vec_mm, vec_units);
}

void mitk::BaseGeometry::BackTransform(const mitk::Vector3D& in, mitk::Vector3D& out) const
{
  // Get WorldToIndex transform
  if (m_IndexToWorldTransformLastModified != m_IndexToWorldTransform->GetMTime())
  {
    m_InvertedTransform = TransformType::New();
    if (!m_IndexToWorldTransform->GetInverse( m_InvertedTransform.GetPointer() ))
    {
      itkExceptionMacro( "Internal ITK matrix inversion error, cannot proceed." );
    }
    m_IndexToWorldTransformLastModified = m_IndexToWorldTransform->GetMTime();
  }

  // Check for valid matrix inversion
  const TransformType::MatrixType& inverse = m_InvertedTransform->GetMatrix();
  if(inverse.GetVnlMatrix().has_nans())
  {
    itkExceptionMacro( "Internal ITK matrix inversion error, cannot proceed. Matrix was: " << std::endl
      << m_IndexToWorldTransform->GetMatrix() << "Suggested inverted matrix is:" << std::endl
      << inverse );
  }

  // Transform vector
  for (unsigned int i = 0; i < 3; i++)
  {
    out[i] = 0.0;
    for (unsigned int j = 0; j < 3; j++)
    {
      out[i] += inverse[i][j]*in[j];
    }
  }
}

void mitk::BaseGeometry::BackTransform(const mitk::Point3D &in, mitk::Point3D& out) const
{
  ScalarType temp[3];
  unsigned int i, j;
  const TransformType::OffsetType& offset = m_IndexToWorldTransform->GetOffset();

  // Remove offset
  for (j = 0; j < 3; j++)
  {
    temp[j] = in[j] - offset[j];
  }

  // Get WorldToIndex transform
  if (m_IndexToWorldTransformLastModified != m_IndexToWorldTransform->GetMTime())
  {
    m_InvertedTransform = TransformType::New();
    if (!m_IndexToWorldTransform->GetInverse( m_InvertedTransform.GetPointer() ))
    {
      itkExceptionMacro( "Internal ITK matrix inversion error, cannot proceed." );
    }
    m_IndexToWorldTransformLastModified = m_IndexToWorldTransform->GetMTime();
  }

  // Check for valid matrix inversion
  const TransformType::MatrixType& inverse = m_InvertedTransform->GetMatrix();
  if(inverse.GetVnlMatrix().has_nans())
  {
    itkExceptionMacro( "Internal ITK matrix inversion error, cannot proceed. Matrix was: " << std::endl
      << m_IndexToWorldTransform->GetMatrix() << "Suggested inverted matrix is:" << std::endl
      << inverse );
  }

  // Transform point
  for (i = 0; i < 3; i++)
  {
    out[i] = 0.0;
    for (j = 0; j < 3; j++)
    {
      out[i] += inverse[i][j]*temp[j];
    }
  }
}

mitk::VnlVector mitk::BaseGeometry::GetOriginVnl() const
{
  return const_cast<Self*>(this)->m_Origin.GetVnlVector();
}

vtkLinearTransform* mitk::BaseGeometry::GetVtkTransform() const
{
  return (vtkLinearTransform*)m_VtkIndexToWorldTransform;
}

void mitk::BaseGeometry::SetIdentity()
{
  mitk::ModifiedLock lock(this);

  m_IndexToWorldTransform->SetIdentity();
  m_Origin.Fill(0);
  CopySpacingFromTransform(m_IndexToWorldTransform, m_Spacing);
  Modified();
  TransferItkToVtkTransform();
}

void mitk::BaseGeometry::TransferVtkToItkTransform()
{
  TransferVtkMatrixToItkTransform(m_VtkMatrix, m_IndexToWorldTransform.GetPointer());
  CopySpacingFromTransform(m_IndexToWorldTransform, m_Spacing);
  vtk2itk(m_IndexToWorldTransform->GetOffset(), m_Origin);
  TransferItkToVtkTransform();
}

void mitk::BaseGeometry::Compose( const mitk::BaseGeometry::TransformType * other, bool pre )
{
  mitk::ModifiedLock lock(this);

  m_IndexToWorldTransform->Compose(other, pre);
  CopySpacingFromTransform(m_IndexToWorldTransform, m_Spacing);
  vtk2itk(m_IndexToWorldTransform->GetOffset(), m_Origin);
  Modified();
  TransferItkToVtkTransform();
}

void mitk::BaseGeometry::Compose( const vtkMatrix4x4 * vtkmatrix, bool pre )
{
  mitk::BaseGeometry::TransformType::Pointer itkTransform = mitk::BaseGeometry::TransformType::New();
  TransferVtkMatrixToItkTransform(vtkmatrix, itkTransform.GetPointer());
  Compose(itkTransform, pre);
}

void mitk::BaseGeometry::Translate(const Vector3D & vector)
{
  if((vector[0] != 0) || (vector[1] != 0) || (vector[2] != 0))
  {
    this->SetOrigin(m_Origin + vector);
  }
}

void mitk::BaseGeometry::IndexToWorld(const mitk::Point3D &pt_units, mitk::Point3D &pt_mm) const
{
  pt_mm = m_IndexToWorldTransform->TransformPoint(pt_units);
}

void mitk::BaseGeometry::IndexToWorld(const mitk::Vector3D &vec_units, mitk::Vector3D &vec_mm) const
{
  vec_mm = m_IndexToWorldTransform->TransformVector(vec_units);
}

#include <vtkTransform.h>
void mitk::BaseGeometry::ExecuteOperation(Operation* operation)
{
  mitk::ModifiedLock lock(this);

  vtkTransform *vtktransform = vtkTransform::New();
  vtktransform->SetMatrix(m_VtkMatrix);
  switch (operation->GetOperationType())
  {
  case OpNOTHING:
    break;
  case OpMOVE:
    {
      mitk::PointOperation *pointOp = dynamic_cast<mitk::PointOperation *>(operation);
      if (pointOp == NULL)
      {
        //mitk::StatusBar::GetInstance()->DisplayText("received wrong type of operation!See mitkAffineInteractor.cpp", 10000);
        return;
      }
      mitk::Point3D newPos = pointOp->GetPoint();
      ScalarType data[3];
      vtktransform->GetPosition(data);
      vtktransform->PostMultiply();
      vtktransform->Translate(newPos[0], newPos[1], newPos[2]);
      vtktransform->PreMultiply();
      break;
    }
  case OpSCALE:
    {
      mitk::PointOperation *pointOp = dynamic_cast<mitk::PointOperation *>(operation);
      if (pointOp == NULL)
      {
        //mitk::StatusBar::GetInstance()->DisplayText("received wrong type of operation!See mitkAffineInteractor.cpp", 10000);
        return;
      }
      mitk::Point3D newScale = pointOp->GetPoint();
      ScalarType data[3];
      /* calculate new scale: newscale = oldscale * (oldscale + scaletoadd)/oldscale */
      data[0] = 1 + (newScale[0] / GetMatrixColumn(0).magnitude());
      data[1] = 1 + (newScale[1] / GetMatrixColumn(1).magnitude());
      data[2] = 1 + (newScale[2] / GetMatrixColumn(2).magnitude());

      mitk::Point3D center = const_cast<mitk::BoundingBox*>(m_BoundingBox.GetPointer())->GetCenter();
      ScalarType pos[3];
      vtktransform->GetPosition(pos);
      vtktransform->PostMultiply();
      vtktransform->Translate(-pos[0], -pos[1], -pos[2]);
      vtktransform->Translate(-center[0], -center[1], -center[2]);
      vtktransform->PreMultiply();
      vtktransform->Scale(data[0], data[1], data[2]);
      vtktransform->PostMultiply();
      vtktransform->Translate(+center[0], +center[1], +center[2]);
      vtktransform->Translate(pos[0], pos[1], pos[2]);
      vtktransform->PreMultiply();
      break;
    }
  case OpROTATE:
    {
      mitk::RotationOperation *rotateOp = dynamic_cast<mitk::RotationOperation *>(operation);
      if (rotateOp == NULL)
      {
        //mitk::StatusBar::GetInstance()->DisplayText("received wrong type of operation!See mitkAffineInteractor.cpp", 10000);
        return;
      }
      Vector3D rotationVector = rotateOp->GetVectorOfRotation();
      Point3D center = rotateOp->GetCenterOfRotation();
      ScalarType angle = rotateOp->GetAngleOfRotation();
      vtktransform->PostMultiply();
      vtktransform->Translate(-center[0], -center[1], -center[2]);
      vtktransform->RotateWXYZ(angle, rotationVector[0], rotationVector[1], rotationVector[2]);
      vtktransform->Translate(center[0], center[1], center[2]);
      vtktransform->PreMultiply();
      break;
    }
  case OpRESTOREPLANEPOSITION:
    {
      //Copy necessary to avoid vtk warning
      vtkMatrix4x4* matrix = vtkMatrix4x4::New();
      TransferItkTransformToVtkMatrix(dynamic_cast<mitk::RestorePlanePositionOperation*>(operation)->GetTransform().GetPointer(), matrix);
      vtktransform->SetMatrix(matrix);
      matrix->Delete();
      break;
    }
  case OpAPPLYTRANSFORMMATRIX:
    {
      ApplyTransformMatrixOperation *applyMatrixOp = dynamic_cast< ApplyTransformMatrixOperation* >( operation );
      vtktransform->SetMatrix(applyMatrixOp->GetMatrix());
      break;
    }
  default:
    vtktransform->Delete();
    return;
  }
  m_VtkMatrix->DeepCopy(vtktransform->GetMatrix());
  TransferVtkToItkTransform();
  Modified();
  vtktransform->Delete();
}

mitk::VnlVector mitk::BaseGeometry::GetMatrixColumn(unsigned int direction) const
{
  return m_IndexToWorldTransform->GetMatrix().GetVnlMatrix().get_column(direction);
}

mitk::BoundingBox::Pointer mitk::BaseGeometry::CalculateBoundingBoxRelativeToTransform(const mitk::AffineTransform3D* transform) const
{
  mitk::BoundingBox::PointsContainer::Pointer pointscontainer=mitk::BoundingBox::PointsContainer::New();

  mitk::BoundingBox::PointIdentifier pointid=0;

  unsigned char i;
  if(transform!=NULL)
  {
    mitk::AffineTransform3D::Pointer inverse = mitk::AffineTransform3D::New();
    transform->GetInverse(inverse);
    for(i=0; i<8; ++i)
      pointscontainer->InsertElement( pointid++, inverse->TransformPoint( GetCornerPoint(i) ));
  }
  else
  {
    for(i=0; i<8; ++i)
      pointscontainer->InsertElement( pointid++, GetCornerPoint(i) );
  }

  mitk::BoundingBox::Pointer result = mitk::BoundingBox::New();
  result->SetPoints(pointscontainer);
  result->ComputeBoundingBox();

  return result;
}

//void mitk::BaseGeometry::SetTimeBounds(const TimeBounds& timebounds)
//{
//  mitk::ModifiedLock lock(this);
//
//  if(m_TimeBounds != timebounds)
//  {
//    m_TimeBounds = timebounds;
//    Modified();
//  }
//  PostSetTimeBounds(timebounds);
//}
//
//void mitk::BaseGeometry::PostSetTimeBounds(const TimeBounds& timebounds)
//{}

const std::string mitk::BaseGeometry::GetTransformAsString( TransformType* transformType )
{
  std::ostringstream out;

  out << '[';

  for( int i=0; i<3; ++i )
  {
    out << '[';
    for( int j=0; j<3; ++j )
      out << transformType->GetMatrix().GetVnlMatrix().get(i, j) << ' ';
    out << ']';
  }

  out << "][";

  for( int i=0; i<3; ++i )
    out << transformType->GetOffset()[i] << ' ';

  out << "]\0";

  return out.str();
}

void mitk::BaseGeometry::SetIndexToWorldTransformByVtkMatrix(vtkMatrix4x4* vtkmatrix)
{
  m_VtkMatrix->DeepCopy(vtkmatrix);
  TransferVtkToItkTransform();
}

void mitk::BaseGeometry::WorldToIndex(const mitk::Point3D & /*atPt3d_mm*/, const mitk::Vector3D &vec_mm, mitk::Vector3D &vec_units) const
{
  MITK_WARN<<"Warning! Call of the deprecated function BaseGeometry::WorldToIndex(point, vec, vec). Use BaseGeometry::WorldToIndex(vec, vec) instead!";
  //BackTransform(atPt3d_mm, vec_mm, vec_units);
  this->WorldToIndex(vec_mm, vec_units);
}

void mitk::BaseGeometry::IndexToWorld(const mitk::Point3D &/*atPt3d_units*/, const mitk::Vector3D &vec_units, mitk::Vector3D &vec_mm) const
{
  MITK_WARN<<"Warning! Call of the deprecated function BaseGeometry::IndexToWorld(point, vec, vec). Use BaseGeometry::IndexToWorld(vec, vec) instead!";
  //vec_mm = m_IndexToWorldTransform->TransformVector(vec_units);
  this->IndexToWorld(vec_units, vec_mm);
}

void mitk::BaseGeometry::BackTransform(const mitk::Point3D &/*at*/, const mitk::Vector3D &in, mitk::Vector3D& out) const
{
  MITK_INFO<<"Warning! Call of the deprecated function BaseGeometry::BackTransform(point, vec, vec). Use BaseGeometry::BackTransform(vec, vec) instead!";
  //// Get WorldToIndex transform
  //if (m_IndexToWorldTransformLastModified != m_IndexToWorldTransform->GetMTime())
  //{
  //  m_InvertedTransform = TransformType::New();
  //  if (!m_IndexToWorldTransform->GetInverse( m_InvertedTransform.GetPointer() ))
  //  {
  //    itkExceptionMacro( "Internal ITK matrix inversion error, cannot proceed." );
  //  }
  //  m_IndexToWorldTransformLastModified = m_IndexToWorldTransform->GetMTime();
  //}

  //// Check for valid matrix inversion
  //const TransformType::MatrixType& inverse = m_InvertedTransform->GetMatrix();
  //if(inverse.GetVnlMatrix().has_nans())
  //{
  //  itkExceptionMacro( "Internal ITK matrix inversion error, cannot proceed. Matrix was: " << std::endl
  //    << m_IndexToWorldTransform->GetMatrix() << "Suggested inverted matrix is:" << std::endl
  //    << inverse );
  //}

  //// Transform vector
  //for (unsigned int i = 0; i < 3; i++)
  //{
  //  out[i] = 0.0;
  //  for (unsigned int j = 0; j < 3; j++)
  //  {
  //    out[i] += inverse[i][j]*in[j];
  //  }
  //}
  this->BackTransform(in, out);
}

vtkMatrix4x4* mitk::BaseGeometry::GetVtkMatrix(){
  return m_VtkMatrix;
}

bool mitk::BaseGeometry::IsBoundingBoxNull() const{
  return m_BoundingBox.IsNull();
}

bool mitk::BaseGeometry::IsIndexToWorldTransformNull() const{
  return m_IndexToWorldTransform.IsNull();
}

void
  mitk::BaseGeometry::ChangeImageGeometryConsideringOriginOffset( const bool isAnImageGeometry )
{
  // If Geometry is switched to ImageGeometry, you have to put an offset to the origin, because
  // imageGeometries origins are pixel-center-based
  // ... and remove the offset, if you switch an imageGeometry back to a normal geometry
  // For more information please see the Geometry documentation page

  if(m_ImageGeometry == isAnImageGeometry)
    return;

  const BoundingBox::BoundsArrayType& boundsarray =
    this->GetBoundingBox()->GetBounds();

  Point3D  originIndex;
  FillVector3D(originIndex,  boundsarray[0], boundsarray[2], boundsarray[4]);

  if(isAnImageGeometry == true)
    FillVector3D( originIndex,
    originIndex[0] + 0.5,
    originIndex[1] + 0.5,
    originIndex[2] + 0.5 );
  else
    FillVector3D( originIndex,
    originIndex[0] - 0.5,
    originIndex[1] - 0.5,
    originIndex[2] - 0.5 );

  Point3D originWorld;

  originWorld = GetIndexToWorldTransform()
    ->TransformPoint( originIndex );
  // instead could as well call  IndexToWorld(originIndex,originWorld);

  SetOrigin(originWorld);

  this->SetImageGeometry(isAnImageGeometry);
}
//itk::LightObject::Pointer mitk::BaseGeometry::InternalClone() const
//{
//  Self::Pointer newGeometry = new Self(*this);
//  newGeometry->UnRegister();
//  return newGeometry.GetPointer();
//}

void mitk::BaseGeometry::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  os << indent << " IndexToWorldTransform: ";
  if(this->IsIndexToWorldTransformNull())
    os << "NULL" << std::endl;
  else
  {
    // from itk::MatrixOffsetTransformBase
    unsigned int i, j;
    os << std::endl;
    os << indent << "Matrix: " << std::endl;
    for (i = 0; i < 3; i++)
    {
      os << indent.GetNextIndent();
      for (j = 0; j < 3; j++)
      {
        os << this->GetIndexToWorldTransform()->GetMatrix()[i][j] << " ";
      }
      os << std::endl;
    }

    os << indent << "Offset: " << this->GetIndexToWorldTransform()->GetOffset() << std::endl;
    os << indent << "Center: " << this->GetIndexToWorldTransform()->GetCenter() << std::endl;
    os << indent << "Translation: " << this->GetIndexToWorldTransform()->GetTranslation() << std::endl;

    os << indent << "Inverse: " << std::endl;
    for (i = 0; i < 3; i++)
    {
      os << indent.GetNextIndent();
      for (j = 0; j < 3; j++)
      {
        os << this->GetIndexToWorldTransform()->GetInverseMatrix()[i][j] << " ";
      }
      os << std::endl;
    }

    // from itk::ScalableAffineTransform
    os << indent << "Scale : ";
    for (i = 0; i < 3; i++)
    {
      os << this->GetIndexToWorldTransform()->GetScale()[i] << " ";
    }
    os << std::endl;
  }

  os << indent << " BoundingBox: ";
  if(this->IsBoundingBoxNull())
    os << "NULL" << std::endl;
  else
  {
    os << indent << "( ";
    for (unsigned int i=0; i<3; i++)
    {
      os << this->GetBoundingBox()->GetBounds()[2*i] << "," << this->GetBoundingBox()->GetBounds()[2*i+1] << " ";
    }
    os << " )" << std::endl;
  }

  os << indent << " Origin: " << this->GetOrigin() << std::endl;
  os << indent << " ImageGeometry: " << this->GetImageGeometry() << std::endl;
  os << indent << " Spacing: " << this->GetSpacing() << std::endl;
  //os << indent << " TimeBounds: " << this->GetTimeBounds() << std::endl;
}

void mitk::BaseGeometry::Modified() const{
  if(!m_ModifiedLockFlag)
    Superclass::Modified();
  else
    m_ModifiedCalledFlag = true;
}

bool mitk::Equal( const mitk::BaseGeometry::BoundingBoxType *leftHandSide, const mitk::BaseGeometry::BoundingBoxType *rightHandSide, ScalarType eps, bool verbose )
{
  if(( leftHandSide == NULL) || ( rightHandSide == NULL ))
  {
    MITK_ERROR << "mitk::Equal( const mitk::Geometry3D::BoundingBoxType *leftHandSide, const mitk::Geometry3D::BoundingBoxType *rightHandSide, ScalarType eps, bool verbose ) does not with NULL pointer input.";
    return false;
  }
  return Equal( *leftHandSide, *rightHandSide, eps, verbose);
}

bool mitk::Equal( const mitk::BaseGeometry::BoundingBoxType& leftHandSide, const mitk::BaseGeometry::BoundingBoxType& rightHandSide, ScalarType eps, bool verbose )
{
  bool result = true;

  BaseGeometry::BoundsArrayType rightBounds = rightHandSide.GetBounds();
  BaseGeometry::BoundsArrayType leftBounds = leftHandSide.GetBounds();
  BaseGeometry::BoundsArrayType::Iterator itLeft = leftBounds.Begin();
  for( BaseGeometry::BoundsArrayType::Iterator itRight = rightBounds.Begin(); itRight != rightBounds.End(); ++itRight)
  {
    if(( !mitk::Equal( *itLeft, *itRight, eps )) )
    {
      if(verbose)
      {
        MITK_INFO << "[( Geometry3D::BoundingBoxType )] bounds are not equal.";
        MITK_INFO << "rightHandSide is " << setprecision(12) << *itRight << " : leftHandSide is " << *itLeft << " and tolerance is " << eps;
      }
      result = false;
    }
    itLeft++;
  }
  return result;
}

bool mitk::Equal(const mitk::BaseGeometry *leftHandSide, const mitk::BaseGeometry *rightHandSide, ScalarType eps, bool verbose)
{
  if(( leftHandSide == NULL) || ( rightHandSide == NULL ))
  {
    MITK_ERROR << "mitk::Equal(const mitk::Geometry3D *leftHandSide, const mitk::Geometry3D *rightHandSide, ScalarType eps, bool verbose) does not with NULL pointer input.";
    return false;
  }
  return Equal( *leftHandSide, *rightHandSide, eps, verbose);
}

bool mitk::Equal(const mitk::BaseGeometry& leftHandSide, const mitk::BaseGeometry& rightHandSide, ScalarType eps, bool verbose)
{
  bool result = true;

  //Compare spacings
  if( !mitk::Equal( leftHandSide.GetSpacing(), rightHandSide.GetSpacing(), eps ) )
  {
    if(verbose)
    {
      MITK_INFO << "[( Geometry3D )] Spacing differs.";
      MITK_INFO << "rightHandSide is " << setprecision(12) << rightHandSide.GetSpacing() << " : leftHandSide is " << leftHandSide.GetSpacing() << " and tolerance is " << eps;
    }
    result = false;
  }

  //Compare Origins
  if( !mitk::Equal( leftHandSide.GetOrigin(), rightHandSide.GetOrigin(), eps ) )
  {
    if(verbose)
    {
      MITK_INFO << "[( Geometry3D )] Origin differs.";
      MITK_INFO << "rightHandSide is " << setprecision(12) << rightHandSide.GetOrigin() << " : leftHandSide is " << leftHandSide.GetOrigin() << " and tolerance is " << eps;
    }
    result = false;
  }

  //Compare Axis and Extents
  for( unsigned int i=0; i<3; ++i)
  {
    if( !mitk::Equal( leftHandSide.GetAxisVector(i), rightHandSide.GetAxisVector(i), eps))
    {
      if(verbose)
      {
        MITK_INFO << "[( Geometry3D )] AxisVector #" << i << " differ";
        MITK_INFO << "rightHandSide is " << setprecision(12) << rightHandSide.GetAxisVector(i) << " : leftHandSide is " << leftHandSide.GetAxisVector(i) << " and tolerance is " << eps;
      }
      result =  false;
    }

    if( !mitk::Equal( leftHandSide.GetExtent(i), rightHandSide.GetExtent(i), eps) )
    {
      if(verbose)
      {
        MITK_INFO << "[( Geometry3D )] Extent #" << i << " differ";
        MITK_INFO << "rightHandSide is " << setprecision(12) << rightHandSide.GetExtent(i) << " : leftHandSide is " << leftHandSide.GetExtent(i) << " and tolerance is " << eps;
      }
      result = false;
    }
  }

  //Compare ImageGeometry Flag
  if( rightHandSide.GetImageGeometry() != leftHandSide.GetImageGeometry() )
  {
    if(verbose)
    {
      MITK_INFO << "[( Geometry3D )] GetImageGeometry is different.";
      MITK_INFO << "rightHandSide is " << rightHandSide.GetImageGeometry() << " : leftHandSide is " << leftHandSide.GetImageGeometry();
    }
    result = false;
  }

  //Compare BoundingBoxes
  if( !mitk::Equal( *leftHandSide.GetBoundingBox(), *rightHandSide.GetBoundingBox(), eps, verbose) )
  {
    result = false;
  }

  //Compare IndexToWorldTransform Matrix
  if( !mitk::Equal( *leftHandSide.GetIndexToWorldTransform(), *rightHandSide.GetIndexToWorldTransform(), eps, verbose) )
  {
    result = false;
  }
  return result;
}

bool mitk::Equal(const BaseGeometry::TransformType *leftHandSide, const BaseGeometry::TransformType *rightHandSide, ScalarType eps, bool verbose )
{
  if(( leftHandSide == NULL) || ( rightHandSide == NULL ))
  {
    MITK_ERROR << "mitk::Equal(const Geometry3D::TransformType *leftHandSide, const Geometry3D::TransformType *rightHandSide, ScalarType eps, bool verbose ) does not with NULL pointer input.";
    return false;
  }
  return Equal( *leftHandSide, *rightHandSide, eps, verbose);
}

bool mitk::Equal(const BaseGeometry::TransformType& leftHandSide, const BaseGeometry::TransformType& rightHandSide, ScalarType eps, bool verbose )
{
  //Compare IndexToWorldTransform Matrix
  if( !mitk::MatrixEqualElementWise(  leftHandSide.GetMatrix(),
    rightHandSide.GetMatrix() ) )
  {
    if(verbose)
    {
      MITK_INFO << "[( Geometry3D::TransformType )] Index to World Transformation matrix differs.";
      MITK_INFO << "rightHandSide is " << setprecision(12) << rightHandSide.GetMatrix() << " : leftHandSide is " << leftHandSide.GetMatrix() << " and tolerance is " << eps;
    }
    return false;
  }
  return true;
}
