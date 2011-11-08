/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2010-03-31 16:40:27 +0200 (Mi, 31 Mrz 2010) $
Version:   $Revision: 21975 $ 
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef QmitkToFUtilView_h
#define QmitkToFUtilView_h

#include <QmitkFunctionality.h>
#include <QmitkOverlayController.h>

#include <ui_QmitkToFUtilViewControls.h>

#include <QTimer>

#include <mitkRealTimeClock.h>
#include <mitkToFImageGrabber.h>
#include <mitkOpenCVVideoSource.h>
#include <mitkSurface.h>
#include <mitkToFDistanceImageToSurfaceFilter.h>
#include <mitkToFSurfaceVtkMapper3D.h>
#include <mitkToFImageRecorder.h>
#include <mitkToFCompositeFilter.h>

/*!
  \brief QmitkToFUtilView 

  Application that allows simple playing, recording, visualization, processing and measurement of Time-of-Flight (ToF) data.
  Currently the following features are implemented:
  <ul>
  <li>Connecting and showing ToF data from various cameras (PMD CamCube 2/3, PMD CamBoard, PMD O3, MESA SwissRanger)</li>
  <li>Recording and playing of ToF data</li>
  <li>Color coded visualization of ToF images</li>
  <li>Preprocessing of the distance data: Threshold, median, average and bilateral filtering; surface generation</li>
  <li>Simple measurement and PointSet definition</li>
  </ul>

  \sa QmitkFunctionality
  \ingroup Functionalities
*/
class QmitkToFUtilView : public QmitkFunctionality
{  
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT
  
  public:  

    static const std::string VIEW_ID;

    QmitkToFUtilView();
    ~QmitkToFUtilView();

    virtual void CreateQtPartControl(QWidget *parent);
    /// \brief Called when the functionality is activated
    virtual void Activated();
    /// \brief Called when the functionality is deactivated
    virtual void Deactivated();
    virtual void StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget);
    virtual void StdMultiWidgetNotAvailable();

  protected slots:
  
    /*!
    \brief Slot triggered from the timer to update the images and visualization
    */
    void OnUpdateCamera();
    /*!
    \brief Slot called when the "Connect" button of the ConnectionWidget is pressed
    */
    void OnToFCameraConnected();
    /*!
    \brief Slot called when the "Disconnect" button of the ConnectionWidget is pressed
    */
    void OnToFCameraDisconnected();
    /*!
    \brief Slot called when the camera selection in the ConnectionWidget has changed
    */
    void OnToFCameraSelected(const QString selected);
    /*!
    \brief Slot called when the "Start" button of the RecorderWidget is pressed
    */
    void OnToFCameraStarted();
    /*!
    \brief Slot called when the "Stop" button of the RecorderWidget is pressed
    */
    void OnToFCameraStopped();
    /*!
    \brief Slot invoked when the texture checkbox is checked. Enables the scalar visibility of the surface
    */
    void OnTextureCheckBoxChecked(bool checked);
    /*!
    \brief Slot invoked when the video texture checkbox is checked. Enables the texture of the surface
    */
    void OnVideoTextureCheckBoxChecked(bool checked);

  protected:

    /*!
    \brief initialize the visibility settings of ToF data (images + surface)
    \param useToF true: distance image: widget1, amplitude image: widget 2, intensity image: widget 3; false: standard
    */
    void UseToFVisibilitySettings(bool useToF);

    Ui::QmitkToFUtilViewControls* m_Controls;

    QmitkStdMultiWidget* m_MultiWidget;

    QTimer* m_Frametimer; ///< Timer used to continuously update the images

    mitk::Image::Pointer m_MitkDistanceImage; ///< member holding a pointer to the distance image of the selected camera
    mitk::Image::Pointer m_MitkAmplitudeImage; ///< member holding a pointer to the amplitude image of the selected camera
    mitk::Image::Pointer m_MitkIntensityImage; ///< member holding a pointer to the intensity image of the selected camera
    mitk::Surface::Pointer m_Surface; ///< member holding a pointer to the surface generated from the distance image of the selected camera

    mitk::DataNode::Pointer m_DistanceImageNode; ///< DataNode holding the distance image of the selected camera
    mitk::DataNode::Pointer m_AmplitudeImageNode; ///< DataNode holding the amplitude image of the selected camera
    mitk::DataNode::Pointer m_IntensityImageNode; ///< DataNode holding the intensity image of the selected camera
    mitk::DataNode::Pointer m_SurfaceNode; ///< DataNode holding the surface generated from the distanc image of the selected camera

    // ToF processing and recording filter
    mitk::ToFImageRecorder::Pointer m_ToFImageRecorder; ///< ToF image recorder used for lossless recording of ToF image data
    mitk::ToFImageGrabber::Pointer m_ToFImageGrabber; ///< Source of a ToF image processing pipeline. Provides pointers to distance, amplitude and intensity image
    mitk::ToFDistanceImageToSurfaceFilter::Pointer m_ToFDistanceImageToSurfaceFilter; ///< Filter for calculating a surface representation from a given distance image
    mitk::ToFCompositeFilter::Pointer m_ToFCompositeFilter; ///< Filter combining several processing steps (thresholding, Median filtering, Bilateral filtering)

    int m_SurfaceDisplayCount; ///< member used to determine whether surface is initialized or not
    int m_2DDisplayCount; ///< member used to determine whether frame rate output should be shown
    // members for calculating the frame rate
    mitk::RealTimeClock::Pointer m_RealTimeClock; ///< real time clock used to calculate the display framerate
    int m_StepsForFramerate; ///< number of steps used for calculating the display framerate
    double m_2DTimeBefore; ///< holds the time stamp at the beginning of the display framerate measurement
    double m_2DTimeAfter; ///< holds the time stamp at the end of the display framerate measurement

    // members used for displaying an external video source
    mitk::OpenCVVideoSource::Pointer m_VideoSource; ///< OpenCV video source to connect a video device
    unsigned char* m_VideoTexture; ///< texture used to show video image
    int m_VideoCaptureWidth; ///< width of the video image
    int m_VideoCaptureHeight; ///< height of the video image
    bool m_VideoEnabled; ///< flag indicating whether video grabbing is enabled. Set via the RGB texture checkbox

  private:

    /*!
    \brief helper method to replace data of the specified node. If node does not exist it will be created
    \param nodeName Name of the node
    \param data Data object to be replaced
    \return returns the node
    */
    mitk::DataNode::Pointer ReplaceNodeData(std::string nodeName, mitk::BaseData* data);

    void ProcessVideoTransform();

    mitk::ToFSurfaceVtkMapper3D::Pointer m_ToFSurfaceVtkMapper3D;
};

#endif // _QMITKTOFUTILVIEW_H_INCLUDED

