/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkTotalSegmentatorToolGUI_h
#define QmitkTotalSegmentatorToolGUI_h

#include <QmitkMultiLabelSegWithPreviewToolGUIBase.h>

#include <mitkIPreferences.h>

#include <MitkPythonSegmentationUIExports.h>

#include <memory>
#include <string>
#include <vector>

namespace Ui
{
  class QmitkTotalSegmentatorToolGUI;
}

namespace mitk
{
  class TotalSegmentatorTool;
}

/**
  \brief GUI for mitk::TotalSegmentatorTool.

  A minimal panel: an Install button shown only until the managed environment
  exists, a Settings button opening the preference page, a task combo box (with
  fast / fastest variants for the whole-body tasks and disabled-but-visible
  licensed tasks) and a Run button. The long-running segmentation is executed by
  a subprocess through RunProcess() so the application stays responsive and the
  run can be cancelled.

  \sa mitk::TotalSegmentatorTool
*/
class MITKPYTHONSEGMENTATIONUI_EXPORT QmitkTotalSegmentatorToolGUI : public QmitkMultiLabelSegWithPreviewToolGUIBase
{
  Q_OBJECT

public:
  mitkClassMacro(QmitkTotalSegmentatorToolGUI, QmitkMultiLabelSegWithPreviewToolGUIBase);
  itkFactorylessNewMacro(Self);

protected slots:

  void OnRunButtonClicked();
  void OnInstallButtonClicked();
  void OnSettingsButtonClicked();

protected:
  QmitkTotalSegmentatorToolGUI();
  ~QmitkTotalSegmentatorToolGUI() override;

  void InitializeUI(QBoxLayout* mainLayout) override;

private:
  mitk::TotalSegmentatorTool* GetTool();

  /** \brief Shows the Install button or the task/run controls based on whether
   *         the managed environment is present. */
  void RefreshInstallState();

  /** \brief Fills the task combo box, marking licensed tasks and preserving the
   *         current selection across rebuilds. */
  void PopulateTasks();

  /** \brief Enables the Run button and sets the status for the selected task, or
   *         disables it with a hint when the task needs a license that is not set. */
  void UpdateRunButtonState();

  /** \brief Runs the TotalSegmentator CLI as a subprocess, keeping the GUI
   *         responsive and offering cancellation. Injected into the tool as its
   *         CommandRunner. Returns true on success (exit 0). */
  bool RunProcess(const std::string& executable, const std::vector<std::string>& args);

  void SetStatus(const QString& message, bool isError = false);

  void OnPreferenceChangedEvent(const mitk::IPreferences::ChangeEvent& event);

  std::unique_ptr<Ui::QmitkTotalSegmentatorToolGUI> m_Ui;
  mitk::IPreferences* m_Preferences = nullptr;
};

#endif
