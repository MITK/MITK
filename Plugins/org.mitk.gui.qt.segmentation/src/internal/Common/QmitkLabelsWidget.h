/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKLABELSWIDGET_H
#define QMITKLABELSWIDGET_H

// mitk core
#include <mitkWeakPointer.h>

// Qt
#include <QWidget>

namespace Ui
{
  class QmitkLabelsWidgetControls;
}

namespace mitk
{
  class DataNode;
  class Image;
  class LabelSetImage;
  class ToolManager;
}

class QmitkLabelsWidget : public QWidget
{
  Q_OBJECT

public:

  explicit QmitkLabelsWidget(QWidget* parent = nullptr);
  ~QmitkLabelsWidget() override;

  void UpdateGUI();

  void SetDefaultLabelNaming(bool defaultLabelNaming);

Q_SIGNALS:

  void LabelsChanged();

  void ShowLabelTable(bool);

private:

  mitk::LabelSetImage* GetWorkingImage();

  mitk::DataNode* GetWorkingNode();

  // reaction to button "New Label"
  void OnNewLabel();
  void OnNewLabelShortcutActivated();

  // reaction to the button "Lock exterior"
  void OnLockExterior(bool);

  // reaction to button "Save Preset"
  void OnSavePreset();

  // reaction to button "Load Preset"
  void OnLoadPreset();

  void WaitCursorOn();

  void WaitCursorOff();

  void RestoreOverrideCursor();

  Ui::QmitkLabelsWidgetControls* m_Controls;

  mitk::ToolManager* m_ToolManager;

  bool m_DefaultLabelNaming;

};

#endif
