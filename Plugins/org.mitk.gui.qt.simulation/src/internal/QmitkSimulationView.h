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

private:
  QmitkSimulationView(const QmitkSimulationView&);
  QmitkSimulationView& operator=(const QmitkSimulationView&);

  bool SetSelectionAsCurrentSimulation() const;

  Ui::QmitkSimulationViewControls m_Controls;
  QTimer m_Timer;

private slots:
  void OnAnimateButtonToggled(bool toggled);
  void OnDTSpinBoxValueChanged(double value);
  void OnResetSceneButtonClicked();
  void OnSimulationComboBoxSelectionChanged(const mitk::DataNode* node);
  void OnStepButtonClicked();
  void OnTimerTimeout();
};

#endif
