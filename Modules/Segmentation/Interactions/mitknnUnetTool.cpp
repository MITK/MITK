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
#include <cstdlib>
#include <itksys/SystemTools.hxx>
#include <mapProcessExecutor.h>
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleContext.h>
#include <usModuleResource.h>

namespace mitk
{
  MITK_TOOL_MACRO(MITKSEGMENTATION_EXPORT, nnUNetTool, "nnUNet tool");
}

mitk::nnUNetTool::nnUNetTool() {
  std::cout<< "in mitk nnUnet constructor"<< std::endl;
  this->SetMitkTempDir(mitk::IOUtil::CreateTemporaryDirectory("mitk-XXXXXX"));
}

mitk::nnUNetTool::~nnUNetTool() {
  std::cout<< "in mitk nnUnet destructor"<< std::endl;
  itksys::SystemTools::RemoveADirectory(this->GetMitkTempDir());
}

void mitk::nnUNetTool::Activated()
{
  Superclass::Activated();
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

void onPythonProcessEvent(itk::Object * /*pCaller*/, const itk::EventObject &e, void *)
{
  std::string testCOUT;
  std::string testCERR;
  const map::events::ExternalProcessStdOutEvent *pEvent =
    dynamic_cast<const map::events::ExternalProcessStdOutEvent *>(&e);

  if (pEvent)
  {
    testCOUT = testCOUT + pEvent->getComment();
    std::cout << testCOUT << "\n";
  }

  const map::events::ExternalProcessStdErrEvent *pErrEvent =
    dynamic_cast<const map::events::ExternalProcessStdErrEvent *>(&e);

  if (pErrEvent)
  {
    testCERR = testCERR + pErrEvent->getComment();
  }
}

mitk::LabelSetImage::Pointer mitk::nnUNetTool::ComputeMLPreview(const Image *inputAtTimeStep, TimeStepType /*timeStep*/)
{
  mitk::Image::Pointer _inputAtTimeStep = inputAtTimeStep->Clone();
  std::string inDir, outDir, inputImagePath, outputImagePath, scriptPath;
  std::string templateFilename = "XXXXXX_000_0000.nii.gz";

  map::utilities::ProcessExecutor::Pointer spExec = map::utilities::ProcessExecutor::New();
  itk::CStyleCommand::Pointer spCommand = itk::CStyleCommand::New();
  spCommand->SetCallback(&onPythonProcessEvent);
  spExec->AddObserver(map::events::ExternalProcessOutputEvent(), spCommand);
  map::utilities::ProcessExecutor::ArgumentListType args;

  inDir = mitk::IOUtil::CreateTemporaryDirectory("nnunet-in-XXXXXX", this->GetMitkTempDir());
  std::ofstream tmpStream;
  inputImagePath =
    mitk::IOUtil::CreateTemporaryFile(tmpStream, templateFilename, inDir + mitk::IOUtil::GetDirectorySeparator());
  tmpStream.close();
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
    mitk::IOUtil::Save(_inputAtTimeStep.GetPointer(), inputImagePath);
    if (this->GetMultiModal())
    {
      /*copy & rename file */
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

  for (mitk::ModelParams modelparam : m_Params)
  {
    outDir = mitk::IOUtil::CreateTemporaryDirectory("nnunet-out-XXXXXX", this->GetMitkTempDir());
    outputImagePath = outDir + mitk::IOUtil::GetDirectorySeparator() + token + "_000.nii.gz";
    modelparam.m_OutputPath = outDir;
    args.clear();
    if (this->GetNoPip())
    {
      args.push_back(scriptPath);
    }
    args.push_back("-i"); // add empty checks since the parameter is 'required'
    args.push_back(inDir);

    args.push_back("-o"); // add empty checks since the parameter is 'required'
    args.push_back(outDir);

    args.push_back("-t");
    args.push_back(modelparam.m_Task);

    if (modelparam.m_Model.find("cascade") != std::string::npos)
    {
      args.push_back("-ctr");
    }
    else
    {
      args.push_back("-tr");
    }
    args.push_back(modelparam.m_Trainer);

    args.push_back("-m");
    args.push_back(modelparam.m_Model);
    setenv("RESULTS_FOLDER", this->GetModelDirectory().c_str(), true);

    args.push_back("-p");
    args.push_back(modelparam.m_PlanId);

    if (!modelparam.m_Folds.empty())
    {
      args.push_back("-f");
      for (auto fold : modelparam.m_Folds)
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
      for (auto arg : args)
      {
        std::cout << arg << std::endl;
      }
      std::cout << "command " << command << std::endl;
      // spExec->execute(this->GetPythonPath(), command, args);
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
    for (mitk::ModelParams modelparam : m_Params)
    {
      args.push_back(modelparam.m_OutputPath);
    }

    args.push_back("-pp");
    args.push_back(this->GetPostProcessingJsonDirectory());

    // spExec->execute(this->GetPythonPath(), command, args);
  }

  mitk::Image::Pointer outputImage = mitk::IOUtil::Load<mitk::Image>(outputImagePath);
  mitk::LabelSetImage::Pointer resultImage = mitk::LabelSetImage::New();
  resultImage->InitializeByLabeledImage(outputImage);
  resultImage->SetGeometry(inputAtTimeStep->GetGeometry());
  //itksys::SystemTools::RemoveADirectory(mitkTempDir); // move to destuctor
  return resultImage;
}

// mitk::LabelSetImage::Pointer mitk::nnUNetTool::ComputeMLPreview(const Image *inputAtTimeStep, TimeStepType
// /*timeStep*/)
// {
//   mitk::Image::Pointer _inputAtTimeStep = inputAtTimeStep->Clone();
//   std::string mitkTempDir, inDir, outDir, inputImagePath, outputImagePath;
//   try
//   {
//     std::string templateFilename = "XXXXXX_000_0000.nii.gz";
//     mitkTempDir = mitk::IOUtil::CreateTemporaryDirectory("mitk-XXXXXX");
//     inDir = mitk::IOUtil::CreateTemporaryDirectory("nnunet-in-XXXXXX", mitkTempDir);
//     outDir = mitk::IOUtil::CreateTemporaryDirectory("nnunet-out-XXXXXX", mitkTempDir);
//     std::ofstream tmpStream;
//     inputImagePath =
//       mitk::IOUtil::CreateTemporaryFile(tmpStream, templateFilename, inDir + mitk::IOUtil::GetDirectorySeparator());
//     tmpStream.close();
//     std::size_t found = inputImagePath.find_last_of(mitk::IOUtil::GetDirectorySeparator());
//     std::string fileName = inputImagePath.substr(found + 1);
//     std::string token = fileName.substr(0, fileName.find("_"));
//     outputImagePath = outDir + mitk::IOUtil::GetDirectorySeparator() + token + "_000.nii.gz";
//   }
//   catch (const mitk::Exception &e)
//   {
//     MITK_ERROR << e.GetDescription();
//     mitkThrow() << "Error creating temporary directories or files on disk.";
//     return nullptr;
//   }
//   try
//   {
//     mitk::IOUtil::Save(_inputAtTimeStep.GetPointer(), inputImagePath);
//     if (this->GetMultiModal())
//     {
//       /*copy & rename file */
//     }
//   }
//   catch (const mitk::Exception &e)
//   {
//     MITK_ERROR << e.GetDescription();
//     mitkThrow() << "Error writing 3D stack on to disk.";
//     return nullptr;
//   }
//   // Code calls external process
//   std::string command = "nnUNet_predict";
//   if (this->GetNoPip())
//   {
//     command = "python3";
//   }
//   map::utilities::ProcessExecutor::Pointer spExec = map::utilities::ProcessExecutor::New();
//   itk::CStyleCommand::Pointer spCommand = itk::CStyleCommand::New();
//   spCommand->SetCallback(&onRegistrationEvent);
//   spExec->AddObserver(map::events::ExternalProcessOutputEvent(), spCommand);
//   map::utilities::ProcessExecutor::ArgumentListType args;
//   args.clear();
//   if (this->GetNoPip())
//   {
//     std::string scriptPath = this->GetnnUNetDirectory() + mitk::IOUtil::GetDirectorySeparator() + "nnunet" +
//                              mitk::IOUtil::GetDirectorySeparator() + "inference" +
//                              mitk::IOUtil::GetDirectorySeparator() + "predict_simple.py";
//     // std::string scriptPath = this->GetnnUNetDirectory() + mitk::IOUtil::GetDirectorySeparator() +
//     "test_process.py"; args.push_back(scriptPath);
//   }
//   args.push_back("-i"); // add empty checks since the parameter is 'required'
//   args.push_back(inDir);

//   args.push_back("-o"); // add empty checks since the parameter is 'required'
//   args.push_back(outDir);

//   args.push_back("-t");
//   args.push_back(this->GetTask());

//   if (this->GetModel().find("cascade") != std::string::npos)
//   {
//     args.push_back("-ctr");
//     args.push_back(this->GetTrainer());
//   }
//   else
//   {
//     args.push_back("-tr");
//     args.push_back(this->GetTrainer());
//   }
//   args.push_back("-m");
//   args.push_back(this->GetModel());
//   setenv("RESULTS_FOLDER", this->GetModelDirectory().c_str(), true);

//   args.push_back("-p");
//   args.push_back(this->GetPlanId());

//   if (!this->m_Folds.empty())
//   {
//     args.push_back("-f");
//     for (auto fold : this->m_Folds)
//     {
//       args.push_back(fold);
//     }
//   }

//   // args.push_back("--all_in_gpu");
//   // args.push_back(this->GetAllInGPU() ? std::string("True") : std::string("False"));

//   // args.push_back("-z");
//   // args.push_back(this->GetExportSegmentation()? std::string("True") : std::string("False"));

//   args.push_back("--num_threads_preprocessing");
//   args.push_back(std::to_string(this->GetPreprocessingThreads()));

//   args.push_back("--num_threads_nifti_save");
//   args.push_back("1"); // fixing to 1

//   if (!this->GetMirror())
//   {
//     args.push_back("--disable_tta");
//   }

//   if (!this->GetMixedPrecision())
//   {
//     args.push_back("--disable_mixed_precision");
//   }

//   try
//   {
//     for (auto arg : args)
//     {
//       std::cout << arg << std::endl;
//     }
//     std::cout << "command " << command << std::endl;
//     spExec->execute(this->GetPythonPath(), command, args);
//   }
//   catch (const mitk::Exception &e)
//   {
//     MITK_ERROR << e.GetDescription();
//     mitkThrow() << "An error occured while calling nnUNet process.";
//     return nullptr;
//   }

//   mitk::Image::Pointer outputImage = mitk::IOUtil::Load<mitk::Image>(outputImagePath);
//   mitk::LabelSetImage::Pointer resultImage = mitk::LabelSetImage::New();
//   resultImage->InitializeByLabeledImage(outputImage);
//   resultImage->SetGeometry(inputAtTimeStep->GetGeometry());
//   itksys::SystemTools::RemoveADirectory(mitkTempDir);
//   return resultImage;
// }