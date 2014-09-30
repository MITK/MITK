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

#ifndef QmitkToFUtilView_h
#define QmitkToFUtilView_h

#include <QmitkAbstractView.h>
#include <berryIWorkbenchPartReference.h>
#include <mitkIZombieViewPart.h>


#include <ui_QmitkToFUtilViewControls.h>

class QTimer;

#include <mitkRealTimeClock.h>
#include <mitkToFImageGrabber.h>
#include <mitkOpenCVVideoSource.h>
#include <mitkSurface.h>
#include <mitkToFDistanceImageToSurfaceFilter.h>
#include <mitkToFImageRecorder.h>
#include <mitkToFCompositeFilter.h>
#include <mitkCameraIntrinsics.h>

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
class QmitkToFUtilView : public QmitkAbstractView, public mitk::IZombieViewPart
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:

  static const std::string VIEW_ID;

  QmitkToFUtilView();
  ~QmitkToFUtilView();


  virtual void CreateQtPartControl(QWidget *parent);
  /// \brief Called when the functionality is activated.
  virtual void Activated();
  /// \brief Called when the functionality is deactivated. In this case the zombie view of this functionality becomes active!
  virtual void ActivatedZombieView(berry::IWorkbenchPartReference::Pointer zombieView);

  virtual void Deactivated();
  virtual void Visible();
  virtual void Hidden();

  void SetFocus();

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
    \brief Slot called when the "Start" button of the RecorderWidget is pressed
    */
  void OnToFCameraStarted();
  /*!
    \brief Slot called when the "Stop" button of the RecorderWidget is pressed
    */
  void OnToFCameraStopped();

  /*!
    \brief Slot invoked when user alters the coronal window input from RGB to Intensity or vice versa.
    */
  void OnChangeCoronalWindowOutput(int index);
  /*!
    \brief Slot invoked when acquisition mode of Kinect is changed
    */
  void OnKinectAcquisitionModeChanged();

protected:

  /*!
    \brief initialize the visibility settings of ToF data (images + surface)
    \param useToF true: distance image: widget1, amplitude image: widget 2, intensity image: widget 3; false: standard
    */
  void UseToFVisibilitySettings(bool useToF);

  Ui::QmitkToFUtilViewControls* m_Controls;

  QmitkStdMultiWidget* m_MultiWidget;

  QTimer* m_Frametimer; ///< Timer used to continuously update the images

  QString m_SelectedCamera; ///< String holding the selected camera

  mitk::Image::Pointer m_MitkDistanceImage; ///< member holding a pointer to the distance image of the selected camera
  mitk::Image::Pointer m_MitkAmplitudeImage; ///< member holding a pointer to the amplitude image of the selected camera
  mitk::Image::Pointer m_MitkIntensityImage; ///< member holding a pointer to the intensity image of the selected camera
  mitk::Surface::Pointer m_Surface; ///< member holding a pointer to the surface generated from the distance image of the selected camera

  mitk::DataNode::Pointer m_DistanceImageNode; ///< DataNode holding the distance image of the selected camera
  mitk::DataNode::Pointer m_AmplitudeImageNode; ///< DataNode holding the amplitude image of the selected camera
  mitk::DataNode::Pointer m_IntensityImageNode; ///< DataNode holding the intensity image of the selected camera
  mitk::DataNode::Pointer m_RGBImageNode; ///< DataNode holding the rgb image of the selected camera
  mitk::DataNode::Pointer m_SurfaceNode; ///< DataNode holding the surface generated from the distanc image of the selected camera

  // ToF processing and recording filter
  mitk::ToFImageRecorder::Pointer m_ToFImageRecorder; ///< ToF image recorder used for lossless recording of ToF image data
  mitk::ToFImageGrabber::Pointer m_ToFImageGrabber; ///< Source of a ToF image processing pipeline. Provides pointers to distance, amplitude and intensity image
  mitk::ToFDistanceImageToSurfaceFilter::Pointer m_ToFDistanceImageToSurfaceFilter; ///< Filter for calculating a surface representation from a given distance image
  mitk::ToFCompositeFilter::Pointer m_ToFCompositeFilter; ///< Filter combining several processing steps (thresholding, Median filtering, Bilateral filtering)

  int m_2DDisplayCount; ///< member used to determine whether frame rate output should be shown
  // members for calculating the frame rate
  mitk::RealTimeClock::Pointer m_RealTimeClock; ///< real time clock used to calculate the display framerate
  int m_StepsForFramerate; ///< number of steps used for calculating the display framerate
  double m_2DTimeBefore; ///< holds the time stamp at the beginning of the display framerate measurement
  double m_2DTimeAfter; ///< holds the time stamp at the end of the display framerate measurement

  mitk::CameraIntrinsics::Pointer m_CameraIntrinsics; ///< member holding the intrinsic parameters of the camera
private:

  /*!
    \brief helper method to replace data of the specified node. If node does not exist it will be created
    \param nodeName Name of the node
    \param data Data object to be replaced
    \return returns the node
    */
  mitk::DataNode::Pointer ReplaceNodeData(std::string nodeName, mitk::BaseData* data);

  void ProcessVideoTransform();

  /*!
      \brief Reset all GUI related things to default. E.g. show sagittal and coronal slices in the renderwindows.
      */
  void ResetGUIToDefault();
};

#endif // _QMITKTOFUTILVIEW_H_INCLUDED

