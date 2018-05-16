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

#include <QmitkAbstractView.h>
#include "ui_QmitkMLBTViewControls.h"

#ifndef Q_MOC_RUN
#include "mitkDataStorage.h"
#include "mitkDataStorageSelection.h"
#include <mitkTrackingHandlerRandomForest.h>
#include <itkStreamlineTrackingFilter.h>
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


class QmitkMLBTView : public QmitkAbstractView
{


  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:

  static const std::string VIEW_ID;

  typedef itk::Image<unsigned char, 3>      ItkUcharImgType;
  typedef itk::StreamlineTrackingFilter         TrackerType;

  QmitkMLBTView();
  virtual ~QmitkMLBTView();
  virtual void CreateQtPartControl(QWidget *parent) override;

  ///
  /// Sets the focus to an internal widget.
  ///
  virtual void SetFocus() override;

protected slots:

  void StartTrainingThread();
  void OnTrainingThreadStop();
  void AddTrainingWidget();
  void RemoveTrainingWidget();

protected:

  void StartTraining();

  Ui::QmitkMLBTViewControls*    m_Controls;
  mitk::TrackingDataHandler*    m_ForestHandler;
  QFutureWatcher<void>          m_TrainingWatcher;
  std::vector< std::shared_ptr<QmitkMlbstTrainingDataWidget> > m_TrainingWidgets;

private:

  bool IsTrainingInputValid(void) const;

};



