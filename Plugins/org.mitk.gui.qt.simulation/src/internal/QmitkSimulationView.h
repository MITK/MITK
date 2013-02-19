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

#ifndef QmitkSimulationView_h
#define QmitkSimulationView_h

#include <mitkSurface.h>
#include <QmitkAbstractView.h>
#include <QTimer>
#include <ui_QmitkSimulationViewControls.h>

class QmitkSimulationView : public QmitkAbstractView
{
  Q_OBJECT

public:
  QmitkSimulationView();
  ~QmitkSimulationView();

  void CreateQtPartControl(QWidget* parent);
  void SetFocus();

private slots:
  void OnAnimateButtonToggled(bool toggled);
  void OnDTSpinBoxValueChanged(double value);
  void OnRecordButtonToggled(bool toggled);
  void OnResetButtonClicked();
  void OnSimulationComboBoxSelectionChanged(const mitk::DataNode* node);
  void OnSnapshotButtonClicked();
  void OnStepButtonClicked();
  void OnTimerTimeout();

private:
  QmitkSimulationView(const QmitkSimulationView&);
  QmitkSimulationView& operator=(const QmitkSimulationView&);

  void OnNodeRemovedFromDataStorage(const mitk::DataNode* node);
  bool SetSelectionAsCurrentSimulation() const;

  Ui::QmitkSimulationViewControls m_Controls;
  bool m_SelectionWasRemovedFromDataStorage;
  mitk::DataNode::Pointer m_Selection;
  mitk::Surface::Pointer m_Record;
  QTimer m_Timer;
};

#endif
