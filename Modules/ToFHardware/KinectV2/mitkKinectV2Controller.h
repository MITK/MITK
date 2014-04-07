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
#ifndef __mitkKinectV2Controller_h
#define __mitkKinectV2Controller_h

#include <MitkKinectV2Exports.h>
#include <mitkCommon.h>
#include "mitkToFConfig.h"

#include <itkObject.h>
#include <itkObjectFactory.h>

#include <vtkSmartPointer.h>
#include <vtkPolyData.h>

namespace mitk
{
  /**
  * @brief Interface to the Kinect 2 camera. Currently, the Microsoft SDK is used.
  *
  * @ingroup ToFHardware
  */
  class MITK_KINECTV2MODULE_EXPORT KinectV2Controller : public itk::Object
  {
  public:

    mitkClassMacro( KinectV2Controller , itk::Object );

    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)
    int GetRGBCaptureWidth() const;
    int GetRGBCaptureHeight() const;
    int GetDepthCaptureWidth() const;
    int GetDepthCaptureHeight() const;

    /**
    \brief Setup MultiFrameReader of Kinect V2.
    * This reader can acquire different types of data. Here it is used
    * to acquire depth, RGB and infrared images.
    */
    bool InitializeMultiFrameReader();

    /*!
    \brief opens a connection to the Kinect V2 camera.
    \throws mitkException In case of no connection, an exception is thrown!
    */
    virtual bool OpenCameraConnection();
    /*!
    \brief closes the connection to the camera
    */
    virtual bool CloseCameraConnection();
    /*!
    \brief updates the camera. The update function of the hardware interface is called only when new data is available
    \throws mitkException In case of no connection, an exception is thrown!
    */
    virtual bool UpdateCamera();
    /*!
    \brief acquire new distance data from the Kinect camera
    \param distances pointer to memory location where distances should be stored
    */
    void GetDistances(float* distances);
    void GetAmplitudes(float* amplitudes);
    void GetIntensities(float* intensities);

    vtkSmartPointer<vtkPolyData> GetVtkPolyData();

    void SetGenerateTriangularMesh(bool flag);
    void SetTriangulationThreshold(double triangulationThreshold);

    /*!
    \brief acquire new rgb data from the Kinect camera
    \param rgb pointer to memory location where rgb information should be stored
    */
    void GetRgb(unsigned char* rgb);
    /*!
    \brief convenience method for faster access to distance and rgb data
    \param distances pointer to memory location where distances should be stored
    \param rgb pointer to memory location where rgb information should be stored
    */
    void GetAllData(float* distances, float* amplitudes, unsigned char* rgb);

  protected:

    KinectV2Controller();

    ~KinectV2Controller();

  private:
    class KinectV2ControllerPrivate;
    KinectV2ControllerPrivate *d;

  };
} //END mitk namespace
#endif
