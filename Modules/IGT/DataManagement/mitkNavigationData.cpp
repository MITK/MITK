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

#include "mitkNavigationData.h"
#include "vnl/vnl_det.h"
#include "mitkException.h"

mitk::NavigationData::NavigationData() : itk::DataObject(),
m_Position(), m_Orientation(0.0, 0.0, 0.0, 1.0), m_CovErrorMatrix(),
m_HasPosition(true), m_HasOrientation(true), m_DataValid(false), m_IGTTimeStamp(0.0),
m_Name()
{
  m_Position.Fill(0.0);
  m_CovErrorMatrix.SetIdentity();
}


mitk::NavigationData::NavigationData(const mitk::NavigationData& toCopy) : itk::DataObject(),
    m_Position(toCopy.GetPosition()), m_Orientation(toCopy.GetOrientation()), m_CovErrorMatrix(toCopy.GetCovErrorMatrix()),
        m_HasPosition(toCopy.GetHasPosition()), m_HasOrientation(toCopy.GetHasOrientation()), m_DataValid(toCopy.IsDataValid()), m_IGTTimeStamp(toCopy.GetIGTTimeStamp()),
        m_Name(toCopy.GetName())
{/* TODO SW: Graft does the same, remove code duplications, set Graft to deprecated, remove duplication in tescode */}

mitk::NavigationData::~NavigationData()
{
}


void mitk::NavigationData::Graft( const DataObject *data )
{
  // Attempt to cast data to an NavigationData
  const Self* nd;
  try
  {
    nd = dynamic_cast<const Self *>( data );
  }
  catch( ... )
  {
    itkExceptionMacro( << "mitk::NavigationData::Graft cannot cast "
      << typeid(data).name() << " to "
      << typeid(const Self *).name() );
    return;
  }
  if (!nd)
  {
    // pointer could not be cast back down
    itkExceptionMacro( << "mitk::NavigationData::Graft cannot cast "
      << typeid(data).name() << " to "
      << typeid(const Self *).name() );
    return;
  }
  // Now copy anything that is needed
  this->SetPosition(nd->GetPosition());
  this->SetOrientation(nd->GetOrientation());
  this->SetDataValid(nd->IsDataValid());
  this->SetIGTTimeStamp(nd->GetIGTTimeStamp());
  this->SetHasPosition(nd->GetHasPosition());
  this->SetHasOrientation(nd->GetHasOrientation());
  this->SetCovErrorMatrix(nd->GetCovErrorMatrix());
  this->SetName(nd->GetName());
}


bool mitk::NavigationData::IsDataValid() const
{
  return m_DataValid;
}


void mitk::NavigationData::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "data valid: "     << this->IsDataValid() << std::endl;
  os << indent << "Position: "       << this->GetPosition() << std::endl;
  os << indent << "Orientation: "    << this->GetOrientation() << std::endl;
  os << indent << "TimeStamp: "      << this->GetIGTTimeStamp() << std::endl;
  os << indent << "HasPosition: "    << this->GetHasPosition() << std::endl;
  os << indent << "HasOrientation: " << this->GetHasOrientation() << std::endl;
  os << indent << "CovErrorMatrix: " << this->GetCovErrorMatrix() << std::endl;
}


void mitk::NavigationData::CopyInformation( const DataObject* data )
{
  this->Superclass::CopyInformation( data );

  const Self * nd = NULL;
  try
  {
    nd = dynamic_cast<const Self*>(data);
  }
  catch( ... )
  {
    // data could not be cast back down
    itkExceptionMacro(<< "mitk::NavigationData::CopyInformation() cannot cast "
      << typeid(data).name() << " to "
      << typeid(Self*).name() );
  }
  if ( !nd )
  {
    // pointer could not be cast back down
    itkExceptionMacro(<< "mitk::NavigationData::CopyInformation() cannot cast "
      << typeid(data).name() << " to "
      << typeid(Self*).name() );
  }
  /* copy all meta data */
}


void mitk::NavigationData::SetPositionAccuracy(mitk::ScalarType error)
{
  for ( int i = 0; i < 3; i++ )
    for ( int j = 0; j < 3; j++ )
    {
      m_CovErrorMatrix[ i ][ j ] = 0;
      // assume independence of position and orientation
      m_CovErrorMatrix[ i + 3 ][ j ] = 0;
      m_CovErrorMatrix[ i ][ j + 3 ] = 0;
    }
  m_CovErrorMatrix[0][0] = m_CovErrorMatrix[1][1] = m_CovErrorMatrix[2][2] = error * error;
}


void mitk::NavigationData::SetOrientationAccuracy(mitk::ScalarType error)
{
  for ( int i = 0; i < 3; i++ )
    for ( int j = 0; j < 3; j++ ) {
      m_CovErrorMatrix[ i + 3 ][ j + 3 ] = 0;
      // assume independence of position and orientation
      m_CovErrorMatrix[ i + 3 ][ j ] = 0;
      m_CovErrorMatrix[ i ][ j + 3 ] = 0;
    }
  m_CovErrorMatrix[3][3] = m_CovErrorMatrix[4][4] = m_CovErrorMatrix[5][5] = error * error;
}

void
mitk::NavigationData::Compose(const mitk::NavigationData::Pointer n, const bool pre)
{
  NavigationData::Pointer nd3;
  if (!pre)
    nd3 = getComposition(this, n);
  else
    nd3 = getComposition(n, this);

  this->Graft(nd3);
}

mitk::NavigationData::NavigationData(
    mitk::AffineTransform3D::Pointer affineTransform3D,
    const bool checkForRotationMatrix) : itk::DataObject(),
        m_Position(),
        m_CovErrorMatrix(), m_HasPosition(true), m_HasOrientation(true), m_DataValid(true), m_IGTTimeStamp(0.0),
        m_Name()
{
  mitk::Vector3D offset = affineTransform3D->GetOffset();

  m_Position[0] = offset[0];
  m_Position[1] = offset[1];
  m_Position[2] = offset[2];

  vnl_matrix_fixed<ScalarType, 3, 3> rotationMatrix           = affineTransform3D->GetMatrix().GetVnlMatrix();
  vnl_matrix_fixed<ScalarType, 3, 3> rotationMatrixTransposed = rotationMatrix.transpose();

  if (checkForRotationMatrix)
  {
    // a quadratic matrix is a rotation matrix exactly when determinant is 1 and transposed is inverse
     if (!Equal(1.0, vnl_det(rotationMatrix), 0.1)
         || !((rotationMatrix*rotationMatrixTransposed).is_identity(0.1)))
     {
       mitkThrow() << "tried to initialize NavigationData with non-rotation matrix :" << rotationMatrix << " (Does your AffineTransform3D object include spacing? This is not supported by NavigationData objects!)";
     }

  }

  // the transpose is because vnl_quaterion expects a transposed rotation matrix
  m_Orientation = Quaternion(rotationMatrixTransposed);
}

mitk::AffineTransform3D::Pointer
mitk::NavigationData::GetAffineTransform3D() const
{
  AffineTransform3D::Pointer affineTransform3D = AffineTransform3D::New();

  // first set rotation
  affineTransform3D->SetMatrix(this->GetRotationMatrix());

  // now set offset
  Vector3D vector3D;

  for (int i = 0; i < 3; ++i) {
    vector3D[i] = m_Position[i];
  }
  affineTransform3D->SetOffset(vector3D);

  return affineTransform3D;
}

mitk::Matrix3D
mitk::NavigationData::GetRotationMatrix() const
{
  vnl_matrix_fixed<ScalarType,3,3> vnl_rotation = m_Orientation.rotation_matrix_transpose().transpose(); // :-)
  Matrix3D mitkRotation;

  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      mitkRotation[i][j] = vnl_rotation[i][j];
    }
  }

  return mitkRotation;
}

mitk::Point3D
mitk::NavigationData::TransformPoint(const mitk::Point3D point) const
{
  vnl_vector_fixed<ScalarType, 3> vnlPoint;

  for (int i = 0; i < 3; ++i) {
    vnlPoint[i] = point[i];
  }

  Quaternion normalizedQuaternion = this->GetOrientation().normalize();
  // first get rotated point
  vnlPoint = normalizedQuaternion.rotate(vnlPoint);

  Point3D resultingPoint;

  for (int i = 0; i < 3; ++i) {
    // now copy it to our format + offset
    resultingPoint[i] = vnlPoint[i] + this->GetPosition()[i];
  }

  return resultingPoint;
}

mitk::NavigationData::Pointer
mitk::NavigationData::GetInverse() const
{
  // non-zero quaternion does not have inverse: throw exception in this case.
  Quaternion zeroQuaternion;
  zeroQuaternion.fill(0);
  if (Equal(zeroQuaternion, this->GetOrientation()))
    mitkThrow() << "tried to invert zero quaternion in NavigationData";

  mitk::NavigationData::Pointer navigationDataInverse = this->Clone();
  navigationDataInverse->SetOrientation(this->GetOrientation().inverse());

  // To vnl_vector
  vnl_vector_fixed<ScalarType, 3> vnlPoint;
  for (int i = 0; i < 3; ++i) {
    vnlPoint[i] = this->GetPosition()[i];
  }

  // invert position
  vnlPoint = -(navigationDataInverse->GetOrientation().rotate(vnlPoint));

  // back to Point3D
  Point3D invertedPosition = this->GetPosition();
  for (int i = 0; i < 3; ++i) {
    invertedPosition[i] = vnlPoint[i];
  }

  navigationDataInverse->SetPosition(invertedPosition);

  // Inversion does not care for covariances for now
  navigationDataInverse->ResetCovarianceValidity();

  return navigationDataInverse;
}

void
mitk::NavigationData::ResetCovarianceValidity()
{
  this->SetHasPosition(false);
  this->SetHasOrientation(false);
}

mitk::NavigationData::Pointer
mitk::NavigationData::getComposition(const mitk::NavigationData::Pointer nd1,
    const mitk::NavigationData::Pointer nd2)
{
  NavigationData::Pointer nd3 = nd1->Clone();

  // A2 * A1
  nd3->SetOrientation(nd2->GetOrientation() * nd1->GetOrientation());

  // first: b1, b2 vnl vector
  vnl_vector_fixed<ScalarType,3> b1, b2, b3;
  for (int i = 0; i < 3; ++i) {
    b1[i] = nd1->GetPosition()[i];
    b2[i] = nd2->GetPosition()[i];
  }

  // b3 = A2b1 + b2
  b3  = nd2->GetOrientation().rotate(b1) + b2;

  // back to mitk::Point3D
  Point3D point;
  for (int i = 0; i < 3; ++i) {
    point[i] = b3[i];
  }

  nd3->SetPosition(point);

  nd3->ResetCovarianceValidity();

  return nd3;
}

bool mitk::Equal(const mitk::NavigationData& leftHandSide, const mitk::NavigationData& rightHandSide, ScalarType eps, bool verbose)
{
  bool returnValue = true;

  // Dimensionality
  if( !mitk::Equal(rightHandSide.GetPosition(),leftHandSide.GetPosition()) )
  {
    if(verbose)
    {
      MITK_INFO << "[( NavigationData )] Position differs.";
      MITK_INFO << "leftHandSide is " << leftHandSide.GetPosition()
       << "rightHandSide is " << rightHandSide.GetPosition();
    }
    returnValue = false;
  }

  // Dimensionality
  if( !mitk::Equal(rightHandSide.GetOrientation(),leftHandSide.GetOrientation()) )
  {
    if(verbose)
    {
      MITK_INFO << "[( NavigationData )] Orientation differs.";
      MITK_INFO << "leftHandSide is " << leftHandSide.GetOrientation()
       << "rightHandSide is " << rightHandSide.GetOrientation();
    }
    returnValue = false;
  }

  if( rightHandSide.GetCovErrorMatrix() != leftHandSide.GetCovErrorMatrix() )
  {
    if(verbose)
    {
      MITK_INFO << "[( NavigationData )] CovErrorMatrix differs.";
      MITK_INFO << "leftHandSide is " << leftHandSide.GetCovErrorMatrix()
       << "rightHandSide is " << rightHandSide.GetCovErrorMatrix();
    }
    returnValue = false;
  }

  if( std::string(rightHandSide.GetName()) != std::string(leftHandSide.GetName()) )
  {
    if(verbose)
    {
      MITK_INFO << "[( NavigationData )] Name differs.";
      MITK_INFO << "leftHandSide is " << leftHandSide.GetName()
       << "rightHandSide is " << rightHandSide.GetName();
    }
    returnValue = false;
  }

  if( rightHandSide.GetIGTTimeStamp() != leftHandSide.GetIGTTimeStamp() )
  {
    if(verbose)
    {
      MITK_INFO << "[( NavigationData )] IGTTimeStamp differs.";
      MITK_INFO << "leftHandSide is " << leftHandSide.GetIGTTimeStamp()
       << "rightHandSide is " << rightHandSide.GetIGTTimeStamp();
    }
    returnValue = false;
  }

  return returnValue;
}
