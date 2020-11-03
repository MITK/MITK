/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkIGTTrackingSemiAutomaticMeasurementView_h
#define QmitkIGTTrackingSemiAutomaticMeasurementView_h

#include <berryISelectionListener.h>

#include <QmitkFunctionality.h>
#include <QmitkStdMultiWidget.h>

//QT
#include <QTimer>

//MITK
#include <mitkNavigationToolStorage.h>
#include <mitkTrackingDeviceSource.h>
#include <mitkNavigationDataObjectVisualizationFilter.h>
#include <mitkNavigationDataRecorderDeprecated.h>
#include <mitkNavigationDataEvaluationFilter.h>

#include "ui_QmitkIGTTrackingSemiAutomaticMeasurementViewControls.h"

/*!
  \brief QmitkIGTTrackingSemiAutomaticMeasurementView

  \warning  This application module is not yet documented. Use "svn blame/praise/annotate" and ask the author to provide basic documentation.

  \sa QmitkFunctionality
  \ingroup Functionalities
  */
class QmitkIGTTrackingSemiAutomaticMeasurementView : public QmitkFunctionality
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:

  static const std::string VIEW_ID;

  QmitkIGTTrackingSemiAutomaticMeasurementView();
  ~QmitkIGTTrackingSemiAutomaticMeasurementView() override;

  void CreateQtPartControl(QWidget *parent) override;

  virtual void MultiWidgetAvailable(QmitkAbstractMultiWidget &multiWidget) override;
  virtual void MultiWidgetNotAvailable() override;

  protected slots:

  void OnLoadMeasurementStorage();
  void OnLoadReferenceStorage();
  void OnStartTracking();
  void OnStopTracking();
  void OnMeasurementLoadFile();
  void OnSetReference();
  void StartNextMeasurement();
  void RepeatLastMeasurement();
  void UpdateTimer();
  void CreateResults();
  void OnUseReferenceToggled(bool state);

protected:

  Ui::QmitkIGTTrackingSemiAutomaticMeasurementViewControls* m_Controls;

  QmitkStdMultiWidget* m_MultiWidget;

  //the tool storages
  mitk::NavigationToolStorage::Pointer m_MeasurementStorage;
  mitk::NavigationToolStorage::Pointer m_ReferenceStorage;

  //members for the filter pipeline
  mitk::TrackingDeviceSource::Pointer m_MeasurementTrackingDeviceSource;
  mitk::NavigationDataObjectVisualizationFilter::Pointer m_MeasurementToolVisualizationFilter;
  mitk::NavigationDataRecorderDeprecated::Pointer m_MeasurementLoggingFilterXML;
  mitk::NavigationDataRecorderDeprecated::Pointer m_MeasurementLoggingFilterCSV;
  mitk::TrackingDeviceSource::Pointer m_ReferenceTrackingDeviceSource;
  mitk::NavigationDataRecorderDeprecated::Pointer m_ReferenceLoggingFilterXML;
  mitk::NavigationDataRecorderDeprecated::Pointer m_ReferenceLoggingFilterCSV;

  //members for file name list
  std::vector<std::string> m_FilenameVector;
  int m_NextFile;

  //help methods
  mitk::NavigationToolStorage::Pointer ReadStorage(std::string file);
  void MessageBox(std::string s);
  void DisableAllButtons();
  void EnableAllButtons();
  void FinishMeasurement();
  void StartLoggingAdditionalCSVFile(std::string filePostfix);
  void LogAdditionalCSVFile();
  void StopLoggingAdditionalCSVFile();

  //timer
  QTimer* m_Timer;

  //memebers for reference checking
  std::vector<mitk::Point3D> m_ReferenceStartPositions;
  bool m_referenceValid;

  //logging members
  int m_loggedFrames;
  bool m_logging;
  std::fstream m_logFileCSV;

  //event filter for key presses
  bool eventFilter(QObject *obj, QEvent *ev) override;

  //results members
  mitk::PointSet::Pointer m_MeanPoints;
  std::vector<double> m_RMSValues;
  mitk::NavigationDataEvaluationFilter::Pointer m_EvaluationFilter;

  bool m_tracking;
};

#endif // _QMITKIGTTRACKINGSEMIAUTOMATICMEASUREMENTVIEW_H_INCLUDED
