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
#include <QTime>
#include <QTimer>
#include <ui_QmitkSimulationViewControls.h>

namespace mitk
{
  class ISimulationService;
}

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
  void OnDtChanged(double dt);
  void OnRecordButtonToggled(bool toggled);
  void OnResetButtonClicked();
  void OnSelectedSceneChanged(const mitk::DataNode* node);
  void OnSnapshotButtonClicked();
  void OnStep(bool renderWindowUpdate);
  void OnStepButtonClicked();
  void OnTimeout();

private:
  void OnNodeRemovedFromDataStorage(const mitk::DataNode* node);
  bool SetSelectionAsCurrentSimulation() const;

  static const int MsPerFrame = 17;

  Ui::QmitkSimulationViewControls m_Controls;
  mitk::ISimulationService* m_SimulationService;
  bool m_SelectionWasRemovedFromDataStorage;
  mitk::DataNode::Pointer m_Selection;
  QTimer m_Timer;
  QTime m_NextRenderWindowUpdate;
};

#endif
