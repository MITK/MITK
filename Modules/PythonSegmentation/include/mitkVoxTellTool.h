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

#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace mitk
{
  class PythonContext;

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
   * prompts, for example "liver" or "left kidney". The tool runs it in-process
   * through a PythonContext on the virtual environment of VoxTell. Each prompt
   * becomes one label of the preview, named after the prompt.
   *
   * VoxTell expects its input in the RAS orientation of the nnU-Net NIfTI
   * reader and does not know about geometry. The tool derives the orientation
   * from the geometry of the image and restores it for the result, so it works
   * with an image of any format. The image is not resampled; VoxTell advises
   * to resample images of very uncommon spacings to about 1.5 mm.
   *
   * The typical lifecycle is:
   * -# Create a Python context via CreatePythonContext()
   * -# Load the model via LoadModel() (downloads it on first use)
   * -# Set the prompts via SetPrompts() and call UpdatePreview()
   * -# Confirm the preview with ConfirmSegmentation()
   * -# Unload the model via UnloadModel() or by deactivating the tool
   *
   * UpdatePreview() runs on the calling thread and keeps the application
   * responsive through GUIProcessEventsMessage while the model computes. It can
   * be cancelled with the progress notification of the tool.
   *
   * \sa SegWithPreviewTool, PythonContext, QmitkVoxTellToolGUI
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

    /** \brief Creates and activates a Python context with the virtual environment.
     *
     * The context is activated without bindings, so that checks run before the
     * model is loaded do not map any native library of the virtual environment.
     * The bindings are imported by LoadModel().
     *
     * \warning The virtual environment is created if it does not exist yet.
     *          Check PythonHelper::VirtualEnvExists() first.
     *
     * \return \c true if the Python context was created, \c false otherwise.
     */
    bool CreatePythonContext();

    /** \brief Returns the current Python context.
     *
     * \return The context, or \c nullptr if none has been created yet.
     */
    PythonContext* GetPythonContext() const;

    /** \brief Checks whether the VoxTell Python package is installed.
     *
     * \pre A Python context has been created via CreatePythonContext().
     *
     * \throw mitk::Exception with a message for the user if Python fails.
     */
    bool IsInstalled() const;

    /** \brief The preferences baked into a loaded model, each paired with its
     *         default value (as stored).
     *
     * LoadModel() seeds these so the stored values match what the model was
     * loaded with, and the GUI unloads the model whenever one of them changes.
     */
    static const std::vector<std::pair<std::string, std::string>>& GetSessionDefiningPreferences();

    /** \brief Returns the files that LoadModel() loads from the Hugging Face Hub.
     *
     * These are the embeddings of the prompts VoxTell knows and, unless a local
     * model folder is configured, the model. LoadModel() downloads what is not
     * cached yet, without telling how far it is, so the caller can download them
     * up front instead.
     *
     * Imports VoxTell into the context, which takes several seconds. Files that
     * VoxTell does not name are left out and a warning is logged.
     *
     * \pre A Python context has been created via CreatePythonContext().
     *
     * \throw mitk::Exception if the context cannot be readied for VoxTell.
     */
    std::vector<VoxTell::RepoFiles> GetModelFiles();

    /** \brief Loads the model.
     *
     * Selects the device from the preferences, then loads the model from the
     * Hugging Face hub (downloading it on first use) or from the configured
     * local folder. A model that is already loaded is unloaded first.
     *
     * \pre A Python context has been created via CreatePythonContext().
     *
     * \throw mitk::Exception if the model cannot be loaded.
     */
    void LoadModel();

    /** \brief Unloads the model and releases the Python context.
     *
     * Frees the memory of the device. This is a no-op without a Python context.
     *
     * \pre The preview is not being computed.
     */
    void UnloadModel();

    /** \brief Checks whether a model is loaded. */
    bool IsModelLoaded() const;

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
     * \pre A model is loaded.
     *
     * \throw mitk::Exception with a message for the user if the prompts cannot
     *        be processed, for example for lack of memory.
     */
    void EmbedPrompts(const std::vector<std::string>& prompts);

    /** \brief Returns a human-readable reason that the last update produced no result.
     *
     * Empty if it succeeded or the user cancelled it. The SegWithPreviewTool
     * base forwards an exception to ErrorMessage, which presents it in a dialog
     * with the full Python traceback. The tool therefore records the failure
     * of a run here instead, and the GUI presents it.
     */
    const std::string& GetLastErrorMessage() const;

    /** \brief Returns the prompts that the last update found nothing for.
     *
     * Their labels are not part of the preview, so they cannot be confirmed as
     * empty labels. A prompt also counts if later prompts covered all of its
     * result, as a voxel holds a single label.
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

    /** \brief Replaces the labels of the preview by one label per prompt. */
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
    /** \brief Readies the context for VoxTell: bindings, helper code, environment. */
    void PrepareContext();

    /** \brief Frees the model from the context, but keeps the context. */
    void ReleaseModel();

    /** \brief Removes the labels that got no voxel in any time step of the update. */
    void RemoveLabelsWithoutResult();

    class Impl;
    std::unique_ptr<Impl> m_Impl;
  };
}

#endif
