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


#ifndef mitkCameraIntrinsics_h
#define mitkCameraIntrinsics_h

#include <mitkCommon.h>
#include <mitkNumericTypes.h>
#include <itkDataObject.h>
#include <itkFastMutexLock.h>
#include <cv.h>
#include <vnl/vnl_matrix_fixed.h>
#include "mitkXMLSerializable.h"
#include <MitkCameraCalibrationExports.h>

int mitkCameraIntrinsicsTest(int argc, char* argv[]);

namespace mitk
{
  ///
  /// \brief class representing camera intrinsics and related functions
  ///
  class MitkCameraCalibration_EXPORT CameraIntrinsics: virtual public itk::Object,
    virtual public mitk::XMLSerializable
  {
  public:
    ///
    /// for testing purposes
    ///
    friend int mitkCameraIntrinsicsTest(int argc, char* argv[]);
    ///
    /// smartpointer typedefs
    ///
    mitkClassMacro(CameraIntrinsics, itk::Object);
    ///
    /// the static new function
    ///
    itkFactorylessNewMacro(Self);

    ///
    /// make a clone of this intrinsics
    ///
    itkCloneMacro(Self)

    ///
    /// copy information from other to this
    ///
    void Copy(const CameraIntrinsics* other);

    ///
    /// checks two intrinsics for equality
    ///
    bool Equals( const CameraIntrinsics* other ) const;

    ///
    /// \return the intrinsic parameter matrix as a 3x3 vnl matrix
    ///
    vnl_matrix_fixed<mitk::ScalarType, 3, 3> GetVnlCameraMatrix() const;

    ///
    /// \return the intrinsic parameter matrix as a 3x4 vnl matrix
    /// (the last column only containing zeros)
    ///
    vnl_matrix_fixed<mitk::ScalarType, 3, 4> GetVnlCameraMatrix3x4() const;

    ///
    /// \return true if the intrinsics are set (some plausibility checks
    /// may be done here)
    ///
    bool IsValid() const;
    void SetValid(bool valid);
    cv::Mat GetCameraMatrix() const;
    cv::Mat GetDistorsionCoeffs();
    cv::Mat GetDistorsionCoeffs() const;
    void ToXML(TiXmlElement* elem) const;
    std::string ToString() const;
    std::string GetString();
    double GetFocalLengthX() const;
    double GetFocalLengthY() const;
    double GetPrincipalPointX() const;
    double GetPrincipalPointY() const;
    mitk::Point4D GetDistorsionCoeffsAsPoint4D() const;
    mitk::Point3D GetFocalPoint() const;
    mitk::Point3D GetPrincipalPoint() const;
    vnl_vector_fixed<mitk::ScalarType, 2> GetFocalPointAsVnlVector() const;
    vnl_vector_fixed<mitk::ScalarType, 2> GetPrincipalPointAsVnlVector() const;

    ///
    /// set a new camera matrix utilizing a vnl matrix
    ///
    void SetCameraMatrix( const vnl_matrix_fixed<mitk::ScalarType, 3, 3>&
                        _CameraMatrix );

    void SetIntrinsics( const cv::Mat& _CameraMatrix
                        , const cv::Mat& _DistorsionCoeffs);

    void SetFocalLength( double x, double y );
    void SetPrincipalPoint( double x, double y );
    void SetDistorsionCoeffs( double k1, double k2, double p1, double p2 );

    void SetIntrinsics( const mitk::Point3D& focalPoint,
                        const mitk::Point3D& principalPoint,
                        const mitk::Point4D& distortionCoefficients);

    void FromXML(TiXmlElement* elem);
    void FromGMLCalibrationXML(TiXmlElement* elem);
    std::string ToOctaveString(const std::string& varName="CameraIntrinsics");

    virtual ~CameraIntrinsics();

  protected:
    CameraIntrinsics();
    CameraIntrinsics(const CameraIntrinsics& other);

    cv::Mat m_CameraMatrix;
    cv::Mat m_DistorsionCoeffs;
    bool m_Valid;
    itk::FastMutexLock::Pointer m_Mutex;

  private:

    virtual itk::LightObject::Pointer InternalClone() const;
  };

} // namespace mitk

MitkCameraCalibration_EXPORT std::ostream& operator<<
    (std::ostream& os, mitk::CameraIntrinsics::Pointer p);

#endif // mitkCameraIntrinsics_h
