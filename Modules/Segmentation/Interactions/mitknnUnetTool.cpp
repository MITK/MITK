/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitknnUnetTool.h"

#include "mitkImage.h"
#include "mitkLabelSetImage.h"
#include "mitkIOUtil.h"
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleContext.h>
#include <usModuleResource.h>
#include <itksys/SystemTools.hxx>
#include <mapProcessExecutor.h>

namespace mitk
{
  MITK_TOOL_MACRO(MITKSEGMENTATION_EXPORT, nnUNetTool, "nnUNet tool");
}

mitk::nnUNetTool::nnUNetTool()
  : m_PythonProjectPath{"Modules/Segmentation/nnUNet/"},
    m_PythonFileName{"test_mitk.py"},
    m_InputImageVarName{"mitkimage"},
    m_OutputImageVarName{"output_image"}
{
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


void onRegistrationEvent(itk::Object* /*pCaller*/, const itk::EventObject& e, void*)
{
      		map::core::String testCOUT;
		map::core::String testCERR;
			const map::events::ExternalProcessStdOutEvent* pEvent =
				dynamic_cast<const map::events::ExternalProcessStdOutEvent*>(&e);

			if (pEvent)
			{
				testCOUT = testCOUT + pEvent->getComment();
        std::cout<< testCOUT << "\n";
			}

			const map::events::ExternalProcessStdErrEvent* pErrEvent =
				dynamic_cast<const map::events::ExternalProcessStdErrEvent*>(&e);

			if (pErrEvent)
			{
				testCERR = testCERR + pErrEvent->getComment();
			}
}

mitk::LabelSetImage::Pointer mitk::nnUNetTool::ComputeMLPreview(const Image* inputAtTimeStep, TimeStepType /*timeStep*/)
{
  mitk::Image::Pointer _inputAtTimeStep = inputAtTimeStep->Clone();
  std::cout<< "In process call" << "\n";
  std::ofstream tmpStream;
  std::string imagePath = mitk::IOUtil::CreateTemporaryFile(tmpStream, "diffpic3d-XXXXXX.nii.gz");
  tmpStream.close();
  mitk::IOUtil::Save(_inputAtTimeStep.GetPointer(), imagePath);
  std::cout<< "saved image path:" <<imagePath << "\n";

  // Code calls external process
  std::string callingPath = "/home/AD/a178n/environments/nnunet/bin";
  map::utilities::ProcessExecutor::Pointer spExec = map::utilities::ProcessExecutor::New();
  itk::CStyleCommand::Pointer spCommand = itk::CStyleCommand::New();
  spCommand->SetCallback(&onRegistrationEvent);
  spExec->AddObserver(map::events::ExternalProcessOutputEvent(), spCommand);
  map::utilities::ProcessExecutor::ArgumentListType args;
  args.clear();
  args.push_back("/home/AD/a178n/DKFZ/nnUNet_T28068/nnunet/test_mitk_process.py");

  args.push_back("-i");
  args.push_back(imagePath);

  args.push_back("-o");
  args.push_back(this->GetOutputDirectory());

  args.push_back("-m");
  args.push_back(this->GetModel());
  
  args.push_back("-mp");
  args.push_back(this->GetModelDirectory());

  args.push_back("-t");
  args.push_back(this->GetTask());

  args.push_back("--use_gpu");
  args.push_back((this->GetUseGPU() ? std::string("True") : std::string("False")));
  
  args.push_back("--all_in_gpu");
  args.push_back(this->GetAllInGPU() ? std::string("True") : std::string("False"));

  //args.push_back("-z");
  //args.push_back(this->GetExportSegmentation()? std::string("True") : std::string("False"));

  args.push_back("--num_threads_preprocessing");
  args.push_back(std::to_string(this->GetPreprocessingThreads()));

  args.push_back("--mixed_precision");
  args.push_back(this->GetMixedPrecision()? std::string("True") : std::string("False"));

  args.push_back("-tta");
  args.push_back(this->GetMirror()? std::string("True") : std::string("False"));

  spExec->execute(callingPath, "python3", args);
  mitk::Image::Pointer outputImage = mitk::IOUtil::Load<mitk::Image>(this->GetOutputDirectory()+"/la_000_0000.nii.gz");
  mitk::LabelSetImage::Pointer resultImage = mitk::LabelSetImage::New();
  resultImage->InitializeByLabeledImage(outputImage);
  resultImage->SetGeometry(inputAtTimeStep->GetGeometry());
  return resultImage;

}
