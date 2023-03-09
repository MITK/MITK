/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkLayersWidget_h
#define QmitkLayersWidget_h

// Qt
#include <QWidget>

namespace Ui
{
  class QmitkLayersWidgetControls;
}

namespace mitk
{
  class DataNode;
  class LabelSetImage;
  class ToolManager;
}

class QmitkLayersWidget : public QWidget
{
  Q_OBJECT

public:

  explicit QmitkLayersWidget(QWidget* parent = nullptr);
  ~QmitkLayersWidget() override;

  void UpdateGUI();

Q_SIGNALS:

  void LayersChanged();

private:

  mitk::LabelSetImage* GetWorkingImage();

  mitk::DataNode* GetWorkingNode();

  // reaction to the button "Add Layer"
  void OnAddLayer();

  // reaction to the button "Delete Layer"
  void OnDeleteLayer();

  // reaction to the button "Previous Layer"
  void OnPreviousLayer();

  // reaction to the button "Next Layer"
  void OnNextLayer();

  // reaction to the combobox change "Change Layer"
  void OnChangeLayer(int);

  void WaitCursorOn();

  void WaitCursorOff();

  void RestoreOverrideCursor();

  Ui::QmitkLayersWidgetControls* m_Controls;

  mitk::ToolManager* m_ToolManager;

};

#endif
