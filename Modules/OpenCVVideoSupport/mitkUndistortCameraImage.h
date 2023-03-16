/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkUndistortCameraImage_h
#define mitkUndistortCameraImage_h

#include "mitkCommon.h"
#include <MitkOpenCVVideoSupportExports.h>
#include "itkObject.h"

#include "mitkPoint.h"

#include <opencv2/core.hpp>
#include <opencv2/core/types_c.h>

/*!
\brief UndistortCameraImage

This class is used to undistort camera images. Before any undistortion the class has to be initialized using the functions:
SetFocalLength(),SetPrinzipalPoint() and SetCameraDistortion().
After this you can either use UndistortPixel() to undistort a single pixel's coordinates or UndistortImage() to undistort an
OpenCV image.

A faster version of UndistortImage() is UndistortImageFast(), however, it has to be initialized once with SetUndistortImageFastInfo()
instead of the Set... methods before use.

\ingroup Functionalities
*/

namespace mitk
{

class MITKOPENCVVIDEOSUPPORT_EXPORT UndistortCameraImage : public itk::Object
{
  public:

    mitkClassMacroItkParent(UndistortCameraImage,itk::Object);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /// Initialization ///
    /*
    * Set the camera's intrinsic focal length
    */
    void SetFocalLength(float fc_x, float fc_y)
    {
      m_fcX = fc_x; m_fcY = fc_y;
    }
    /*
    * Set the camera's intrinsic principal point
    */
    void SetPrincipalPoint(float cc_x, float cc_y)
    {
      m_ccX = cc_x; m_ccY = cc_y;
    }
    /*
    * Set the camera's intrinsic distortion parameters
    */
    void SetCameraDistortion(float kc1, float kc2, float kc3, float kc4)
    {
      m_distortionMatrixData[0] = kc1; m_distortionMatrixData[1] = kc2;
      m_distortionMatrixData[2] = kc3; m_distortionMatrixData[3] = kc4;
    }
    /*
    * Pre-Calculates matrices for the later use of UndistortImageFast()
    */
    void InitRemapUndistortion(int sizeX, int sizeY);

    /// USAGE ///
    /*
    * Undistort a single pixel, returns undistorted pixel
    */
    mitk::Point2D UndistortPixel(const mitk::Point2D& src);
    /*
    * Complete undistortion of an OpenCV image, including all calculations
    */
    void UndistortImage(IplImage* src, IplImage* dst);


    /*
    * Complete undistortion of an OpenCV image, using pre-calculated matrices from SetUndistortImageFastInfo()
    * The use of only a source parameter will cause the source to be overwritten.
    * NOTE: Using the Fast undistortion methods does not require a initialization via the Set... methods.
    */
    void UndistortImageFast( IplImage * src, IplImage* dst = nullptr );
    void SetUndistortImageFastInfo(float in_dF1, float in_dF2,
                                   float in_dPrincipalX, float in_dPrincipalY,
                                   float in_Dist[4], float ImageSizeX, float ImageSizeY);

    UndistortCameraImage();
    ~UndistortCameraImage() override;

  protected:

    // principal point and focal length parameters
    float m_ccX, m_ccY, m_fcX, m_fcY;
    // undistortion parameters
    float m_distortionMatrixData[4];
    // intrinsic camera parameters
    float m_intrinsicMatrixData[9];
    // precalculated matrices for fast image undistortion with UndistortImageFast()
    CvMat * m_mapX, * m_mapY;
    // intrinsic and undistortion camera matrices
    CvMat m_intrinsicMatrix, m_distortionMatrix;
    // temp image
    IplImage * m_tempImage;



    CvMat *m_DistortionCoeffs;
    CvMat *m_CameraMatrix;
};

}

#endif
