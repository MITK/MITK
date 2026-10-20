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
#include <mitkTotalSegmentatorTool.h>

#include <MitkPythonSegmentationUIExports.h>

#include <QStringList>

#include <memory>

class QListWidgetItem;

namespace Ui
{
  class QmitkTotalSegmentatorToolGUI;
}

/**
  \brief GUI for mitk::TotalSegmentatorTool.

  An Install button shown only until the managed environment exists, a
  Settings button opening the preference page, a task combo box (with the
  coarser speeds of the tasks that have them, and visible licensed tasks), a
  checkable list of the structures to segment for the tasks that take a subset,
  and a Run button. Task, speed and structures are the tool's, so the panel
  follows the tool when someone else chooses them, and its status follows
  every run of the tool, whoever started it.

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
  void OnTaskSelected();
  void OnStructureChanged(QListWidgetItem* item);
  void OnClearStructuresClicked();
  void OnStructuresFilterChanged(const QString& text);

protected:
  QmitkTotalSegmentatorToolGUI();
  ~QmitkTotalSegmentatorToolGUI() override;

  void InitializeUI(QBoxLayout* mainLayout) override;
  void ConnectNewTool(mitk::SegWithPreviewTool* newTool) override;
  void DisconnectOldTool(mitk::SegWithPreviewTool* oldTool) override;

  /** \brief Shows that a run goes on, and its result once it ended. */
  void BusyStateChanged(bool isBusy) override;

  /** \brief Shortens the structure summary again when its label is resized. */
  bool eventFilter(QObject* watched, QEvent* event) override;

private:
  mitk::TotalSegmentatorTool* GetTool();

  /** \brief Shows the Install button, or the controls of the tool, which is started if it is not yet. */
  void RefreshInstallState();

  /** \brief Starts the tool, which learns the tasks of TotalSegmentator. */
  void InitializeTool();

  void OnToolInitialized();
  void OnToolInitializationFailed(std::string message);
  void OnToolOptionsChanged();

  /** \brief Fills the task combo box with the tool's tasks, marking licensed ones. */
  void PopulateTasks();

  /** \brief Shows the task, speed and structures of the tool. */
  void ShowToolOptions();

  /** \brief Shows the structures of the tool's task, or hides them for a task that segments all of its structures only. */
  void ShowStructures();

  /** \brief Shows or hides the structure summary, its Clear button, the filter and the list together. */
  void SetStructuresVisible(bool visible);

  /** \brief Names as many checked structures as fit the summary label, and counts the rest; the tool tip names all. */
  void ShowStructuresSummary();

  /** \brief Enables the Run button and sets the status for the selected task, or
   *         disables it with a hint when the task needs a license that is not set. */
  void UpdateRunButtonState();

  /** \brief Shows the preview of the run that just ended, or why there is none. */
  void ShowRunResult();

  void SetStatus(const QString& message, bool isError = false);

  void OnPreferenceChangedEvent(const mitk::IPreferences::ChangeEvent& event);

  std::unique_ptr<Ui::QmitkTotalSegmentatorToolGUI> m_Ui;
  mitk::IPreferences* m_Preferences = nullptr;
  bool m_Running = false;

  /** \brief Whether Run started the tool, which runs once it has started. */
  bool m_RunAfterInitialization = false;

  /** \brief The task whose structures the list shows. */
  std::string m_ListedTask;

  /** \brief The structures of the tool's subset, in the order of the task. */
  QStringList m_CheckedStructures;
};

#endif
