/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitknnUnetTool.h"

#include "mitkIOUtil.h"
#include "mitkImage.h"
#include "mitkLabelSetImage.h"
#include "mitkProcessExecutor.h"
#include <cstdlib>
#include <fstream>
#include <itksys/SystemTools.hxx>
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleContext.h>
#include <usModuleResource.h>

namespace mitk
{
  MITK_TOOL_MACRO(MITKSEGMENTATION_EXPORT, nnUNetTool, "nnUNet tool");
}

mitk::nnUNetTool::nnUNetTool()
{
  this->SetMitkTempDir(mitk::IOUtil::CreateTemporaryDirectory("mitk-XXXXXX"));
  this->test = true;
}

mitk::nnUNetTool::~nnUNetTool()
{
  itksys::SystemTools::RemoveADirectory(this->GetMitkTempDir());
}

void mitk::nnUNetTool::Activated()
{
  Superclass::Activated();
}

void mitk::nnUNetTool::UpdateCleanUp()
{
  MITK_INFO << "In update cleanup ashis: Does nothing here";
}

void mitk::nnUNetTool::RenderSegmentation() // add LabelSetImage::Pointer argument here
{
  Superclass::SetNodeProperties(this->temp);
  try
  {
    if (nullptr != this->GetPreviewSegmentationNode())
    {
      this->GetPreviewSegmentationNode()->SetVisibility(!this->GetSelectedLabels().empty());
    }
    if (this->GetSelectedLabels().empty())
    {
      this->ResetPreviewNode();
    }
  }
  catch (const mitk::Exception &e)
  {
    MITK_INFO << e.GetDescription();
  }
}

void mitk::nnUNetTool::SetNodeProperties(mitk::LabelSetImage::Pointer _temp)
{
  MITK_INFO << "In SetNodeProperties ashis: Does nothing here";
  this->temp = _temp; //remove
}

us::ModuleResource mitk::nnUNetTool::GetIconResource() const
{
  us::Module *module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("Watershed_48x48.png");
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

mitk::LabelSetImage::Pointer mitk::nnUNetTool::ComputeMLPreview(const Image *inputAtTimeStep, TimeStepType /*timeStep*/)
{
  mitk::Image::Pointer _inputAtTimeStep = inputAtTimeStep->Clone();
  std::string inDir, outDir, inputImagePath, outputImagePath, scriptPath;
  std::string templateFilename = "XXXXXX_000_0000.nii.gz";

  mitk::ProcessExecutor::Pointer spExec = mitk::ProcessExecutor::New();
  itk::CStyleCommand::Pointer spCommand = itk::CStyleCommand::New();
  spCommand->SetCallback(&onPythonProcessEvent);
  spExec->AddObserver(mitk::ExternalProcessOutputEvent(), spCommand);
  mitk::ProcessExecutor::ArgumentListType args;

  inDir = mitk::IOUtil::CreateTemporaryDirectory("nnunet-in-XXXXXX", this->GetMitkTempDir());
  std::ofstream tmpStream;
  inputImagePath =
    mitk::IOUtil::CreateTemporaryFile(tmpStream, templateFilename, inDir + mitk::IOUtil::GetDirectorySeparator());
  tmpStream.close(); // required ?
  std::size_t found = inputImagePath.find_last_of(mitk::IOUtil::GetDirectorySeparator());
  std::string fileName = inputImagePath.substr(found + 1);
  std::string token = fileName.substr(0, fileName.find("_"));

  if (this->GetNoPip())
  {
    scriptPath = this->GetnnUNetDirectory() + mitk::IOUtil::GetDirectorySeparator() + "nnunet" +
                 mitk::IOUtil::GetDirectorySeparator() + "inference" + mitk::IOUtil::GetDirectorySeparator() +
                 "predict_simple.py";
    // std::string scriptPath = this->GetnnUNetDirectory() + mitk::IOUtil::GetDirectorySeparator() +
    // "test_process.py";
  }

  try
  {
    std::cout << "saving............ " << std::endl;
    if (!this->test)
      mitk::IOUtil::Save(_inputAtTimeStep.GetPointer(), inputImagePath);
    if (this->GetMultiModal())
    {
      for (size_t i = 0; i < this->otherModalPaths.size(); ++i)
      {
        std::string inModalFile = this->otherModalPaths[i];
        std::string outModalFile =
          inDir + mitk::IOUtil::GetDirectorySeparator() + token + "_000_000" + std::to_string(i + 1) + ".nii.gz";
        std::ifstream src(inModalFile, std::ios::binary);
        std::ofstream dst(outModalFile, std::ios::binary);
        dst << src.rdbuf();
        dst.close();
        src.close();
      }
    }
  }
  catch (const mitk::Exception &e)
  {
    MITK_ERROR << e.GetDescription();
    mitkThrow() << "Error writing 3D stack on to disk.";
    return nullptr;
  }
  // Code calls external process
  std::string command = "nnUNet_predict";
  if (this->GetNoPip())
  {
    command = "python3";
  }

  for (mitk::ModelParams &modelparam : m_ParamQ)
  {
    outDir = mitk::IOUtil::CreateTemporaryDirectory("nnunet-out-XXXXXX", this->GetMitkTempDir());
    outputImagePath = outDir + mitk::IOUtil::GetDirectorySeparator() + token + "_000.nii.gz";
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
    setenv("RESULTS_FOLDER", this->GetModelDirectory().c_str(), true);

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

    // args.push_back("--all_in_gpu");
    // args.push_back(this->GetAllInGPU() ? std::string("True") : std::string("False"));

    args.push_back("--num_threads_preprocessing");
    args.push_back(std::to_string(this->GetPreprocessingThreads()));

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

    if (this->GetEnsemble() && !this->GetPostProcessingJsonDirectory().empty())
    {
      args.push_back("--save_npz");
    }

    try
    {
      setenv("RESULTS_FOLDER", this->GetModelDirectory().c_str(), true);
      setenv("CUDA_VISIBLE_DEVICES", std::to_string(this->GetGpuId()).c_str(), true);
      for (auto arg : args)
      {
        std::cout << arg << std::endl;
      }
      std::cout << "command at " << command << std::endl;
      if (this->test)
      {
        outputImagePath = "/Users/ashis/DKFZ/nnUNet/mitk-32uGaS/nnunet-out-E211EZ/yFw1zN_000.nii.gz";
      }
      else
      {
        spExec->execute(this->GetPythonPath(), command, args);
      }
    }
    catch (const mitk::Exception &e)
    {
      MITK_ERROR << e.GetDescription();
      mitkThrow() << "An error occured while calling nnUNet process.";
      return nullptr;
    }
  }
  if (this->GetEnsemble() && !this->GetPostProcessingJsonDirectory().empty())
  {
    args.clear();
    command = "nnUNet_ensemble";
    outDir = mitk::IOUtil::CreateTemporaryDirectory("nnunet-ensemble-out-XXXXXX", this->GetMitkTempDir());
    outputImagePath = outDir + mitk::IOUtil::GetDirectorySeparator() + token + "_000.nii.gz";

    args.push_back("-f");
    for (mitk::ModelParams &modelparam : m_ParamQ)
    {
      args.push_back(modelparam.outputDir);
    }

    args.push_back("-o");
    args.push_back(outDir);

    args.push_back("-pp");
    args.push_back(this->GetPostProcessingJsonDirectory());

    for (auto arg : args)
    {
      std::cout << arg << std::endl;
    }

    if (this->test)
      outputImagePath = "/Users/ashis/DKFZ/nnUNet/mitk-32uGaS/nnunet-ensemble-out-cJ9sjG/yFw1zN_000.nii.gz";
    else
      spExec->execute(this->GetPythonPath(), command, args);
  }

  mitk::Image::Pointer outputImage = mitk::IOUtil::Load<mitk::Image>(outputImagePath);
  mitk::LabelSetImage::Pointer resultImage = mitk::LabelSetImage::New();
  resultImage->InitializeByLabeledImage(outputImage);
  resultImage->SetGeometry(_inputAtTimeStep->GetGeometry());
  // itksys::SystemTools::RemoveADirectory(mitkTempDir); // moved to destuctor
  return resultImage;
}
