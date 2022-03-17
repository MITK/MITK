/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkCameraIntrinsics.h"
#include <mitkEndoMacros.h>
#include <mitkEndoDebug.h>
#include <tinyxml2.h>

mitk::CameraIntrinsics::CameraIntrinsics()
  : m_Valid(false)
{
  m_CameraMatrix = cv::Mat::zeros(3, 3, cv::DataType<double>::type);
  m_CameraMatrix.at<double>(2,2) = 1.0;
  m_DistorsionCoeffs = cv::Mat::zeros(1, 5, cv::DataType<double>::type);
}

mitk::CameraIntrinsics::CameraIntrinsics(const CameraIntrinsics& other)
  : itk::Object()
  , mitk::XMLSerializable()
  , m_Valid(false)
{
  this->Copy(&other);
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
  std::lock_guard<std::mutex> lock(m_Mutex);
  return m_Valid;
}

vnl_matrix_fixed<mitk::ScalarType, 3, 3>
    mitk::CameraIntrinsics::GetVnlCameraMatrix() const
{
  vnl_matrix_fixed<mitk::ScalarType, 3, 3> mat;
  mat.set_identity();

  {
    std::lock_guard<std::mutex> lock(m_Mutex);
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
  std::lock_guard<std::mutex> lock(m_Mutex);
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
    std::lock_guard<std::mutex> lock(m_Mutex);
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
    std::lock_guard<std::mutex> lock(m_Mutex);
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
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_CameraMatrix.at<double>(0,0) = x;
    m_CameraMatrix.at<double>(1,1) = y;
  }
  this->Modified();
}

void mitk::CameraIntrinsics::SetPrincipalPoint( double x, double y )
{
  {
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_CameraMatrix.at<double>(0,2) = x;
    m_CameraMatrix.at<double>(1,2) = y;
  }
  this->Modified();
}

void mitk::CameraIntrinsics::SetDistorsionCoeffs( double k1, double k2,
                                                 double p1, double p2 )
{

  {
    std::lock_guard<std::mutex> lock(m_Mutex);

    m_DistorsionCoeffs.at<double>(0,0) = k1;
    m_DistorsionCoeffs.at<double>(0,1) = k2;
    m_DistorsionCoeffs.at<double>(0,2) = p1;
    m_DistorsionCoeffs.at<double>(0,3) = p2;
  }
  this->Modified();
}

cv::Mat mitk::CameraIntrinsics::GetCameraMatrix() const
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  return m_CameraMatrix.clone(); // return a copy of this small matrix
}

cv::Mat mitk::CameraIntrinsics::GetDistorsionCoeffs() const
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  return m_DistorsionCoeffs.clone(); // return a copy of this small matrix
}

cv::Mat mitk::CameraIntrinsics::GetDistorsionCoeffs()
{
  const CameraIntrinsics* intrinsics = this;
  return intrinsics->GetDistorsionCoeffs();
}

std::string mitk::CameraIntrinsics::ToString() const
{
  std::lock_guard<std::mutex> lock(m_Mutex);
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

void mitk::CameraIntrinsics::ToXML(tinyxml2::XMLElement* elem) const
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  elem->SetValue(this->GetNameOfClass());
  std::ostringstream s; s.precision(12);
  const cv::Mat& CameraMatrix = m_CameraMatrix;
  s.str(""); s << CameraMatrix.at<double>(0,0);
  elem->SetAttribute( "fx", s.str().c_str() );
  s.str(""); s << CameraMatrix.at<double>(1,1);
  elem->SetAttribute( "fy", s.str().c_str());
  s.str(""); s << CameraMatrix.at<double>(0,2);
  elem->SetAttribute( "cx", s.str().c_str());
  s.str(""); s << CameraMatrix.at<double>(1,2);
  elem->SetAttribute( "cy", s.str().c_str());

  const cv::Mat& DistorsionCoeffs = m_DistorsionCoeffs;
  s.str(""); s << DistorsionCoeffs.at<double>(0,0);
  elem->SetAttribute( "k1", s.str().c_str());
  s.str(""); s << DistorsionCoeffs.at<double>(0,1);
  elem->SetAttribute( "k2", s.str().c_str());
  s.str(""); s << DistorsionCoeffs.at<double>(0,2);
  elem->SetAttribute( "p1", s.str().c_str());
  s.str(""); s << DistorsionCoeffs.at<double>(0,3);
  elem->SetAttribute( "p2", s.str().c_str());
  elem->SetAttribute("Valid", static_cast<int>(m_Valid));
  //s.str(""); s << DistorsionCoeffs.at<double>(4,0);
  //elem->SetAttribute( "k3", s.str().c_str() );
}

void mitk::CameraIntrinsics::FromGMLCalibrationXML(const tinyxml2::XMLElement* elem)
{
  assert( elem );
  assert( std::string(elem->Value()) == "results" );
  cv::Mat CameraMatrix = cv::Mat::zeros(3, 3, cv::DataType<double>::type);
  CameraMatrix.at<double>(2,2) = 1.0;
  cv::Mat DistorsionCoeffs = cv::Mat::zeros(1, 5, cv::DataType<double>::type);

  const auto* focus_lenXElem = elem->FirstChildElement("focus_lenX");
  endoAssert( focus_lenXElem != nullptr );
  CameraMatrix.at<double>(0,0) = atof( focus_lenXElem->GetText() );

  const auto* focus_lenYElem = elem->FirstChildElement("focus_lenY");
  endoAssert( focus_lenYElem != nullptr );
  CameraMatrix.at<double>(1,1) = atof( focus_lenYElem->GetText() );

  const auto* PrincipalXElem = elem->FirstChildElement("PrincipalX");
  endoAssert( PrincipalXElem != nullptr );
  CameraMatrix.at<double>(0,2) = atof( PrincipalXElem->GetText() );

  const auto* PrincipalYElem = elem->FirstChildElement("PrincipalY");
  endoAssert( PrincipalYElem != nullptr );
  CameraMatrix.at<double>(1,2) = atof( PrincipalYElem->GetText() );

  // DISTORSION COEFFS

  const auto* Dist1Elem = elem->FirstChildElement("Dist1");
  endoAssert( Dist1Elem != nullptr );
  DistorsionCoeffs.at<double>(0,0) = atof( Dist1Elem->GetText() );

  const auto* Dist2Elem = elem->FirstChildElement("Dist2");
  endoAssert( Dist2Elem != nullptr );
  DistorsionCoeffs.at<double>(0,1) = atof( Dist2Elem->GetText() );

  const auto* Dist3Elem = elem->FirstChildElement("Dist3");
  endoAssert( Dist3Elem != nullptr );
  DistorsionCoeffs.at<double>(0,2) = atof( Dist3Elem->GetText() );

  const auto* Dist4Elem = elem->FirstChildElement("Dist4");
  endoAssert( Dist4Elem != nullptr );
  DistorsionCoeffs.at<double>(0,3) = atof( Dist4Elem->GetText() );

  int valid = 0;
  elem->QueryIntAttribute("Valid", &valid);

  {
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_Valid = static_cast<bool>(valid);
    m_CameraMatrix = CameraMatrix;
    m_DistorsionCoeffs = DistorsionCoeffs;
  }

  this->Modified();
}

void mitk::CameraIntrinsics::FromXML(const tinyxml2::XMLElement* elem)
{
  endoAssert ( elem );
  MITK_DEBUG << elem->Value();
  const char* filename = elem->Attribute("file");
  if(nullptr != filename)
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
  double val = 0.0;
  if(elem->QueryDoubleAttribute("fx", &val) == tinyxml2::XML_SUCCESS)
    CameraMatrix.at<double>(0,0) = val;
  else
    err << "fx, ";

  if(elem->QueryDoubleAttribute("fy", &val) == tinyxml2::XML_SUCCESS)
    CameraMatrix.at<double>(1,1) = val;
  else
    err << "fy, ";

  if(elem->QueryDoubleAttribute("cx", &val) == tinyxml2::XML_SUCCESS)
    CameraMatrix.at<double>(0,2) = val;
  else
    err << "cx, ";

  if(elem->QueryDoubleAttribute("cy", &val) == tinyxml2::XML_SUCCESS)
    CameraMatrix.at<double>(1,2) = val;
  else
    err << "cy, ";

  // DISTORSION COEFFS
  endodebug( "creating DistorsionCoeffs from XML file")
  cv::Mat DistorsionCoeffs = cv::Mat::zeros(1, 5, cv::DataType<double>::type);
  if(elem->QueryDoubleAttribute("k1", &val) == tinyxml2::XML_SUCCESS)
    DistorsionCoeffs.at<double>(0,0) = val;
  else
    err << "k1, ";

  if(elem->QueryDoubleAttribute("k2", &val) == tinyxml2::XML_SUCCESS)
    DistorsionCoeffs.at<double>(0,1) = val;
  else
    err << "k2, ";

  if(elem->QueryDoubleAttribute("p1", &val) == tinyxml2::XML_SUCCESS)
    DistorsionCoeffs.at<double>(0,2) = val;
  else
    err << "p1, ";

  if(elem->QueryDoubleAttribute("p2", &val) == tinyxml2::XML_SUCCESS)
    DistorsionCoeffs.at<double>(0,3) = val;
  else
    err << "p2, ";

  DistorsionCoeffs.at<double>(0,4) = 0.0;

  /*if(elem->QueryDoubleAttribute("k3", &val) == tinyxml2::XML_SUCCESS)
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
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_Valid = static_cast<bool>(valid);
    m_CameraMatrix = CameraMatrix;
    m_DistorsionCoeffs = DistorsionCoeffs;
  }

  this->Modified();
}

double mitk::CameraIntrinsics::GetFocalLengthX() const
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  double FocalLengthX = m_CameraMatrix.at<double>(0,0);

  return FocalLengthX;
}
double mitk::CameraIntrinsics::GetFocalLengthY() const
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  double FocalLengthY = m_CameraMatrix.at<double>(1,1);;
  return FocalLengthY;
}
double mitk::CameraIntrinsics::GetPrincipalPointX() const
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  double PrincipalPointX = m_CameraMatrix.at<double>(0,2);
  return PrincipalPointX;
}
double mitk::CameraIntrinsics::GetPrincipalPointY() const
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  double PrincipalPointY = m_CameraMatrix.at<double>(1,2);
  return PrincipalPointY;
}
mitk::Point4D mitk::CameraIntrinsics::GetDistorsionCoeffsAsPoint4D() const
{
  std::lock_guard<std::mutex> lock(m_Mutex);
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
  std::lock_guard<std::mutex> lock(m_Mutex);
  m_Valid = valid;
}

itk::LightObject::Pointer mitk::CameraIntrinsics::InternalClone() const
{
  itk::LightObject::Pointer result(new Self(*this));
  result->UnRegister();
  return result;
}
