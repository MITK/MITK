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

#include <QmitkFunctionality.h>
#include "ui_QmitkMLBTViewControls.h"

#ifndef Q_MOC_RUN
#include "mitkDataStorage.h"
#include "mitkDataStorageSelection.h"
#include <mitkTrackingForestHandler.h>
#include <itkMLBSTrackingFilter.h>
#endif

#include <QtConcurrentRun>
#include <QFutureWatcher>
#include <QTimer>
#include <QmitkMlbstTrainingDataWidget.h>
#include <memory>
#include <QGridLayout>

/*!
\brief View to perform machine learning based fiber tractography. Includes training of the random forst classifier as well as the actual tractography.
*/

// Forward Qt class declarations


class QmitkMLBTView : public QmitkFunctionality
{


  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:

  static const std::string VIEW_ID;

  typedef itk::Image<unsigned char, 3>      ItkUcharImgType;
  typedef itk::MLBSTrackingFilter<>         TrackerType;

  QmitkMLBTView();
  virtual ~QmitkMLBTView();

  virtual void CreateQtPartControl(QWidget *parent) override;

  virtual void StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget) override;
  virtual void StdMultiWidgetNotAvailable() override;
  virtual void Activated() override;

  protected slots:

  void StartTrackingThread();
  void OnTrackingThreadStop();
  void StartTrainingThread();
  void OnTrainingThreadStop();
  void SaveForest();
  void LoadForest();
  void BuildFibers();
  void ChangeTimerInterval(int value);
  void ToggleDemoMode(int state);
  void PauseTracking();
  void AbortTracking();
  void AddTrainingWidget();
  void RemoveTrainingWidget();

protected:

  void StartTracking();
  void StartTraining();
  void UpdateGui();

  Ui::QmitkMLBTViewControls* m_Controls;
  QmitkStdMultiWidget* m_MultiWidget;

  mitk::TrackingForestHandler<> m_ForestHandler;

  QFutureWatcher<void> m_TrainingWatcher;
  QFutureWatcher<void> m_TrackingWatcher;
  bool m_TrackingThreadIsRunning;
  TrackerType::Pointer tracker;
  std::shared_ptr<QTimer>   m_TrackingTimer;
  mitk::DataNode::Pointer m_TractogramNode;
  mitk::DataNode::Pointer m_SamplingPointsNode;
  mitk::DataNode::Pointer m_AlternativePointsNode;

  std::vector< std::shared_ptr<QmitkMlbstTrainingDataWidget> > m_TrainingWidgets;

private:

  bool IsTrainingInputValid(void) const;

  std::string m_LastLoadedForestName;

 };



