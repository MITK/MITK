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

#include <optional>

namespace mitk::nnInteractive
{
  class Interactor;
}

namespace mitk
{
  class PythonContext;

  /** \brief Segmentation tool integrating the nnInteractive deep learning model.
   *
   * This tool extends SegWithPreviewTool to provide AI-assisted interactive
   * segmentation powered by the nnInteractive Python package. Users can
   * provide prompts through various interaction types (points, boxes,
   * scribbles, contours) to guide the segmentation model.
   *
   * The tool manages:
   * - A set of Interactor instances (one per InteractionType) for user input
   * - A Python session that hosts the nnInteractive inference model
   * - A virtual environment for Python dependency isolation
   * - Backend selection (CUDA GPU or CPU)
   *
   * The typical lifecycle is:
   * -# Create a Python context via CreatePythonContext()
   * -# Start a session via StartSession() (downloads and loads the model)
   * -# Enable an interactor via EnableInteractor() for user interaction
   * -# The tool automatically updates the preview segmentation on each interaction
   * -# End the session via EndSession() or tool deactivation
   *
   * \sa SegWithPreviewTool, PythonContext, nnInteractive::Interactor,
   *     QmitknnInteractiveToolGUI
   */
  class MITKPYTHONSEGMENTATION_EXPORT nnInteractiveTool : public SegWithPreviewTool
  {
  public:
    /** \brief Information about a CUDA-capable GPU device.
     */
    struct CUDADeviceInfo
    {
      std::string Name;     /**< \brief Device name (e.g., "NVIDIA GeForce RTX 3080"). */
      int Major;            /**< \brief CUDA compute capability major version. */
      int Minor;            /**< \brief CUDA compute capability minor version. */
      int TotalMemoryMB;    /**< \brief Total device memory in megabytes. */
    };

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
     * \sa DeactivatedEvent
     */
    void Deactivated() override;

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

    /** \brief Creates and activates a Python context with a virtual environment.
     *
     * The virtual environment name is determined by GetVirtualEnvName().
     *
     * \return \c true if the Python context was successfully created,
     *         \c false if an error occurred.
     *
     * \sa GetPythonContext(), PythonContext
     */
    bool CreatePythonContext();

    /** \brief Returns the current Python context.
     *
     * \return Pointer to the PythonContext, or \c nullptr if none has been
     *         created yet.
     *
     * \sa CreatePythonContext()
     */
    PythonContext* GetPythonContext() const;

    /** \brief Checks whether the nnInteractive Python package is installed.
     *
     * Uses Python's importlib to check for the presence of the nnInteractive
     * package in the active virtual environment.
     *
     * \pre A Python context must have been created via CreatePythonContext().
     *
     * \return \c true if the package is installed, \c false otherwise.
     */
    bool IsInstalled() const;

    /** \brief Queries CUDA device information via PyTorch.
     *
     * Checks for CUDA availability and retrieves device properties from the
     * GPU device specified in the application preferences.
     *
     * \param[out] info The CUDADeviceInfo struct to populate with device
     *                  information.
     *
     * \pre A Python context must have been created via CreatePythonContext().
     *
     * \return \c true if a CUDA device was found and info was populated,
     *         \c false otherwise.
     */
    bool GetCUDADeviceInfo(CUDADeviceInfo& info) const;

    /** \brief Starts an nnInteractive inference session in the configured mode.
     *
     * In local mode it downloads/loads the model checkpoint and initializes the
     * inference session on a CUDA or CPU backend (determined from CUDA device
     * availability and user preferences). In remote mode it claims a session on
     * the configured nninteractive-server, which provides the model and compute.
     * In both modes the reference image is bound to the session.
     *
     * If a session is already running, it is ended first.
     *
     * \pre A Python context must have been created via CreatePythonContext().
     * \pre A reference image must be available through the ToolManager.
     *
     * \throw mitk::Exception if the session setup fails (local or remote).
     *
     * \sa EndSession(), IsSessionRunning(), IsRemoteSession()
     */
    void StartSession();

    /** \brief Ends the current nnInteractive inference session.
     *
     * For a local session it cleans up the Python session, releases model
     * resources, and optionally empties the CUDA cache. For a remote session it
     * releases the server lease (best-effort and idempotent) so the slot frees up
     * for other users. This is a no-op if no session is running.
     *
     * \sa StartSession(), IsSessionRunning()
     */
    void EndSession();

    /** \brief Tears down a remote session whose connection was lost.
     *
     * Disables the active interactor, ends the (now dead) Python session,
     * clears all interactions/prompts and the preview, and refreshes the
     * render windows. SessionEndedEvent (emitted by the internal EndSession)
     * lets the GUI revert its session-dependent controls so the user only has
     * to click Initialize to reconnect.
     *
     * \warning Must be called from a clean call stack (e.g. deferred from the
     *          GUI), never from within an interactor's event handling, because
     *          it disables and resets the interactors.
     *
     * \sa SessionExpiredEvent, EndSession()
     */
    void AbortSession();

    /** \brief Checks whether an nnInteractive session is currently running.
     *
     * \return \c true if a Python context exists and contains a valid
     *         session variable, \c false otherwise.
     */
    bool IsSessionRunning() const;

    /** \brief Returns whether the running session is a remote (server) session.
     *
     * \return \c true if the current session was started in remote mode (i.e.
     *         it talks to an nninteractive-server), \c false for a local
     *         in-process session or when no session is running.
     *
     * \sa StartSession()
     */
    bool IsRemoteSession() const;

    /** \brief Sends a single keep-alive heartbeat for a running remote session.
     *
     * Remote nnInteractive sessions are reaped server-side after a liveness
     * timeout unless the client periodically proves it is alive. The client
     * library ships its own background heartbeat daemon, but that Python thread
     * cannot run while MITK is idle: the embedded interpreter holds the GIL on
     * the Qt main thread between PythonContext::Execute() calls, so the daemon
     * never gets to beat. This method drives the heartbeat from the Qt event
     * loop instead and must be called periodically (see GetHeartbeatIntervalMs)
     * by the GUI for as long as a remote session is running.
     *
     * A transient transport error is tolerated (the next beat retries), matching
     * the library's own heartbeat loop. A definitive lease expiry emits
     * SessionExpiredEvent so the GUI tears the dead session down. This is a no-op
     * for local sessions or when no session is running, and never throws (it is
     * invoked from a Qt timer slot).
     *
     * \sa GetHeartbeatIntervalMs(), SessionExpiredEvent
     */
    void Heartbeat();

    /** \brief Returns the interval, in milliseconds, at which Heartbeat() should
     *         be called for the running remote session.
     *
     * Derived from the server-provided liveness timeout (half of it, mirroring
     * the client library's own cadence). Returns \c 0 for local sessions, when
     * no session is running, or when the server has disabled the liveness
     * timeout (no heartbeat needed). The GUI starts a timer only when this is
     * greater than zero.
     *
     * \sa Heartbeat()
     */
    int GetHeartbeatIntervalMs() const;

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
     * Reads the nnInteractive session's \c license attribute, populated
     * identically for local sessions (first line of the checkpoint's LICENSE
     * file, or a fallback for the official model) and remote sessions (mirrored
     * from the server's /capabilities response). The special value
     * "!!MISSING!!" means the license is unknown. Returns \c std::nullopt when
     * no session is running or no license is reported. This is a cached local
     * attribute read (no network call).
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
     *                 segmentation. If \c nullptr, this is a no-op.
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
     * Emitted at the end of DoUpdatePreview() when the update was driven by
     * an enabled interactor (i.e., the user placed a point/box/scribble/lasso)
     * and the prediction result has been written to the preview image.
     *
     * Not emitted for initial-mask-based updates or reset operations.
     */
    Message<> PreviewUpdatedEvent;

    /** \brief Event triggered at the end of Deactivated().
     *
     * Emitted after the tool has released interactors and ended its Python
     * session. GUI code can subscribe to run teardown logic that must happen
     * on a proper user-initiated deactivation (as opposed to the GUI's Qt
     * destructor, which may run during application shutdown when observers
     * and widgets are in a partially-destructed state).
     */
    Message<> DeactivatedEvent;

    /** \brief Event triggered when the Python session has ended.
     *
     * Emitted from EndSession() after the Python session has been torn down.
     * Fires both on a normal Deactivated() (in which case DeactivatedEvent
     * follows) and when the session is auto-ended because the current image
     * or segmentation time step changed during a 4D session. GUI code can
     * subscribe to revert session-dependent UI controls to their pre-init
     * state.
     */
    Message<> SessionEndedEvent;

    /** \brief Event triggered when a remote session was lost server-side.
     *
     * Emitted while the (now dead) remote session is still nominally running,
     * when a remote operation or the heartbeat failed because the lease expired
     * or the server is at capacity / unreachable. Teardown has NOT happened yet:
     * the GUI must defer it, calling AbortSession() on the next event-loop tick
     * (teardown must not run inside an interactor's event handling), which then
     * ends the session and fires SessionEndedEvent. Do not assume the session is
     * already ended when this fires.
     *
     * \sa SessionEndedEvent, AbortSession()
     */
    Message<> SessionExpiredEvent;

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
     * model and writes the prediction result into the preview image.
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

  private:
    /** \brief Constructs a local in-process inference session.
     *
     * Detects the backend (CUDA/CPU), resolves the model checkpoint, and
     * instantiates the local nnInteractiveInferenceSession. Called by
     * StartSession() when inference mode is "local".
     */
    void ConstructLocalSession();

    /** \brief Constructs a remote inference session against an nninteractive-server.
     *
     * Reads the server URL and API key from the preferences and instantiates
     * nnInteractiveRemoteInferenceSession (which claims a session on the
     * server). Called by StartSession() when inference mode is "remote".
     *
     * \throw mitk::Exception if no server URL is configured or the server
     *        cannot be reached / is at capacity.
     */
    void ConstructRemoteSession();

    /** \brief Allocates the target buffer, binds the reference image and target
     *         buffer to the Python session, and pins the session to the current
     *         time step.
     *
     * Shared by ConstructLocalSession() and ConstructRemoteSession(). For
     * remote sessions the numpy target buffer is handed to the session
     * directly (the server mirrors prediction diffs into it in place); for
     * local sessions it is wrapped in a torch tensor as before.
     */
    void BindSessionImageAndTargetBuffer();

    /** \brief Does this error indicate a lost or failed remote connection?
     *
     * Remote session calls are wrapped so that httpx transport and status
     * errors (httpx.HTTPError, the base of all of them) and the typed lease
     * errors are caught by type and re-raised as a single stable sentinel.
     * This checks for that sentinel, so detection does not depend on httpx or
     * OS error wording. Always \c false for local sessions.
     *
     * \param[in] message The description of a caught mitk::Exception.
     */
    bool IsRemoteConnectionError(const std::string& message) const;

    /** \brief If an error indicates a lost remote connection, request teardown
     *         and report it as handled.
     *
     * Emits SessionExpiredEvent (the GUI then tears the session down on the
     * next event-loop tick via AbortSession(), since teardown must not run
     * while an interactor is mid-event) and returns \c true so the caller can
     * swallow the error. Returns \c false for any other error, which the
     * caller should rethrow.
     *
     * \param[in] errorMessage The description of a caught mitk::Exception.
     */
    bool HandleSessionError(const std::string& errorMessage);

    class Impl;
    std::unique_ptr<Impl> m_Impl;
  };
}

#endif
