/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkTotalSegmentatorTool.h>

#include <mitkExceptionMacro.h>
#include <mitkIOUtil.h>
#include <mitkLookupTable.h>
#include <mitkFileSystem.h>

#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleContext.h>
#include <usModuleResource.h>

#include <array>
#include <fstream>
#include <system_error>
#include <utility>

namespace mitk
{
  MITK_TOOL_MACRO(MITKPYTHONSEGMENTATION_EXPORT, TotalSegmentatorTool, "Total Segmentator");
}

namespace
{
  // Removes a directory tree when it goes out of scope. Uses the non-throwing
  // std::filesystem overload so it is safe on error paths and in destructors.
  // Non-copyable so a guard uniquely owns its path (no double-remove).
  struct ScopedDir
  {
    std::string path;

    explicit ScopedDir(std::string dir) : path(std::move(dir)) {}
    ScopedDir(const ScopedDir&) = delete;
    ScopedDir& operator=(const ScopedDir&) = delete;

    ~ScopedDir()
    {
      if (!path.empty())
      {
        std::error_code error;
        fs::remove_all(path, error);
      }
    }
  };
}

mitk::TotalSegmentatorTool::TotalSegmentatorTool()
  : SegWithPreviewTool(true) // prevents auto-compute across all time steps
{
  this->IsTimePointChangeAwareOff();
  this->RequestDeactivationConfirmationOn();

  // Stay active after a segmentation is confirmed, so the user can run another
  // task without having to reselect the tool.
  this->KeepActiveAfterAcceptOn();
}

mitk::TotalSegmentatorTool::~TotalSegmentatorTool()
{
  // Destructors are noexcept: use the non-throwing overload so a locked file
  // (a lingering child, antivirus, ...) cannot escalate cleanup into a crash.
  if (!m_MitkTempDir.empty())
  {
    std::error_code error;
    fs::remove_all(m_MitkTempDir, error);
  }
}

void mitk::TotalSegmentatorTool::Activated()
{
  Superclass::Activated();
  this->SetLabelTransferScope(LabelTransferScope::AllLabels);
  this->SetLabelTransferMode(LabelTransferMode::AddLabel);
}

us::ModuleResource mitk::TotalSegmentatorTool::GetIconResource() const
{
  // The shared segmentation icon lives in MitkSegmentation, not in this module.
  auto module = us::GetModuleContext()->GetModule("MitkSegmentation");
  return module->GetResource("AI.svg");
}

const char *mitk::TotalSegmentatorTool::GetName() const
{
  return "TotalSegmentator";
}

mitk::Label::AlgorithmType mitk::TotalSegmentatorTool::GetAlgorithmType() const
{
  return mitk::Label::AlgorithmType::AUTOMATIC;
}

void mitk::TotalSegmentatorTool::SetLabelNameLookup(const std::map<mitk::Label::PixelType, std::string>& lookup)
{
  m_LabelNameLookup = lookup;
}

void mitk::TotalSegmentatorTool::SetCommandRunner(CommandRunner runner)
{
  m_CommandRunner = std::move(runner);
}

std::vector<std::string> mitk::TotalSegmentatorTool::BuildArguments(const std::string& inputImagePath,
                                                                   const std::string& outputImagePath) const
{
  std::vector<std::string> args;
  args.push_back("-i");
  args.push_back(inputImagePath);
  args.push_back("-o");
  args.push_back(outputImagePath);
  args.push_back("-ta");
  args.push_back(m_Task);
  args.push_back("--ml"); // single multi-label output file whose ids match class_map

  switch (m_Speed)
  {
    case Speed::Fast:
      args.push_back("--fast");
      break;
    case Speed::Fastest:
      args.push_back("--fastest");
      break;
    case Speed::Normal:
      break;
  }

  args.push_back("-d");
  args.push_back(m_Device.empty() ? std::string("gpu") : m_Device);

  return args;
}

void mitk::TotalSegmentatorTool::UpdatePrepare()
{
  Superclass::UpdatePrepare();
  auto preview = this->GetPreviewSegmentation();
  preview->RemoveLabels(preview->GetAllLabelValues());
}

void mitk::TotalSegmentatorTool::DoUpdatePreview(const Image *inputAtTimeStep,
                                                 const Image * /*oldSegAtTimeStep*/,
                                                 MultiLabelSegmentation *previewImage,
                                                 TimeStepType timeStep)
{
  m_LastErrorMessage.clear();

  // The base UpdatePreview catches itk::ExceptionObject (what mitkThrow produces)
  // and only forwards it to ErrorMessage, which no GUI here listens to. So catch
  // every failure locally and record it in m_LastErrorMessage; the GUI shows that
  // instead of the generic "cancelled or produced no result".
  try
  {
    // Programmer-guarantee guards: the GUI always injects both before running.
    if (m_ExecutablePath.empty())
      mitkThrow() << "TotalSegmentator executable path is not set.";

    if (!m_CommandRunner)
      mitkThrow() << "No command runner is set for the TotalSegmentator tool.";

    if (m_MitkTempDir.empty())
      m_MitkTempDir = IOUtil::CreateTemporaryDirectory("mitk-XXXXXX");

    const std::string inDir = IOUtil::CreateTemporaryDirectory("totalseg-in-XXXXXX", m_MitkTempDir);
    const std::string outDir = IOUtil::CreateTemporaryDirectory("totalseg-out-XXXXXX", m_MitkTempDir);

    // Remove this run's scratch directories on every exit path; only the parent
    // m_MitkTempDir persists (until the destructor), so repeated runs in one tool
    // activation do not pile up input copies and outputs.
    const ScopedDir inDirGuard{inDir};
    const ScopedDir outDirGuard{outDir};

    std::ofstream tmpStream;
    const std::string inputImagePath = IOUtil::CreateTemporaryFile(tmpStream, "XXXXXX.nii.gz", inDir);
    tmpStream.close();

    const std::string outputImagePath = outDir + IOUtil::GetDirectorySeparator() + "segmentation.nii.gz";

    IOUtil::Save(inputAtTimeStep, inputImagePath);

    const auto args = this->BuildArguments(inputImagePath, outputImagePath);
    const bool success = m_CommandRunner(m_ExecutablePath, args);

    if (!success)
      return; // Cancelled by the user or the process failed: leave the preview empty.

    if (!fs::exists(outputImagePath))
      mitkThrow() << "TotalSegmentator finished but did not produce an output file.";

    Image::Pointer outputImage = IOUtil::Load<Image>(outputImagePath);
    outputImage->SetGeometry(inputAtTimeStep->GetGeometry());

    auto outputBuffer = mitk::MultiLabelSegmentation::New();
    outputBuffer->InitializeByLabeledImage(outputImage);

    this->MapLabelsToSegmentation(outputBuffer, previewImage);
    previewImage->UpdateGroupImage(
      previewImage->GetActiveLayer(), outputBuffer->GetGroupImage(outputBuffer->GetActiveLayer()), timeStep);
  }
  catch (const mitk::Exception& e)
  {
    // Leave no half-populated preview behind: an empty preview is how the GUI
    // tells a failure apart from a successful run.
    previewImage->RemoveLabels(previewImage->GetAllLabelValues());
    m_LastErrorMessage = e.GetDescription();
  }
  catch (const std::exception& e)
  {
    previewImage->RemoveLabels(previewImage->GetAllLabelValues());
    m_LastErrorMessage = e.what();
  }
}

const std::string& mitk::TotalSegmentatorTool::GetLastErrorMessage() const
{
  return m_LastErrorMessage;
}

void mitk::TotalSegmentatorTool::MapLabelsToSegmentation(const mitk::MultiLabelSegmentation* source,
                                                         mitk::MultiLabelSegmentation* dest) const
{
  auto lookupTable = mitk::LookupTable::New();
  lookupTable->SetType(mitk::LookupTable::LookupTableType::MULTILABEL);

  // Iterate the label values actually present in the output, not the name lookup,
  // so a structure is never dropped just because its id is missing from the lookup.
  for (auto value : source->GetAllLabelValues())
  {
    if (value == mitk::Label::UNLABELED_VALUE)
      continue;

    std::string name;
    if (auto it = m_LabelNameLookup.find(value); it != m_LabelNameLookup.end())
      name = it->second;
    else
      name = m_Task + "_" + std::to_string(value);

    auto label = mitk::Label::New(value, name);

    std::array<double, 3> lookupTableColor;
    lookupTable->GetColor(value, lookupTableColor.data());
    Color color;
    color.SetRed(lookupTableColor[0]);
    color.SetGreen(lookupTableColor[1]);
    color.SetBlue(lookupTableColor[2]);
    label->SetColor(color);

    dest->AddLabel(label, 0, false);
  }
}
