/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// MITK
#include "mitkTotalSegmentatorTool.h"


#include "mitkIOUtil.h"
#include <itksys/SystemTools.hxx>
#include "mitkProcessExecutor.h"

// us
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleContext.h>
#include <usModuleResource.h>
#include <usServiceReference.h>

namespace mitk
{
  MITK_TOOL_MACRO(MITKSEGMENTATION_EXPORT, TotalSegmentatorTool, "Total Segmentator");
}

mitk::TotalSegmentatorTool::TotalSegmentatorTool()
{
  this->SetMitkTempDir(IOUtil::CreateTemporaryDirectory("mitk-XXXXXX"));
}

mitk::TotalSegmentatorTool::~TotalSegmentatorTool()
{
  itksys::SystemTools::RemoveADirectory(this->GetMitkTempDir());
}

void mitk::TotalSegmentatorTool::Activated()
{
  Superclass::Activated();
  this->SetLabelTransferMode(LabelTransferMode::AllLabels);
}

const char **mitk::TotalSegmentatorTool::GetXPM() const
{
  return nullptr;
}

us::ModuleResource mitk::TotalSegmentatorTool::GetIconResource() const
{
  us::Module *module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("AI.svg");
  return resource;
}

const char *mitk::TotalSegmentatorTool::GetName() const
{
  return "Total Segmentator";
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


void mitk::TotalSegmentatorTool::DoUpdatePreview(const Image *inputAtTimeStep,
                                           const Image * /*oldSegAtTimeStep*/,
                                           LabelSetImage *previewImage,
                                           TimeStepType timeStep)
{ 
  std::string inDir, outDir, inputImagePath, outputImagePath, scriptPath;

  ProcessExecutor::Pointer spExec = ProcessExecutor::New();
  itk::CStyleCommand::Pointer spCommand = itk::CStyleCommand::New();
  spCommand->SetCallback(&onPythonProcessEvent);
  spExec->AddObserver(ExternalProcessOutputEvent(), spCommand);

  inDir = IOUtil::CreateTemporaryDirectory("totalseg-in-XXXXXX", this->GetMitkTempDir());
  std::ofstream tmpStream;
  inputImagePath = IOUtil::CreateTemporaryFile(tmpStream, m_TEMPLATE_FILENAME, inDir + IOUtil::GetDirectorySeparator());
  tmpStream.close();
  std::size_t found = inputImagePath.find_last_of(IOUtil::GetDirectorySeparator());
  std::string fileName = inputImagePath.substr(found + 1);
  std::string token = fileName.substr(0, fileName.find("_"));
  outDir = IOUtil::CreateTemporaryDirectory("totalseg-out-XXXXXX", this->GetMitkTempDir());
  outputImagePath = outDir + IOUtil::GetDirectorySeparator() + token + "_000.nii.gz";
  ProcessExecutor::ArgumentListType args;

  MITK_INFO << inputImagePath;
  MITK_INFO << outputImagePath;

  IOUtil::Save(inputAtTimeStep, inputImagePath);
  
   // Code calls external process
  std::string command = "TotalSegmentator";
#ifdef _WIN32
    command = "python";
#else
    command = "TotalSegmentator";
#endif
  
    args.clear();

#ifdef _WIN32
    args.push_back("TotalSegmentator");
#endif

    args.push_back("-i");
    args.push_back(inputImagePath);

    args.push_back("-o");
    args.push_back(outputImagePath);

    args.push_back("--ml");

   try
    {
      std::string cudaEnv = "CUDA_VISIBLE_DEVICES=" + std::to_string(this->GetGpuId());
      itksys::SystemTools::PutEnv(cudaEnv.c_str());

      for (auto &arg : args)
      MITK_INFO << arg;
      MITK_INFO << this->GetPythonPath();

      spExec->Execute(this->GetPythonPath(), command, args);
    }
    catch (const mitk::Exception &e)
    {
      MITK_ERROR << e.GetDescription();
      return;
    }

    Image::Pointer outputImage = IOUtil::Load<Image>(outputImagePath);
    auto outputBuffer = mitk::LabelSetImage::New();
    outputBuffer->InitializeByLabeledImage(outputImage);
    outputBuffer->SetGeometry(inputAtTimeStep->GetGeometry());

    TransferLabelSetImageContent(outputBuffer, previewImage, timeStep);
   
}
