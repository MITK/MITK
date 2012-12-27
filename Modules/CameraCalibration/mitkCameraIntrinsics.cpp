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

#include "mitkCameraIntrinsics.h"
#include <itkMutexLockHolder.h>
#include <mitkEndoMacros.h>
#include <mitkEndoDebug.h>

mitk::CameraIntrinsics::CameraIntrinsics()
  : m_Valid(false), m_Mutex(itk::FastMutexLock::New())
{
  m_CameraMatrix = cv::Mat::zeros(3, 3, cv::DataType<double>::type);
  m_CameraMatrix.at<double>(2,2) = 1.0;
  m_DistorsionCoeffs = cv::Mat::zeros(1, 5, cv::DataType<double>::type);
}

mitk::CameraIntrinsics::~CameraIntrinsics()
{

}

bool mitk::CameraIntrinsics::Equals( const CameraIntrinsics* other ) const
{
  return other->GetDistorsionCoeffsAsPoint4D()==
      this->GetDistorsionCoeffsAsPoint4D() &&
    other->GetFocalPoint()==
      this->GetFocalPoint() &&
    other->GetPrincipalPoint()
      == this->GetPrincipalPoint();
}

void mitk::CameraIntrinsics::Copy(const CameraIntrinsics* other)
{
  this->SetIntrinsics( other->GetCameraMatrix().clone()
    , other->GetDistorsionCoeffs().clone() );
  this->SetValid(other->m_Valid);
}

bool mitk::CameraIntrinsics::IsValid() const
{
  itk::MutexLockHolder<itk::FastMutexLock> lock(*m_Mutex);
  return m_Valid;
}

mitk::CameraIntrinsics::Pointer mitk::CameraIntrinsics::Clone() const
{
  mitk::CameraIntrinsics::Pointer copy = mitk::CameraIntrinsics::New();
  copy->SetIntrinsics( this->GetCameraMatrix(), this->GetDistorsionCoeffs() );
  copy->SetValid(this->IsValid());
  return copy;
}

vnl_matrix_fixed<mitk::ScalarType, 3, 3>
    mitk::CameraIntrinsics::GetVnlCameraMatrix() const
{
  vnl_matrix_fixed<mitk::ScalarType, 3, 3> mat;
  mat.set_identity();

  {
    itk::MutexLockHolder<itk::FastMutexLock> lock(*m_Mutex);
    mat(0,0) = m_CameraMatrix.at<double>(0,0);
    mat(1,1) = m_CameraMatrix.at<double>(1,1);

    mat(0,2) = m_CameraMatrix.at<double>(0,2);
    mat(1,2) = m_CameraMatrix.at<double>(1,2);
  }

  return mat;
}

void mitk::CameraIntrinsics::SetCameraMatrix(
    const vnl_matrix_fixed<mitk::ScalarType, 3, 3>& _CameraMatrix )
{
  itk::MutexLockHolder<itk::FastMutexLock> lock(*m_Mutex);
  m_CameraMatrix.at<double>(0,0) = _CameraMatrix(0,0);
  m_CameraMatrix.at<double>(1,1) = _CameraMatrix(1,1);

  m_CameraMatrix.at<double>(0,2) = _CameraMatrix(0,2);
  m_CameraMatrix.at<double>(1,2) = _CameraMatrix(1,2);
}

vnl_matrix_fixed<mitk::ScalarType, 3, 4>
    mitk::CameraIntrinsics::GetVnlCameraMatrix3x4() const
{
  vnl_matrix_fixed<mitk::ScalarType, 3, 4> mat;
  mat.fill(0);
  mat.update( this->GetVnlCameraMatrix().as_matrix() );

  return mat;
}

void mitk::CameraIntrinsics::SetIntrinsics( const cv::Mat& _CameraMatrix
                        , const cv::Mat& _DistorsionCoeffs)
{
  {
    itk::MutexLockHolder<itk::FastMutexLock> lock(*m_Mutex);
    if( _CameraMatrix.cols != 3 || _CameraMatrix.rows != 3)
      throw std::invalid_argument("Wrong format of camera matrix. Should be 3x3"
                                  " double.");

    endoAssertMsg( (_DistorsionCoeffs.cols == 5) &&
      _DistorsionCoeffs.rows == 1, "Wrong format of distorsion coefficients"
                                  " vector. Should be 5x1 double.");

    m_CameraMatrix = _CameraMatrix.clone();
    m_DistorsionCoeffs = _DistorsionCoeffs.clone();

    m_Valid = true;
  }
  this->Modified();
}

void mitk::CameraIntrinsics::SetIntrinsics( const mitk::Point3D& focalPoint,
                    const mitk::Point3D& principalPoint,
                    const mitk::Point4D& distortionCoefficients)

{
  {
    itk::MutexLockHolder<itk::FastMutexLock> lock(*m_Mutex);
    m_CameraMatrix.at<double>(0,0) = focalPoint[0];
    m_CameraMatrix.at<double>(1,1) = focalPoint[1];

    m_CameraMatrix.at<double>(0,2) = principalPoint[0];
    m_CameraMatrix.at<double>(1,2) = principalPoint[1];

    m_DistorsionCoeffs.at<double>(0,0) = distortionCoefficients[0];
    m_DistorsionCoeffs.at<double>(0,1) = distortionCoefficients[1];
    m_DistorsionCoeffs.at<double>(0,2) = distortionCoefficients[2];
    m_DistorsionCoeffs.at<double>(0,3) = distortionCoefficients[3];
  }
  this->Modified();
}

void mitk::CameraIntrinsics::SetFocalLength( double x, double y )
{
  {
    itk::MutexLockHolder<itk::FastMutexLock> lock(*m_Mutex);
    m_CameraMatrix.at<double>(0,0) = x;
    m_CameraMatrix.at<double>(1,1) = y;
  }
  this->Modified();
}

void mitk::CameraIntrinsics::SetPrincipalPoint( double x, double y )
{
  {
    itk::MutexLockHolder<itk::FastMutexLock> lock(*m_Mutex);
    m_CameraMatrix.at<double>(0,2) = x;
    m_CameraMatrix.at<double>(1,2) = y;
  }
  this->Modified();
}

void mitk::CameraIntrinsics::SetDistorsionCoeffs( double k1, double k2,
                                                 double p1, double p2 )
{

  {
    itk::MutexLockHolder<itk::FastMutexLock> lock(*m_Mutex);

    m_DistorsionCoeffs.at<double>(0,0) = k1;
    m_DistorsionCoeffs.at<double>(0,1) = k2;
    m_DistorsionCoeffs.at<double>(0,2) = p1;
    m_DistorsionCoeffs.at<double>(0,3) = p2;
  }
  this->Modified();
}

cv::Mat mitk::CameraIntrinsics::GetCameraMatrix() const
{
  itk::MutexLockHolder<itk::FastMutexLock> lock(*m_Mutex);
  return m_CameraMatrix.clone(); // return a copy of this small matrix
}

cv::Mat mitk::CameraIntrinsics::GetDistorsionCoeffs() const
{
  itk::MutexLockHolder<itk::FastMutexLock> lock(*m_Mutex);
  return m_DistorsionCoeffs.clone(); // return a copy of this small matrix
}

cv::Mat mitk::CameraIntrinsics::GetDistorsionCoeffs()
{
  const CameraIntrinsics* intrinsics = this;
  return intrinsics->GetDistorsionCoeffs();
}

std::string mitk::CameraIntrinsics::ToString() const
{
  itk::MutexLockHolder<itk::FastMutexLock> lock(*m_Mutex);
  std::ostringstream s; s.precision(12);
  const cv::Mat& CameraMatrix = m_CameraMatrix;
  const cv::Mat& DistorsionCoeffs = m_DistorsionCoeffs;

  s.str(""); s << this->GetNameOfClass() << ": ";
  s << "fx = " << CameraMatrix.at<double>(0,0);
  s << ", fy = " << CameraMatrix.at<double>(1,1);
  s << ", cx = " << CameraMatrix.at<double>(0,2);
  s << ", cy = " << CameraMatrix.at<double>(1,2);

  s << ", k1 = " << DistorsionCoeffs.at<double>(0,0);
  s << ", k2 = " << DistorsionCoeffs.at<double>(0,1);
  s << ", p1 = " << DistorsionCoeffs.at<double>(0,2);
  s << ", p2 = " << DistorsionCoeffs.at<double>(0,3);
  //s << ", k3 = " << DistorsionCoeffs.at<double>(0,4);

  return s.str();
}

void mitk::CameraIntrinsics::ToXML(TiXmlElement* elem) const
{
  itk::MutexLockHolder<itk::FastMutexLock> lock(*m_Mutex);
  elem->SetValue(this->GetNameOfClass());
  std::ostringstream s; s.precision(12);
  const cv::Mat& CameraMatrix = m_CameraMatrix;
  s.str(""); s << CameraMatrix.at<double>(0,0);
  elem->SetAttribute( "fx", s.str() );
  s.str(""); s << CameraMatrix.at<double>(1,1);
  elem->SetAttribute( "fy", s.str() );
  s.str(""); s << CameraMatrix.at<double>(0,2);
  elem->SetAttribute( "cx", s.str() );
  s.str(""); s << CameraMatrix.at<double>(1,2);
  elem->SetAttribute( "cy", s.str() );

  const cv::Mat& DistorsionCoeffs = m_DistorsionCoeffs;
  s.str(""); s << DistorsionCoeffs.at<double>(0,0);
  elem->SetAttribute( "k1", s.str() );
  s.str(""); s << DistorsionCoeffs.at<double>(0,1);
  elem->SetAttribute( "k2", s.str() );
  s.str(""); s << DistorsionCoeffs.at<double>(0,2);
  elem->SetAttribute( "p1", s.str() );
  s.str(""); s << DistorsionCoeffs.at<double>(0,3);
  elem->SetAttribute( "p2", s.str() );
  elem->SetAttribute("Valid", m_Valid);
  //s.str(""); s << DistorsionCoeffs.at<double>(4,0);
  //elem->SetAttribute( "k3", s.str() );
}

void mitk::CameraIntrinsics::FromGMLCalibrationXML(TiXmlElement* elem)
{
  assert( elem );
  assert( elem->ValueStr() == "results" );
  cv::Mat CameraMatrix = cv::Mat::zeros(3, 3, cv::DataType<double>::type);
  CameraMatrix.at<double>(2,2) = 1.0;
  cv::Mat DistorsionCoeffs = cv::Mat::zeros(1, 5, cv::DataType<double>::type);

  TiXmlElement* focus_lenXElem = elem->FirstChildElement("focus_lenX");
  endoAssert( focus_lenXElem != 0 );
  CameraMatrix.at<double>(0,0) = atof( focus_lenXElem->GetText() );

  TiXmlElement* focus_lenYElem = elem->FirstChildElement("focus_lenY");
  endoAssert( focus_lenYElem != 0 );
  CameraMatrix.at<double>(1,1) = atof( focus_lenYElem->GetText() );

  TiXmlElement* PrincipalXElem = elem->FirstChildElement("PrincipalX");
  endoAssert( PrincipalXElem != 0 );
  CameraMatrix.at<double>(0,2) = atof( PrincipalXElem->GetText() );

  TiXmlElement* PrincipalYElem = elem->FirstChildElement("PrincipalY");
  endoAssert( PrincipalYElem != 0 );
  CameraMatrix.at<double>(1,2) = atof( PrincipalYElem->GetText() );

  // DISTORSION COEFFS

  TiXmlElement* Dist1Elem = elem->FirstChildElement("Dist1");
  endoAssert( Dist1Elem != 0 );
  DistorsionCoeffs.at<double>(0,0) = atof( Dist1Elem->GetText() );

  TiXmlElement* Dist2Elem = elem->FirstChildElement("Dist2");
  endoAssert( Dist2Elem != 0 );
  DistorsionCoeffs.at<double>(0,1) = atof( Dist2Elem->GetText() );

  TiXmlElement* Dist3Elem = elem->FirstChildElement("Dist3");
  endoAssert( Dist3Elem != 0 );
  DistorsionCoeffs.at<double>(0,2) = atof( Dist3Elem->GetText() );

  TiXmlElement* Dist4Elem = elem->FirstChildElement("Dist4");
  endoAssert( Dist4Elem != 0 );
  DistorsionCoeffs.at<double>(0,3) = atof( Dist4Elem->GetText() );

  int valid = 0;
  elem->QueryIntAttribute("Valid", &valid);

  {
    itk::MutexLockHolder<itk::FastMutexLock> lock(*m_Mutex);
    m_Valid = static_cast<bool>(valid);
    m_CameraMatrix = CameraMatrix;
    m_DistorsionCoeffs = DistorsionCoeffs;
  }

  this->Modified();
}

void mitk::CameraIntrinsics::FromXML(TiXmlElement* elem)
{
  endoAssert ( elem );
  MITK_DEBUG << elem->Value();
  std::string filename;
  if(elem->QueryStringAttribute("file", &filename) == TIXML_SUCCESS)
  {
    this->FromXMLFile(filename);
    return;
  }
  else if(strcmp(elem->Value(), "CalibrationProject") == 0)
  {
    this->FromGMLCalibrationXML(elem->FirstChildElement("results"));
    return;
  }

  assert ( elem );
  if(strcmp(elem->Value(), this->GetNameOfClass()) != 0)
    elem = elem->FirstChildElement(this->GetNameOfClass());

  std::ostringstream err;
  // CAMERA MATRIX
  cv::Mat CameraMatrix = cv::Mat::zeros(3, 3, cv::DataType<double>::type);
  CameraMatrix.at<double>(2,2) = 1.0;
  float val = 0.0f;
  if(elem->QueryFloatAttribute("fx", &val) == TIXML_SUCCESS)
    CameraMatrix.at<double>(0,0) = val;
  else
    err << "fx, ";

  if(elem->QueryFloatAttribute("fy", &val) == TIXML_SUCCESS)
    CameraMatrix.at<double>(1,1) = val;
  else
    err << "fy, ";

  if(elem->QueryFloatAttribute("cx", &val) == TIXML_SUCCESS)
    CameraMatrix.at<double>(0,2) = val;
  else
    err << "cx, ";

  if(elem->QueryFloatAttribute("cy", &val) == TIXML_SUCCESS)
    CameraMatrix.at<double>(1,2) = val;
  else
    err << "cy, ";

  // DISTORSION COEFFS
  endodebug( "creating DistorsionCoeffs from XML file")
  cv::Mat DistorsionCoeffs = cv::Mat::zeros(1, 5, cv::DataType<double>::type);
  if(elem->QueryFloatAttribute("k1", &val) == TIXML_SUCCESS)
    DistorsionCoeffs.at<double>(0,0) = val;
  else
    err << "k1, ";

  if(elem->QueryFloatAttribute("k2", &val) == TIXML_SUCCESS)
    DistorsionCoeffs.at<double>(0,1) = val;
  else
    err << "k2, ";

  if(elem->QueryFloatAttribute("p1", &val) == TIXML_SUCCESS)
    DistorsionCoeffs.at<double>(0,2) = val;
  else
    err << "p1, ";

  if(elem->QueryFloatAttribute("p2", &val) == TIXML_SUCCESS)
    DistorsionCoeffs.at<double>(0,3) = val;
  else
    err << "p2, ";

  DistorsionCoeffs.at<double>(0,4) = 0.0;

  /*if(elem->QueryFloatAttribute("k3", &val) == TIXML_SUCCESS)
    DistorsionCoeffs.at<double>(4,0) = val;
  else
    err << "k3, ";*/

  std::string errorStr = err.str();
  int errLength = errorStr.length();
  if(errLength > 0)
  {
    errorStr = errorStr.substr(0, errLength-2);
    errorStr.append(" not found");
    throw std::invalid_argument(err.str());
  }

  int valid = 0;
  elem->QueryIntAttribute("Valid", &valid);

  {
    itk::MutexLockHolder<itk::FastMutexLock> lock(*m_Mutex);
    m_Valid = static_cast<bool>(valid);
    m_CameraMatrix = CameraMatrix;
    m_DistorsionCoeffs = DistorsionCoeffs;
  }

  this->Modified();
}

double mitk::CameraIntrinsics::GetFocalLengthX() const
{
  itk::MutexLockHolder<itk::FastMutexLock> lock(*m_Mutex);
  double FocalLengthX = m_CameraMatrix.at<double>(0,0);

  return FocalLengthX;
}
double mitk::CameraIntrinsics::GetFocalLengthY() const
{
  itk::MutexLockHolder<itk::FastMutexLock> lock(*m_Mutex);
  double FocalLengthY = m_CameraMatrix.at<double>(1,1);;
  return FocalLengthY;
}
double mitk::CameraIntrinsics::GetPrincipalPointX() const
{
  itk::MutexLockHolder<itk::FastMutexLock> lock(*m_Mutex);
  double PrincipalPointX = m_CameraMatrix.at<double>(0,2);
  return PrincipalPointX;
}
double mitk::CameraIntrinsics::GetPrincipalPointY() const
{
  itk::MutexLockHolder<itk::FastMutexLock> lock(*m_Mutex);
  double PrincipalPointY = m_CameraMatrix.at<double>(1,2);
  return PrincipalPointY;
}
mitk::Point4D mitk::CameraIntrinsics::GetDistorsionCoeffsAsPoint4D() const
{
  itk::MutexLockHolder<itk::FastMutexLock> lock(*m_Mutex);
  mitk::Point4D coeffs;

  coeffs[0] = m_DistorsionCoeffs.at<double>(0,0);
  coeffs[1] = m_DistorsionCoeffs.at<double>(0,1);
  coeffs[2] = m_DistorsionCoeffs.at<double>(0,2);
  coeffs[3] = m_DistorsionCoeffs.at<double>(0,3);

  return coeffs;
}

mitk::Point3D mitk::CameraIntrinsics::GetFocalPoint() const
{
  mitk::Point3D p;
  p[0] = this->GetFocalLengthX();
  p[1] = this->GetFocalLengthY();
  p[2] = 0;
  return p;
}

mitk::Point3D mitk::CameraIntrinsics::GetPrincipalPoint() const
{
  mitk::Point3D p;
  p[0] = this->GetPrincipalPointX();
  p[1] = this->GetPrincipalPointY();
  p[2] = 0;
  return p;
}

vnl_vector_fixed<mitk::ScalarType, 2>
     mitk::CameraIntrinsics::GetFocalPointAsVnlVector() const
{
  vnl_vector_fixed<mitk::ScalarType, 2> vec;
  vec[0] = this->GetFocalLengthX();
  vec[1] = this->GetFocalLengthY();
  return vec;
}

vnl_vector_fixed<mitk::ScalarType, 2>
     mitk::CameraIntrinsics::GetPrincipalPointAsVnlVector() const
{
  vnl_vector_fixed<mitk::ScalarType, 2> vec;
  vec[0] = this->GetPrincipalPointX();
  vec[1] = this->GetPrincipalPointY();
  return vec;
}

std::ostream& operator<< (std::ostream& os, mitk::CameraIntrinsics::Pointer p)
{
  os << p->ToString();
  return os;
}

std::string mitk::CameraIntrinsics::GetString()
{
  return this->ToString();
}

std::string mitk::CameraIntrinsics::ToOctaveString(
    const std::string& varName)
{
  std::ostringstream s;
  s << varName << " = [" << this->GetFocalLengthX() << " 0 "
      << this->GetPrincipalPointX() << "; 0 " <<
      this->GetFocalLengthY() << " " << this->GetPrincipalPointY() << ";"
      << " 0 0 1 ];";
  return s.str();
}

void mitk::CameraIntrinsics::SetValid( bool valid )
{
  itk::MutexLockHolder<itk::FastMutexLock> lock(*m_Mutex);
  m_Valid = valid;
}
