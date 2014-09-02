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
#ifndef MITKTRANSFORM_H
#define MITKTRANSFORM_H

#include <itkObject.h>
#include <cv.h>
#include <mitkCommon.h>
#include <mitkNumericTypes.h>
#include <mitkNavigationData.h>
#include <tinyxml.h>
#include <sstream>
#include <vtkMatrix4x4.h>
#include <mitkXMLSerializable.h>
#include <MitkCameraCalibrationExports.h>

namespace mitk {

  ///
  /// \brief class representing a transfrom in 3D
  ///
  /// internally it stores a mitk navigation data. this is more
  /// or less a wrapper for navigation data for easy casting
  /// between opencv/vnl/mitk/xml representations of transform
  /// data
  ///
  class MitkCameraCalibration_EXPORT Transform: public itk::Object,
    public XMLSerializable
  {
  public:
    mitkClassMacro(Transform, itk::Object);
    itkFactorylessNewMacro(Transform);
    mitkNewMacro1Param(Transform, const mitk::NavigationData*);
    mitkNewMacro1Param(Transform, const std::string&);

    ///
    /// constants describing the type of transform
    /// represented here
    ///
    static const std::string UNKNOWN_TYPE;
    static const std::string ENDOSCOPE_SCOPE_TOOL;
    static const std::string ENDOSCOPE_CAM_TOOL;
    static const std::string CHESSBOARD_TOOL;
    static const std::string POINTER_TOOL;
    static const std::string POINTER_TO_CHESSBOARD_ORIGIN;
    static const std::string POINTER_TO_CHESSBOARD_X_SUPPORT_POINT;
    static const std::string POINTER_TO_CHESSBOARD_Y_SUPPORT_POINT;
    static const std::string BOARD_TO_BOARD_TOOL;
    static const std::string REFERENCE_CAMERA_TRANSFORM;
    static const std::string REFERENCE_SCOPE_TRANSFORM;
    static const std::string EYE_TO_HAND_TRANSFORM;
    static const std::string CAMERA_EXTRINSICS;

    itkGetConstMacro(Type, std::string);
    itkSetMacro(Type, std::string&);
    ///
    /// Copies the content of transform to this
    /// instance
    ///
    void Copy( const mitk::Transform* transform );

    ///
    /// Copies the content of transform to this
    /// instance
    ///
    void Copy( const mitk::NavigationData* transform );

    ///
    /// Inverts the rotation of this transform
    /// (Polaris navigation Data have inverted rotation
    /// so you may want to call this function when using
    /// polaris data)
    ///
    void TransposeRotation();

    ///
    /// get a copy of this transform
    ///
    mitk::Transform::Pointer Clone() const;

    ///
    /// concatenate this transform with the given one,
    /// i.e. this transform is done first, then transform
    /// ( if x is this transform, y is transform, then this will be y*x)
    /// post multiply semantics!
    /// \see vtkTransform
    ///
    void Concatenate( mitk::Transform* transform );
    ///
    /// same as above with vnl mat argument
    ///
    void Concatenate( const vnl_matrix_fixed<mitk::ScalarType, 4, 4>& transform );

    ///
    /// same as above with vtk mat argument
    ///
    void Concatenate( const vtkMatrix4x4* transform );

    ///
    /// invert this transform
    ///
    void Invert();
    ///
    /// resets the internal variables except type
    ///
    void Reset();

    ///
    /// read from xml
    ///
    void FromXML(TiXmlElement* elem);
    ///
    /// read csv file
    ///
    void FromCSVFile(const std::string& file);
    ///
    /// grafts the data from naviData to this transform
    ///
    void SetNavigationData( const mitk::NavigationData* naviData );
    ///
    /// method to set orientation quat
    ///
    void SetOrientation( const vnl_quaternion<mitk::ScalarType>& orientation);
    ///
    /// method to set float valued orientation quat
    ///
    void SetOrientation( const vnl_quaternion<float>& orientation);
    ///
    /// method to set translation
    ///
    void SetTranslation( const vnl_vector_fixed<mitk::ScalarType, 3>& transl);
    ///
    /// method to set a vector of doubles as translation
    ///
    void SetTranslation( const vnl_vector<double>& transl);
    ///
    /// method to set a mitk::Point3D as position
    ///
    void SetPosition( const mitk::Point3D& transl);
    ///
    /// sets rotation with a rotation matrix
    ///
    void SetRotation( vnl_matrix_fixed<mitk::ScalarType, 3, 3>& mat);
    ///
    /// sets rotation with a non fixed rotation matrix
    ///
    void SetRotation( vnl_matrix<mitk::ScalarType>& mat);
    ///
    /// sets rotation and translation with a transformation matrix
    ///
    void SetMatrix( const vnl_matrix_fixed<mitk::ScalarType, 4, 4>& mat);
    ///
    /// sets rotation and translation with a vtk transformation matrix
    ///
    void SetMatrix( const vtkMatrix4x4* mat);
    ///
    /// sets translation from a POD vector
    ///
    void SetTranslation( float* array );
    ///
    /// sets translation from a POD vector. this must be a
    /// 3x3=9 sized vector in row major format (first row = first
    /// three elements)
    ///
    void SetRotation( float* array );
    ///
    /// sets translation from a POD vector
    ///
    void SetTranslation( double array[3] );
    ///
    /// sets translation from a POD vector
    ///
    void SetRotation( double array[3][3] );

    ///
    /// method to set translation by cv vector
    ///
    void SetTranslation( const cv::Mat& transl);
    ///
    /// sets rotation with a rotation matrix
    ///
    void SetRotation( const cv::Mat& mat );
    ///
    /// sets rotation with a rodrigues rotation vector
    ///
    void SetRotationVector( const cv::Mat& rotVec);

    ///
    /// \return the navigation data that stores all information
    ///
    mitk::NavigationData::Pointer GetNavigationData() const;
    ///
    /// calls navigationdata::GetPosition()
    ///
    mitk::Point3D GetPosition() const;
    ///
    /// same as GetPosition
    ///
    mitk::Point3D GetTranslation() const;
    ///
    /// calls navigationdata::IsValid()
    ///
    bool IsValid() const;
    ///
    /// calls navigationdata::SetValid()
    ///
    void SetValid(bool valid);

    ///
    /// calls navigationdata::GetOrientation()
    ///
    mitk::Quaternion GetOrientation() const;

    ///
    /// \return the homogeneous matrix representing this transform
    ///
    vnl_matrix_fixed<mitk::ScalarType, 4, 4> GetMatrix() const;
    ///
    /// \return the homogeneous vtk matrix representing this transform
    ///
    void GetMatrix(vtkMatrix4x4* matrix) const;
    ///
    /// \return the homogeneous vtk matrix representing this transform
    /// in !OpenGL! left handed coordinate system
    ///
    void GetVtkOpenGlMatrix(vtkMatrix4x4* matrix) const;

    mitk::Point3D TransformPoint(mitk::Point3D point) const;

    ///
    /// create xml representation
    ///
    void ToXML(TiXmlElement* elem) const;
    ///
    /// create string representation
    ///
    std::string ToString() const;
    ///
    /// create string csv representation (only the transformation values!!!!)
    ///
    std::string ToCSVString() const;
    ///
    /// create matlab representation
    ///
    std::string ToMatlabString(const std::string& varname="transform",
                               bool printLastRow=true) const;
    ///
    /// write csv representation to file (only the transformation values!!!!)
    ///
    void ToCSVFile(const std::string& file) const;
    ///
    /// write matlab representation to file
    ///
    void ToMatlabFile(const std::string& file
                      , const std::string& varname="transform") const;
    ///
    /// conversion to cv types
    ///
    cv::Mat GetCvTranslation() const;
    cv::Mat GetCvRotationVector() const;
    cv::Mat GetCvRotationMatrix() const;
    cv::Mat GetCvMatrix() const;

    ///
    /// conversion to vnl types
    ///
    vnl_vector_fixed<mitk::ScalarType, 3> GetVnlTranslation() const;
    vnl_vector_fixed<double, 3> GetVnlDoubleTranslation() const;
    vnl_quaternion<double> GetVnlDoubleQuaternion() const;
    vnl_matrix_fixed<mitk::ScalarType, 3, 3> GetVnlRotationMatrix() const;
    vnl_matrix_fixed<double, 4, 4> GetVnlDoubleMatrix() const;

  protected:
    Transform();
    Transform(const mitk::NavigationData* nd);
    Transform(const std::string& s);

    // everything is stored here
    mitk::NavigationData::Pointer m_NavData;

    ///
    /// saves the type of the transform (Default is UNKNOWN_TYPE)
    ///
    std::string m_Type;
};

} // namespace mitk

MitkCameraCalibration_EXPORT std::ostream& operator<<
    (std::ostream& os, mitk::Transform::Pointer p);

#endif // MITKTRANSFORM_H
