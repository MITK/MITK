/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitknnInteractiveTool_h
#define mitknnInteractiveTool_h

#include <mitkSegWithPreviewTool.h>
#include <mitknnInteractiveEnums.h>

#include <exception>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace mitk::nnInteractive
{
  class Interactor;
}

namespace mitk
{
  class ProgressTask;

  /** \brief Segmentation tool integrating the nnInteractive deep learning model.
   *
   * This tool extends SegWithPreviewTool to provide AI-assisted interactive
   * segmentation powered by the nnInteractive Python package. Users can
   * provide prompts through various interaction types (points, boxes,
   * scribbles, contours) to guide the segmentation model.
   *
   * The tool manages:
   * - A set of Interactor instances (one per InteractionType) for user input
   * - An inference session, hosted by a PythonWorker process that runs the
   *   interpreter of the tool's virtual environment
   * - Backend selection (CUDA GPU or CPU)
   *
   * The typical lifecycle is:
   * -# Check the installation of the virtual environment (GetVirtualEnvName())
   *    with a PythonPackage::PackageProbe
   * -# Start a session via StartSession() (downloads and loads the model)
   * -# Enable an interactor via EnableInteractor() for user interaction
   * -# The tool automatically updates the preview segmentation on each interaction
   * -# End the session via EndSession() or tool deactivation
   *
   * \sa SegWithPreviewTool, PythonWorker, nnInteractive::Interactor,
   *     QmitknnInteractiveToolGUI
   */
  class MITKPYTHONSEGMENTATION_EXPORT nnInteractiveTool : public SegWithPreviewTool
  {
  public:
    /** \brief Which interaction types the running session's model checkpoint
     *         supports.
     *
     * Populated from the session's capability metadata
     * (\c supported_interactions and \c supports_initial_label). Used by the
     * GUI to enable or disable the interaction buttons. Applies to both local
     * and remote sessions; a remote server may host a checkpoint with
     * different capabilities than the local default.
     *
     * \sa GetSupportedInteractions()
     */
    struct SupportedInteractions
    {
      bool Point = true;    /**< \brief Point ("points") interaction supported. */
      bool Box = true;      /**< \brief Box ("bbox2d"/"bbox3d") interaction supported. */
      bool Scribble = true; /**< \brief Scribble ("scribble") interaction supported. */
      bool Lasso = true;    /**< \brief Lasso ("lasso") interaction supported. */
      bool Mask = true;     /**< \brief Initial-segmentation mask ("initial_label") supported. */
    };

    /** \brief Why a running session was lost.
     *
     * \sa SessionLostEvent
     */
    struct SessionLoss
    {
      enum class Cause
      {
        RemoteConnection, /**< \brief The lease expired, or the server is at capacity or unreachable. */
        WorkerExited,     /**< \brief The worker process that hosted the session ended unexpectedly. */
        Cancelled         /**< \brief The user cancelled a prompt, which ended the worker process of the session. */
      };

      Cause Reason;
      std::string Details; /**< \brief What was reported, e.g. the exit code and last output of the worker. */
    };

    mitkClassMacro(nnInteractiveTool, SegWithPreviewTool)
    itkFactorylessNewMacro(Self)

    /** \brief Returns the display name of this tool.
     *
     * \return The string "nnInteractive".
     */
    const char* GetName() const override;

    /** \brief Returns the icon resource for this tool.
     *
     * Loads the "AI.svg" icon from the MitkSegmentation module resources.
     *
     * \return The icon as a us::ModuleResource.
     */
    us::ModuleResource GetIconResource() const override;

    /** \brief Called when the tool is deactivated.
     *
     * Disables any active interactor, resets all interactions, and ends the
     * Python session. Emits DeactivatedEvent at the end so observers can
     * run cleanup that must not happen from the destructor path (e.g.
     * removing unused auto-created labels in the GUI).
     *
     * While a prompt computes, the session ends only once the prompt has
     * stopped, after DeactivatedEvent. While StartSession() runs, the start
     * ends itself, and DeactivatedEvent is not emitted.
     *
     * \sa DeactivatedEvent
     */
    void Deactivated() override;

    /** \brief Returns whether unconfirmed results would be lost on deactivation.
     *
     * Honors the RequestDeactivationConfirmation flag like the base class, but
     * takes pending interactions instead of unconfirmed preview content as the
     * signal: DoUpdatePreview() defines the preview label on every call, which
     * keeps the base class flag set even for an empty preview.
     *
     * \return \c true if any interaction or initial mask awaits confirmation.
     *
     * \sa HasInteractions()
     */
    bool ConfirmBeforeDeactivation() override;

    /** \brief Map type associating InteractionType keys to Interactor instances.
     */
    using InteractorMap = std::unordered_map<nnInteractive::InteractionType, std::unique_ptr<nnInteractive::Interactor>>;

    /** \brief Returns a reference to the map of all interactors.
     *
     * \return A const reference to the InteractorMap containing all
     *         registered interactors.
     */
    const InteractorMap& GetInteractors() const;

    /** \brief Returns the interactor for a specific interaction type.
     *
     * \param[in] interactionType The interaction type to look up.
     *
     * \return Pointer to the corresponding Interactor.
     *
     * \throw std::out_of_range if the interaction type is not registered.
     */
    const nnInteractive::Interactor* GetInteractor(nnInteractive::InteractionType interactionType) const;

    /** \brief Enables an interactor for the given interaction and prompt type.
     *
     * Any other currently enabled interactor is automatically disabled first.
     * Only one interactor may be active at a time.
     *
     * \param[in] nextInteractionType The interaction type to enable.
     * \param[in] promptType The prompt type (positive or negative) for the
     *                       interaction.
     *
     * \sa DisableInteractor()
     */
    void EnableInteractor(nnInteractive::InteractionType nextInteractionType, nnInteractive::PromptType promptType);

    /** \brief Disables an interactor.
     *
     * If a specific interaction type is provided, that interactor is disabled.
     * Otherwise, the currently enabled interactor (if any) is disabled.
     *
     * \param[in] interactionType The interaction type to disable. If
     *            \c std::nullopt, the currently enabled interactor is disabled.
     *
     * \sa EnableInteractor()
     */
    void DisableInteractor(std::optional<nnInteractive::InteractionType> interactionType = std::nullopt);

    /** \brief Resets all interactors and clears the current preview.
     *
     * All interaction data (points, boxes, scribbles, contours) is removed
     * and the Python session's interaction state is reset. Also clears any
     * initial segmentation mask.
     */
    void ResetInteractions();

    /** \brief Checks whether any interactor has recorded interactions.
     *
     * Also returns \c true if an initial segmentation mask has been set via
     * InitializeSessionWithMask().
     *
     * \return \c true if any interactions or initial mask exist,
     *         \c false otherwise.
     */
    bool HasInteractions() const;

    /** \brief Reverts the most recent interaction (single-level undo).
     *
     * Calls the nnInteractive session's \c undo(), which restores the
     * prediction to the state before the last interaction (point, box,
     * scribble, lasso, or initial-segmentation mask). On success, the
     * visualization of the undone prompt is removed and the preview is
     * refreshed from the restored target buffer.
     *
     * nnInteractive supports only single-level undo: after one undo, nothing
     * is undoable again until a new interaction is added. This is a no-op when
     * no session is running or CanUndo() is \c false.
     *
     * \sa CanUndo(), SupportsUndo()
     */
    void UndoLastInteraction();

    /** \brief Returns whether an interaction is currently available to undo.
     *
     * \c true after a new interaction has been applied, until it is undone or
     * the interactions are reset. Cheap (no Python call); reflects the tool's
     * record of the last applied interaction.
     *
     * \return \c true if UndoLastInteraction() would revert something.
     *
     * \sa UndoLastInteraction(), SupportsUndo()
     */
    bool CanUndo() const;

    /** \brief Returns whether the running session's backend supports undo.
     *
     * The session's \c supports_undo capability (present on both local and
     * remote sessions). Older nnInteractive versions without undo report
     * \c false, letting the GUI keep the Undo control disabled. Returns
     * \c false when no session is running.
     *
     * \sa UndoLastInteraction()
     */
    bool SupportsUndo() const;

    /** \brief Returns whether auto-zoom is enabled.
     *
     * When auto-zoom is enabled, the nnInteractive model automatically
     * zooms into the region of interest during inference.
     *
     * \return \c true if auto-zoom is enabled, \c false otherwise.
     *
     * \sa SetAutoZoom()
     */
    bool GetAutoZoom() const;

    /** \brief Enables or disables auto-zoom.
     *
     * If a session is currently running, the setting is applied immediately
     * to the Python session.
     *
     * \param[in] autoZoom \c true to enable auto-zoom, \c false to disable.
     *
     * \note Auto-zoom is automatically disabled when falling back to CPU
     *       backend.
     *
     * \sa GetAutoZoom()
     */
    void SetAutoZoom(bool autoZoom);

    /** \brief Returns whether auto-refine is enabled.
     *
     * When auto-refine is enabled, predictions are automatically run on
     * initial segmentation masks provided via InitializeSessionWithMask().
     *
     * \return \c true if auto-refine is enabled, \c false otherwise.
     *
     * \sa SetAutoRefine()
     */
    bool GetAutoRefine() const;

    /** \brief Enables or disables auto-refine.
     *
     * \param[in] autoRefine \c true to enable auto-refine, \c false to
     *                       disable.
     *
     * \sa GetAutoRefine()
     */
    void SetAutoRefine(bool autoRefine);

    /** \brief Returns the currently active computation backend.
     *
     * The backend is determined during StartSession() based on hardware
     * availability and user preferences.
     *
     * \return The active Backend, or \c std::nullopt if no session has been
     *         started.
     */
    std::optional<nnInteractive::Backend> GetBackend() const;

    /** \brief Returns the name of the Python virtual environment used.
     *
     * The virtual environment name matches the tool's display name
     * ("nnInteractive").
     *
     * \return The virtual environment name as a string.
     */
    std::string GetVirtualEnvName() const;

    /** \brief The preferences baked into a session at initialization, each paired
     *         with its default value (as stored).
     *
     * Single source of truth shared by the tool and its GUI:
     * - StartSession() seeds these so the stored values match what the session
     *   actually uses. Otherwise the first preferences "OK" after a fresh install
     *   writes a default into a never-stored key, which mitk::Preferences reports
     *   as a value change and which the GUI would act on by ending the session.
     * - The GUI ends a running session whenever one of these keys changes, since
     *   the session can no longer match the new settings.
     *
     * Live or GUI-only settings (auto-zoom, auto-refine, automation, shortcut
     * labels, install mode) are deliberately excluded.
     */
    static const std::vector<std::pair<std::string, std::string>>& GetSessionDefiningPreferences();

    /** \brief Starts an nnInteractive inference session in the configured mode.
     *
     * Starts a worker process with the interpreter of the virtual environment.
     * In local mode it downloads/loads the model checkpoint and initializes the
     * inference session on a CUDA or CPU backend (determined from CUDA device
     * availability and user preferences). In remote mode it claims a session on
     * the configured nninteractive-server, which provides the model and compute.
     * In both modes the reference image is bound to the session.
     *
     * If a session is already running, it is ended first.
     *
     * The worker downloads and loads off the GUI thread, so this returns before
     * the session has started. A deactivation of the tool or an AbortSession()
     * meanwhile ends the start, and so does cancelling \p task, which kills the
     * worker right away.
     *
     * \param[in] task A cancelable task the user sees, kept until \p done.
     * \param[in] done Receives null once the session runs, or a mitk::Exception
     *            if the session setup fails (local or remote), was cancelled or
     *            was aborted. Runs on the GUI thread, before this returns if
     *            the start fails right away, e.g. while the tool is busy or
     *            no server is configured.
     *
     * \pre nnInteractive must be installed in the virtual environment.
     * \pre A reference image must be available through the ToolManager.
     *
     * \sa EndSession(), IsSessionRunning(), IsRemoteSession()
     */
    void StartSession(std::shared_ptr<ProgressTask> task, std::function<void(std::exception_ptr error)> done);

    /** \brief Ends the current nnInteractive inference session.
     *
     * For a remote session it first releases the server lease (best-effort) so
     * the slot frees up for other users. Then it ends the worker process, which
     * frees the model and all memory of the session. This is a no-op if no
     * session is running.
     *
     * \sa StartSession(), IsSessionRunning()
     */
    void EndSession();

    /** \brief Tears down a session that was lost.
     *
     * Disables the active interactor, ends the (now dead) session, clears all
     * interactions/prompts and the preview, and refreshes the render windows.
     * SessionEndedEvent (emitted by the internal EndSession) lets the GUI
     * revert its session-dependent controls so the user only has to click
     * Initialize to start over.
     *
     * While StartSession() runs or a prompt computes, this only asks them to
     * stop. The teardown follows once they have.
     *
     * \warning Must be called from a clean call stack (e.g. deferred from the
     *          GUI), never from within an interactor's event handling, because
     *          it disables and resets the interactors.
     *
     * \sa SessionLostEvent, EndSession()
     */
    void AbortSession();

    /** \brief Checks whether an nnInteractive session is currently running.
     *
     * Stays \c true for a session whose worker has died until the session is
     * ended, so that AbortSession() can clean up.
     */
    bool IsSessionRunning() const;

    /** \brief Returns whether StartSession() has not ended yet.
     *
     * AbortSession() meanwhile ends the start once its work has stopped.
     */
    bool IsSessionStarting() const;

    /** \brief Tells that a prompt can be cancelled, which ends the session. */
    bool IsCancelable() const override;

    /** \brief Returns whether the running session is a remote (server) session.
     *
     * \return \c true if the current session was started in remote mode (i.e.
     *         it talks to an nninteractive-server), \c false for a local
     *         session or when no session is running.
     *
     * \sa StartSession()
     */
    bool IsRemoteSession() const;

    /** \brief Returns which interaction types the running session supports.
     *
     * Reads the session's capability metadata. If no session is running, all
     * interactions are reported as supported (the defaults).
     *
     * \return A SupportedInteractions struct.
     *
     * \sa SupportedInteractions
     */
    SupportedInteractions GetSupportedInteractions() const;

    /** \brief Returns the running session's model checkpoint license, if any.
     *
     * The nnInteractive session reports it identically for local sessions
     * (first line of the checkpoint's LICENSE file, or a fallback for the
     * official model) and remote sessions (mirrored from the server's
     * /capabilities response). The special value "!!MISSING!!" means the
     * license is unknown. Returns \c std::nullopt when no session is running or
     * no license is reported.
     *
     * \sa GetSupportedInteractions()
     */
    std::optional<std::string> GetModelLicense() const;

    /** \brief Initializes or reinitializes the session with an existing mask.
     *
     * Provides an existing segmentation mask as an initial segmentation to
     * the nnInteractive model, allowing users to refine an existing
     * segmentation. The mask replaces any previous initial segmentation and
     * triggers a preview update.
     *
     * \param[in] mask Pointer to the mask Image to use as initial
     *                 segmentation. If \c nullptr, this is a no-op. A mask
     *                 with several time steps is read at the time step of
     *                 the segmentation that the session works on.
     *
     * \sa ResetInteractions()
     */
    void InitializeSessionWithMask(Image* mask);

    /** \brief Event triggered when the tool requests cleanup confirmation.
     *
     * Emitted when the segmentation is confirmed and the tool needs to
     * clean up interactor data. The boolean parameter indicates whether the
     * cleanup was confirmed.
     */
    Message1<bool> ConfirmCleanUpEvent;

    /** \brief Event triggered after the preview has been updated from a user interaction.
     *
     * Emitted on the GUI thread once the prediction of an update driven by an
     * enabled interactor (i.e., the user placed a point/box/scribble/lasso) has
     * been written to the preview image. That is after DoUpdatePreview() has
     * returned, as the prediction runs off the GUI thread.
     *
     * Not emitted for initial-mask-based updates or reset operations.
     */
    Message<> PreviewUpdatedEvent;

    /** \brief Event triggered at the end of Deactivated().
     *
     * Emitted after the tool has released interactors and ended its Python
     * session, or before the session ends if a prompt still computes (see
     * Deactivated()). GUI code can subscribe to run teardown logic that must happen
     * on a proper user-initiated deactivation (as opposed to the GUI's Qt
     * destructor, which may run during application shutdown when observers
     * and widgets are in a partially-destructed state).
     */
    Message<> DeactivatedEvent;

    /** \brief Event triggered when the session has ended.
     *
     * Emitted from EndSession() after the session has been torn down.
     * Fires both on a normal Deactivated() (in which case DeactivatedEvent
     * follows, or precedes it if a prompt still computed) and when the session is auto-ended because the current image
     * or segmentation time step changed during a 4D session. GUI code can
     * subscribe to revert session-dependent UI controls to their pre-init
     * state.
     */
    Message<> SessionEndedEvent;

    /** \brief Event triggered when a running session was lost.
     *
     * Emitted while the (now dead) session is still nominally running, when an
     * operation failed because the remote lease expired, the server is at
     * capacity or unreachable, the worker process ended, or the user cancelled
     * a prompt, which ends the worker process as well. Teardown has NOT
     * happened yet: the GUI must defer it, calling AbortSession() on the next
     * event-loop tick (teardown must not run inside an interactor's event
     * handling), which then ends the session and fires SessionEndedEvent. Do
     * not assume the session is already ended when this fires.
     *
     * A loss is noticed only by the next operation: nothing watches an idle
     * session, so a remote lease that expires meanwhile is reported when the
     * user interacts again.
     *
     * \sa SessionEndedEvent, AbortSession()
     */
    Message1<const SessionLoss&> SessionLostEvent;

  protected:
    /** \brief Default constructor. Initializes interactors and connects events.
     */
    nnInteractiveTool();

    /** \brief Destructor. Disconnects interactor events.
     */
    ~nnInteractiveTool() override;

    /** \brief Sets the ToolManager and forwards it to all interactors.
     *
     * \param[in] toolManager Pointer to the ToolManager to associate with
     *                        this tool and its interactors.
     */
    void SetToolManager(ToolManager* toolManager) override;

    /** \brief Updates the preview segmentation based on the latest interaction.
     *
     * Dispatches the latest interaction data to the nnInteractive Python
     * model and writes the prediction result into the preview image. The model
     * predicts off the GUI thread.
     *
     * \param[in] inputAtTimeStep The input image at the current time step.
     * \param[in] oldSegAtTimeStep The previous segmentation at the current
     *                             time step (unused).
     * \param[in,out] previewImage The preview segmentation to update with
     *                             the prediction result.
     * \param[in] timeStep The current time step.
     */
    void DoUpdatePreview(const Image* inputAtTimeStep, const Image* oldSegAtTimeStep, MultiLabelSegmentation* previewImage, TimeStepType timeStep) override;

    /** \brief Tears the running session down when the current time step no
     *         longer matches the time step the session was bound to.
     *
     * The Python inference model is intrinsically 3D, so each session is
     * bound to a single 3D slice extracted at the time step that was active
     * when StartSession() ran. If the current image or segmentation time
     * step diverges from that bound time step, all unconfirmed interactions
     * are discarded, the preview is cleared, and the session is ended. This
     * override deliberately does not chain to the base implementation, whose
     * lazy-preview UpdatePreview() call would invoke DoUpdatePreview()
     * against a stale Python binding.
     */
    void OnTimePointChanged() override;

    /** \brief Forwards unhandled interaction events to the enabled interactor.
     *
     * \param[in] event The interaction event to process.
     * \param[in] isHandled Whether the event has already been handled.
     */
    void Notify(InteractionEvent* event, bool isHandled) override;

    /** \brief Called when a confirmed segmentation needs cleanup.
     *
     * Emits the ConfirmCleanUpEvent to notify the GUI.
     */
    void ConfirmCleanUp() override;

    /** \brief Configures a label in the preview segmentation with a given
     *         value and color.
     *
     * Creates the label if it does not yet exist, updates its color, and
     * sets it as the active label.
     *
     * \param[in] value The label pixel value.
     * \param[in] color The color to assign to the label.
     */
    void SetPreviewLabel(MultiLabelSegmentation::LabelValueType value, const Color& color);

    /** \brief Suspends the input of the interactors, so a prompt made while
     *         this one computes is ignored.
     */
    void UpdatePrepare() override;

    /** \brief Resumes the input of the interactors, and ends the session
     *         once a prompt has stopped, if it was asked for meanwhile.
     *
     * The tool can be deactivated, the session aborted or the time point
     * changed while a prompt computes off the GUI thread, which uses the worker.
     */
    void UpdateCleanUp() override;

  private:
    /** \brief Ends the session and clears the interactions and the preview.
     *
     * Runs through AbortSession() once nothing uses the worker: right away, or
     * once a prompt that used it has stopped, also for a deactivation of the
     * tool.
     */
    void TearDownSession();

    /** \brief Reports why a prompt failed: a lost or cancelled session goes to
     *         SessionLostEvent, any other error is rethrown.
     */
    void EndFailedPrompt(std::exception_ptr error, const ProgressTask* task);

    /** \brief If an error means the session is lost, request teardown and
     *         report it as handled.
     *
     * Emits SessionLostEvent (the GUI then tears the session down on the next
     * event-loop tick via AbortSession(), since teardown must not run while an
     * interactor is mid-event) and returns \c true so the caller can swallow
     * the error. Returns \c false for any other error, which the caller should
     * rethrow.
     */
    bool HandleSessionError(const Exception& error);

    class Impl;
    std::unique_ptr<Impl> m_Impl;
  };
}

#endif
