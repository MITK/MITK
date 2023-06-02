/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkSegmentAnythingPythonService.h"

#include "mitkIOUtil.h"
#include <itksys/SystemTools.hxx>
#include <chrono>
#include <thread>

using namespace std::chrono_literals;

namespace mitk
{
  const std::string SIGNALCONSTANTS::READY = "READY";
  const std::string SIGNALCONSTANTS::KILL = "KILL";
  bool mitk::SegmentAnythingPythonService::IsPythonReady = false;
}

mitk::SegmentAnythingPythonService::SegmentAnythingPythonService(
  std::string workingDir, std::string inDir, std::string outDir, std::string modelType, std::string checkPointPath, unsigned int gpuId)
  : m_PythonPath(workingDir),
    m_InDir(inDir),
    m_OutDir(outDir),
    m_ModelType(modelType),
    m_CheckpointPath(checkPointPath),
    m_GpuId(gpuId) {}

mitk::SegmentAnythingPythonService::~SegmentAnythingPythonService()
{
  if (mitk::SegmentAnythingPythonService::IsPythonReady)
  {
    this->StopAsyncProcess();
  }
  mitk::SegmentAnythingPythonService::IsPythonReady = false;
}

void mitk::SegmentAnythingPythonService::onPythonProcessEvent(itk::Object * /*pCaller*/,
                                                              const itk::EventObject &e,
                                                              void *)
{
  std::string testCOUT,testCERR;
  const auto *pEvent = dynamic_cast<const mitk::ExternalProcessStdOutEvent *>(&e);
  if (pEvent)
  {
    testCOUT = testCOUT + pEvent->GetOutput();
    if (SIGNALCONSTANTS::READY == testCOUT)
    {
      mitk::SegmentAnythingPythonService::IsPythonReady = true;
    }
    if (SIGNALCONSTANTS::KILL == testCOUT)
    {
      mitk::SegmentAnythingPythonService::IsPythonReady = false;
    }
    MITK_INFO << testCOUT;
  }
  const auto *pErrEvent = dynamic_cast<const mitk::ExternalProcessStdErrEvent *>(&e);
  if (pErrEvent)
  {
    testCERR = testCERR + pErrEvent->GetOutput();
    MITK_ERROR << testCERR;
  }
}

void mitk::SegmentAnythingPythonService::StopAsyncProcess()
{
  std::stringstream controlStream;
  controlStream << SIGNALCONSTANTS::KILL;
  this->WriteControlFile(controlStream);
  m_Future.get();
}

void mitk::SegmentAnythingPythonService::StartAsyncProcess()
{
  if (nullptr != m_DaemonExec)
  {
    this->StopAsyncProcess();
  }
  std::stringstream controlStream;
  controlStream << SIGNALCONSTANTS::READY;
  this->WriteControlFile(controlStream);

  m_DaemonExec = ProcessExecutor::New();
  itk::CStyleCommand::Pointer spCommand = itk::CStyleCommand::New();
  spCommand->SetCallback(&mitk::SegmentAnythingPythonService::onPythonProcessEvent);
  m_DaemonExec->AddObserver(ExternalProcessOutputEvent(), spCommand);
  m_Future = std::async(std::launch::async, &mitk::SegmentAnythingPythonService::start_python_daemon, this);
}


void mitk::SegmentAnythingPythonService::WriteControlFile(std::stringstream &statusStream)
{
  std::string controlFilePath = m_InDir + IOUtil::GetDirectorySeparator() + "control.txt";
  std::ofstream controlFile;
  controlFile.open(controlFilePath, std::ofstream::out | std::ofstream::trunc);
  controlFile << statusStream.rdbuf();
  controlFile.close();
}

void mitk::SegmentAnythingPythonService::start_python_daemon()
{
  ProcessExecutor::ArgumentListType args;
  std::string command = "python";
  args.push_back("-u");

  args.push_back("C:\\DKFZ\\SAM_work\\sam-playground\\endpoints\\run_inference_daemon.py");
  
  args.push_back("--input-folder");
  args.push_back(m_InDir);

  args.push_back("--output-folder");
  args.push_back(m_OutDir);

  args.push_back("--trigger-file");
  args.push_back(m_TRIGGER_FILENAME);

  args.push_back("--model-type");
  args.push_back(m_ModelType);

  args.push_back("--checkpoint");
  args.push_back(m_CheckpointPath);

  try
  {
    std::string cudaEnv = "CUDA_VISIBLE_DEVICES=" + std::to_string(m_GpuId);
    itksys::SystemTools::PutEnv(cudaEnv.c_str());

    std::stringstream logStream;
    for (const auto &arg : args)
      logStream << arg << " ";
    logStream << m_PythonPath;
    MITK_INFO << logStream.str();

    m_DaemonExec->Execute(m_PythonPath, command, args);
  }
  catch (const mitk::Exception &e)
  {
    MITK_ERROR << e.GetDescription();
    return;
  }
  MITK_INFO << "ending python process.....";
}
