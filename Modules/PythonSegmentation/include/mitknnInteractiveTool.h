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

    /** \brief Checks whether this tool can handle the given data.
     *
     * The tool requires valid reference and working data (as checked by the
     * superclass) and additionally requires the reference image to have at
     * most 3 dimensions.
     *
     * \param[in] referenceData The reference image data.
     * \param[in] workingData The working segmentation data.
     *
     * \return \c true if the tool can handle the data, \c false otherwise.
     */
    bool CanHandle(const BaseData* referenceData, const BaseData* workingData) const override;

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

    /** \brief Starts an nnInteractive inference session.
     *
     * Downloads the model checkpoint (if necessary), initializes the
     * inference session on the selected backend (CUDA or CPU), and binds
     * the reference image to the session.
     *
     * If a session is already running, it is ended first. The backend is
     * determined based on CUDA device availability and user preferences.
     *
     * \pre A Python context must have been created via CreatePythonContext().
     * \pre A reference image must be available through the ToolManager.
     *
     * \throw mitk::Exception if the Python session setup fails.
     *
     * \sa EndSession(), IsSessionRunning()
     */
    void StartSession();

    /** \brief Ends the current nnInteractive inference session.
     *
     * Cleans up the Python session, releases model resources, and optionally
     * empties the CUDA cache. This is a no-op if no session is running.
     *
     * \sa StartSession(), IsSessionRunning()
     */
    void EndSession();

    /** \brief Checks whether an nnInteractive session is currently running.
     *
     * \return \c true if a Python context exists and contains a valid
     *         session variable, \c false otherwise.
     */
    bool IsSessionRunning() const;

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

  protected:
    /** \brief Default constructor. Initializes interactors and connects events.
     */
    nnInteractiveTool();

    /** \brief Destructor. Disconnects interactor events.
     */
    ~nnInteractiveTool() override;

    /** \brief Sets the ToolManager and creates an internal ToolManager for interactors.
     *
     * Creates a dedicated internal ToolManager that shares the DataStorage
     * with the provided ToolManager. All interactors are configured to use
     * the internal ToolManager.
     *
     * \param[in] toolManager Pointer to the ToolManager to associate with
     *                        this tool.
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
    class Impl;
    std::unique_ptr<Impl> m_Impl;
  };
}

#endif
