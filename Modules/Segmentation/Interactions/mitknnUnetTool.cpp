/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitknnUnetTool.h"

#include "mitkIOUtil.h"
#include "mitkProcessExecutor.h"
#include <itksys/SystemTools.hxx>
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleContext.h>
#include <usModuleResource.h>
#include <filesystem>

namespace mitk
{
  MITK_TOOL_MACRO(MITKSEGMENTATION_EXPORT, nnUNetTool, "nnUNet tool");
}

mitk::nnUNetTool::~nnUNetTool()
{
  std::filesystem::remove_all(this->GetMitkTempDir());
}

void mitk::nnUNetTool::Activated()
{
  Superclass::Activated();
  this->SetLabelTransferScope(LabelTransferScope::AllLabels);
  this->SetLabelTransferMode(LabelTransferMode::AddLabel);
}

void mitk::nnUNetTool::RenderOutputBuffer()
{
  if (m_OutputBuffer != nullptr)
  {
    try
    {
      if (nullptr != this->GetPreviewSegmentationNode())
      {
        auto previewImage = this->GetPreviewSegmentation();
        previewImage->InitializeByLabeledImage(m_OutputBuffer);
      }
    }
    catch (const mitk::Exception &e)
    {
      MITK_INFO << e.GetDescription();
    }
  }
}

void mitk::nnUNetTool::SetOutputBuffer(LabelSetImage::Pointer segmentation)
{
  m_OutputBuffer = segmentation;
}

mitk::LabelSetImage::Pointer mitk::nnUNetTool::GetOutputBuffer()
{
  return m_OutputBuffer;
}

void mitk::nnUNetTool::ClearOutputBuffer()
{
  m_OutputBuffer = nullptr;
}

us::ModuleResource mitk::nnUNetTool::GetIconResource() const
{
  us::Module *module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("AI.svg");
  return resource;
}

const char **mitk::nnUNetTool::GetXPM() const
{
  return nullptr;
}

const char *mitk::nnUNetTool::GetName() const
{
  return "nnUNet";
}

mitk::DataStorage *mitk::nnUNetTool::GetDataStorage()
{
  return this->GetToolManager()->GetDataStorage();
}

mitk::DataNode *mitk::nnUNetTool::GetRefNode()
{
  return this->GetToolManager()->GetReferenceData(0);
}

void mitk::nnUNetTool::UpdatePrepare()
{
  Superclass::UpdatePrepare();
  auto preview = this->GetPreviewSegmentation();
  auto labelset = preview->GetLabelSet(preview->GetActiveLayer());
  labelset->RemoveAllLabels();
}

namespace
{
  void onPythonProcessEvent(itk::Object * /*pCaller*/, const itk::EventObject &e, void *)
  {
    std::string testCOUT;
    std::string testCERR;
    const auto *pEvent = dynamic_cast<const mitk::ExternalProcessStdOutEvent *>(&e);

    if (pEvent)
    {
      testCOUT = testCOUT + pEvent->GetOutput();
      MITK_INFO << testCOUT;
    }

    const auto *pErrEvent = dynamic_cast<const mitk::ExternalProcessStdErrEvent *>(&e);

    if (pErrEvent)
    {
      testCERR = testCERR + pErrEvent->GetOutput();
      MITK_ERROR << testCERR;
    }
  }
} // namespace

void mitk::nnUNetTool::DoUpdatePreview(const Image* inputAtTimeStep, const Image* /*oldSegAtTimeStep*/, LabelSetImage* previewImage, TimeStepType /*timeStep*/)
{
  if (this->GetMitkTempDir().empty())
  {
    this->SetMitkTempDir(IOUtil::CreateTemporaryDirectory("mitk-nnunet-XXXXXX"));
  }
  std::string inDir, outDir, inputImagePath, outputImagePath, scriptPath;

  ProcessExecutor::Pointer spExec = ProcessExecutor::New();
  itk::CStyleCommand::Pointer spCommand = itk::CStyleCommand::New();
  spCommand->SetCallback(&onPythonProcessEvent);
  spExec->AddObserver(ExternalProcessOutputEvent(), spCommand);
  ProcessExecutor::ArgumentListType args;

  inDir = IOUtil::CreateTemporaryDirectory("nnunet-in-XXXXXX", this->GetMitkTempDir());
  std::ofstream tmpStream;
  inputImagePath = IOUtil::CreateTemporaryFile(tmpStream, m_TEMPLATE_FILENAME, inDir + IOUtil::GetDirectorySeparator());
  tmpStream.close();
  std::size_t found = inputImagePath.find_last_of(IOUtil::GetDirectorySeparator());
  std::string fileName = inputImagePath.substr(found + 1);
  std::string token = fileName.substr(0, fileName.find("_"));

  if (this->GetNoPip())
  {
    scriptPath = this->GetnnUNetDirectory() + IOUtil::GetDirectorySeparator() + "nnunet" +
                 IOUtil::GetDirectorySeparator() + "inference" + IOUtil::GetDirectorySeparator() + "predict_simple.py";
  }

  try
  {
    if (this->GetMultiModal())
    {
      const std::string fileFormat(".nii.gz");
      const std::string fileNamePart("_000_000");
      std::string outModalFile;
      size_t len = inDir.length() + 1 + token.length() + fileNamePart.length() + 1 + fileFormat.length();
      outModalFile.reserve(len); // The 1(s) indicates a directory separator char and an underscore.
      for (size_t i = 0; i < m_OtherModalPaths.size(); ++i)
      {
        mitk::Image::ConstPointer modalImage = m_OtherModalPaths[i];
        outModalFile.append(inDir);
        outModalFile.push_back(IOUtil::GetDirectorySeparator());
        outModalFile.append(token);
        outModalFile.append(fileNamePart);
        outModalFile.append(std::to_string(i));
        outModalFile.append(fileFormat);
        IOUtil::Save(modalImage.GetPointer(), outModalFile);
        outModalFile.clear();
      }
    }
    else
    {
      IOUtil::Save(inputAtTimeStep, inputImagePath);
    }
  }
  catch (const mitk::Exception &e)
  {
    MITK_ERROR << e.GetDescription();
    return;
  }
  // Code calls external process
  std::string command = "nnUNet_predict";
  if (this->GetNoPip())
  {
#ifdef _WIN32
    command = "python";
#else
    command = "python3";
#endif
  }
  for (ModelParams &modelparam : m_ParamQ)
  {
    outDir = IOUtil::CreateTemporaryDirectory("nnunet-out-XXXXXX", this->GetMitkTempDir());
    outputImagePath = outDir + IOUtil::GetDirectorySeparator() + token + "_000.nii.gz";
    modelparam.outputDir = outDir;
    args.clear();
    if (this->GetNoPip())
    {
      args.push_back(scriptPath);
    }
    args.push_back("-i");
    args.push_back(inDir);

    args.push_back("-o");
    args.push_back(outDir);

    args.push_back("-t");
    args.push_back(modelparam.task);

    if (modelparam.model.find("cascade") != std::string::npos)
    {
      args.push_back("-ctr");
    }
    else
    {
      args.push_back("-tr");
    }
    args.push_back(modelparam.trainer);

    args.push_back("-m");
    args.push_back(modelparam.model);

    args.push_back("-p");
    args.push_back(modelparam.planId);

    if (!modelparam.folds.empty())
    {
      args.push_back("-f");
      for (auto fold : modelparam.folds)
      {
        args.push_back(fold);
      }
    }

    args.push_back("--num_threads_nifti_save");
    args.push_back("1"); // fixing to 1

    if (!this->GetMirror())
    {
      args.push_back("--disable_tta");
    }

    if (!this->GetMixedPrecision())
    {
      args.push_back("--disable_mixed_precision");
    }

    if (this->GetEnsemble())
    {
      args.push_back("--save_npz");
    }

    try
    {
      std::string resultsFolderEnv = "RESULTS_FOLDER=" + this->GetModelDirectory();
      itksys::SystemTools::PutEnv(resultsFolderEnv.c_str());
      std::string cudaEnv = "CUDA_VISIBLE_DEVICES=" + std::to_string(this->GetGpuId());
      itksys::SystemTools::PutEnv(cudaEnv.c_str());

      spExec->Execute(this->GetPythonPath(), command, args);
    }
    catch (const mitk::Exception &e)
    {
      /*
      Can't throw mitk exception to the caller. Refer: T28691
      */
      MITK_ERROR << e.GetDescription();
      return;
    }
  }
  if (this->GetEnsemble() && !this->GetPostProcessingJsonDirectory().empty())
  {
    args.clear();
    command = "nnUNet_ensemble";
    outDir = IOUtil::CreateTemporaryDirectory("nnunet-ensemble-out-XXXXXX", this->GetMitkTempDir());
    outputImagePath = outDir + IOUtil::GetDirectorySeparator() + token + "_000.nii.gz";

    args.push_back("-f");
    for (ModelParams &modelparam : m_ParamQ)
    {
      args.push_back(modelparam.outputDir);
    }

    args.push_back("-o");
    args.push_back(outDir);

    if (!this->GetPostProcessingJsonDirectory().empty())
    {
      args.push_back("-pp");
      args.push_back(this->GetPostProcessingJsonDirectory());
    }

    spExec->Execute(this->GetPythonPath(), command, args);
  }
  try
  {
    Image::Pointer outputImage = IOUtil::Load<Image>(outputImagePath);
    previewImage->InitializeByLabeledImage(outputImage);
    previewImage->SetGeometry(inputAtTimeStep->GetGeometry());
    m_InputBuffer = inputAtTimeStep;
    m_OutputBuffer = mitk::LabelSetImage::New();
    m_OutputBuffer->InitializeByLabeledImage(outputImage);
    m_OutputBuffer->SetGeometry(inputAtTimeStep->GetGeometry());
  }
  catch (const mitk::Exception &e)
  {
    MITK_ERROR << e.GetDescription();
    return;
  }
}
