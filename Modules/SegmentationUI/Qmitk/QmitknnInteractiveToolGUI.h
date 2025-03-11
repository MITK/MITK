/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitknnInteractiveToolGUI_h
#define QmitknnInteractiveToolGUI_h

#include "QmitkSegWithPreviewToolGUIBase.h"
#include <mitknnInteractiveTool.h>

class QButtonGroup;
class QPushButton;

namespace Ui
{
  class QmitknnInteractiveToolGUI;
}

class MITKSEGMENTATIONUI_EXPORT QmitknnInteractiveToolGUI : public QmitkSegWithPreviewToolGUIBase
{
  Q_OBJECT

public:
  mitkClassMacro(QmitknnInteractiveToolGUI, QmitkSegWithPreviewToolGUIBase);
  itkFactorylessNewMacro(Self);

protected:
  using Tool = mitk::nnInteractiveTool::Tool;
  using PromptType = mitk::nnInteractiveTool::PromptType;

  QmitknnInteractiveToolGUI();
  ~QmitknnInteractiveToolGUI() override;

  void InitializeUI(QBoxLayout* mainLayout) override;
  void ThemeIcons();
  void InitializePromptType();
  void InitializeToolButtons();
  void OnInitializeButtonToggled(bool checked);
  void OnResetInteractionsButtonClicked();
  void OnAutoRefineCheckBoxToggled(bool checked);
  void OnAutoZoomCheckBoxToggled(bool checked);
  void OnPromptTypeChanged();
  void OnToolToggled(Tool tool, bool checked);
  void OnMaskButtonClicked();
  void UncheckOtherToolButtons(QPushButton* toolButton);
  void StatusMessageListener(bool isConfirmed);

private:
  mitk::nnInteractiveTool* GetTool();

  Ui::QmitknnInteractiveToolGUI* m_Ui;
  QButtonGroup* m_PromptTypeButtonGroup;
  PromptType m_PromptType;
  std::unordered_map<Tool, QPushButton*> m_ToolButtons;
};

#endif
