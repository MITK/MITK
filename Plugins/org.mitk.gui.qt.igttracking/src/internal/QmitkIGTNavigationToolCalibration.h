/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef QmitkIGTNavigationToolCalibration_h
#define QmitkIGTNavigationToolCalibration_h

#include <berryISelectionListener.h>

#include <QmitkAbstractView.h>

#include <mitkNavigationData.h>

#include "QmitkInteractiveTransformationWidget.h"


#include "ui_QmitkIGTNavigationToolCalibrationControls.h"

#include <mitkNavigationDataRecorder.h>

//QT headers
#include <QTimer>


/*!
  \brief IGTNavigationToolCalibration

  \warning  This class is not yet documented. Use "git blame" and ask the author to provide basic documentation.

  \ingroup ${plugin_target}_internal
  */
class  QmitkIGTNavigationToolCalibration : public QmitkAbstractView
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:

	QmitkIGTNavigationToolCalibration();

  ~QmitkIGTNavigationToolCalibration() override;

  static const std::string VIEW_ID;

  void CreateQtPartControl(QWidget *parent) override;

  protected slots:


  void OnAddPivotPose();
  void OnComputePivot();
  void OnUseComputedPivotPoint();
  void SetToolToCalibrate();
  void SetCalibrationPointer();
  void UpdateTrackingTimer();
  void AddLandmark();
  void SaveCalibratedTool();
  void OnToolCalibrationMethodChanged(int index);
  void OnStartManualToolTipCalibration();
  void OnRunSingleRefToolCalibrationClicked();
  void OnLoginSingleRefToolNavigationDataClicked();
  void OnSetNewToolTipPosButtonClicked();
  void OnGetPositions();
  void OnCalibrateToolAxis();
  void OnToolAxisSpinboxChanged();
  void OnManualEditToolTipFinished(mitk::AffineTransform3D::Pointer toolTip);

protected:

  void SetFocus() override;

  void UpdateOffsetCoordinates();

  int m_IndexCurrentCalibrationMethod;

  Ui::IGTNavigationToolCalibrationControls m_Controls;

  //some general members
  mitk::NavigationTool::Pointer m_ToolToCalibrate; //<<< tool that will be calibrated
  int m_IDToolToCalibrate; //<<< id of tool that will be calibrated (of the navigation data source)
  mitk::NavigationDataSource::Pointer m_NavigationDataSourceOfToolToCalibrate; //<<< navigation data source of the tool that will be calibrated
  mitk::NavigationDataSource::Pointer m_NavigationDataSourceOfCalibrationPointer; //<<< navigation data source of the calibration pointer
  mitk::DataNode::Pointer m_ToolSurfaceInToolCoordinatesDataNode; //<<< holds the tool surface in tool coordinates (for preview purposes)
  int m_IDCalibrationPointer; //<<< id of the calibration pointer (of the corresponding navigation data source)
  QTimer* m_TrackingTimer; //<<< tracking timer that updates the status widgets
  void ApplyToolTipTransform(mitk::NavigationData::Pointer ToolTipTransformInToolCoordinates, std::string message = "Tool was updated with the calibrated tool tip!"); //<<< applys the given tool tip transform to the tool to calibrate
  bool CheckInitialization(bool CalibrationPointerRequired = true); //<<< checks if the tool to calibrate and (if required) the calibration pointer is initialized. Displays a warning and returns false if not.
  mitk::NavigationData::Pointer m_ComputedToolTipTransformation; //<<< holds the new tooltip transformation after it was computed to write it into the tool later

  // members and helper methods for pivot tool calibration
  std::vector<mitk::NavigationData::Pointer> m_PivotPoses;
  void AddPivotPose();
  void ClearOldPivot();
  void UpdatePivotCount();
  bool  m_OnAddPivotPoseClicked;
  int PivotCount;

  // members and helper methods for manual tool calibration
  void UpdateManualToolTipCalibrationView();
  QmitkInteractiveTransformationWidget* m_ToolTransformationWidget;

  // members and helper methods for single reference tool calibration
  void LoginSingleRefToolNavigationData();
  std::vector< mitk::Point3D > m_LoggedNavigationDataOffsets;
  std::vector< mitk::NavigationData::Pointer > m_LoggedNavigationDataDifferences;
  bool m_OnLoginSingleRefToolNavigationDataClicked;
  int m_NumberOfNavigationData;
  int m_NumberOfNavigationDataCounter;
  mitk::Point3D m_ResultOffsetVector;

  // members and helper methods for tool tip preview
  mitk::DataNode::Pointer m_ToolTipPointPreview; //<<< Data node of the tool tip preview
  void ShowToolTipPreview(mitk::NavigationData::Pointer ToolTipInTrackingCoordinates); //<<< Adds a preview of the tool tip into the data storage
  void RemoveToolTipPreview(); //<<< Removes the preview

  // members for the tool landmark calibration
  mitk::PointSet::Pointer m_CalibrationLandmarks;
  mitk::DataNode::Pointer m_CalibrationLandmarksNode;
  mitk::PointSet::Pointer m_RegistrationLandmarks;
  mitk::DataNode::Pointer m_RegistrationLandmarksNode;

  //members and helper methods for tool axis calibration
  mitk::Vector3D m_CalibratedToolAxis;
  mitk::NavigationData::Pointer  m_AxisCalibration_ToolToCalibrate;
  mitk::NavigationData::Pointer  m_AxisCalibration_NavDataCalibratingTool;

};

#endif
