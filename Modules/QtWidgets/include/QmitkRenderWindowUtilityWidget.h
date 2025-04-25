/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkRenderWindowUtilityWidget_h
#define QmitkRenderWindowUtilityWidget_h

#include "MitkQtWidgetsExports.h"

// qt widgets module
#include <QmitkSynchronizedNodeSelectionWidget.h>
#include <QmitkSliceNavigationWidget.h>
#include <QmitkStepperAdapter.h>
#include <mitkRenderWindowLayerController.h>
#include <mitkRenderWindowViewDirectionController.h>

// qt
#include <QWidget>
#include <QHBoxLayout>
#include <QMenuBar>
#include <QComboBox>

namespace mitk
{
  class DataStorage;
}

class QmitkRenderWindow;

class MITKQTWIDGETS_EXPORT QmitkRenderWindowUtilityWidget : public QWidget
{
	Q_OBJECT

public:

  QmitkRenderWindowUtilityWidget(
    QWidget* parent = nullptr,
    QmitkRenderWindow* renderWindow = nullptr,
    mitk::DataStorage* dataStorage = nullptr,
    const int nSynchGroups = 1
  );

  ~QmitkRenderWindowUtilityWidget() override;

  using GroupSyncIndexType = int;

  void SetSynchGroup(const GroupSyncIndexType index);
  GroupSyncIndexType GetSynchGroup() const;
  void OnSynchGroupSelectionChanged(int index);

  void SetGeometry(const itk::EventObject& event);
  QmitkSynchronizedNodeSelectionWidget* GetNodeSelectionWidget() const;

public Q_SLOTS:
  void UpdateViewPlaneSelection();
  void OnSynchGroupAdded(const GroupSyncIndexType index);

Q_SIGNALS:

  void SynchronizationToggled(QmitkSynchronizedNodeSelectionWidget* synchronizedWidget);
  void SynchGroupChanged(QmitkSynchronizedNodeSelectionWidget* synchronizedWidget, GroupSyncIndexType index);
  void SetDataSelection(const QList<mitk::DataNode::Pointer>& newSelection);

private:

  mitk::BaseRenderer* m_BaseRenderer;
  QmitkSynchronizedNodeSelectionWidget* m_NodeSelectionWidget;
  QPushButton* m_SynchPushButton;
  QComboBox* m_SynchGroupSelector;
  QmitkSliceNavigationWidget* m_SliceNavigationWidget;
  QmitkStepperAdapter* m_StepperAdapter;
  std::unique_ptr<mitk::RenderWindowLayerController> m_RenderWindowLayerController;
  std::unique_ptr<mitk::RenderWindowViewDirectionController> m_RenderWindowViewDirectionController;
  QComboBox* m_ViewDirectionSelector;

  void ChangeViewDirection(const QString& viewDirection);

};

#endif
