/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2009-07-17 15:44:24 +0200 (Fr, 17 Jul 2009) $
Version:   $Revision: 18261 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef MITKPINHOLECAMERAMODEL_H_HEADER_INCLUDED_
#define MITKPINHOLECAMERAMODEL_H_HEADER_INCLUDED_

//mitk headers
#include "mitkToFProcessingExports.h"
#include "mitkToFProcessingCommon.h"
#include <mitkCommon.h>
#include <mitkVector.h>


//itk headers
#include <itkObject.h>
#include <itkDataObject.h>
#include <itkObjectFactory.h>
#include <itkMatrix.h>
#include <itkVector.h>

namespace mitk {
  //##Documentation
  //## \brief An object of this class represents an ideal pinhole camera. 
  
  class mitkToFProcessing_EXPORT PinholeCameraModel : public itk::DataObject
  {
  public:
    typedef mitk::ToFProcessingCommon::ToFScalarType ToFScalarType;
    typedef mitk::ToFProcessingCommon::ToFPoint2D ToFPoint2D;
    typedef mitk::ToFProcessingCommon::ToFPoint3D ToFPoint3D;

    mitkClassMacro(PinholeCameraModel, itk::DataObject);
    itkNewMacro(Self);

    //getter
    itkGetMacro(FocalLength,ToFScalarType);
    /** @return Returns the image area in mm x mm */
    itkGetMacro(ImageArea,ToFPoint2D);
    /** @return Returns the size of one pixel in mm x mm */
    itkGetMacro(InterPixelDistance,ToFPoint2D);
    /** @return Returns the image shift in relation to the pinhole in mm x mm */
    itkGetMacro(ImageShift,ToFPoint2D);
    /** @return Returns the maximum view distance of the camera. (in mm) */
    itkGetMacro(MaxDistance,ToFScalarType);
    void GetCameraToWorldTransform(itk::Matrix<ToFScalarType,3,3>& TransformR,itk::Vector<ToFScalarType,3>& TransformT);
    /** @return Returns the point of pixel (x|y) on the image area in camera coordinates (mm). */
    ToFPoint3D GetPixelMidPointInWorldCoordinates(unsigned int indexX, unsigned int indexY);
    /** @return Returns the pixel position (x|y) principal point of this camera. */
    itkGetMacro(PrincipalPoint,ToFPoint2D);
    itkGetMacro(DistanceImageDimension,unsigned int);
    /** @brief Sets the image dimension */
    itkGetMacro(DistanceImageDimensions,unsigned int*);
    /** @brief Sets the image dimensions */
        
    //setter
    /** @brief Sets the focal distance in pixels */
    itkSetMacro(FocalLength,ToFScalarType);
    /** @brief Sets the size of one pixel in mm x mm */
    itkSetMacro(InterPixelDistance,ToFPoint2D);
    /** @brief Sets the image shift in relation to the pinhole in mm x mm */
    itkSetMacro(ImageShift,ToFPoint2D);
    /** @brief Sets the maximum view distance of the camera. (in mm) */
    itkSetMacro(MaxDistance,ToFScalarType);
    /** @return Returns the camera positon in world coordinates */
    ToFPoint3D GetCameraPositionInWorldCoordinates();
    itkSetMacro(PrincipalPoint,ToFPoint2D);

    itkSetMacro(DistanceImageDimension,unsigned int);
    itkSetMacro(ImageArea,ToFPoint2D);

    void SetDistanceImageDimensions(unsigned int* dimensions);

    void SetCameraToWorldTransform(itk::Matrix<ToFScalarType,3,3> TransformR,itk::Vector<ToFScalarType,3> TransformT);

    /** @brief Initializes the camera model with default settings which are needed often.
     *         (focal length: 12.8 mm; pixel size: 0.045 x 0.045; image area: 9.18 x 9.18; max distance: 2000)
     */
    void InitializeWithDefaultSettings();
    /*!
    \brief calculate image area in mm from m_DistanceImageDimensions and m_InterPixelDistance
    */
    void CalculateImageArea();
    /*!
    \brief calculate m_DistanceImageDimensions from m_ImageArea and m_InterPixelDistance
    */
    void CalculateImageDimensions();

  protected:
    PinholeCameraModel();
    ~PinholeCameraModel();

    //########### member variables ####################
    ToFScalarType m_FocalLength;       ///< focal length in mm
    ToFScalarType m_MaxDistance; ///< range of the virtual camera
    unsigned int m_DistanceImageDimension; ///< dimension of the distance image
    unsigned int* m_DistanceImageDimensions; ///< x-, y-, z- (optional) dimension
    ToFPoint2D  m_PrincipalPoint; ///< principal point in pixel coordinates
    ToFPoint2D  m_ImageArea;   ///< image area (mm x mm)
    ToFPoint2D  m_InterPixelDistance;   ///< size of one pixel (mm x mm)
    ToFPoint2D  m_ImageShift;  ///< image shift of pinhole in relation to the image center
    itk::Matrix<ToFScalarType,3,3> m_CameraToWorldTransform_R; ///< rotation matrix of camera to world transform
    itk::Vector<ToFScalarType,3> m_CameraToWorldTransform_T; ///< translation vector of camera to world transform
  };

} // namespace mitk
#endif /* MITKPINHOLECAMERAMODEL_H_HEADER_INCLUDED_ */
