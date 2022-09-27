/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKRENDERWINDOWUTILITYWIDGET_H
#define QMITKRENDERWINDOWUTILITYWIDGET_H

// qt widgets module
#include "MitkQtWidgetsExports.h"
#include "QmitkSliceNavigationWidget.h"
#include "QmitkStepperAdapter.h"
#include "QmitkRenderWindow.h"
#include "QmitkRenderWindowContextDataStorageInspector.h"
#include "mitkRenderWindowViewDirectionController.h"

// mitk core
#include "mitkDataStorage.h"

// qt
#include <QWidget>
#include <QHBoxLayout>
#include <QMenuBar>
#include <QComboBox>

class MITKQTWIDGETS_EXPORT QmitkRenderWindowUtilityWidget : public QWidget
{
	Q_OBJECT

public:
  QmitkRenderWindowUtilityWidget(
    QWidget* parent = nullptr,
    QmitkRenderWindow* renderWindow = nullptr,
    mitk::DataStorage* dataStorage = nullptr
  );

  ~QmitkRenderWindowUtilityWidget() override;

  void SetInvertedSliceNavigation(bool inverted);

Q_SIGNALS:

  void ReinitAction(QList<mitk::DataNode::Pointer> selectedNodes);
  void ResetAction(QList<mitk::DataNode::Pointer> selectedNodes);

private:

  QHBoxLayout* m_Layout;
  QMenuBar* m_MenuBar;

  QmitkRenderWindow* m_RenderWindow;
  mitk::DataStorage* m_DataStorage;

  QmitkRenderWindowContextDataStorageInspector* m_RenderWindowInspector;
  QmitkSliceNavigationWidget* m_SliceNavigationWidget;
  QmitkStepperAdapter* m_StepperAdapter;
  std::unique_ptr<mitk::RenderWindowViewDirectionController> m_RenderWindowViewDirectionController;
  QComboBox* m_ViewDirectionSelector;

  void ChangeViewDirection(const QString& viewDirection);
};

#endif // QMITKRENDERWINDOWUTILITYWIDGET_H
