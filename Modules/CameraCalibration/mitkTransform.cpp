#include "mitkTransform.h"
#include <fstream>
#include <vnl/vnl_inverse.h>
#include <mitkVnlVectorFixedCaster.h>
#include <mitkVnlVectorFromCvMat.h>
#include <mitkVnlMatrixFromCvMat.h>
#include <mitkCvMatFromVnlVector.h>
namespace mitk
{
  // DO NOT CHANGE THE VALUES OF THESE CONSTANTS!!
  const std::string Transform::UNKNOWN_TYPE = "Unknown type";
  const std::string Transform::ENDOSCOPE_SCOPE_TOOL = "Endoscope scope tool";
  const std::string Transform::ENDOSCOPE_CAM_TOOL = "Endoscope camera tool";
  const std::string Transform::CHESSBOARD_TOOL = "Chessboard tool";
  const std::string Transform::POINTER_TOOL = "Pointer tool";
  const std::string Transform::POINTER_TO_CHESSBOARD_ORIGIN = "Pointer to chessboard origin";
  const std::string Transform::POINTER_TO_CHESSBOARD_X_SUPPORT_POINT = "Pointer to chessboard X support origin";
  const std::string Transform::POINTER_TO_CHESSBOARD_Y_SUPPORT_POINT = "Pointer to chessboard Y support origin";
  const std::string Transform::BOARD_TO_BOARD_TOOL = "Board to board tool";
  const std::string Transform::REFERENCE_CAMERA_TRANSFORM = "Reference camera transform";
  const std::string Transform::REFERENCE_SCOPE_TRANSFORM = "Reference scope transform";
  const std::string Transform::EYE_TO_HAND_TRANSFORM = "Eye to hand transform";
  const std::string Transform::CAMERA_EXTRINSICS = "Camera extrinsics";

  Transform::Transform()
    : m_NavData(mitk::NavigationData::New()), m_Type( UNKNOWN_TYPE )
  {
    vnl_matrix_fixed<mitk::ScalarType, 3, 3> rot;
    rot.set_identity();
    this->SetRotation( rot );
  }

  Transform::Transform(const mitk::NavigationData* nd)
    : m_NavData(mitk::NavigationData::New()), m_Type( UNKNOWN_TYPE )
  {
        m_NavData->Graft(nd);
  }

  Transform::Transform(const std::string& s)
    : m_NavData(mitk::NavigationData::New()), m_Type( s )
  {
    vnl_matrix_fixed<mitk::ScalarType, 3, 3> rot;
    rot.set_identity();
    this->SetRotation( rot );
  }

  void Transform::Copy(const mitk::NavigationData* nd)
  {
    (const_cast<mitk::NavigationData*>(m_NavData.GetPointer()))->Graft(nd);
  }

  void Transform::Concatenate( mitk::Transform* transform )
  {
    vnl_matrix_fixed<mitk::ScalarType, 4, 4> mat = transform->GetMatrix();
    mat = mat * this->GetMatrix(); //
    this->SetMatrix( mat );

  }

  void Transform::Concatenate( const vnl_matrix_fixed<mitk::ScalarType, 4, 4>&
                               transform )
  {
    Transform::Pointer t = Transform::New();
    t->SetMatrix( transform );
    this->Concatenate( t );
  }

  void Transform::Concatenate( const vtkMatrix4x4* transform )
  {
    Transform::Pointer t = Transform::New();
    t->SetMatrix( transform );
    this->Concatenate( t );
  }

  void Transform::Reset()
  {
    mitk::NavigationData::Pointer nd
      = NavigationData::New();
    this->Copy( nd );
  }
  void  Transform::SetOrientation(
      const vnl_quaternion<mitk::ScalarType>& orientation)
  {
    m_NavData->SetOrientation(orientation);
    this->Modified();
  }

  void Transform::SetTranslation( const vnl_vector_fixed<mitk::ScalarType, 3>&
                                  transl)
  {
    mitk::Point3D p;
    for(unsigned int i=0; i<3; ++i)
      p[i] = transl[i];

    m_NavData->SetPosition(p);
    this->Modified();
  }

  void Transform::SetTranslation( float* array )
  {
    vnl_vector_fixed<mitk::ScalarType, 3> vec;
    for(unsigned int i=0; i<vec.size(); i++)
      vec(i) = array[i];
    this->SetTranslation( vec );
  }

  void Transform::SetRotation( float* array )
  {
    vnl_matrix_fixed<mitk::ScalarType, 3, 3> mat;

    unsigned int row = 0;
    unsigned int col = 0;
    for(unsigned int i=0; i<mat.rows()*mat.cols(); i++)
    {
      if( i > 0 && i % 3 == 0 )
      {
        ++row;
        col = 0;
      }

      mat(row,col) = array[i];
      ++col;
    }

    this->SetRotation( mat );
  }

  void Transform::SetOrientation( const vnl_quaternion<double>& orientation)
  {
    vnl_vector_fixed<mitk::ScalarType, 4> qvec;
    VnlVectorFixedCaster<double, mitk::ScalarType, 4> caster( &orientation, &qvec );
    caster.Update();

    mitk::Quaternion p( qvec );

    this->SetOrientation( p );
  }

  vnl_vector_fixed<double, 3> Transform::GetVnlDoubleTranslation() const
  {
    vnl_vector_fixed<mitk::ScalarType, 3> vecFloat = this->GetVnlTranslation();
    vnl_vector_fixed<double, 3> vecDouble;
    VnlVectorFixedCaster<mitk::ScalarType, double, 3> caster( &vecFloat, &vecDouble );
    caster.Update();

    return vecDouble;
  }

  void Transform::SetTranslation( const vnl_vector<double>& transl)
  {
    vnl_vector_fixed<double, 3> dTransl(transl);
    vnl_vector_fixed<mitk::ScalarType, 3> fTransl;
    VnlVectorFixedCaster<double, mitk::ScalarType, 3> caster( &dTransl, &fTransl );
    caster.Update();

    this->SetTranslation( fTransl );
  }

  vnl_quaternion<double> Transform::GetVnlDoubleQuaternion() const
  {
    mitk::Quaternion fOrientation = this->GetOrientation();
    vnl_quaternion<double> dOrientation;
    VnlVectorFixedCaster<mitk::ScalarType, double, 4> caster( &fOrientation, &dOrientation );
    caster.Update();

    return dOrientation;
  }

  void Transform::FromCSVFile(const std::string& file)
  {
    std::ifstream csvFile (file.c_str());
    endoAssert ( csvFile.fail() == false );

    mitk::Transform::Pointer transform = mitk::Transform::New();
    vnl_matrix_fixed<mitk::ScalarType, 4, 4> mat;
    std::string line;
    mitk::ScalarType d = 0.0f;
    int row=0,column = 0;

    while (std::getline (csvFile, line))
    {
      std::istringstream linestream(line);
      std::string item;
      column = 0;
      while (std::getline (linestream, item, ','))
      {
        std::istringstream number;
        number.str(item);
        number >> d;
        mat(row, column) = d;
        ++column;
      }
      ++row;
    }
    endoAssert( row == 4 && column == 4 );
    transform->SetMatrix( mat );

    this->SetNavigationData( transform->GetNavigationData() );
    // modified is called in SetNavigationData
  }

  std::string Transform::ToCSVString() const
  {
    std::ostringstream s; s.precision(12);

    vnl_matrix_fixed<mitk::ScalarType, 4, 4> mat
      = this->GetMatrix();

    for( unsigned int j=0; j<mat.rows(); ++j )
    {
      for( unsigned int k=0; k<mat.cols(); ++k )
      {
        s << mat(j,k);
        if(k+1<mat.cols())
          s  << ",";
      }
      if(j+1<mat.rows())
        s << std::endl;
    }

    return s.str();

  }

  std::string Transform::ToMatlabString(const std::string& varname
                                        , bool printLastRow) const
  {
    std::ostringstream s; s.precision(12);

    vnl_matrix_fixed<mitk::ScalarType, 4, 4> mat
      = this->GetMatrix();

    s << varname << " = [";
    for( unsigned int j=0; j<mat.rows(); ++j )
    {
      if( !printLastRow && j+1 == mat.rows() )
        break;
      for( unsigned int k=0; k<mat.cols(); ++k )
      {
        s << mat(j,k) << " ";
      }
      s << ";";
    }
    s << "];" << std::endl;

    return s.str();
  }

  void Transform::Copy( const mitk::Transform* transform ) 
  {
    m_NavData->Graft(transform->GetNavigationData());
    m_Type = transform->GetType();
  }

  mitk::Transform::Pointer Transform::Clone() const
  {
    Transform::Pointer copy = Transform::New();
    copy->Copy( this );
    return copy;
  }

  void Transform::SetMatrix( const vtkMatrix4x4* mat)
  {
    vnl_matrix_fixed<mitk::ScalarType, 4, 4> vnlMat;
    for(unsigned int i=0; i<4; ++i)
      for(unsigned int j=0; j<4; ++j)
        vnlMat(i,j) = mat->GetElement(i, j);

    this->SetMatrix( vnlMat );
  }

  void Transform::ToCSVFile(const std::string& file) const
  {
    std::ofstream csvFile;
    csvFile.open(file.c_str());
    endoAssert ( csvFile.fail() == false );
    csvFile << this->ToCSVString();
    csvFile.close();
  }

  void Transform::ToMatlabFile(const std::string& file
                               , const std::string& varname) const
  {
    std::ofstream csvFile;
    csvFile.open(file.c_str());
    endoAssert ( csvFile.fail() == false );
    csvFile << this->ToMatlabString(varname);
    csvFile.close();
  }

  void Transform::SetNavigationData( const mitk::NavigationData* naviData )
  {
    endoAssert( naviData != 0 );
    m_NavData->Graft( naviData );
    this->Modified();
  }

  void Transform::SetRotation( vnl_matrix_fixed<mitk::ScalarType, 3, 3>& mat)
  {
    this->m_NavData->SetOrientation( mitk::Quaternion(mat) );
    this->Modified();
  }

  void Transform::SetRotation( vnl_matrix<mitk::ScalarType>& mat)
  {
    vnl_matrix_fixed<mitk::ScalarType, 3, 3> tmp(mat);
    this->SetRotation( tmp );
  }

  void Transform::SetPosition( const mitk::Point3D& transl)
  {
    this->SetTranslation( transl.GetVnlVector() );
  }

  void Transform::SetTranslation( double array[3] )
  {
    mitk::Point3D p;
    for(unsigned int i = 0; i < 3; ++i)
      p.SetElement(i, array[i]);
    this->SetTranslation( p.GetVnlVector() );
  }


  void Transform::SetRotation( double array[3][3] )
  {
    vnl_matrix_fixed<mitk::ScalarType, 3, 3> mat;

    for(unsigned int i = 0; i < 3; ++i)
      for(unsigned int j = 0; j < 3; ++j)
        mat(i, j) = array[i][j];
    this->SetRotation( mat );
  }

  void Transform::Invert()
  {
    vnl_matrix_fixed<mitk::ScalarType, 4, 4> tmp(this->GetMatrix());
    this->SetMatrix( vnl_inverse( tmp ) );
  }

  void Transform::SetMatrix(
     const vnl_matrix_fixed<mitk::ScalarType, 4, 4>& mat)
  {
    // set translation first
    vnl_vector<mitk::ScalarType> transl = mat.get_column(3);
    mitk::Point3D p;
    for(unsigned int i=0; i<3; ++i)
      p[i] = transl[i];

    m_NavData->SetPosition(p);

    // set rotation
    vnl_matrix_fixed<mitk::ScalarType, 3, 3> rotMatFixed(
      mat.extract(3,3));
    this->SetRotation(rotMatFixed);
  }

  bool Transform::IsValid() const
  {
    return m_NavData->IsDataValid();
  }

  void Transform::SetTranslation( const cv::Mat& transl)
  {
    vnl_vector<mitk::ScalarType> vec(3);
    VnlVectorFromCvMat<mitk::ScalarType> _VnlVectorFromCvMat( &transl, &vec );
    _VnlVectorFromCvMat.Update();
    this->SetTranslation( vnl_vector_fixed<mitk::ScalarType, 3>( vec ) );
  }

  void Transform::SetRotation( const cv::Mat& mat )
  {
    vnl_matrix<mitk::ScalarType> vnlMat(3, 3);
    VnlMatrixFromCvMat<mitk::ScalarType> _VnlMatrixFromCvMat( &mat, &vnlMat );
    _VnlMatrixFromCvMat.Update();
    vnl_matrix_fixed<mitk::ScalarType, 3, 3> vnlMatFixed(vnlMat);

    this->SetRotation(vnlMatFixed);
  }

  void Transform::SetRotationVector( const cv::Mat& rotVec )
  {
    cv::Mat rotMat;
    cv::Rodrigues( rotVec, rotMat );

    vnl_matrix<mitk::ScalarType> vnlMat(3, 3);
    VnlMatrixFromCvMat<mitk::ScalarType> _VnlMatrixFromCvMat( &rotMat, &vnlMat );
    _VnlMatrixFromCvMat.Update();


    vnl_matrix_fixed<mitk::ScalarType, 3, 3> vnlMatFixed(vnlMat);

    this->SetRotation( vnlMatFixed );
  }

  //# getter
  mitk::NavigationData::Pointer Transform::GetNavigationData() const
  {
    return m_NavData;
  }

  mitk::Point3D Transform::GetTranslation() const
  {
    return m_NavData->GetPosition();
  }

  mitk::Point3D Transform::GetPosition() const
  {
    return m_NavData->GetPosition();
  }

  mitk::Quaternion Transform::GetOrientation() const
  {
    return m_NavData->GetOrientation();
  }

  void Transform::GetMatrix(vtkMatrix4x4* matrix) const
  {
    vnl_matrix_fixed<mitk::ScalarType, 4, 4> vnlMat = this->GetMatrix();
    for(unsigned int i=0; i<vnlMat.rows(); ++i)
      for(unsigned int j=0; j<vnlMat.cols(); ++j)
        matrix->SetElement(i,j, vnlMat(i,j));
  }

  void Transform::GetVtkOpenGlMatrix(vtkMatrix4x4* matrix) const
  {
    vnl_matrix<mitk::ScalarType> vnlRotation
        = this->GetVnlRotationMatrix().as_matrix();

    // normalize rows of rotation matrix
    vnlRotation.normalize_rows();

    vnl_matrix<mitk::ScalarType> vnlInverseRotation(3,3);
    // invert rotation
    vnlInverseRotation = vnl_matrix_inverse<mitk::ScalarType>(vnlRotation);

    vnl_vector<mitk::ScalarType> vnlTranslation
        = this->GetPosition().GetVnlVector();
    // rotate translation vector by inverse rotation P = P'
    vnlTranslation = vnlInverseRotation * vnlTranslation;
    vnlTranslation *= -1;  // save -P'

    // set position
    mitk::Transform::Pointer tmp = mitk::Transform::New();

    tmp->SetTranslation( vnlTranslation );
    tmp->SetRotation( vnlRotation );
    tmp->GetMatrix(matrix);
  }

  //# cv getter
  cv::Mat Transform::GetCvTranslation() const
  {
    cv::Mat mat;
    vnl_vector<mitk::ScalarType> vec = this->GetVnlTranslation().as_vector();
    endodebugvar( vec )
    CvMatFromVnlVector<mitk::ScalarType> _CvMatFromVnlVector(&vec, &mat);
    _CvMatFromVnlVector.Update();
    return mat;
  }

  cv::Mat Transform::GetCvRotationMatrix() const
  {
    cv::Mat mat;
    vnl_matrix<mitk::ScalarType> vec = this->GetVnlRotationMatrix().as_matrix();
    endodebugvar( vec )
    CvMatFromVnlMatrix<mitk::ScalarType> _CvMatFromVnlMatrix(&vec, &mat);
    _CvMatFromVnlMatrix.Update();
    return mat;
  }

  cv::Mat Transform::GetCvMatrix() const
  {
    cv::Mat mat;
    vnl_matrix<mitk::ScalarType> vec = this->GetMatrix().as_matrix();
    CvMatFromVnlMatrix<mitk::ScalarType> _CvMatFromVnlMatrix(&vec, &mat);
    _CvMatFromVnlMatrix.Update();
    return mat;
  }

  cv::Mat Transform::GetCvRotationVector() const
  {
    cv::Mat rotVec(3,1,cv::DataType<mitk::ScalarType>::type);
    cv::Rodrigues( this->GetCvRotationMatrix(), rotVec );
    return rotVec;
  }

  //# vnl getter
  vnl_vector_fixed<mitk::ScalarType, 3> Transform::GetVnlTranslation() const
  {
    vnl_vector_fixed<mitk::ScalarType, 3> vec(m_NavData->GetPosition()
                                              .GetVnlVector());
    return vec;
  }

  vnl_matrix_fixed<mitk::ScalarType, 3, 3> Transform::GetVnlRotationMatrix() const
  {
    return m_NavData->GetOrientation().rotation_matrix_transpose();
  }

  vnl_matrix_fixed<double, 4, 4> Transform::GetVnlDoubleMatrix() const
  {
    vnl_matrix_fixed<mitk::ScalarType, 4, 4> mat = this->GetMatrix();

    vnl_matrix_fixed<double, 4, 4> doubleMat;

    for(unsigned int i=0; i<mat.rows(); ++i)
      for(unsigned int j=0; j<mat.cols(); ++j)
        doubleMat(i,j) = static_cast<double>( mat(i,j) );

    return doubleMat;
  }

  vnl_matrix_fixed<mitk::ScalarType, 4, 4> Transform::GetMatrix()
      const
  {
    vnl_vector_fixed<mitk::ScalarType, 3> transl = this->GetVnlTranslation();
    vnl_matrix_fixed<mitk::ScalarType, 3, 3> rot = this->GetVnlRotationMatrix();

    vnl_matrix_fixed<mitk::ScalarType, 4, 4> homMat;
    homMat.set_identity();
    //std::cout << homMat << std::endl;
    for(unsigned int i=0; i<rot.rows(); ++i)
      for(unsigned int j=0; j<rot.cols(); ++j)
        homMat(i,j) = rot(i,j);
    for(unsigned int i=0; i<transl.size(); ++i)
      homMat(i,3) = transl[i];

    return homMat;
  }

  void Transform::TransposeRotation()
  {
    vnl_matrix_fixed<mitk::ScalarType, 3, 3> rotMat = this->GetVnlRotationMatrix().transpose();
    this->SetRotation( rotMat );
  }

  void Transform::SetValid( bool valid )
  {
    if( m_NavData->IsDataValid() == valid )
      return;

    m_NavData->SetDataValid( valid );
    this->Modified();    
  }

  std::string mitk::Transform::ToString() const
  {
    std::ostringstream s; s.precision(12);

    mitk::NavigationData::PositionType position;
    position.Fill(0.0);
    position = m_NavData->GetPosition();

    mitk::NavigationData::OrientationType orientation(0.0, 0.0, 0.0, 0.0);
    orientation = m_NavData->GetOrientation();

    s << "Translation: [" << position[0] << ", " << position[1] << ", "
        << position[2] << "]";
    s << ", orientation: [" << orientation[0] << ", " << orientation[1] << ", "
        << orientation[2] << ", " << orientation[3] << "]";
    s << ", valid: [" << (this->IsValid()? "true": "false") << "]";

    return s.str();
  }

  void mitk::Transform::ToXML(TiXmlElement* elem) const
  {
    std::string value = elem->ValueStr();
    if(value.empty())
      elem->SetValue(this->GetNameOfClass());

    mitk::NavigationData::PositionType position;
    position.Fill(0.0);
    position = m_NavData->GetPosition();

    mitk::NavigationData::OrientationType orientation(0.0, 0.0, 0.0, 0.0);
    orientation = m_NavData->GetOrientation();

    mitk::NavigationData::CovarianceMatrixType matrix;
    matrix.SetIdentity();
    matrix = m_NavData->GetCovErrorMatrix();

    bool hasPosition = true;
    hasPosition = m_NavData->GetHasPosition();
    bool hasOrientation = true;
    hasOrientation = m_NavData->GetHasOrientation();
    bool dataValid = false;
    dataValid = m_NavData->IsDataValid();
    mitk::NavigationData::TimeStampType timestamp=0.0;

    elem->SetAttribute("Type", m_Type);
    elem->SetDoubleAttribute("Time", timestamp);
    elem->SetDoubleAttribute("X", position[0]);
    elem->SetDoubleAttribute("Y", position[1]);
    elem->SetDoubleAttribute("Z", position[2]);

    elem->SetDoubleAttribute("QX", orientation[0]);
    elem->SetDoubleAttribute("QY", orientation[1]);
    elem->SetDoubleAttribute("QZ", orientation[2]);
    elem->SetDoubleAttribute("QR", orientation[3]);

    elem->SetDoubleAttribute("C00", matrix[0][0]);
    elem->SetDoubleAttribute("C01", matrix[0][1]);
    elem->SetDoubleAttribute("C02", matrix[0][2]);
    elem->SetDoubleAttribute("C03", matrix[0][3]);
    elem->SetDoubleAttribute("C04", matrix[0][4]);
    elem->SetDoubleAttribute("C05", matrix[0][5]);
    elem->SetDoubleAttribute("C10", matrix[1][0]);
    elem->SetDoubleAttribute("C11", matrix[1][1]);
    elem->SetDoubleAttribute("C12", matrix[1][2]);
    elem->SetDoubleAttribute("C13", matrix[1][3]);
    elem->SetDoubleAttribute("C14", matrix[1][4]);
    elem->SetDoubleAttribute("C15", matrix[1][5]);

    if (dataValid)
      elem->SetAttribute("Valid",1);
    else
      elem->SetAttribute("Valid",0);

    if (hasOrientation)
      elem->SetAttribute("hO",1);
    else
      elem->SetAttribute("hO",0);

    if (hasPosition)
      elem->SetAttribute("hP",1);
    else
      elem->SetAttribute("hP",0);
  }

  void mitk::Transform::FromXML(TiXmlElement* elem)
  {
    assert(elem);

    mitk::NavigationData::Pointer nd = mitk::NavigationData::New();
    mitk::NavigationData::PositionType position;
    mitk::NavigationData::OrientationType orientation(0.0,0.0,0.0,0.0);
    mitk::NavigationData::TimeStampType timestamp = -1;
    mitk::NavigationData::CovarianceMatrixType matrix;

    bool hasPosition = true;
    bool hasOrientation = true;
    bool dataValid = false;

    position.Fill(0.0);
    matrix.SetIdentity();

    std::string type = Transform::UNKNOWN_TYPE;
    elem->QueryStringAttribute("Type", &type);
    elem->QueryDoubleAttribute("Time",&timestamp);

    // position and orientation is mandatory!
    if(elem->QueryFloatAttribute("X", &position[0]) != TIXML_SUCCESS)
      throw std::invalid_argument("No X position found in xml");
    if(elem->QueryFloatAttribute("Y", &position[1]) != TIXML_SUCCESS)
      throw std::invalid_argument("No Y position found in xml");
    if(elem->QueryFloatAttribute("Z", &position[2]) != TIXML_SUCCESS)
      throw std::invalid_argument("No Z position found in xml");

    if(elem->QueryFloatAttribute("QX", &orientation[0]) != TIXML_SUCCESS)
      throw std::invalid_argument("No QX orientation found in xml");
    if(elem->QueryFloatAttribute("QY", &orientation[1]) != TIXML_SUCCESS)
      throw std::invalid_argument("No QY orientation found in xml");
    if(elem->QueryFloatAttribute("QZ", &orientation[2]) != TIXML_SUCCESS)
      throw std::invalid_argument("No QZ orientation found in xml");
    if(elem->QueryFloatAttribute("QR", &orientation[3]) != TIXML_SUCCESS)
      throw std::invalid_argument("No QR orientation found in xml");

    elem->QueryFloatAttribute("C00", &matrix[0][0]);
    elem->QueryFloatAttribute("C01", &matrix[0][1]);
    elem->QueryFloatAttribute("C02", &matrix[0][2]);
    elem->QueryFloatAttribute("C03", &matrix[0][3]);
    elem->QueryFloatAttribute("C04", &matrix[0][4]);
    elem->QueryFloatAttribute("C05", &matrix[0][5]);
    elem->QueryFloatAttribute("C10", &matrix[1][0]);
    elem->QueryFloatAttribute("C11", &matrix[1][1]);
    elem->QueryFloatAttribute("C12", &matrix[1][2]);
    elem->QueryFloatAttribute("C13", &matrix[1][3]);
    elem->QueryFloatAttribute("C14", &matrix[1][4]);
    elem->QueryFloatAttribute("C15", &matrix[1][5]);

    int tmpval = 0;
    elem->QueryIntAttribute("Valid", &tmpval);
    if (tmpval == 0)
      dataValid = false;
    else
      dataValid = true;

    tmpval = 0;
    elem->QueryIntAttribute("hO", &tmpval);
    if (tmpval == 0)
      hasOrientation = false;
    else
      hasOrientation = true;

    tmpval = 0;
    elem->QueryIntAttribute("hP", &tmpval);
    if (tmpval == 0)
      hasPosition = false;
    else
      hasPosition = true;

    nd->SetTimeStamp(timestamp);
    nd->SetPosition(position);
    nd->SetOrientation(orientation);
    nd->SetCovErrorMatrix(matrix);
    nd->SetDataValid(dataValid);
    nd->SetHasOrientation(hasOrientation);
    nd->SetHasPosition(hasPosition);

    m_NavData = nd;
    m_Type = type;

    this->Modified();
  }

} // namespace mitk

std::ostream& operator<< (std::ostream& os, mitk::Transform::Pointer p)
{
  os << p->ToString();
  return os;
}
