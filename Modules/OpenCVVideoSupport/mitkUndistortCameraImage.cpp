/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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


///      The list of internal camera parameters:
///    * Focal length: The focal length in pixels is stored in m_fcX and m_fcY.
///    * Principal point: The principal point coordinates are stored in the m_ccX and m_ccY.
///    * Skew coefficient: The skew coefficient defining the angle between the x and y pixel axes is stored in the scalar alpha_c = 0.
///    * Distortions: The image distortion coefficients (radial and tangential distortions) are stored in the 4x1 vector.


#include "mitkUndistortCameraImage.h"
#include "itkTimeProbe.h"

#include "highgui.h"

mitk::UndistortCameraImage::UndistortCameraImage()
{
  m_tempImage = NULL;
  
}
mitk::UndistortCameraImage::~UndistortCameraImage()
{
  if(m_tempImage != NULL)
    cvReleaseImage( &m_tempImage );
}


/// undistort one pixel coordinate using floating point accuracy...
mitk::Point2D mitk::UndistortCameraImage::UndistortPixel(mitk::Point2D src)
{
  float k1, k2, p1, p2;        // distortion values
  float k_radial;
  float x, y;
  float delta_x, delta_y;
  float r_2;                   // radial distance squared
  float distx, disty;          // distored xy
  mitk::Point2D desPnt;
  mitk::Point2D dstd, dst;
  mitk::Point2D old_src = src;  // copy of the original distorted point

  // distortion coefficients
  k1 = m_distortionMatrixData[0];
  k2 = m_distortionMatrixData[1];
  p1 = m_distortionMatrixData[2];
  p2 = m_distortionMatrixData[3];

  // Shift points to principal point and use focal length
  dstd[0] = (src[0] - m_ccX) / m_fcX;
  dstd[1] = (src[1] - m_ccY) / m_fcY;
  desPnt[0] = dstd[0];
  desPnt[1] = dstd[1];

  // Compensate lens distortion
  x = dstd[0];
  y = dstd[1];
  
  for (int iter = 0; iter < 5; iter++)
  {
    r_2 = x*x + y*y;
    k_radial = 1 + k1 * r_2 + k2 * r_2 * r_2;
    delta_x = 2 * p1*x*y + p2 * (r_2 + 2*x*x);
    delta_y = 2 * p2*x*y + p1 * (r_2 + 2*y*y);
    x = (desPnt[0] - delta_x) / k_radial;
    y = (desPnt[1] - delta_y) / k_radial;
  }
  dstd[0] = x;
  dstd[1] = y;
  dstd[0] *= m_fcX;
  dstd[1] *= m_fcY;
  dstd[0] += m_ccX;
  dstd[1] += m_ccY;
  
  // ready
  dst[0] = (float)dstd[0];
  dst[1] = (float)dstd[1];
 
  // do a sanity check to make sure this ideal point translates properly to the distorted point
  // this does the reverse of the above.  It maps ideal undistorted to distorted image coordinates
  x = dstd[0] - m_ccX;
  y = dstd[1] - m_ccY;
  x /= m_fcX;
  y /= m_fcY;
  r_2 = x*x + y*y;
  distx = x + x*(k1*r_2 + k2*r_2*r_2) + (2*p1*x*y + p2*(r_2 + 2*x*x));
  disty = y + y*(k1*r_2 + k2*r_2*r_2) + (2*p2*x*y + p1*(r_2 + 2*y*y));
  distx *= m_fcX;
  disty *= m_fcY;
  distx += m_ccX;
  disty += m_ccY;

  // this should never be more than .2 pixels...
  float diffx = old_src[0] - distx;
  float diffy = old_src[1] - disty;
  if (fabs(diffx) > .1 || fabs(diffy) > .1)
  {
    std::cout << "undistort sanity check error: diffx =" << diffx << " , diffy = " << diffy;
    
  }
  return dst;
}

void mitk::UndistortCameraImage::UndistortImage(IplImage *src, IplImage *dst)
{
   // init intrinsic camera matrix [fx 0 cx; 0 fy cy; 0 0 1].
  m_intrinsicMatrixData[0] = (double)m_fcX;
	m_intrinsicMatrixData[1] = 0.0;
	m_intrinsicMatrixData[2] = (double)m_ccX;
	m_intrinsicMatrixData[3] = 0.0;
	m_intrinsicMatrixData[4] = (double)m_fcY;
	m_intrinsicMatrixData[5] = (double)m_ccY;
	m_intrinsicMatrixData[6] = 0.0;
	m_intrinsicMatrixData[7] = 0.0;
	m_intrinsicMatrixData[8] = 1.0;
  m_intrinsicMatrix        = cvMat(3, 3, CV_32FC1, m_intrinsicMatrixData);
  
  // init distortion matrix
  m_distortionMatrix       = cvMat(1, 4, CV_32F, m_distortionMatrixData);

  // undistort
  cvUndistort2(src,dst, &m_intrinsicMatrix, &m_distortionMatrix);
}


// FAST METHODS FOR UNDISTORTION IN REALTIME //

void mitk::UndistortCameraImage::UndistortImageFast(IplImage * src, IplImage* dst)
{
  /*if(dst == NULL)
    dst = src;

  if(src->nChannels == 3)
  {
    IplImage *r = cvCreateImage(cvGetSize(src),src->depth,1);//subpixel
    IplImage *g = cvCreateImage(cvGetSize(src),src->depth,1);//subpixel
    IplImage *b = cvCreateImage(cvGetSize(src),src->depth,1);//subpixel

    cvSplit(src, r,g,b, NULL);

    cvRemap( r, r, m_mapX, m_mapY ); // Undistort image
    cvRemap( g, g, m_mapX, m_mapY ); // Undistort image
    cvRemap( b, b, m_mapX, m_mapY ); // Undistort image

    cvMerge(r,g,b, NULL, dst);
  }
  else
  {
    cvRemap(src, dst, m_mapX, m_mapY);
  }*/

  
  if(m_tempImage == NULL)
    m_tempImage = cvCreateImage(cvSize(src->width,src->height),src->depth,src->nChannels);
  
  cvRemap(src, m_tempImage, m_mapX, m_mapY);
  
  m_tempImage->origin = src->origin;
 
  if(dst == NULL)
    memcpy( src->imageData, m_tempImage->imageData, m_tempImage->imageSize );
  else
    memcpy( dst->imageData, m_tempImage->imageData, m_tempImage->imageSize );

  //cvUnDistort(m_srcImg, m_dstImg, m_undistMap,m_interpolationMode);
	//cvUndistort2(m_srcImg, m_dstImg, &m_intrinsicMatrix,&m_distortionMatrixDataCoefficients);
}



void mitk::UndistortCameraImage::SetUndistortImageFastInfo(float in_dF1, float in_dF2, 
                                                 float in_dPrincipalX, float in_dPrincipalY, 
                                                 float in_Dist[4], float ImageSizeX, float ImageSizeY)
{
  //create new matrix
  m_DistortionCoeffs  = cvCreateMat(4, 1, CV_64FC1);
  m_CameraMatrix      = cvCreateMat(3, 3, CV_64FC1);
  

  //set the camera matrix [fx 0 cx; 0 fy cy; 0 0 1].
  cvSetReal2D(m_CameraMatrix, 0, 0, in_dF1);
  cvSetReal2D(m_CameraMatrix, 0, 1, 0.0);
  cvSetReal2D(m_CameraMatrix, 0, 2, in_dPrincipalX);

  cvSetReal2D(m_CameraMatrix, 1, 0, 0.0);
  cvSetReal2D(m_CameraMatrix, 1, 1, in_dF2);
  cvSetReal2D(m_CameraMatrix, 1, 2, in_dPrincipalY);
 
  cvSetReal2D(m_CameraMatrix, 2, 0, 0.0);
  cvSetReal2D(m_CameraMatrix, 2, 1, 0.0);
  cvSetReal2D(m_CameraMatrix, 2, 2, 1.0); 
  
  //set distortions coefficients
  cvSetReal1D(m_DistortionCoeffs, 0, in_Dist[0]);
  cvSetReal1D(m_DistortionCoeffs, 1, in_Dist[1]);
  cvSetReal1D(m_DistortionCoeffs, 2, in_Dist[2]);
  cvSetReal1D(m_DistortionCoeffs, 3, in_Dist[3]);

  m_mapX = cvCreateMat(ImageSizeY, ImageSizeX, CV_32FC1);
	m_mapY = cvCreateMat(ImageSizeY, ImageSizeX, CV_32FC1);

  //cv::initUndistortRectifyMap(m_CameraMatrix, m_DistortionCoeffs, m_mapX, m_mapY);
  cvInitUndistortMap(m_CameraMatrix, m_DistortionCoeffs, m_mapX, m_mapY);
}




