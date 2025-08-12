/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitknnInteractiveToolGUI_h
#define QmitknnInteractiveToolGUI_h

#include <QmitkSegWithPreviewToolGUIBase.h>
#include <mitknnInteractiveTool.h>
#include <MitkPythonSegmentationUIExports.h>

class QButtonGroup;
class QPushButton;

namespace Ui
{
  class QmitknnInteractiveToolGUI;
}

class MITKPYTHONSEGMENTATIONUI_EXPORT QmitknnInteractiveToolGUI : public QmitkSegWithPreviewToolGUIBase
{
  Q_OBJECT

public:
  mitkClassMacro(QmitknnInteractiveToolGUI, QmitkSegWithPreviewToolGUIBase);
  itkFactorylessNewMacro(Self);

protected:
  using InteractionType = mitk::nnInteractive::InteractionType;
  using PromptType = mitk::nnInteractive::PromptType;

  QmitknnInteractiveToolGUI();
  ~QmitknnInteractiveToolGUI() override;

  void InitializeUI(QBoxLayout* mainLayout) override;
  void InitializePromptType();
  void InitializeInteractorButtons();

  void OnInitializeButtonToggled(bool checked);
  void OnResetInteractionsButtonClicked();
  void OnAutoRefineCheckBoxToggled(bool checked);
  void OnAutoZoomCheckBoxToggled(bool checked);
  void OnPromptTypeChanged();
  void OnInteractorToggled(mitk::nnInteractive::InteractionType interactionType, bool checked);
  void OnMaskButtonClicked();
  void OnConfirmCleanUp(bool isConfirmed);

  mitk::nnInteractiveTool* GetTool();
  void UncheckOtherInteractorButtons(QPushButton* interactorButton);

  bool CreateVirtualEnv();
  bool Install();

private:
  Ui::QmitknnInteractiveToolGUI* m_Ui;
  QButtonGroup* m_PromptTypeButtonGroup;
  PromptType m_PromptType;
  std::unordered_map<InteractionType, QPushButton*> m_InteractorButtons;
};

#endif
