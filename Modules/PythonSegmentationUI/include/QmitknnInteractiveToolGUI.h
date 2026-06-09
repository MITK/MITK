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
#include <mitkIPreferences.h>
#include <mitkLabelSetImage.h>
#include <mitknnInteractiveTool.h>
#include <mitkWeakPointer.h>
#include <MitkPythonSegmentationUIExports.h>

#include <QString>

#include <memory>
#include <optional>
#include <vector>

class QAbstractButton;
class QButtonGroup;
class QPushButton;
class QTimer;

namespace Ui
{
  class QmitknnInteractiveToolGUI;
}

/** \brief GUI widget for the nnInteractiveTool segmentation tool.
 *
 * This class provides the user interface for the nnInteractive segmentation
 * tool. It extends QmitkSegWithPreviewToolGUIBase and manages:
 *
 * - Session initialization and settings (initialize button, settings)
 * - Prompt type selection (positive / negative)
 * - Interactor mode buttons (point, box, scribble, lasso)
 * - Auto-zoom and auto-refine options
 * - Mask-based session initialization
 * - Automation modes (auto-confirm after a single interaction;
 *   auto-create-next-label on confirm, with an optional bypass of the
 *   global label-naming preferences)
 * - Optional shortcut hints in button labels
 * - Keyboard shortcuts for common actions (R: reset, C: confirm,
 *   T: toggle prompt type, P/B/S/L: toggle interactors)
 *
 * \sa mitk::nnInteractiveTool, QmitkSegWithPreviewToolGUIBase,
 *     QmitkPipInstallDialog
 */
class MITKPYTHONSEGMENTATIONUI_EXPORT QmitknnInteractiveToolGUI : public QmitkSegWithPreviewToolGUIBase
{
  Q_OBJECT

public:
  mitkClassMacro(QmitknnInteractiveToolGUI, QmitkSegWithPreviewToolGUIBase);
  itkFactorylessNewMacro(Self);

protected:
  /** \brief Alias for mitk::nnInteractive::InteractionType. */
  using InteractionType = mitk::nnInteractive::InteractionType;

  /** \brief Alias for mitk::nnInteractive::PromptType. */
  using PromptType = mitk::nnInteractive::PromptType;

  /** \brief Constructs the GUI with confirm segmentation button disabled by default.
   */
  QmitknnInteractiveToolGUI();

  /** \brief Destructor. Restores any override cursors and disconnects events.
   */
  ~QmitknnInteractiveToolGUI() override;

  /** \brief Initializes the UI widgets and connects signals.
   *
   * Sets up the form from the .ui file, initializes icons, prompt type
   * buttons, interactor buttons, checkboxes, keyboard shortcuts, and
   * connects all signal/slot connections.
   *
   * \param[in] mainLayout The parent layout to add the GUI widgets to.
   */
  void InitializeUI(QBoxLayout* mainLayout) override;

  /** \brief Initializes the prompt type button group and shortcut.
   */
  void InitializePromptType();

  /** \brief Initializes the interactor toggle buttons with icons and shortcuts.
   */
  void InitializeInteractorButtons();

  /** \brief Enables or disables the initialize and settings buttons.
   *
   * \param[in] enabled Whether the buttons should be enabled.
   */
  void EnableInitializeButtons(bool enabled);

  /** \brief Handles the initialize button toggle event.
   *
   * Creates the virtual environment, Python context, and installs
   * nnInteractive if needed, then starts the inference session.
   *
   * \param[in] checked Whether the button is checked (unused).
   */
  void OnInitializeButtonToggled(bool checked);

  /** \brief Opens the nnInteractive preferences dialog.
   */
  void OnSettingsButtonClicked();

  /** \brief Resets all interactions and switches to positive prompt type.
   *
   * Also clears the auto-created-label tracking via
   * InvalidateAutoCreatedLabel() so the next deactivation does not try to
   * remove a label the user has effectively re-claimed by resetting.
   */
  void OnResetInteractionsButtonClicked();

  /** \brief Handles auto-refine checkbox toggle.
   *
   * \param[in] checked Whether auto-refine should be enabled.
   */
  void OnAutoRefineCheckBoxToggled(bool checked);

  /** \brief Handles auto-zoom checkbox toggle.
   *
   * \param[in] checked Whether auto-zoom should be enabled.
   */
  void OnAutoZoomCheckBoxToggled(bool checked);

  /** \brief Handles prompt type changes between positive and negative.
   *
   * Re-enables the currently active interactor with the new prompt type
   * and updates the cursor accordingly.
   */
  void OnPromptTypeChanged();

  /** \brief Handles toggling of an interactor button.
   *
   * When checked, enables the corresponding interactor and sets an
   * appropriate cursor. When unchecked, disables the interactor and
   * restores the default cursor.
   *
   * \param[in] interactionType The interaction type being toggled.
   * \param[in] checked Whether the interactor should be enabled.
   */
  void OnInteractorToggled(mitk::nnInteractive::InteractionType interactionType, bool checked);

  /** \brief Handles the mask initialization button click.
   *
   * Prompts the user to confirm initialization with the active label of
   * the working segmentation, then resets interactions and initializes
   * the session with the label mask.
   */
  void OnMaskButtonClicked();

  /** \brief Handles cleanup confirmation from the tool.
   *
   * \param[in] isConfirmed Whether the cleanup was confirmed.
   */
  void OnConfirmCleanUp(bool isConfirmed);

  /** \brief Handles the tool's PreviewUpdatedEvent.
   *
   * When "auto-confirm" is enabled, schedules an auto-click of the Confirm
   * button on the next event-loop tick. No-op otherwise.
   */
  void OnPreviewUpdated();

  /** \brief Handles the tool's DeactivatedEvent.
   *
   * Runs on a proper user-initiated tool deactivation (as opposed to the
   * GUI's Qt destructor, which may fire during application shutdown when
   * observers and widgets are partially destroyed). Performs the unused-
   * auto-label cleanup here so we never touch the segmentation from the
   * destructor path.
   */
  void OnToolDeactivated();

  /** \brief Handles the tool's SessionEndedEvent.
   *
   * Reverts the session-dependent UI controls (interactor toggles, prompt
   * type, reset, settings) back to their pre-init state and re-enables the
   * Initialize button. Fires when the user changes the time step during a
   * 4D session, and also as part of a normal tool deactivation (where the
   * extra UI revert is harmless because the GUI is being torn down).
   */
  void OnSessionEnded();

  /** \brief Handles the tool's SessionExpiredEvent.
   *
   * A remote session was lost server-side (idle timeout, server restart, or
   * the server is at capacity / unreachable). The tool has already ended the
   * session, so this only informs the user that they need to re-initialize.
   * The message box is shown deferred to avoid re-entrancy when the event
   * fires from within an interaction.
   */
  void OnSessionExpired();

  /** \brief Sends a keep-alive heartbeat to the remote nnInteractive server.
   *
   * Connected to the heartbeat timer's timeout while a remote session runs.
   * Delegates to nnInteractiveTool::Heartbeat(), which keeps the server-side
   * lease alive and, on a definitive expiry, triggers the SessionExpiredEvent
   * teardown path. Has no effect for local sessions.
   */
  void OnHeartbeatTimeout();

  /** \brief Shows the model checkpoint license below the Initialize button, or
   *         clears it when passed \c std::nullopt.
   *
   * Mirrors the napari plugin: a normal license string is shown as
   * "Model license: <value>", the "!!MISSING!!" sentinel is shown as a red
   * "Model license: UNKNOWN (warning!)", and an empty/absent license hides the
   * label. Driven by the session lifecycle, so it works for local and remote
   * sessions alike.
   */
  void UpdateModelLicenseDisplay(const std::optional<std::string>& license);

  /** \brief Returns the connected nnInteractiveTool.
   *
   * \return Pointer to the connected nnInteractiveTool.
   */
  mitk::nnInteractiveTool* GetTool();

  /** \brief Unchecks all interactor buttons except the specified one.
   *
   * \param[in] interactorButton The button to keep checked, or \c nullptr
   *                             to uncheck all.
   */
  void UncheckOtherInteractorButtons(QPushButton* interactorButton);

  /** \brief Checks if nnInteractive is installed and shows the install dialog if not.
   *
   * Creates the virtual environment and installs all required packages.
   *
   * \return \c true if nnInteractive is installed (or was successfully
   *         installed), \c false otherwise.
   */
  bool Install();

private:
  /** \brief Reads the "auto-create next label" preference. Not cached. */
  bool IsAutoCreateNextLabelEnabled() const;

  /** \brief Reads the "auto-confirm after single interaction" preference. Not cached. */
  bool IsAutoConfirmEnabled() const;

  /** \brief Reads the "skip naming prompt on auto-create" sub-preference. Not cached.
   *
   * When true, AutoCreateAndSelectNewLabel() bypasses the global "default
   * label naming" and "enforce suggestions" preferences for this flow only.
   */
  bool IsNamingPromptSkippedOnAutoCreate() const;

  /** \brief Reads the "show shortcuts in button labels" preference. Not cached. */
  bool AreShortcutsShownInLabels() const;

  /** \brief Decorates each shortcut-bound widget's text with its key, or
   *         restores the plain base text, based on the current preference.
   *
   * Idempotent: labels are always derived from the cached base text in
   * \c m_ShortcutLabels, never from the widget's current text.
   */
  void ApplyShortcutLabels();

  /** \brief Reacts to preference node changes and refreshes shortcut labels
   *         when the relevant key changes. Other keys are ignored.
   */
  void OnPreferenceChangedEvent(const mitk::IPreferences::ChangeEvent& event);

  /** \brief Creates a new label in the working segmentation and selects it.
   *
   * The new label is placed into the group of the previously-active label.
   * Tracks the new label value and segmentation so it can be removed on
   * tool deactivation if it remains unused.
   *
   * \return \c true if a new label was created, \c false otherwise (a
   *         precondition was not met, or the user canceled the rename
   *         dialog when "Ignore label naming preferences" is off).
   */
  bool AutoCreateAndSelectNewLabel();

  /** \brief Stops tracking the auto-created label without removing it. */
  void InvalidateAutoCreatedLabel();

  /** \brief Callback fired via itk::DeleteEvent when the tracked segmentation
   *         is destroyed. Clears the tracking state so we never dereference a
   *         dangling pointer in the destructor.
   */
  void OnAutoCreatedSegmentationDeleted();

  /** \brief Re-checks the last-active interactor button, if any. */
  void ReEnableLastInteractor();

  /** \brief Enables or disables the interaction buttons based on the running
   *         session's reported capabilities.
   *
   * Applies to both local and remote sessions. A remote server may host a
   * checkpoint that supports a different set of interactions than the local
   * default, so unsupported interactions are disabled in the GUI.
   */
  void ApplyCapabilityGating();

  /** \brief Updates the Initialize button label to reflect the configured
   *         inference mode, e.g. "Initialize (local)" or
   *         "Initialize (remote server)", so the active mode is visible in the
   *         tool panel. Reads the preference fresh; called on init and whenever
   *         the inference-mode preference changes.
   */
  void UpdateInitializeButtonText();

  struct ShortcutLabel
  {
    QPushButton* button;
    Qt::Key key;
    QString baseText;
  };

  std::unique_ptr<Ui::QmitknnInteractiveToolGUI> m_Ui;
  QButtonGroup* m_PromptTypeButtonGroup;
  PromptType m_PromptType;
  std::unordered_map<InteractionType, QPushButton*> m_InteractorButtons;

  std::optional<mitk::MultiLabelSegmentation::LabelValueType> m_AutoCreatedLabelValue;
  std::optional<mitk::MultiLabelSegmentation::LabelValueType> m_PreviousActiveLabelValue;
  mitk::WeakPointer<mitk::MultiLabelSegmentation> m_AutoCreatedLabelSegmentation;
  QAbstractButton* m_LastInteractorButton = nullptr;
  bool m_AutoConfirmInProgress = false;

  QTimer* m_HeartbeatTimer = nullptr;

  // Set when a lost remote session is being torn down so a second
  // SessionExpiredEvent (the tool emits it from both the heartbeat and a
  // mid-interaction failure) cannot queue a duplicate teardown/dialog. Cleared
  // in OnSessionEnded once teardown completes.
  bool m_SessionExpiredHandled = false;

  mitk::IPreferences* m_Preferences = nullptr;
  std::vector<ShortcutLabel> m_ShortcutLabels;
  QString m_PromptTypeBaseTitle;
};

#endif
