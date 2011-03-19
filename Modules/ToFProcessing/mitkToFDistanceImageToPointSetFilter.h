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
#ifndef __mitkToFDistanceImageToPointSetFilter_h
#define __mitkToFDistanceImageToPointSetFilter_h


#include "mitkImage.h"
#include "mitkPointSet.h"
#include <mitkPointSetSource.h> 
#include "mitkImageSource.h"
#include <mitkPinholeCameraModel.h>
#include "mitkToFProcessingExports.h"

namespace mitk
{
  /**
  * @brief Converts a Time-of-Flight (ToF) distance image to a PointSet using the pinhole camera model for coordinate computation.
  * The intrinsic parameters of the camera (FocalLength, PrincipalPoint, InterPixelDistance) are set via SetIntrinsicParameters(). The
  * measured distance for each pixel corresponds to the distance between the object point and the corresponding image point on the
  * image plane.
  * If a subset of indizes of the image is defined via SetSubset(), the output PointSet will only contain the cartesian coordinates
  * of the corresponding 3D points.
  *
  * <img src="PinholeCameraModel.png" alt="Pinhole camera model"> 
  * <img src="ImagePlane.png" alt="Image plane"> 
  *
  * @ingroup SurfaceFilters
  * @ingroup ToFProcessing
  */
  class mitkToFProcessing_EXPORT ToFDistanceImageToPointSetFilter : public PointSetSource
  {
  public:
    typedef mitk::PinholeCameraModel::ToFScalarType ToFScalarType;

    mitkClassMacro( ToFDistanceImageToPointSetFilter , PointSetSource );
    itkNewMacro( Self );

    itkSetMacro(CameraModel,mitk::PinholeCameraModel::Pointer);

    itkGetMacro(CameraModel,mitk::PinholeCameraModel::Pointer);


    /*!
    \brief Sets the input of this filter
    \param distanceImage input is the distance image of e.g. a ToF camera
    */
    virtual void SetInput(const Image* distanceImage);

    /*!
    \brief Sets the input of this filter at idx
    \param idx number of the current input
    \param distanceImage input is the distance image of e.g. a ToF camera
    */
    virtual void SetInput(unsigned int idx,const Image* distanceImage);

    /*!
    \brief Returns the input of this filter
    */
     Image* GetInput();
   
    /*!
    \brief Returns the input with id idx of this filter
    */
     Image* GetInput(unsigned int idx);

    /*!
    \brief If this subset is defined, the cartesian coordinates are only computed for the contained indizes. Make sure the indizes are contained in the input image
    \param subset index subset specified in index coordinates.
    */
    void SetSubset( std::vector<Index3D> subset);

    /*!
    \brief Sets the subset of indizes used for caluclation of output PointSet as a PointSet.
    Warning: make sure the points in your PointSet are index coordinates.
    \param PointSet specified in index coordinates.
    */
    void SetSubset( mitk::PointSet::Pointer pointSet);

  protected:
    /*!
    \brief Standard constructor
    */
    ToFDistanceImageToPointSetFilter();
    /*!
    \brief Standard destructor
    */
    ~ToFDistanceImageToPointSetFilter();
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

    std::vector<Index3D> m_Subset; ///< If this subset is specified only the contained indizes are converted to cartesian coordinates
    mitk::PinholeCameraModel::Pointer m_CameraModel; ///< Pinhole camera model holding the intrinsic parameters needed for PointSet calculation
  };
} //END mitk namespace
#endif
