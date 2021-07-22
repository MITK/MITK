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

mitk::nnUNetTool::nnUNetTool() {}

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

void onRegistrationEvent(itk::Object * /*pCaller*/, const itk::EventObject &e, void *)
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
  std::string mitkTempDir, inDir, outDir, inputImagePath, outputImagePath;
  try
  {
    std::string templateFilename = "XXXXXX_000_0000.nii.gz";
    mitkTempDir = mitk::IOUtil::CreateTemporaryDirectory("mitk-XXXXXX");
    inDir = mitk::IOUtil::CreateTemporaryDirectory("nnunet-in-XXXXXX", mitkTempDir);
    outDir = mitk::IOUtil::CreateTemporaryDirectory("nnunet-out-XXXXXX", mitkTempDir);
    std::ofstream tmpStream;
    inputImagePath =
      mitk::IOUtil::CreateTemporaryFile(tmpStream, templateFilename, inDir + mitk::IOUtil::GetDirectorySeparator());
    tmpStream.close();
    std::size_t found = inputImagePath.find_last_of(mitk::IOUtil::GetDirectorySeparator());
    std::string fileName = inputImagePath.substr(found + 1);
    std::string token = fileName.substr(0, fileName.find("_"));
    outputImagePath = outDir + mitk::IOUtil::GetDirectorySeparator() + token + "_000.nii.gz";
  }
  catch (const mitk::Exception &e)
  {
    MITK_ERROR << e.GetDescription();
    mitkThrow() << "Error creating temporary directories or files on disk.";
    return nullptr;
  }
  try
  {
    mitk::IOUtil::Save(_inputAtTimeStep.GetPointer(), inputImagePath);
  }
  catch (const mitk::Exception &e)
  {
    MITK_ERROR << e.GetDescription();
    mitkThrow() << "Error writing 3D stack on to disk.";
    return nullptr;
  }
  // Code calls external process
  std::string callingPath = "/home/AD/a178n/environments/nnunet/bin";
  std::string scriptPath = this->GetnnUNetDirectory() + "/nnunet/inference/predict.py";
  map::utilities::ProcessExecutor::Pointer spExec = map::utilities::ProcessExecutor::New();
  itk::CStyleCommand::Pointer spCommand = itk::CStyleCommand::New();
  spCommand->SetCallback(&onRegistrationEvent);
  spExec->AddObserver(map::events::ExternalProcessOutputEvent(), spCommand);
  map::utilities::ProcessExecutor::ArgumentListType args;
  args.clear();
  args.push_back(scriptPath);

  args.push_back("-i"); // add empty checks since the parameter is 'required'
  args.push_back(inDir);

  args.push_back("-o"); // add empty checks since the parameter is 'required'
  args.push_back(outDir);

  args.push_back("-m");                      // add empty checks since the parameter is 'required'
  args.push_back(this->GetModelDirectory() + mitk::IOUtil::GetDirectorySeparator() + this->GetModel()
                                           + mitk::IOUtil::GetDirectorySeparator() + this->GetTask()
                                           + mitk::IOUtil::GetDirectorySeparator() + this->GetTrainer());

  // args.push_back("--all_in_gpu");
  // args.push_back(this->GetAllInGPU() ? std::string("True") : std::string("False"));

  // args.push_back("-z");
  // args.push_back(this->GetExportSegmentation()? std::string("True") : std::string("False"));

  args.push_back("--num_threads_preprocessing");
  args.push_back(std::to_string(this->GetPreprocessingThreads()));

  args.push_back("--tta");
  args.push_back(this->GetMirror() ? std::string("1") : std::string("0"));

  args.push_back("-f");
  args.push_back(this->GetFold());

  if (!this->GetMixedPrecision())
  {
    args.push_back("--disable_mixed_precision");
  }

  try
  {
    spExec->execute(callingPath, "python3", args);
  }
  catch (const mitk::Exception &e)
  {
    MITK_ERROR << e.GetDescription();
    mitkThrow() << "An error occured while calling nnUNet.";
    return nullptr;
  }

  mitk::Image::Pointer outputImage = mitk::IOUtil::Load<mitk::Image>(outputImagePath);
  mitk::LabelSetImage::Pointer resultImage = mitk::LabelSetImage::New();
  resultImage->InitializeByLabeledImage(outputImage);
  resultImage->SetGeometry(inputAtTimeStep->GetGeometry());
  itksys::SystemTools::RemoveADirectory(mitkTempDir);
  return resultImage;
}