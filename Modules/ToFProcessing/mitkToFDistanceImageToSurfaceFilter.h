/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkToFDistanceImageToSurfaceFilter_h
#define mitkToFDistanceImageToSurfaceFilter_h


#include <mitkImage.h>
#include <mitkSurfaceSource.h>
#include <MitkToFProcessingExports.h>
#include <mitkToFProcessingCommon.h>
#include <mitkCameraIntrinsics.h>
#include "mitkCameraIntrinsics.h"
#include <mitkPointSet.h>

#include <vtkSmartPointer.h>
#include <vtkIdList.h>

#include <opencv2/core/types_c.h>

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
  * \image html Modules/ToFProcessing/Documentation/PinholeCameraModel.png
  * The definition of the image plane and its coordinate systems (pixel and mm) is depicted in the following image
  * \image html Modules/ToFProcessing/Documentation/ImagePlane.png
  *
  * @ingroup SurfaceFilters
  * @ingroup ToFProcessing
  */
  class MITKTOFPROCESSING_EXPORT ToFDistanceImageToSurfaceFilter : public SurfaceSource
  {
  public:

    mitkClassMacro( ToFDistanceImageToSurfaceFilter , SurfaceSource );
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);
    itkSetMacro(CameraIntrinsics, mitk::CameraIntrinsics::Pointer);
    itkGetMacro(CameraIntrinsics, mitk::CameraIntrinsics::Pointer);
    itkSetMacro(InterPixelDistance,ToFProcessingCommon::ToFPoint2D);
    itkGetMacro(InterPixelDistance,ToFProcessingCommon::ToFPoint2D);
    itkSetMacro(TextureIndex,int);

    /**
     * @brief SetTriangulationThreshold Sets a triangulation threshold in order
     * to remove unusually huge faces from the surface. If this value is set,
     * the filter will check whether the distance between two neighboring vertices
     * exceeds the triangulation threshold. If yes, there vertices will not be
     * triangulated (connected with lines). The vertices will still be added to
     * the surface, but only as single point (if they have no other neighbors).
     * @param triangulationThreshold The triangulationThreshold in mm. (not mm*mm!)
     * @note vtkMath::Distance2BetweenPoints returns the squared distance
     * between two points and hence we square m_TriangulationThreshold in
     * order to save run-time.
     */
    void SetTriangulationThreshold( double triangulationThreshold );
    itkGetMacro(TriangulationThreshold, double);

    itkSetMacro(VertexIdList, vtkSmartPointer<vtkIdList>);
    itkGetMacro(VertexIdList, vtkSmartPointer<vtkIdList>);

    itkSetMacro(GenerateTriangularMesh,bool);
    itkGetMacro(GenerateTriangularMesh,bool);


    /**
     * @brief The ReconstructionModeType enum: Defines the reconstruction mode, if using no interpixeldistances and focal lenghts in pixel units  or interpixeldistances and focal length in mm. The Kinect option defines a special reconstruction mode for the kinect.
     */
    enum ReconstructionModeType{ WithOutInterPixelDistance = 1, WithInterPixelDistance = 2, Kinect = 3};

    itkSetEnumMacro(ReconstructionMode,ReconstructionModeType);
    itkGetEnumMacro(ReconstructionMode,ReconstructionModeType);
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

    using itk::ProcessObject::SetInput;

    /*!
    \brief Sets the input of this filter
    \param distanceImage input is the distance image of e.g. a ToF camera
    */
    virtual void SetInput( Image* distanceImage);

    /*!
    \brief Sets the input of this filter and the intrinsic parameters
    \param distanceImage input is the distance image of e.g. a ToF camera
    \param cameraIntrinsics
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
    ~ToFDistanceImageToSurfaceFilter() override;
    void GenerateOutputInformation() override;
    /*!
    \brief Method generating the output of this filter. Called in the updated process of the pipeline.
    This method generates the output of the ToFSurfaceSource: The generated surface of the 3d points
    */
    void GenerateData() override;
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

    int m_TextureImageWidth; ///< Width (x-dimension) of the texture image
    int m_TextureImageHeight; ///< Height (y-dimension) of the texture image
    ToFProcessingCommon::ToFPoint2D m_InterPixelDistance; ///< distance in mm between two adjacent pixels on the ToF camera chip

    int m_TextureIndex; ///< Index of the input used as texture image when no scalar image was set via SetIplScalarImage(). 0 = Distance, 1 = Amplitude, 2 = Intensity
    bool m_GenerateTriangularMesh;

    ReconstructionModeType m_ReconstructionMode; ///< The ReconstructionModeType enum: Defines the reconstruction mode, if using no interpixeldistances and focal lenghts in pixel units  or interpixeldistances and focal length in mm. The Kinect option defines a special reconstruction mode for the kinect.

    vtkSmartPointer<vtkIdList> m_VertexIdList; ///< Make a vtkIdList to save the ID's of the polyData corresponding to the image pixel ID's. This can be accessed after generate data to obtain the mapping.

    double m_TriangulationThreshold;

  };
} //END mitk namespace
#endif
