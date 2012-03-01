/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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
#ifndef __mitkToFDistanceImageToSurfaceFilter_h
#define __mitkToFDistanceImageToSurfaceFilter_h


#include <mitkImage.h>
#include <mitkSurfaceSource.h>
#include <mitkToFProcessingExports.h>
#include <mitkToFProcessingCommon.h>
#include <mitkCameraIntrinsics.h>
#include "mitkCameraIntrinsics.h"
#include <mitkPointSet.h>
#include <cv.h>

namespace mitk
{
  /**
  * @brief Converts a Time-of-Flight (ToF) distance image to a 3D surface using the pinhole camera model for coordinate computation.
  * The intrinsic parameters of the camera (FocalLength, PrincipalPoint, InterPixelDistance) are set via SetCameraIntrinsics(). The
  * measured distance for each pixel corresponds to the distance between the object point and the corresponding image point on the
  * image plane.
  *
  * The coordinate conversion follows the model of a common pinhole camera where the origin of the camera 
  * coordinate system (world coordinates) is at the pinhole
  * \image html ../Modules/ToFProcessing/Documentation/PinholeCameraModel.png
  * The definition of the image plane and its coordinate systems (pixel and mm) is depicted in the following image
  * \image html ../Modules/ToFProcessing/Documentation/ImagePlane.png
  *
  * @ingroup SurfaceFilters
  * @ingroup ToFProcessing
  */
  class mitkToFProcessing_EXPORT ToFDistanceImageToSurfaceFilter : public SurfaceSource
  {
  public:

    mitkClassMacro( ToFDistanceImageToSurfaceFilter , SurfaceSource );
    itkNewMacro( Self );


    itkSetMacro(CameraIntrinsics, mitk::CameraIntrinsics::Pointer);
    itkGetMacro(CameraIntrinsics, mitk::CameraIntrinsics::Pointer);
    itkSetMacro(InterPixelDistance,ToFProcessingCommon::ToFPoint2D);
    itkGetMacro(InterPixelDistance,ToFProcessingCommon::ToFPoint2D);
    itkSetMacro(TextureIndex,int);
    /*!
    \brief Set scalar image used as texture of the surface.
    \param iplScalarImage OpenCV image for texturing
    */
    void SetScalarImage(IplImage* iplScalarImage);
    /*!
    \brief Set scalar image used as texture of the surface.
    \return OpenCV image for texturing
    */
    IplImage* GetScalarImage();
    /*!
    \brief Set width of the scalar image used for texturing the surface
    \param width width (x-dimension) of the texture image
    */
    void SetTextureImageWidth(int width);
    /*!
    \brief Set height of the scalar image used for texturing the surface
    \param height height (y-dimension) of the texture image
    */
    void SetTextureImageHeight(int height);
    /*!
    \brief Sets the input of this filter
    \param distanceImage input is the distance image of e.g. a ToF camera
    */
    virtual void SetInput( Image* distanceImage);

    /*!
    \brief Sets the input of this filter and the intrinsic parameters
    \param distanceImage input is the distance image of e.g. a ToF camera
    */
    virtual void SetInput( Image* distanceImage, mitk::CameraIntrinsics::Pointer cameraIntrinsics );

    /*!
    \brief Sets the input of this filter at idx
    \param idx number of the current input
    \param distanceImage input is the distance image of e.g. a ToF camera
    */
    virtual void SetInput(unsigned int idx,  Image* distanceImage);

    /*!
    \brief Sets the input of this filter at idx and the intrinsic parameters
    \param idx number of the current input
    \param distanceImage input is the distance image of e.g. a ToF camera
    \param cameraIntrinsics This is the camera model which holds parameters like focal length, pixel size, etc. which are needed for the reconstruction of the surface.
    */
    virtual void SetInput( unsigned int idx,  Image* distanceImage, mitk::CameraIntrinsics::Pointer cameraIntrinsics );

    /*!
    \brief Returns the input of this filter
    */
     Image* GetInput();

    /*!
    \brief Returns the input with id idx of this filter
    */
     Image* GetInput(unsigned int idx);

  protected:
    /*!
    \brief Standard constructor
    */
    ToFDistanceImageToSurfaceFilter();
    /*!
    \brief Standard destructor
    */
    ~ToFDistanceImageToSurfaceFilter();
    virtual void GenerateOutputInformation();
    /*!
    \brief Method generating the output of this filter. Called in the updated process of the pipeline.
    This method generates the output of the ToFSurfaceSource: The generated surface of the 3d points
    */
    virtual void GenerateData();
    /**
    * \brief Create an output for each input
    *
    * This Method sets the number of outputs to the number of inputs
    * and creates missing outputs objects.
    * \warning any additional outputs that exist before the method is called are deleted
    */
    void CreateOutputsForAllInputs();

    IplImage* m_IplScalarImage; ///< Scalar image used for surface texturing

    mitk::CameraIntrinsics::Pointer m_CameraIntrinsics; ///< Specifies the intrinsic parameters
    //mitk::CameraIntrinsics::Pointer m_CameraModel; ///< Specifies the intrinsic parameters

    int m_TextureImageWidth; ///< Width (x-dimension) of the texture image
    int m_TextureImageHeight; ///< Height (y-dimension) of the texture image
    ToFProcessingCommon::ToFPoint2D m_InterPixelDistance; ///< distance in mm between two adjacent pixels on the ToF camera chip

    int m_TextureIndex; ///< Index of the input used as texture image when no scalar image was set via SetIplScalarImage(). 0 = Distance, 1 = Amplitude, 2 = Intensity

  };
} //END mitk namespace
#endif
