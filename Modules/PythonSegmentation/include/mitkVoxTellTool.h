/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkVoxTellTool_h
#define mitkVoxTellTool_h

#include <mitkPythonPackageVersion.h>
#include <mitkSegWithPreviewTool.h>
#include <mitkTorchDevice.h>
#include <MitkPythonSegmentationExports.h>

#include <nlohmann/json_fwd.hpp>

#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace mitk
{
  class ProgressTask;
  class PythonWorker;

  namespace VoxTell
  {
    /** \brief Name of the managed virtual environment that VoxTell is installed into. */
    inline constexpr const char* VENV_NAME = "VoxTell";

    /** \brief Name of the pip distribution that provides VoxTell. */
    inline constexpr const char* DISTRIBUTION_NAME = "voxtell";

    /** \brief Returns the VoxTell versions this MITK build supports.
     *
     * VoxTell is below version 1.0, where a new minor release may change the
     * API the tool relies on. Only the releases the tool was written against
     * are supported. Keep in sync with the install requirement.
     */
    inline PythonPackage::VersionRange SupportedVersions()
    {
      return { "0.1.2", "0.2.0" };
    }

    /** \brief Where the model is loaded from. */
    enum class ModelSource
    {
      HuggingFace, /**< The Hugging Face Hub, through the cache of VoxTell. */
      Local        /**< A folder on this machine, configured in the preferences. */
    };

    /** \brief Reads a model source as the preference page stores it.
     *
     * \param[in] value "huggingface" or "local".
     *
     * \throw mitk::Exception if \p value is neither.
     */
    MITKPYTHONSEGMENTATION_EXPORT ModelSource ParseModelSource(const std::string& value);

    /** \brief Returns the index-to-world transform of a geometry as the four rows of a 4x4 matrix.
     *
     * The VoxTell worker derives the RAS affine of the image from it.
     */
    MITKPYTHONSEGMENTATION_EXPORT nlohmann::json IndexToWorld(const BaseGeometry* geometry);

    /** \brief Files of a repository on the Hugging Face Hub. */
    struct RepoFiles
    {
      std::string RepoId;                /**< \brief The repository, e.g. "mrokuss/VoxTell". */
      std::vector<std::string> Patterns; /**< \brief The files, as glob patterns relative to the repository. */
    };

    /** \brief Returns the Python source that names the files VoxTell loads from the Hugging Face Hub.
     *
     * Defines the functions <tt>voxtell_prompt_list_files()</tt>,
     * <tt>voxtell_model_files()</tt> and <tt>voxtell_text_model_files()</tt>, each
     * returning a list of <tt>(repo_id, allow_patterns)</tt> pairs as VoxTell itself
     * names them, and <tt>voxtell_text_model_cached()</tt>. They import VoxTell,
     * so they run in its virtual environment.
     *
     * \throw mitk::Exception if the source cannot be read.
     */
    MITKPYTHONSEGMENTATION_EXPORT std::string ReadHubFilesCode();
  }

  /** \brief Segmentation tool that integrates the VoxTell deep learning model.
   *
   * VoxTell segments anatomical structures in a 3D image from free-text
   * prompts, for example "liver" or "left kidney". The tool runs it in a
   * PythonWorker process with the interpreter of the virtual environment of
   * VoxTell, which also keeps the loaded model. Each prompt
   * becomes one label of the preview, named after the prompt. The results of
   * different prompts can overlap, a structure and a part of it for example,
   * and are kept apart in groups (see SetCreateGroupsAsNeeded()).
   *
   * VoxTell expects its input in the RAS orientation of the nnU-Net NIfTI
   * reader and does not know about geometry. The tool derives the orientation
   * from the geometry of the image and restores it for the result, so it works
   * with an image of any format. The image is not resampled; VoxTell advises
   * to resample images of very uncommon spacings to about 1.5 mm.
   *
   * The typical lifecycle is:
   * -# Check the installation of the virtual environment (GetVirtualEnvName())
   *    with a PythonPackage::PackageProbe
   * -# Load the model via LoadModel() (downloads it on first use)
   * -# Set the prompts via SetPrompts() and call UpdatePreview()
   * -# Confirm the preview with ConfirmSegmentation()
   * -# Unload the model via UnloadModel() or by deactivating the tool
   *
   * The calls of the worker that take long run off the GUI thread (see
   * RunOffGUIThread()): UpdatePreview() returns before the model has computed,
   * and GetModelFiles(), LoadModel() and EmbedPrompts() report their end to a
   * callback on the GUI thread. A run can be cancelled with the progress
   * notification of the tool. A run that has not stopped seconds after the
   * cancellation is ended with its worker. If the worker ends, the model is no
   * longer loaded (see IsModelLoaded()).
   *
   * \sa SegWithPreviewTool, PythonWorker, QmitkVoxTellToolGUI
   */
  class MITKPYTHONSEGMENTATION_EXPORT VoxTellTool : public SegWithPreviewTool
  {
  public:
    mitkClassMacro(VoxTellTool, SegWithPreviewTool)
    itkFactorylessNewMacro(Self)

    /** \brief Returns the display name of this tool.
     *
     * \return The string "VoxTell".
     */
    const char* GetName() const override;

    /** \brief Returns the icon resource for this tool. */
    us::ModuleResource GetIconResource() const override;

    void Activated() override;

    /** \brief Called when the tool is deactivated.
     *
     * Unloads the model. If the preview is being computed at this moment, the
     * computation is aborted and the model is unloaded as soon as it has
     * stopped.
     */
    void Deactivated() override;

    /** \brief Tells that the computation of a preview can be cancelled. */
    bool IsCancelable() const override;

    /** \brief Returns Label::AlgorithmType::AUTOMATIC.
     *
     * A prompt names the structure to segment, like the task of
     * TotalSegmentator does, but nothing in the image is pointed at.
     */
    Label::AlgorithmType GetAlgorithmType() const override;

    /** \brief Returns the name of the Python virtual environment used.
     *
     * \return VoxTell::VENV_NAME.
     */
    std::string GetVirtualEnvName() const;

    /** \brief The preferences baked into a loaded model, each paired with its
     *         default value (as stored).
     *
     * LoadModel() seeds these so the stored values match what the model was
     * loaded with, and the GUI unloads the model whenever one of them changes.
     */
    static const std::vector<std::pair<std::string, std::string>>& GetSessionDefiningPreferences();

    /** \brief Receives the end of an operation of the tool on the GUI thread:
     *         what it threw, or null.
     */
    using DoneCallback = std::function<void(std::exception_ptr error)>;

    /** \brief Receives the files of GetModelFiles(), or what it threw. */
    using ModelFilesCallback = std::function<void(std::exception_ptr error, const std::vector<VoxTell::RepoFiles>& files)>;

    /** \brief Asks for the files that LoadModel() loads from the Hugging Face Hub.
     *
     * These are the embeddings of the prompts VoxTell knows and, unless a local
     * model folder is configured, the model. LoadModel() downloads what is not
     * cached yet, without telling how far it is, so the caller can download them
     * up front instead.
     *
     * Starts the worker if needed and imports VoxTell there off the GUI thread,
     * which takes several seconds. Files that VoxTell does not name are left out
     * and a warning is logged. Cancelling \p task or deactivating the tool ends
     * the worker right away.
     *
     * \param[in] task A cancelable task the user sees, kept until \p done.
     * \param[in] done Receives the files, or a mitk::Exception if the tool is
     *            busy, the worker cannot be started, or the call was
     *            cancelled or aborted.
     */
    void GetModelFiles(std::shared_ptr<ProgressTask> task, ModelFilesCallback done);

    /** \brief Loads the model.
     *
     * Selects the device from the preferences, then loads the model from the
     * Hugging Face hub (downloading it on first use) or from the configured
     * local folder, off the GUI thread. A model that is already loaded is
     * unloaded first. Cancelling \p task or deactivating the tool ends the
     * loading, which kills the worker right away.
     *
     * \param[in] task A cancelable task the user sees, kept until \p done.
     * \param[in] done Receives null once the model is loaded, or a
     *            mitk::Exception if the tool is busy, the model cannot be
     *            loaded, or the loading was cancelled or aborted.
     */
    void LoadModel(std::shared_ptr<ProgressTask> task, DoneCallback done);

    /** \brief Unloads the model and ends the worker.
     *
     * Frees the memory of the device and releases the Python context.
     *
     * \pre The preview is not being computed.
     */
    void UnloadModel();

    /** \brief Checks whether a model is loaded and its worker still runs. */
    bool IsModelLoaded() const;

    /** \brief Why the worker stopped, see PythonWorker::GetExitDescription().
     *
     * \return The description, or an empty string if there is no worker or it
     *         still runs.
     */
    std::string GetWorkerExitDescription() const;

    /** \brief Returns the backend the model was loaded on.
     *
     * \return The backend, or \c std::nullopt if no model is loaded.
     */
    std::optional<Torch::Backend> GetBackend() const;

    /** \brief Returns where the model was loaded from.
     *
     * \return The model source, or \c std::nullopt if no model is loaded.
     */
    std::optional<VoxTell::ModelSource> GetModelSource() const;

    /** \brief Sets the prompts that UpdatePreview() segments.
     *
     * The prompt at position i becomes the label with value i + 1. An empty set
     * of prompts leaves the preview empty.
     */
    void SetPrompts(const std::vector<std::string>& prompts);

    /** \brief Returns the prompts that UpdatePreview() segments. */
    const std::vector<std::string>& GetPrompts() const;

    /** \brief Sets whether results go into groups where they do not overlap.
     *
     * If true, which is the default, each result goes into the first group in
     * which it overlaps neither a label of the segmentation nor an earlier
     * result: the active group, another group, or a group that confirming
     * adds to the segmentation. Where it shares just a few border voxels with
     * a label (up to 2% of the smaller of the two), those voxels stay with the
     * label. Labels of the segmentation are never changed.
     *
     * If false, all results go into the active group. A later prompt takes the
     * voxels of an earlier one, and confirming overwrites the labels of the
     * segmentation that are not locked.
     *
     * Takes effect with the next UpdatePreview(), without running VoxTell
     * again for prompts it has just segmented.
     */
    void SetCreateGroupsAsNeeded(bool createGroupsAsNeeded);
    bool GetCreateGroupsAsNeeded() const;

    /** \brief Returns the prompts that VoxTell cannot look up, in their given order.
     *
     * VoxTell ships embeddings of a large set of anatomical terms. Any other
     * prompt needs the text encoder of VoxTell, a model of about 8 GB that is
     * downloaded on first use and has to fit into the memory of the device.
     * Prompts that were segmented or processed by EmbedPrompts() before in
     * this session are known.
     *
     * Prompts must not contain line breaks. Returns an empty list while no
     * model is loaded.
     *
     * \throw mitk::Exception with a message for the user if Python fails.
     */
    std::vector<std::string> GetPromptsWithoutPrecomputedEmbedding(const std::vector<std::string>& prompts) const;

    /** \brief Returns the prompts that VoxTell knows, in alphabetical order.
     *
     * These are the prompts of its prompt bank, which are in lower case, and
     * the prompts it learned in this session. None of them needs the text
     * encoder. Returns an empty list while no model is loaded.
     *
     * \throw mitk::Exception with a message for the user if Python fails.
     */
    std::vector<std::string> GetKnownPrompts() const;

    /** \brief Returns the files of the text encoder on the Hugging Face Hub.
     *
     * EmbedPrompts() downloads what is not cached yet, without telling how far
     * it is, so the caller can download them up front instead. Empty if VoxTell
     * does not name them, which is logged as a warning.
     *
     * \pre A model is loaded.
     */
    std::vector<VoxTell::RepoFiles> GetTextModelFiles();

    /** \brief Checks whether the text encoder has been loaded in this session.
     *
     * \throw mitk::Exception with a message for the user if Python fails.
     */
    bool IsTextModelLoaded() const;

    /** \brief Checks whether the text encoder can be loaded without a download.
     *
     * Looks into the cache of the Hugging Face Hub only, so it works offline.
     *
     * \pre A model is loaded.
     *
     * \throw mitk::Exception with a message for the user if Python fails.
     */
    bool IsTextModelCached() const;

    /** \brief Makes the given prompts known to VoxTell with its text encoder.
     *
     * Loads the text encoder first if it has not been loaded in this session,
     * which takes long. Afterwards the prompts no longer appear in
     * GetPromptsWithoutPrecomputedEmbedding() and segmenting them takes no
     * longer than segmenting any other prompt.
     *
     * Runs off the GUI thread. Cancelling \p task or deactivating the tool ends
     * the worker right away, and the model with it.
     *
     * \param[in] prompts The prompts to make known.
     * \param[in] task A cancelable task the user sees, kept until \p done.
     * \param[in] done Receives null once the prompts are known, or a
     *            mitk::Exception with a message for the user if the prompts
     *            cannot be processed, for example for lack of memory, if the
     *            tool is busy, if no model is loaded, or if the call was
     *            cancelled or aborted.
     */
    void EmbedPrompts(const std::vector<std::string>& prompts, std::shared_ptr<ProgressTask> task, DoneCallback done);

    /** \brief Returns a human-readable reason that the last update produced no result.
     *
     * Empty if it succeeded or the user cancelled it. The SegWithPreviewTool
     * base forwards an exception to ErrorMessage, which presents it in a dialog
     * with the full Python traceback. The tool therefore records the failure
     * of a run here instead, and the GUI presents it.
     */
    const std::string& GetLastErrorMessage() const;

    /** \brief Returns whether the user cancelled the last update.
     *
     * Its preview is left empty, as after a failure, but GetLastErrorMessage()
     * stays empty.
     */
    bool WasLastUpdateCancelled() const;

    /** \brief Returns the prompts that the last update found nothing for.
     *
     * Their labels are not part of the preview, so they cannot be confirmed as
     * empty labels. A prompt also counts if other labels took all voxels of
     * its result (see SetCreateGroupsAsNeeded()).
     */
    const std::vector<std::string>& GetPromptsWithoutResult() const;

  protected:
    VoxTellTool();
    ~VoxTellTool() override;

    /** \brief Removes the labels that the preview inherits from the segmentation.
     *
     * Only the labels of VoxTell are transferred on confirmation.
     */
    void InitiateToolByInput() override;

    /** \brief Empties the preview and makes one label per prompt for it. */
    void UpdatePrepare() override;

    /** \brief Removes the labels without result from the preview.
     *
     * Unloads the model instead if the tool was deactivated while computing.
     */
    void UpdateCleanUp() override;

    /** \brief Empties the preview, so a confirmed result cannot be confirmed again.
     *
     * The tool stays active after a confirmation (see KeepActiveAfterAccept),
     * and in AddLabel transfer mode a second confirmation would add the same
     * labels once more.
     */
    void ConfirmCleanUp() override;

    /** \brief Segments the prompts in the reference image and writes the result into the preview.
     *
     * The input of the base class is not used. A region of interest is not
     * supported by the transfer of the preview into the segmentation, and
     * VoxTell needs the reference image as it is.
     *
     * Failures do not propagate: the preview is left empty and the reason is
     * available from GetLastErrorMessage(). Only a cancellation by the user
     * propagates, so the base class can tell it from a failure.
     */
    void DoUpdatePreview(const Image* inputAtTimeStep, const Image* oldSegAtTimeStep, MultiLabelSegmentation* previewImage, TimeStepType timeStep) override;

  private:
    /** \brief Returns the worker, which is started if it is not running. */
    PythonWorker& GetWorker();

    /** \brief Ends the worker after GetModelFiles() or LoadModel() failed, and
     *         unloads the model if the tool was deactivated meanwhile.
     */
    void EndFailedInitialization();

    /** \brief Leaves the preview empty after a failed run, and records why.
     *
     * Rethrows a cancellation by the user, so the base class can tell it from a
     * failure.
     */
    void EndFailedRun(std::exception_ptr error, const ProgressTask* task);

    /** \brief Removes the labels that got no voxel in any time step of the update. */
    void RemoveLabelsWithoutResult();

    /** \brief Writes the masks of the cached run into the preview as one label per prompt.
     *
     * The first time step of an update decides which group a label goes into.
     *
     * \param segmentation The segmentation the run was computed for, whose
     *        groups and occupied voxels decide where the labels go.
     */
    void PlaceMasks(const MultiLabelSegmentation* segmentation, MultiLabelSegmentation* preview, TimeStepType timeStep);

    class Impl;
    std::unique_ptr<Impl> m_Impl;
  };
}

#endif
