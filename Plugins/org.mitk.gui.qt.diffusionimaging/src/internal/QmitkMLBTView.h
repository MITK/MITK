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

/*!
\brief
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
  typedef itk::MLBSTrackingFilter<100>      TrackerType;

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

protected:

  void StartTracking();
  void StartTraining();

  /// \brief called by QmitkFunctionality when DataManager's selection has changed
  virtual void OnSelectionChanged( std::vector<mitk::DataNode*> nodes ) override;

  Ui::QmitkMLBTViewControls* m_Controls;
  QmitkStdMultiWidget* m_MultiWidget;

  mitk::TrackingForestHandler<> m_ForestHandler;
  std::vector< mitk::Image::Pointer > m_SelectedDiffImages;
  std::vector< mitk::FiberBundle::Pointer > m_SelectedFB;
  std::vector< ItkUcharImgType::Pointer > m_MaskImages;
  std::vector< ItkUcharImgType::Pointer > m_WhiteMatterImages;

  QFutureWatcher<void> m_TrainingWatcher;
  QFutureWatcher<void> m_TrackingWatcher;
  bool m_TrackingThreadIsRunning;
  TrackerType::Pointer tracker;
  QTimer*   m_TrackingTimer;
  mitk::DataNode::Pointer m_TractogramNode;

private:

 };



