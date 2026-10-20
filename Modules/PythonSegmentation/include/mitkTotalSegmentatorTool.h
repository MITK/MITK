/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkTotalSegmentatorTool_h
#define mitkTotalSegmentatorTool_h

#include <mitkSegWithPreviewTool.h>
#include <MitkPythonSegmentationExports.h>

#include <nlohmann/json_fwd.hpp>

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace us
{
  class ModuleResource;
}

namespace mitk
{
  class ProgressTask;
  class PythonWorker;

  namespace TotalSegmentator
  {
    /** \brief Name of the managed virtual environment created under the MITK venv base path. */
    inline constexpr const char* VENV_NAME = "TotalSegmentator";

    /** \brief The exact TotalSegmentator release this MITK build installs and requires.
     *
     * Pinned exactly (not a range) because TotalSegmentator ships breaking changes
     * even in minor releases. Every installation of MITK has environments of its
     * own, so an environment with another release is left from an earlier MITK
     * in the same folder; the user reinstalls it.
     */
    inline constexpr const char* REQUIRED_VERSION = "2.18.0";

    /** \brief The nnU-Net release validated with REQUIRED_VERSION.
     *
     * TotalSegmentator leaves its nnunetv2 dependency unbounded, and new nnunetv2
     * releases have repeatedly broken the environment; nnunetv2 is also what
     * drives the resolved torch version. Bump it together with REQUIRED_VERSION.
     */
    inline constexpr const char* NNUNETV2_VERSION = "2.8.1";
  }

  /**
    \brief TotalSegmentator segmentation tool.

    Runs TotalSegmentator in a worker process with the interpreter of its virtual
    environment, which keeps MITK's own interpreter free of that environment.
    Images pass through shared memory. The calls of the worker run off the GUI
    thread (see RunOffGUIThread()), so UpdatePreview() returns before a run is
    done. The progress notification of the tool shows a run and cancels it.

    Initialize() starts the worker and learns the tasks of TotalSegmentator from
    its registry. Device and license are the user's settings in the preferences.

    \ingroup Interaction
    \ingroup ToolManagerEtAl

    \warning Only to be instantiated by mitk::ToolManager.
  */
  class MITKPYTHONSEGMENTATION_EXPORT TotalSegmentatorTool : public SegWithPreviewTool
  {
  public:
    mitkClassMacro(TotalSegmentatorTool, SegWithPreviewTool);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /** \brief The resolution of the model: its full one, or the coarser and quicker ones of some tasks. */
    enum class Speed
    {
      Standard,
      Fast,
      Fastest
    };

    struct SpeedInfo
    {
      Speed Value;

      /** \brief The spacing the model works at, in millimeters, or 0 if TotalSegmentator does not tell. */
      double Resolution;
    };

    /** \brief A task of TotalSegmentator whose models can be downloaded. */
    struct TaskInfo
    {
      std::string Name;

      /** \brief Whether the task is for MR images rather than CT images. */
      bool MR;

      /** \brief Whether the task needs a TotalSegmentator license. */
      bool Licensed;

      /** \brief Whether the task can segment a subset of its classes, see SetRoiSubset(). */
      bool SupportsRoiSubset;

      std::vector<SpeedInfo> Speeds;

      /** \brief What the task segments: the label value of each class and its name. */
      std::map<Label::PixelType, std::string> Classes;

      bool HasSpeed(Speed speed) const
      {
        for (const auto& info : Speeds)
        {
          if (info.Value == speed)
            return true;
        }

        return false;
      }
    };

    /** \brief Whether the virtual environment of TotalSegmentator exists. */
    static bool IsInstalled();

    /** \brief Whether the user set a license, which the licensed tasks need. */
    static bool HasLicense();

    /** \brief The device the user chose for TotalSegmentator, like "gpu" or "cpu". */
    static std::string GetDevice();

    const char *GetName() const override;

    /** \brief TotalSegmentator runs with no per-image human input, so AUTOMATIC. */
    mitk::Label::AlgorithmType GetAlgorithmType() const override;

    us::ModuleResource GetIconResource() const override;
    void Activated() override;

    /** \brief Ends the worker, and stops a run or a start that is in progress. */
    void Deactivated() override;

    /** \brief Tells that a run can be cancelled. */
    bool IsCancelable() const override;

    /**
     * \brief Starts the worker and learns the tasks of TotalSegmentator.
     *
     * Returns at once, as the worker calls run off the GUI thread.
     * InitializedEvent follows, or InitializationFailedEvent if
     * TotalSegmentator is not installed, is not TotalSegmentator::REQUIRED_VERSION,
     * or does not start. Does nothing if the tool is initialized or starting
     * already.
     *
     * \param task Shows what goes on, and stops the start when cancelled. The
     *        tool keeps it until the start has ended.
     */
    void Initialize(std::shared_ptr<ProgressTask> task);

    /** \brief Whether the tasks are known, see Initialize(). */
    bool IsInitialized() const;

    /** \brief Whether Initialize() has not ended yet. */
    bool IsInitializing() const;

    /**
     * \brief Ends the worker and forgets the tasks, for example before the
     *        environment is removed, which the worker holds on to.
     *
     * Call Initialize() before the next use.
     */
    void Uninitialize();

    /**
     * \brief The tasks, in the order a user picks from: the whole-body tasks
     *        first, then the free ones, then the licensed ones.
     *
     * Empty until the tool is initialized.
     */
    const std::vector<TaskInfo>& GetTasks() const;

    /** \brief The task of that name, or nullptr. */
    const TaskInfo* FindTask(const std::string& name) const;

    /**
     * \brief Chooses what the next UpdatePreview() runs, for all classes of the task.
     *
     * A licensed task can be chosen without a license; UpdatePreview() then
     * fails and says so.
     *
     * \throw mitk::Exception if the tool is not initialized, or for an unknown
     *        task or a speed that the task does not offer.
     */
    void SetTask(const std::string& task, Speed speed = Speed::Standard);

    const std::string& GetTask() const;
    Speed GetSpeed() const;

    /**
     * \brief Segments only these classes of the task in the next run, or all of
     *        them for an empty list.
     *
     * TotalSegmentator then locates the classes with a coarse model first and
     * runs only the models that segment them, on the region around them, which
     * is quicker. SetTask() selects all classes again.
     *
     * \throw mitk::Exception if the task does not support a subset, or for a
     *        class it does not segment.
     */
    void SetRoiSubset(const std::vector<std::string>& classes);

    /** \brief The classes the next run segments, in the order of the task; empty for all of them. */
    const std::vector<std::string>& GetRoiSubset() const;

    /** \brief Human-readable reason the last run produced no result, or empty if
     *         it succeeded or was cancelled by the user.
     *
     * The SegWithPreviewTool base forwards an exception to ErrorMessage, whose
     * listeners present it in a message box apart from the tool. The tool
     * therefore records the failure of a run here instead, letting the GUI
     * tell a genuine failure from a user cancellation and present the reason.
     */
    const std::string& GetLastErrorMessage() const;

    /**
     * \brief Whether the last run ended with a result, rather than failing or
     *        being cancelled.
     *
     * The result has a label for each class that TotalSegmentator found, so it
     * is empty when the image shows none of them.
     */
    bool LastRunSucceeded() const;

    /** \brief Event triggered when Initialize() has learned the tasks, whoever called it. */
    Message<> InitializedEvent;

    /**
     * \brief Event triggered when Initialize() has failed, with a message for the user.
     *
     * The message is empty if the start was cancelled or the tool deactivated.
     */
    Message1<std::string> InitializationFailedEvent;

    /** \brief Event triggered when the task, the speed or the subset of classes changed, whoever changed it. */
    Message<> OptionsChangedEvent;

  protected:
    TotalSegmentatorTool();
    ~TotalSegmentatorTool() override;

    /**
     * \brief Runs TotalSegmentator on the input and puts its result into the preview.
     *
     * Downloads the models of the task first if they are missing. On
     * cancellation or failure the preview is left empty.
     *
     * \pre The tool is initialized, see Initialize().
     */
    void DoUpdatePreview(const Image* inputAtTimeStep, const Image* oldSegAtTimeStep, MultiLabelSegmentation* previewImage, TimeStepType timeStep) override;
    void UpdatePrepare() override;

    /** \brief Empties the preview, so a confirmed result cannot be confirmed again.
     *
     * The tool stays active after a confirmation (see KeepActiveAfterAccept),
     * and in AddLabel transfer mode a second confirmation would add the same
     * labels once more.
     */
    void ConfirmCleanUp() override;

  private:
    PythonWorker& GetWorker();

    /** \brief Learns the tasks from what the worker told, on the GUI thread. */
    void EndInitialization(std::exception_ptr error, const nlohmann::json& info, const ProgressTask* task);

    /** \brief Leaves the preview empty after a failed run, and tells why unless the user stopped it. */
    void EndFailedRun(std::exception_ptr error, const ProgressTask* task);

    class Impl;
    std::unique_ptr<Impl> m_Impl;
  };
} // namespace mitk
#endif
