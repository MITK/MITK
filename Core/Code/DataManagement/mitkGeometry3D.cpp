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

#include "mitkGeometry3D.h"
#include "mitkMatrixConvert.h"
#include "mitkRotationOperation.h"
#include "mitkRestorePlanePositionOperation.h"
#include "mitkPointOperation.h"
#include "mitkInteractionConst.h"

#include <vtkMatrixToLinearTransform.h>
#include <vtkMatrix4x4.h>

// Standard constructor for the New() macro. Sets the geometry to 3 dimensions
mitk::Geometry3D::Geometry3D()
  : m_ParametricBoundingBox(NULL),
    m_ImageGeometry(false), m_Valid(true), m_FrameOfReferenceID(0), m_IndexToWorldTransformLastModified(0)
{
  FillVector3D(m_FloatSpacing, 1,1,1);
  m_VtkMatrix = vtkMatrix4x4::New();
  m_VtkIndexToWorldTransform = vtkMatrixToLinearTransform::New();
  m_VtkIndexToWorldTransform->SetInput(m_VtkMatrix);
  Initialize();
}
mitk::Geometry3D::Geometry3D(const Geometry3D& other) : Superclass(), mitk::OperationActor(), m_ParametricBoundingBox(other.m_ParametricBoundingBox),m_TimeBounds(other.m_TimeBounds),
  m_ImageGeometry(other.m_ImageGeometry), m_Valid(other.m_Valid), m_FrameOfReferenceID(other.m_FrameOfReferenceID), m_IndexToWorldTransformLastModified(other.m_IndexToWorldTransformLastModified), m_RotationQuaternion( other.m_RotationQuaternion ) , m_Origin(other.m_Origin)
{
  // AffineGeometryFrame
  SetBounds(other.GetBounds());
  //SetIndexToObjectTransform(other.GetIndexToObjectTransform());
  //SetObjectToNodeTransform(other.GetObjectToNodeTransform());
  //SetIndexToWorldTransform(other.GetIndexToWorldTransform());
  // this is not used in AffineGeometryFrame of ITK, thus there are not Get and Set methods
  // m_IndexToNodeTransform = other.m_IndexToNodeTransform;
  // m_InvertedTransform = TransformType::New();
  // m_InvertedTransform = TransformType::New();
  // m_InvertedTransform->DeepCopy(other.m_InvertedTransform);
  m_VtkMatrix = vtkMatrix4x4::New();
  m_VtkMatrix->DeepCopy(other.m_VtkMatrix);
  if (other.m_ParametricBoundingBox.IsNotNull())
  {
    m_ParametricBoundingBox = other.m_ParametricBoundingBox->DeepCopy();
  }
  FillVector3D(m_FloatSpacing,other.m_FloatSpacing[0],other.m_FloatSpacing[1],other.m_FloatSpacing[2]);
  m_VtkIndexToWorldTransform = vtkMatrixToLinearTransform::New();
  m_VtkIndexToWorldTransform->DeepCopy(other.m_VtkIndexToWorldTransform);
  m_VtkIndexToWorldTransform->SetInput(m_VtkMatrix);
  other.InitializeGeometry(this);
}

mitk::Geometry3D::~Geometry3D()
{
  m_VtkMatrix->Delete();
  m_VtkIndexToWorldTransform->Delete();
}


static void CopySpacingFromTransform(mitk::AffineTransform3D* transform, mitk::Vector3D& spacing, float floatSpacing[3])
{
  mitk::AffineTransform3D::MatrixType::InternalMatrixType vnlmatrix;
  vnlmatrix = transform->GetMatrix().GetVnlMatrix();

  spacing[0]=vnlmatrix.get_column(0).magnitude();
  spacing[1]=vnlmatrix.get_column(1).magnitude();
  spacing[2]=vnlmatrix.get_column(2).magnitude();
  floatSpacing[0]=spacing[0];
  floatSpacing[1]=spacing[1];
  floatSpacing[2]=spacing[2];
}


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
  CopySpacingFromTransform(m_IndexToWorldTransform, m_Spacing, m_FloatSpacing);
  vtk2itk(m_IndexToWorldTransform->GetOffset(), m_Origin);

  m_VtkMatrix->Identity();

  m_TimeBounds[0]=ScalarTypeNumericTraits::NonpositiveMin(); m_TimeBounds[1]=ScalarTypeNumericTraits::max();

  m_FrameOfReferenceID = 0;

  m_ImageGeometry = false;
}

void mitk::Geometry3D::TransferItkToVtkTransform()
{
  // copy m_IndexToWorldTransform into m_VtkIndexToWorldTransform
  TransferItkTransformToVtkMatrix(m_IndexToWorldTransform.GetPointer(), m_VtkMatrix);
  m_VtkIndexToWorldTransform->Modified();
}

void mitk::Geometry3D::TransferVtkToItkTransform()
{
  TransferVtkMatrixToItkTransform(m_VtkMatrix, m_IndexToWorldTransform.GetPointer());
  CopySpacingFromTransform(m_IndexToWorldTransform, m_Spacing, m_FloatSpacing);
  vtk2itk(m_IndexToWorldTransform->GetOffset(), m_Origin);
}

void mitk::Geometry3D::SetIndexToWorldTransformByVtkMatrix(vtkMatrix4x4* vtkmatrix)
{
  m_VtkMatrix->DeepCopy(vtkmatrix);
  TransferVtkToItkTransform();
}

void mitk::Geometry3D::SetTimeBounds(const TimeBounds& timebounds)
{
  if(m_TimeBounds != timebounds)
  {
    m_TimeBounds = timebounds;
    Modified();
  }
}

void mitk::Geometry3D::SetFloatBounds(const float bounds[6])
{
  mitk::BoundingBox::BoundsArrayType b;
  const float *input = bounds;
  int i=0;
  for(mitk::BoundingBox::BoundsArrayType::Iterator it = b.Begin(); i < 6 ;++i) *it++ = (mitk::ScalarType)*input++;
  SetBoundsArray(b, m_BoundingBox);
}

void mitk::Geometry3D::SetFloatBounds(const double bounds[6])
{
  mitk::BoundingBox::BoundsArrayType b;
  const double *input = bounds;
  int i=0;
  for(mitk::BoundingBox::BoundsArrayType::Iterator it = b.Begin(); i < 6 ;++i) *it++ = (mitk::ScalarType)*input++;
  SetBoundsArray(b, m_BoundingBox);
}

void mitk::Geometry3D::SetParametricBounds(const BoundingBox::BoundsArrayType& bounds)
{
  SetBoundsArray(bounds, m_ParametricBoundingBox);
}

void mitk::Geometry3D::WorldToIndex(const mitk::Point3D &pt_mm, mitk::Point3D &pt_units) const
{
  BackTransform(pt_mm, pt_units);
}

void mitk::Geometry3D::IndexToWorld(const mitk::Point3D &pt_units, mitk::Point3D &pt_mm) const
{
  pt_mm = m_IndexToWorldTransform->TransformPoint(pt_units);
}

void mitk::Geometry3D::WorldToIndex(const mitk::Point3D & /*atPt3d_mm*/, const mitk::Vector3D &vec_mm, mitk::Vector3D &vec_units) const
{
  MITK_WARN<<"Warning! Call of the deprecated function Geometry3D::WorldToIndex(point, vec, vec). Use Geometry3D::WorldToIndex(vec, vec) instead!";
  //BackTransform(atPt3d_mm, vec_mm, vec_units);
  this->WorldToIndex(vec_mm, vec_units);
}

void mitk::Geometry3D::WorldToIndex( const mitk::Vector3D &vec_mm, mitk::Vector3D &vec_units) const
{
  BackTransform( vec_mm, vec_units);
}

void mitk::Geometry3D::IndexToWorld(const mitk::Point3D &/*atPt3d_units*/, const mitk::Vector3D &vec_units, mitk::Vector3D &vec_mm) const
{
  MITK_WARN<<"Warning! Call of the deprecated function Geometry3D::IndexToWorld(point, vec, vec). Use Geometry3D::IndexToWorld(vec, vec) instead!";
  //vec_mm = m_IndexToWorldTransform->TransformVector(vec_units);
  this->IndexToWorld(vec_units, vec_mm);
}

void mitk::Geometry3D::IndexToWorld(const mitk::Vector3D &vec_units, mitk::Vector3D &vec_mm) const
{
  vec_mm = m_IndexToWorldTransform->TransformVector(vec_units);
}

void mitk::Geometry3D::SetIndexToWorldTransform(mitk::AffineTransform3D* transform)
{
  if(m_IndexToWorldTransform.GetPointer() != transform)
  {
    Superclass::SetIndexToWorldTransform(transform);
    CopySpacingFromTransform(m_IndexToWorldTransform, m_Spacing, m_FloatSpacing);
    vtk2itk(m_IndexToWorldTransform->GetOffset(), m_Origin);
    TransferItkToVtkTransform();
    Modified();
  }
}

itk::LightObject::Pointer mitk::Geometry3D::InternalClone() const
{
  Self::Pointer newGeometry = new Self(*this);
  newGeometry->UnRegister();
  return newGeometry.GetPointer();
}
/*
void mitk::Geometry3D::InitializeGeometry(Geometry3D * newGeometry) const
{
  Superclass::InitializeGeometry(newGeometry);

  newGeometry->SetTimeBounds(m_TimeBounds);

  //newGeometry->GetVtkTransform()->SetMatrix(m_VtkIndexToWorldTransform->GetMatrix()); IW
  //newGeometry->TransferVtkToItkTransform(); //MH

  newGeometry->SetFrameOfReferenceID(GetFrameOfReferenceID());
  newGeometry->m_ImageGeometry = m_ImageGeometry;
}
*/
void mitk::Geometry3D::SetExtentInMM(int direction, ScalarType extentInMM)
{
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
    Modified();
  }
}

mitk::BoundingBox::Pointer mitk::Geometry3D::CalculateBoundingBoxRelativeToTransform(const mitk::AffineTransform3D* transform) const
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

#include <vtkTransform.h>
void mitk::Geometry3D::ExecuteOperation(Operation* operation)
{
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

void mitk::Geometry3D::BackTransform(const mitk::Point3D &in, mitk::Point3D& out) const
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

void mitk::Geometry3D::BackTransform(const mitk::Point3D &/*at*/, const mitk::Vector3D &in, mitk::Vector3D& out) const
{
  MITK_INFO<<"Warning! Call of the deprecated function Geometry3D::BackTransform(point, vec, vec). Use Geometry3D::BackTransform(vec, vec) instead!";
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

void mitk::Geometry3D::BackTransform(const mitk::Vector3D& in, mitk::Vector3D& out) const
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

const float* mitk::Geometry3D::GetFloatSpacing() const
{
  return m_FloatSpacing;
}

void mitk::Geometry3D::SetSpacing(const mitk::Vector3D& aSpacing)
{
  if(mitk::Equal(m_Spacing, aSpacing) == false)
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

    itk2vtk(m_Spacing, m_FloatSpacing);
  }
}

void mitk::Geometry3D::SetOrigin(const Point3D & origin)
{
  if(origin!=GetOrigin())
  {
    m_Origin = origin;
    m_IndexToWorldTransform->SetOffset(m_Origin.GetVectorFromOrigin());
    Modified();
    TransferItkToVtkTransform();
  }
}

void mitk::Geometry3D::Translate(const Vector3D & vector)
{
    if((vector[0] != 0) || (vector[1] != 0) || (vector[2] != 0))
    {
        this->SetOrigin(m_Origin + vector);
//        m_IndexToWorldTransform->SetOffset(m_IndexToWorldTransform->GetOffset()+vector);
//        TransferItkToVtkTransform();
//        Modified();
    }
}

void mitk::Geometry3D::SetIdentity()
{
  m_IndexToWorldTransform->SetIdentity();
  m_Origin.Fill(0);
  Modified();
  TransferItkToVtkTransform();
}

void mitk::Geometry3D::Compose( const mitk::AffineGeometryFrame3D::TransformType * other, bool pre )
{
  m_IndexToWorldTransform->Compose(other, pre);
  CopySpacingFromTransform(m_IndexToWorldTransform, m_Spacing, m_FloatSpacing);
  vtk2itk(m_IndexToWorldTransform->GetOffset(), m_Origin);
  Modified();
  TransferItkToVtkTransform();
}

void mitk::Geometry3D::Compose( const vtkMatrix4x4 * vtkmatrix, bool pre )
{
  mitk::AffineGeometryFrame3D::TransformType::Pointer itkTransform = mitk::AffineGeometryFrame3D::TransformType::New();
  TransferVtkMatrixToItkTransform(vtkmatrix, itkTransform.GetPointer());
  Compose(itkTransform, pre);
}

const std::string mitk::Geometry3D::GetTransformAsString( TransformType* transformType )
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

void mitk::Geometry3D::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  os << indent << " IndexToWorldTransform: ";
  if(m_IndexToWorldTransform.IsNull())
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
        os << m_IndexToWorldTransform->GetMatrix()[i][j] << " ";
      }
      os << std::endl;
    }

    os << indent << "Offset: " << m_IndexToWorldTransform->GetOffset() << std::endl;
    os << indent << "Center: " << m_IndexToWorldTransform->GetCenter() << std::endl;
    os << indent << "Translation: " << m_IndexToWorldTransform->GetTranslation() << std::endl;

    os << indent << "Inverse: " << std::endl;
    for (i = 0; i < 3; i++)
    {
      os << indent.GetNextIndent();
      for (j = 0; j < 3; j++)
      {
        os << m_IndexToWorldTransform->GetInverseMatrix()[i][j] << " ";
      }
      os << std::endl;
    }

    // from itk::ScalableAffineTransform
    os << indent << "Scale : ";
    for (i = 0; i < 3; i++)
    {
      os << m_IndexToWorldTransform->GetScale()[i] << " ";
    }
    os << std::endl;
  }

  os << indent << " BoundingBox: ";
  if(m_BoundingBox.IsNull())
    os << "NULL" << std::endl;
  else
  {
    os << indent << "( ";
    for (unsigned int i=0; i<3; i++)
    {
      os << m_BoundingBox->GetBounds()[2*i] << "," << m_BoundingBox->GetBounds()[2*i+1] << " ";
    }
    os << " )" << std::endl;
  }

  os << indent << " Origin: " << m_Origin << std::endl;
  os << indent << " ImageGeometry: " << m_ImageGeometry << std::endl;
  os << indent << " Spacing: " << m_Spacing << std::endl;
  os << indent << " TimeBounds: " << m_TimeBounds << std::endl;
}

mitk::Point3D mitk::Geometry3D::GetCornerPoint(int id) const
{
  assert(id >= 0);
  assert(m_BoundingBox.IsNotNull());

  BoundingBox::BoundsArrayType bounds = m_BoundingBox->GetBounds();

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
  return m_IndexToWorldTransform->TransformPoint(cornerpoint);
}

mitk::Point3D mitk::Geometry3D::GetCornerPoint(bool xFront, bool yFront, bool zFront) const
{
  assert(m_BoundingBox.IsNotNull());
  BoundingBox::BoundsArrayType bounds = m_BoundingBox->GetBounds();

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

  return m_IndexToWorldTransform->TransformPoint(cornerpoint);
}

void
mitk::Geometry3D::ResetSubTransforms()
{
}

void
mitk::Geometry3D::ChangeImageGeometryConsideringOriginOffset( const bool isAnImageGeometry )
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

bool mitk::Geometry3D::Is2DConvertable()
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

bool mitk::Equal( const mitk::Geometry3D::BoundingBoxType *leftHandSide, const mitk::Geometry3D::BoundingBoxType *rightHandSide, ScalarType eps, bool verbose )
{
  bool result = true;
  if( rightHandSide == NULL )
  {
    if(verbose)
      MITK_INFO << "[( Geometry3D::BoundingBoxType )] rightHandSide NULL.";
    return false;
  }
  if( leftHandSide == NULL )
  {
    if(verbose)
      MITK_INFO << "[( Geometry3D::BoundingBoxType )] leftHandSide NULL.";
    return false;
  }

  Geometry3D::BoundsArrayType rightBounds = rightHandSide->GetBounds();
  Geometry3D::BoundsArrayType leftBounds = leftHandSide->GetBounds();
  Geometry3D::BoundsArrayType::Iterator itLeft = leftBounds.Begin();
  for( Geometry3D::BoundsArrayType::Iterator itRight = rightBounds.Begin(); itRight != rightBounds.End(); ++itRight)
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

bool mitk::Equal(const mitk::Geometry3D *leftHandSide, const mitk::Geometry3D *rightHandSide, ScalarType eps, bool verbose)
{
  bool result = true;

  if( rightHandSide == NULL )
  {
    if(verbose)
      MITK_INFO << "[( Geometry3D )] rightHandSide NULL.";
    return false;
  }
  if( leftHandSide == NULL)
  {
    if(verbose)
      MITK_INFO << "[( Geometry3D )] leftHandSide NULL.";
    return false;
  }

  //Compare spacings
  if( !mitk::Equal( leftHandSide->GetSpacing(), rightHandSide->GetSpacing(), eps ) )
  {
    if(verbose)
    {
      MITK_INFO << "[( Geometry3D )] Spacing differs.";
      MITK_INFO << "rightHandSide is " << setprecision(12) << rightHandSide->GetSpacing() << " : leftHandSide is " << leftHandSide->GetSpacing() << " and tolerance is " << eps;
    }
    result = false;
  }

  //Compare Origins
  if( !mitk::Equal( leftHandSide->GetOrigin(), rightHandSide->GetOrigin(), eps ) )
  {
    if(verbose)
    {
      MITK_INFO << "[( Geometry3D )] Origin differs.";
      MITK_INFO << "rightHandSide is " << setprecision(12) << rightHandSide->GetOrigin() << " : leftHandSide is " << leftHandSide->GetOrigin() << " and tolerance is " << eps;
    }
    result = false;
  }

  //Compare Axis and Extents
  for( unsigned int i=0; i<3; ++i)
  {
    if( !mitk::Equal( leftHandSide->GetAxisVector(i), rightHandSide->GetAxisVector(i), eps))
    {
      if(verbose)
      {
        MITK_INFO << "[( Geometry3D )] AxisVector #" << i << " differ";
        MITK_INFO << "rightHandSide is " << setprecision(12) << rightHandSide->GetAxisVector(i) << " : leftHandSide is " << leftHandSide->GetAxisVector(i) << " and tolerance is " << eps;
      }
      result =  false;
    }

    if( !mitk::Equal( leftHandSide->GetExtent(i), rightHandSide->GetExtent(i), eps) )
    {
      if(verbose)
      {
        MITK_INFO << "[( Geometry3D )] Extent #" << i << " differ";
        MITK_INFO << "rightHandSide is " << setprecision(12) << rightHandSide->GetExtent(i) << " : leftHandSide is " << leftHandSide->GetExtent(i) << " and tolerance is " << eps;
      }
      result = false;
    }
  }

  //Compare ImageGeometry Flag
  if( rightHandSide->GetImageGeometry() != leftHandSide->GetImageGeometry() )
  {
    if(verbose)
    {
      MITK_INFO << "[( Geometry3D )] GetImageGeometry is different.";
      MITK_INFO << "rightHandSide is " << rightHandSide->GetImageGeometry() << " : leftHandSide is " << leftHandSide->GetImageGeometry();
    }
    result = false;
  }

  //Compare BoundingBoxes
  if( !mitk::Equal( leftHandSide->GetBoundingBox(), rightHandSide->GetBoundingBox(), eps, verbose) )
  {
    result = false;
  }

  //Compare IndexToWorldTransform Matrix
  if( !mitk::Equal( leftHandSide->GetIndexToWorldTransform(), rightHandSide->GetIndexToWorldTransform(), eps, verbose) )
  {
    result = false;
  }
  return result;
}

bool mitk::Equal(const Geometry3D::TransformType *leftHandSide, const Geometry3D::TransformType *rightHandSide, ScalarType eps, bool verbose )
{
  //Compare IndexToWorldTransform Matrix
  if( !mitk::MatrixEqualElementWise(  leftHandSide->GetMatrix(),
                                      rightHandSide->GetMatrix() ) )
  {
    if(verbose)
    {
      MITK_INFO << "[( Geometry3D::TransformType )] Index to World Transformation matrix differs.";
      MITK_INFO << "rightHandSide is " << setprecision(12) << rightHandSide->GetMatrix() << " : leftHandSide is " << leftHandSide->GetMatrix() << " and tolerance is " << eps;
    }
    return false;
  }
  return true;
}
