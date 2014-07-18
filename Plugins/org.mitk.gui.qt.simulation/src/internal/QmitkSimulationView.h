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

#include <mitkSimulationInteractor.h>
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
  void OnResetButtonClicked();
  void OnSelectedSimulationChanged(const mitk::DataNode* node);
  void OnSelectedBaseChanged();
  void OnStep(bool renderWindowUpdate);
  void OnStepButtonClicked();
  void OnTimeout();
  void OnButtonClicked();

private:
  void OnNodeRemovedFromDataStorage(const mitk::DataNode* node);
  void ResetSceneTreeWidget();
  void SetSimulationControlsEnabled(bool enabled);

  static const int MSecsPerFrame = 17;

  Ui::QmitkSimulationViewControls m_Controls;
  mitk::ISimulationService* m_SimulationService;
  mitk::DataNode::Pointer m_Selection;
  mitk::SimulationInteractor::Pointer m_Interactor;
  QTimer m_Timer;
  QTime m_NextRenderWindowUpdate;
};

#endif
