/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkVoxTellToolGUI_h
#define QmitkVoxTellToolGUI_h

#include <QmitkMultiLabelSegWithPreviewToolGUIBase.h>
#include <QmitkVenvProcess.h>

#include <mitkIPreferences.h>
#include <mitkVoxTellTool.h>

#include <MitkPythonSegmentationUIExports.h>

#include <memory>
#include <string>
#include <vector>

class QString;

namespace Ui
{
  class QmitkVoxTellToolGUI;
}

/** \brief GUI for mitk::VoxTellTool.
 *
 * A panel with an Initialize button that installs VoxTell if needed and loads
 * its model, a Settings button that opens the preference page, a text field for
 * the prompts, one per line, and a Segment button. The result of a run is
 * shown as a preview with one label per prompt, which the controls of
 * QmitkMultiLabelSegWithPreviewToolGUIBase confirm into the segmentation.
 *
 * \sa mitk::VoxTellTool, QmitkPipInstallDialog
 */
class MITKPYTHONSEGMENTATIONUI_EXPORT QmitkVoxTellToolGUI : public QmitkMultiLabelSegWithPreviewToolGUIBase
{
  Q_OBJECT

public:
  mitkClassMacro(QmitkVoxTellToolGUI, QmitkMultiLabelSegWithPreviewToolGUIBase);
  itkFactorylessNewMacro(Self);

protected slots:

  void OnInitializeButtonToggled(bool checked);
  void OnSettingsButtonClicked();
  void OnSegmentButtonClicked();

protected:
  QmitkVoxTellToolGUI();
  ~QmitkVoxTellToolGUI() override;

  void InitializeUI(QBoxLayout* mainLayout) override;

  /** \brief Enables the controls according to the state of the tool.
   *
   * Besides the busy state of the tool, which the base class handles, the
   * prompts can only be segmented while a model is loaded. The controls stay
   * disabled while the tool computes, even if a label of the preview changes
   * in the meantime and asks for them to be enabled.
   */
  void EnableWidgets(bool enabled) override;

private:
  mitk::VoxTellTool* GetTool();

  /** \brief Makes sure that VoxTell is installed, up to date, and has a Python context.
   *
   * Offers the installation if the package is missing, and an update if the
   * installed version is not supported or a newer one is available.
   *
   * \return \c true if the model can be loaded, \c false if the user gave up.
   */
  bool Install();

  /** \brief Offers an in-place update of the installed package.
   *
   * \return \c true if initialization should proceed (updated successfully, or
   *         the user chose to continue with the installed version), \c false to abort.
   */
  bool OfferInPlaceUpdate(const mitk::PythonPackage::VersionCheckResult& versionCheck);

  /** \brief Runs the pip upgrade dialog and recreates the Python context. */
  bool RunUpdate();

  /** \brief Downloads the files the model needs, then loads the model.
   *
   * The download shows a progress notification. Loading blocks the
   * application, so a message says so first.
   */
  void LoadModel();

  /** \brief Restores the controls after an initialization that did not finish. */
  void OnInitializationAborted();

  /** \brief Downloads files of the Hugging Face Hub with a progress notification.
   *
   * Runs a local event loop, during which this GUI can be deleted.
   *
   * \param[out] output What the download printed, for an error report.
   */
  QmitkVenvProcess::StepResult Download(const std::string& displayName, const std::vector<mitk::VoxTell::RepoFiles>& files, QString& output);

  /** \brief Makes prompts that VoxTell does not know known with its text encoder.
   *
   * Downloads the text encoder if needed, then loads it after a message, since
   * that blocks the application.
   *
   * \return \c false if that failed, was cancelled, or this GUI was deleted
   *         meanwhile, in which case the caller must not touch it.
   */
  bool EmbedUnknownPrompts(const std::vector<std::string>& prompts);

  /** \brief Brings the controls into the state of a model that is loaded. */
  void OnModelLoaded();

  /** \brief Brings the controls into the state of a model that is not loaded. */
  void OnModelUnloaded();

  /** \brief Returns the prompts from the text field: one per line, trimmed,
   *         without empty lines and without repetitions. */
  std::vector<std::string> ReadPrompts() const;

  /** \brief Offers the prompts that VoxTell knows as suggestions in the text field. */
  void UpdatePromptCompletions();

  /** \brief Reverts the toggle of the Initialize button without starting or
   *         ending anything, then updates its label. */
  void UncheckInitializeButton();

  /** \brief Sets the label of the Initialize button to what the next click does. */
  void UpdateInitializeButtonText();

  void SetStatus(const QString& message, bool isError = false);

  void OnPreferenceChangedEvent(const mitk::IPreferences::ChangeEvent& event);

  /** \brief Unloads the model if settings it was loaded with have changed.
   *
   * \return \c true if the model was unloaded.
   */
  bool UnloadIfSettingsChanged();

  std::unique_ptr<Ui::QmitkVoxTellToolGUI> m_Ui;
  mitk::IPreferences* m_Preferences = nullptr;

  /** Whether the user agreed to prompts that VoxTell cannot look up in this session. */
  bool m_UnknownPromptsConfirmed = false;

  /** Whether a download or a load for the tool keeps the controls disabled. */
  bool m_IsBusy = false;

  /** Whether settings that the loaded model depends on changed while it was in use. */
  bool m_SettingsChanged = false;
};

#endif
