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
#include <filesystem>
#include <itksys/SystemTools.hxx>

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

void mitk::TotalSegmentatorTool::onPythonProcessEvent(itk::Object * /*pCaller*/, const itk::EventObject &e, void *)
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

void mitk::TotalSegmentatorTool::DoUpdatePreview(const Image *inputAtTimeStep,
                                                 const Image * /*oldSegAtTimeStep*/,
                                                 LabelSetImage *previewImage,
                                                 TimeStepType timeStep)
{
  if (!m_IsMitkTempDirAvailable) // create temp directory if not done already
  {
    this->SetMitkTempDir(IOUtil::CreateTemporaryDirectory("mitk-XXXXXX"));
    m_IsMitkTempDirAvailable = true;
  }
  this->ParseLabelNames(this->GetLabelMapPath());

  ProcessExecutor::Pointer spExec = ProcessExecutor::New();
  itk::CStyleCommand::Pointer spCommand = itk::CStyleCommand::New();
  spCommand->SetCallback(&onPythonProcessEvent);
  spExec->AddObserver(ExternalProcessOutputEvent(), spCommand);

  std::string inDir, outDir, inputImagePath, outputImagePath, scriptPath;
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

  IOUtil::Save(inputAtTimeStep, inputImagePath);

  std::string *outArg = &outputImagePath;
  bool isSubTask = false;
  if (this->GetSubTask() != m_DEFAULT_TOTAL_TASK)
  {
    isSubTask = true;
    outputImagePath = outDir + IOUtil::GetDirectorySeparator() + this->GetSubTask() + ".nii.gz";
    outArg = &outDir;
  }

  this->run_totalsegmentator(
    spExec, inputImagePath, *outArg, this->GetFast(), !isSubTask, this->GetGpuId(), m_DEFAULT_TOTAL_TASK);

  if (isSubTask)
  {
    this->run_totalsegmentator(
      spExec, inputImagePath, *outArg, !isSubTask, !isSubTask, this->GetGpuId(), this->GetSubTask());
  }

  Image::Pointer outputImage = IOUtil::Load<Image>(outputImagePath);
  auto outputBuffer = mitk::LabelSetImage::New();
  outputBuffer->InitializeByLabeledImage(outputImage);
  outputBuffer->SetGeometry(inputAtTimeStep->GetGeometry());
  this->MapLabelsToSegmentation(outputBuffer, m_LabelMapTotal);
  TransferLabelSetImageContent(outputBuffer, previewImage, timeStep);
}

void mitk::TotalSegmentatorTool::run_totalsegmentator(ProcessExecutor::Pointer spExec,
                                                      const std::string &inputImagePath,
                                                      const std::string &outputImagePath,
                                                      bool isFast,
                                                      bool isMultiLabel,
                                                      unsigned int gpuId,
                                                      const std::string &subTask)
{
  ProcessExecutor::ArgumentListType args;
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

  if (subTask != m_DEFAULT_TOTAL_TASK)
  {
    args.push_back("-ta");
    args.push_back(subTask);
  }

  if (isMultiLabel)
  {
    args.push_back("--ml");
  }

  if (isFast)
  {
    args.push_back("--fast");
  }

  try
  {
    std::string cudaEnv = "CUDA_VISIBLE_DEVICES=" + std::to_string(gpuId);
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
}

void mitk::TotalSegmentatorTool::ParseLabelNames(const std::string &fileName)
{
  std::fstream newfile;
  newfile.open(fileName, ios::in);
  std::stringstream buffer;
  if (newfile.is_open())
  {
    int line = 0;
    std::string temp;
    while (std::getline(newfile, temp))
    {
      if (line > 1 && line < 106)
      {
        buffer << temp;
      }
      ++line;
    }
  }
  std::string key, val;
  while (std::getline(std::getline(buffer, key, ':'), val, ','))
  {
    m_LabelMapTotal[std::stoi(key)] = val;
  }
}

void mitk::TotalSegmentatorTool::MapLabelsToSegmentation(mitk::LabelSetImage::Pointer outputBuffer,
                                                         std::map<int, std::string> &labelMap)
{
  for (auto const &[key, val] : labelMap)
  {
    mitk::Label *labelptr = outputBuffer->GetLabel(key, 0);
    if (nullptr != labelptr)
    {
      labelptr->SetName(val);
    }
  }
}

std::string mitk::TotalSegmentatorTool::GetLabelMapPath()
{
  std::string pythonFileName;
  std::filesystem::path pathToLabelMap(this->GetPythonPath());
  pathToLabelMap = pathToLabelMap.parent_path();
#ifdef _WIN32
  pythonFileName = pathToLabelMap.string() + "/Lib/site-packages/totalsegmentator/map_to_binary.py";
#else
  pathToLabelMap.append("lib");
  for (auto const &dir_entry : std::filesystem::directory_iterator{pathToLabelMap})
  {
    if (dir_entry.is_directory())
    {
      auto dirName = dir_entry.path().filename().string();
      if (dirName.rfind("python", 0) == 0)
      {
        pathToLabelMap.append(dir_entry.path().filename().string());
        break;
      }
    }
  }
  pythonFileName = pathToLabelMap.string() + "/site-packages/totalsegmentator/map_to_binary.py";
#endif
  return pythonFileName;
}
