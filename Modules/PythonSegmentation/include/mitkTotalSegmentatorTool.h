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

#include <functional>
#include <map>
#include <string>
#include <vector>

namespace us
{
  class ModuleResource;
}

namespace mitk
{
  /**
    \brief TotalSegmentator segmentation tool.

    Wraps the TotalSegmentator command-line tool. The heavy work runs in a
    subprocess (never in this process), which keeps MITK's own interpreter free
    of the virtual environment and lets the GUI keep the application responsive.
    To stay free of any Qt dependency, this class does not spawn the process
    itself: the GUI injects the resolved executable path (SetExecutablePath), a
    label-id-to-name lookup for the selected task (SetLabelNameLookup) and a
    CommandRunner that actually runs the process while showing progress and
    offering cancellation.

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

    /** \brief Model resolution: full (1.5 mm), --fast (3 mm) or --fastest (6 mm). */
    enum class Speed
    {
      Normal,
      Fast,
      Fastest
    };

    /** \brief Runs \p executable with \p args, returning \c true on success (exit 0)
     *         and \c false on cancellation or failure.
     *
     * Injected by the GUI so the long-running CLI call can keep the application
     * responsive and cancellable while this core module stays free of Qt.
     */
    using CommandRunner = std::function<bool(const std::string& executable, const std::vector<std::string>& args)>;

    const char *GetName() const override;

    /** \brief TotalSegmentator runs with no per-image human input, so AUTOMATIC. */
    mitk::Label::AlgorithmType GetAlgorithmType() const override;

    us::ModuleResource GetIconResource() const override;
    void Activated() override;

    itkSetMacro(Task, std::string);
    itkGetConstMacro(Task, std::string);

    itkSetMacro(Speed, Speed);
    itkGetConstMacro(Speed, Speed);

    itkSetMacro(Device, std::string);
    itkGetConstMacro(Device, std::string);

    itkSetMacro(ExecutablePath, std::string);
    itkGetConstMacro(ExecutablePath, std::string);

    /** \brief Sets the label-id-to-name lookup for the selected task.
     *
     * Ids present in the segmentation output but absent from this lookup are
     * auto-named, so an incomplete or missing lookup never drops a structure.
     */
    void SetLabelNameLookup(const std::map<mitk::Label::PixelType, std::string>& lookup);

    void SetCommandRunner(CommandRunner runner);

    /** \brief Human-readable reason the last run produced no result, or empty if
     *         it succeeded or was cancelled by the user.
     *
     * The SegWithPreviewTool base catches itk::ExceptionObject (which mitkThrow
     * produces) and only forwards it to ErrorMessage, so such a throw never
     * reaches the GUI's handler. DoUpdatePreview therefore catches its failures
     * internally and records them here, letting the GUI tell a genuine failure
     * from a user cancellation and show a meaningful message.
     */
    const std::string& GetLastErrorMessage() const;

  protected:
    TotalSegmentatorTool();
    ~TotalSegmentatorTool() override;

    /**
     * \brief Runs TotalSegmentator on the input and loads its multi-label output.
     *
     * Saves the input to a temporary NIfTI, runs the CLI via the injected
     * CommandRunner, then loads the single --ml output and names its labels. On
     * cancellation or failure the preview is left empty.
     */
    void DoUpdatePreview(const Image* inputAtTimeStep, const Image* oldSegAtTimeStep, MultiLabelSegmentation* previewImage, TimeStepType timeStep) override;
    void UpdatePrepare() override;

  private:
    /** \brief Assembles the TotalSegmentator command-line arguments. */
    std::vector<std::string> BuildArguments(const std::string& inputImagePath, const std::string& outputImagePath) const;

    /** \brief Names and colors the labels present in the output segmentation. */
    void MapLabelsToSegmentation(const MultiLabelSegmentation* source, MultiLabelSegmentation* dest) const;

    std::string m_MitkTempDir;
    std::string m_Task = "total";
    Speed m_Speed = Speed::Normal;
    std::string m_Device = "gpu";
    std::string m_ExecutablePath;
    std::map<mitk::Label::PixelType, std::string> m_LabelNameLookup;
    CommandRunner m_CommandRunner;
    std::string m_LastErrorMessage;
  };
} // namespace mitk
#endif
