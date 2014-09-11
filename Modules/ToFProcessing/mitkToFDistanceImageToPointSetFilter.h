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
#ifndef __mitkToFDistanceImageToPointSetFilter_h
#define __mitkToFDistanceImageToPointSetFilter_h


#include <mitkCameraIntrinsics.h>
#include "mitkImage.h"
#include "mitkPointSet.h"
#include <mitkPointSetSource.h>
#include "mitkImageSource.h"
#include <mitkToFProcessingCommon.h>
#include <MitkToFProcessingExports.h>

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
  * The coordinate conversion follows the model of a common pinhole camera where the origin of the camera
  * coordinate system (world coordinates) is at the pinhole
  * \image html ../Modules/ToFProcessing/Documentation/PinholeCameraModel.png
  * The definition of the image plane and its coordinate systems (pixel and mm) is depicted in the following image
  * \image html ../Modules/ToFProcessing/Documentation/ImagePlane.png
  *
  * @ingroup SurfaceFilters
  * @ingroup ToFProcessing
  */
  class MitkToFProcessing_EXPORT ToFDistanceImageToPointSetFilter : public PointSetSource
  {
  public:

    mitkClassMacro( ToFDistanceImageToPointSetFilter , PointSetSource );
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    itkSetMacro(CameraIntrinsics,mitk::CameraIntrinsics::Pointer);

    itkGetMacro(CameraIntrinsics,mitk::CameraIntrinsics::Pointer);
    itkSetMacro(InterPixelDistance,mitk::ToFProcessingCommon::ToFPoint2D);
    itkGetMacro(InterPixelDistance,mitk::ToFProcessingCommon::ToFPoint2D);


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
    void SetSubset( std::vector<itk::Index<3> > subset);

    /*!
    \brief Sets the subset of indizes used for caluclation of output PointSet as a PointSet.
    Warning: make sure the points in your PointSet are index coordinates.
    \param PointSet specified in index coordinates.
    */
    void SetSubset( mitk::PointSet::Pointer pointSet);

    /*!
    \brief Sets the reconstruction mode, if using no interpixeldistances and focal lenghts in pixel units (=true) or interpixeldistances and focal length in mm (=false)
    */
    void SetReconstructionMode(bool withoutInterpixdist = true);

    /*!
    \brief Returns the reconstruction mode
    */
    bool GetReconstructionMode();


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

    std::vector<itk::Index<3> > m_Subset; ///< If this subset is specified only the contained indizes are converted to cartesian coordinates
    mitk::CameraIntrinsics::Pointer m_CameraIntrinsics; ///< Member holding the intrinsic parameters needed for PointSet calculation
    ToFProcessingCommon::ToFPoint2D m_InterPixelDistance; ///< distance in mm between two adjacent pixels on the ToF camera chip
    bool m_ReconstructionMode; ///< true = Reconstruction without interpixeldistance and with focal lengths in pixel units. false = Reconstruction with interpixeldistance and with focal length in mm.
  };
} //END mitk namespace
#endif
