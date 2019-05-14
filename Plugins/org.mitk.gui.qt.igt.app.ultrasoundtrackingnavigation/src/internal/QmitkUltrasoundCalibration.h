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

#ifndef QmitkUltrasoundCalibration_h
#define QmitkUltrasoundCalibration_h

#include <berryISelectionListener.h>

#include <QmitkAbstractView.h>

//QT
//#include <QTimer>

//MITK
#include <mitkUSCombinedModality.h>
#include <mitkPointSet.h>
#include <mitkNeedleProjectionFilter.h>
#include <mitkPointSetDifferenceStatisticsCalculator.h>
#include <mitkIGTLClient.h>
#include <mitkNavigationDataToIGTLMessageFilter.h>

// Microservices
#include "ui_QmitkUltrasoundCalibrationControls.h"

#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkLandmarkTransform.h>

#include <ctkServiceEvent.h>

/*!
  \brief QmitkUltrasoundCalibration

  \warning  This view provides a simple calibration process.

  \sa QmitkFunctionality
  \ingroup ${plugin_target}_internal
  */
class QmitkUltrasoundCalibration : public QmitkAbstractView
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:

  QmitkUltrasoundCalibration();
  ~QmitkUltrasoundCalibration();

  static const std::string VIEW_ID;

  virtual void CreateQtPartControl(QWidget *parent);

  void OnUSDepthChanged(const std::string&, const std::string&);

  protected slots:

  /**
  * \brief Triggered, whenever the user switches Tabs
  *
  */
  void OnTabSwitch(int index);

  /**
  * \brief Triggered, when the user has clicked "select Devices".
  *
  */
  //void OnSelectDevice(mitk::USCombinedModality::Pointer);
  void OnDeviceSelected();
  void OnDeviceDeselected();

  /**
  * \brief Triggered, when the user clicks "Add Point"
  *
  */
  void OnAddCalibPoint();

  /**
  * \brief Triggered, when the user clicks "Calibrate"
  *
  */
  void OnCalibration();

  /**
  * \brief Triggered, when the user clicks "Add Target Points".
  *
  * Adds an image point and an tracking point to their respective evaluation pointsets
  */
  void OnAddEvalTargetPoint();

  /**
  * \brief Triggered, when the user clicks "Add Point".
  *
  * Adds a projected point to the projected point evaluation set.
  */
  void OnAddEvalProjectedPoint();

  /**
  * \brief Triggered when the user clicks "Save Results" in the Evaluation tab.
  */
  void OnSaveEvaluation();

  /**
  * \brief Triggered when the user clicks "Save Calibration" in the Calibration tab.
  */
  void OnSaveCalibration();

  /**
  * \brief Triggered when the user clicks "Run Next Round". Also used as a reset mechanism.
  */
  void OnReset();

  /**
  * \brief Triggered in regular intervals by a timer, when live view is enabled.
  *
  */
  void Update();

  /**
  * \brief Freezes or unfreezes the image.
  */
  void SwitchFreeze();

  /**
  *
  */
  void OnStartCalibrationProcess();

  /**
  *\brief Method to use the PLUS-Toolkoit for Calibration of USTrackingNavigation
  */
  void OnStartPlusCalibration();

  void OnStopPlusCalibration();

  /**
  *\ brief Starts the Streaming of USImage and Navigation Data when PLUS is connected
  */
  void OnStartStreaming();

  void OnNewConnection();

  /**
  \*brief Get the Calibration from the PLUS-Toolkit once Calibration with fCal is done
  */
  void OnGetPlusCalibration();

  /**
  \*brief Convert the recieved igtl::Matrix into an mitk::AffineTransform3D which can be used to calibrate the CombinedModality
  */
  void ProcessPlusCalibration(igtl::Matrix4x4& imageToTracker);

  void OnStreamingTimerTimeout();

  /**
  *
  */
  void OnStopCalibrationProcess();

  void OnAddCurrentTipPositionToReferencePoints();

  void OnStartVerification();

  void OnAddCurrentTipPositionForVerification();

  void OnDeviceServiceEvent(const ctkServiceEvent event);

  void OnFreezeClicked();

  void OnAddSpacingPoint();

  void OnCalculateSpacing();

signals:
  /**
  * \brief used for thread seperation, the worker thread must not call OnNewConnection directly.
  * QT signals are thread safe and separate the threads
  */
  void NewConnectionSignal();

protected:

  virtual void SetFocus();

  /// \brief called by QmitkFunctionality when DataManager's selection has changed
  virtual void OnSelectionChanged(berry::IWorkbenchPart::Pointer source,
    const QList<mitk::DataNode::Pointer>& nodes);

  Ui::QmitkUltrasoundCalibrationControls m_Controls;

  /**
  * \brief Internal function that activates display of the needle path.
  */
  void ShowNeedlePath();

  /**
   * \brief Clears all member attributes which are holding intermediate results for the calibration.
   */
  void ClearTemporaryMembers();

  void OnPlusConnected();

  /**
  * \brief The combined modality used for imaging and tracking.
  */
  mitk::AbstractUltrasoundTrackerDevice::Pointer m_CombinedModality;

  /**
  * \brief NavigationDataSource used for tracking data.
  * This will be gotten by the combined modality.
  */
  mitk::NavigationDataSource::Pointer m_Tracker;

  QTimer *m_Timer;

  mitk::DataNode::Pointer m_Node;
  mitk::DataNode::Pointer m_CalibNode;
  mitk::DataNode::Pointer m_WorldNode;

  //IGTL Servers and Devices needed for the communication with PLUS
  mitk::IGTLServer::Pointer m_USServer;
  mitk::IGTLMessageProvider::Pointer m_USMessageProvider;
  mitk::ImageToIGTLMessageFilter::Pointer m_USImageToIGTLMessageFilter;

  mitk::IGTLServer::Pointer m_TrackingServer;
  mitk::IGTLMessageProvider::Pointer m_TrackingMessageProvider;
  mitk::NavigationDataToIGTLMessageFilter::Pointer m_TrackingToIGTLMessageFilter;

  mitk::IGTLClient::Pointer m_TransformClient;
  mitk::IGTLDeviceSource::Pointer m_TransformDeviceSource;

  QTimer *m_StreamingTimer;

  unsigned long m_NewConnectionObserverTag;

  /**
  * \brief The current Ultrasound Image.
  */
  mitk::Image::Pointer m_Image;
  /**
  * \brief Current point when the image was last frozen.
  */
  mitk::Point3D m_FreezePoint;

  /**
  * \brief Pointset containing all tool points.
  */
  mitk::PointSet::Pointer m_CalibPointsImage;
  /**
  * \brief Pointset containing corresponding points on the image.
  */
  mitk::PointSet::Pointer m_CalibPointsTool;
  /**
  * \brief Pointset containing Projected Points (aka "where we thought the needle was gonna land")
  */
  mitk::PointSet::Pointer m_EvalPointsProjected;
  /**
  * \brief Pointset containing the evaluated points on the image.
  */
  mitk::PointSet::Pointer m_EvalPointsImage;
  /**
  * \brief Pointset containing tracked evaluation points.
  */
  mitk::PointSet::Pointer m_EvalPointsTool;

  /**
  * \brief Pointset containing tracked evaluation points.
  */
  mitk::PointSet::Pointer m_VerificationReferencePoints;
  mitk::DataNode::Pointer m_VerificationReferencePointsDataNode;

  int m_currentPoint;
  std::vector<mitk::Point3D> m_allReferencePoints;
  std::vector<double> m_allErrors;

  /**
  * \brief Pointset containing points along the needle's prohected path. Only used for visualization. The first point is the needle tip.
  */
  //mitk::PointSet::Pointer m_NeedlePathPoints;

  /**
  * \brief Creates a Pointset that projects the needle's path
  */
  mitk::NeedleProjectionFilter::Pointer m_NeedleProjectionFilter;
  /**
  * \brief Total number of calibration points set.
  */
  int m_CalibPointsCount;

  QString m_CurrentDepth;

  /**
  * \brief StatisticsRegarding Projection Accuracy.
  * (Compares m_EvalPointsProjected to m_EvalPointsImage)
  */
  mitk::PointSetDifferenceStatisticsCalculator::Pointer m_ProjectionStatistics;
  /**
  * \brief StatisticsRegarding Evaluation Accuracy.
  * (Compares m_EvalPointsTool to m_EvalPointsImage)
  */
  mitk::PointSetDifferenceStatisticsCalculator::Pointer m_EvaluationStatistics;
  /**
  * \brief StatisticsRegarding Calibration Accuracy.
  * (Compares m_CalibPointsTool to a transformed copy of m_CalibPointsImage).
  */
  mitk::PointSetDifferenceStatisticsCalculator::Pointer m_CalibrationStatistics;

  /**
  * \brief Result of the Calibration.
  */
  mitk::AffineTransform3D::Pointer m_Transformation;

  /**
  * This method is copied from PointSetModifier which is part of MBI. It should be replaced
  * by external method call as soon as this functionality will be available in MITK.
  */
  vtkSmartPointer<vtkPolyData> ConvertPointSetToVtkPolyData(mitk::PointSet::Pointer PointSet);

  double ComputeFRE(mitk::PointSet::Pointer imageFiducials, mitk::PointSet::Pointer realWorldFiducials, vtkSmartPointer<vtkLandmarkTransform> transform = nullptr);

  void ApplyTransformToPointSet(mitk::PointSet::Pointer pointSet, vtkSmartPointer<vtkLandmarkTransform> transform);

  mitk::PointSet::Pointer m_SpacingPoints;
  mitk::DataNode::Pointer m_SpacingNode;
  int m_SpacingPointsCount;

private:
  mitk::MessageDelegate2<QmitkUltrasoundCalibration, const std::string&, const std::string&> m_USDeviceChanged;
};

#endif // UltrasoundCalibration_h
