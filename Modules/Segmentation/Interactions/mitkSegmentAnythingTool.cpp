/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkSegmentAnythingTool.h"

#include "mitkProperties.h"
#include "mitkToolManager.h"

#include "mitkInteractionPositionEvent.h"
// us
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleContext.h>
#include <usModuleResource.h>

#include "mitkIOUtil.h"
#include "mitkSegTool2D.h"
#include <filesystem>
#include <itksys/SystemTools.hxx>
#include <chrono>
#include <thread>

using namespace std::chrono_literals;

namespace mitk
{
  MITK_TOOL_MACRO(MITKSEGMENTATION_EXPORT, SegmentAnythingTool, "SegmentAnythingTool");
  const std::string SIGNALCONSTANTS::READY = "READY";
  const std::string SIGNALCONSTANTS::KILL = "KILL";
  bool mitk::SegmentAnythingTool::IsPythonReady = false;
}

mitk::SegmentAnythingTool::SegmentAnythingTool() : SegWithPreviewTool(false, "PressMoveReleaseAndPointSetting")
{
  this->ResetsToEmptyPreviewOn();
  this->IsTimePointChangeAwareOff();
}

mitk::SegmentAnythingTool::~SegmentAnythingTool()
{
  this->StopAsyncProcess();
  std::filesystem::remove_all(this->GetMitkTempDir());
}

const char **mitk::SegmentAnythingTool::GetXPM() const
{
  return nullptr;
}

const char *mitk::SegmentAnythingTool::GetName() const
{
  return "SAM";
}

us::ModuleResource mitk::SegmentAnythingTool::GetIconResource() const
{
  us::Module *module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("AI.svg");
  return resource;
}

void mitk::SegmentAnythingTool::Activated()
{
  Superclass::Activated();
  m_PointSetPositive = mitk::PointSet::New();
  //ensure that the seed points are visible for all timepoints.
  //dynamic_cast<ProportionalTimeGeometry*>(m_PointSet->GetTimeGeometry())->SetStepDuration(std::numeric_limits<TimePointType>::max());
  m_PointSetNode = mitk::DataNode::New();
  m_PointSetNode->SetData(m_PointSetPositive);
  m_PointSetNode->SetName(std::string(this->GetName()) + "_PointSetPositive");
  m_PointSetNode->SetBoolProperty("helper object", true);
  m_PointSetNode->SetColor(0.0, 1.0, 0.0);
  m_PointSetNode->SetVisibility(true);
  this->GetDataStorage()->Add(m_PointSetNode, this->GetToolManager()->GetWorkingData(0));

  m_PointSetNegative = mitk::PointSet::New();
  m_PointSetNodeNegative = mitk::DataNode::New();
  m_PointSetNodeNegative->SetData(m_PointSetNegative);
  m_PointSetNodeNegative->SetName(std::string(this->GetName()) + "_PointSetNegative");
  m_PointSetNodeNegative->SetBoolProperty("helper object", true);
  m_PointSetNodeNegative->SetColor(1.0, 0.0, 0.0);
  m_PointSetNodeNegative->SetVisibility(true);
  this->GetDataStorage()->Add(m_PointSetNodeNegative, this->GetToolManager()->GetWorkingData(0));

  this->SetLabelTransferScope(LabelTransferScope::AllLabels);
  this->SetLabelTransferMode(LabelTransferMode::AddLabel);
}

void mitk::SegmentAnythingTool::Deactivated()
{
  this->ClearSeeds();
  // remove from data storage and disable interaction
  GetDataStorage()->Remove(m_PointSetNode);
  GetDataStorage()->Remove(m_PointSetNodeNegative);
  m_PointSetNode = nullptr;
  m_PointSetNodeNegative = nullptr;
  m_PointSetPositive = nullptr;
  m_PointSetNegative = nullptr;
  Superclass::Deactivated();
}

void mitk::SegmentAnythingTool::ConnectActionsAndFunctions()
{
  CONNECT_FUNCTION("ShiftSecondaryButtonPressed", OnAddNegativePoint);
  CONNECT_FUNCTION("ShiftPrimaryButtonPressed", OnAddPoint);
  CONNECT_FUNCTION("DeletePoint", OnDelete);
}

void mitk::SegmentAnythingTool::OnAddNegativePoint(StateMachineAction *, InteractionEvent *interactionEvent)
{
  if (!this->GetIsReady() || m_PointSetPositive->GetSize() == 0)
  {
    return;
  }
  if (!this->IsUpdating() && m_PointSetNegative.IsNotNull())
  {
    const auto positionEvent = dynamic_cast<mitk::InteractionPositionEvent *>(interactionEvent);
    if (positionEvent != nullptr)
    {
      m_PointSetNegative->InsertPoint(m_PointSetCount, positionEvent->GetPositionInWorld());
      m_PointSetCount++;
      this->UpdatePreview();
    }
  }
}

void mitk::SegmentAnythingTool::OnAddPoint(StateMachineAction*, InteractionEvent* interactionEvent)
{
  if (!this->GetIsReady())
  {
    return;
  }
  m_IsGenerateEmbeddings = false; 
  if ((nullptr == this->GetWorkingPlaneGeometry()) || !mitk::Equal(*(interactionEvent->GetSender()->GetCurrentWorldPlaneGeometry()), *(this->GetWorkingPlaneGeometry())))
  {
    m_IsGenerateEmbeddings = true;
    this->ClearSeeds();
    this->SetWorkingPlaneGeometry(interactionEvent->GetSender()->GetCurrentWorldPlaneGeometry()->Clone());
  }
  if (!this->IsUpdating() && m_PointSetPositive.IsNotNull())
  {
    const auto positionEvent = dynamic_cast<mitk::InteractionPositionEvent*>(interactionEvent);
    if (positionEvent != nullptr)
    {
      m_PointSetPositive->InsertPoint(m_PointSetCount, positionEvent->GetPositionInWorld());
      m_PointSetCount++;
      this->UpdatePreview();
    }
  }
}

void mitk::SegmentAnythingTool::OnDelete(StateMachineAction*, InteractionEvent* /*interactionEvent*/)
{
  if (!this->IsUpdating() && m_PointSetPositive.IsNotNull())
  {
    PointSet::Pointer removeSet = m_PointSetPositive;
    decltype(m_PointSetPositive->GetMaxId().Index()) maxId = 0;
    if (m_PointSetPositive->GetSize() > 0)
    {
      maxId = m_PointSetPositive->GetMaxId().Index();
    }
    if (m_PointSetNegative->GetSize() > 0 && (maxId < m_PointSetNegative->GetMaxId().Index()))
    {
      removeSet = m_PointSetNegative;
    }
    removeSet->RemovePointAtEnd(0);
    --m_PointSetCount;
    this->UpdatePreview();
  }
}

void mitk::SegmentAnythingTool::ClearPicks()
{
  this->ClearSeeds();
  this->UpdatePreview();
}

bool mitk::SegmentAnythingTool::HasPicks() const
{
  return this->m_PointSetPositive.IsNotNull() && this->m_PointSetPositive->GetSize()>0;
}

void mitk::SegmentAnythingTool::ClearSeeds()
{
  if (this->m_PointSetPositive.IsNotNull())
  {
    m_PointSetCount -= m_PointSetPositive->GetSize();
    this->m_PointSetPositive = mitk::PointSet::New();  // renew pointset
    //ensure that the seed points are visible for all timepoints.
    dynamic_cast<ProportionalTimeGeometry*>(m_PointSetPositive->GetTimeGeometry())->SetStepDuration(std::numeric_limits<TimePointType>::max());
    this->m_PointSetNode->SetData(this->m_PointSetPositive);
  }
  if (this->m_PointSetNegative.IsNotNull())
  {
    m_PointSetCount -= m_PointSetNegative->GetSize();
    this->m_PointSetNegative = mitk::PointSet::New(); // renew pointset
    // ensure that the seed points are visible for all timepoints.
    dynamic_cast<ProportionalTimeGeometry *>(m_PointSetNegative->GetTimeGeometry())->SetStepDuration(std::numeric_limits<TimePointType>::max());
    this->m_PointSetNodeNegative->SetData(this->m_PointSetNegative);
  }
}

void mitk::SegmentAnythingTool::onPythonProcessEvent(itk::Object * /*pCaller*/, const itk::EventObject &e, void *)
{
  std::string testCOUT;
  std::string testCERR;
  const auto *pEvent = dynamic_cast<const mitk::ExternalProcessStdOutEvent *>(&e);
  if (pEvent)
  {
    testCOUT = testCOUT + pEvent->GetOutput();
    if (SIGNALCONSTANTS::READY == testCOUT)
    {
      mitk::SegmentAnythingTool::IsPythonReady = true;
    }
    if (SIGNALCONSTANTS::KILL == testCOUT)
    {
      mitk::SegmentAnythingTool::IsPythonReady = false;
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

void mitk::SegmentAnythingTool::DoUpdatePreview(const Image* inputAtTimeStep, const Image* oldSegAtTimeStep, LabelSetImage* previewImage, TimeStepType timeStep)
{
  if (nullptr != oldSegAtTimeStep && nullptr != previewImage && m_PointSetPositive.IsNotNull())
  {
    if (this->m_MitkTempDir.empty())
    {
      MITK_INFO << "Python Path: " << this->GetPythonPath();
      MITK_INFO << "Checkpoint Path: " << this->GetCheckpointPath();
      MITK_INFO << "Model type: " << this->GetModelType();
      this->CreateTempDirs(m_PARENT_TEMP_DIR_PATTERN);
    }
    if (this->HasPicks())
    {
      std::string uniquePlaneID = GetHashForCurrentPlane();
      std::string inputImagePath = m_InDir + IOUtil::GetDirectorySeparator() + uniquePlaneID + ".nii.gz";
      outputImagePath = m_OutDir + IOUtil::GetDirectorySeparator() + uniquePlaneID + ".nii.gz";
      IOUtil::Save(inputAtTimeStep, inputImagePath);
      auto csvStream = this->GetPointsAsCSVString(inputAtTimeStep->GetGeometry());
      this->WriteCSVFile(csvStream);

      auto status = m_Future.wait_for(0ms); // check if python daemon has stopped for some reason
      if (status == std::future_status::ready)
      {
        MITK_INFO << "Thread finished... restarting..";
        m_Future = std::async(std::launch::async, &mitk::SegmentAnythingTool::start_python_daemon, this);
        //StartAsyncProcess();
      }
      //outputImagePath = "C:\\DKFZ\\SAM_work\\test_seg_3d.nii.gz"; 
      std::this_thread::sleep_for(10ms);
      while (!std::filesystem::exists(outputImagePath));
      Image::Pointer outputImage = IOUtil::Load<Image>(outputImagePath);
      //auto endloading = std::chrono::system_clock::now();
      //MITK_INFO << "Loaded image in MITK. Elapsed: "
      //        << std::chrono::duration_cast<std::chrono::milliseconds>(endloading- endPython).count();
      //mitk::SegTool2D::WriteSliceToVolume(previewImage, this->GetWorkingPlaneGeometry(), outputImage, timeStep, true);
      previewImage->InitializeByLabeledImage(outputImage);
      previewImage->SetGeometry(this->GetWorkingPlaneGeometry()->Clone());
      std::filesystem::remove(outputImagePath);
    }
    else if (nullptr != this->GetWorkingPlaneGeometry())
    {
      previewImage->SetGeometry(this->GetWorkingPlaneGeometry()->Clone());
      this->ResetPreviewContentAtTimeStep(timeStep);
    }
  }
}

std::string mitk::SegmentAnythingTool::GetHashForCurrentPlane() 
{
  mitk::Vector3D normal = this->GetWorkingPlaneGeometry()->GetNormal();
  std::stringstream hashstream;
  hashstream << normal[0] << normal[1] << normal[2];
  mitk::Point3D point = m_PointSetPositive->GetPoint(0);
  for (int i = 0; i < 3; ++i)
  {
    if (normal[i]!=0)
    {
      hashstream << point[i];
    }
  }
  size_t hashVal = std::hash<std::string>{}(hashstream.str());
  return std::to_string(hashVal);
}

void mitk::SegmentAnythingTool::StopAsyncProcess()
{
  std::stringstream controlStream;
  controlStream << SIGNALCONSTANTS::KILL;
  this->WriteControlFile(controlStream);
}

void mitk::SegmentAnythingTool::CreateTempDirs(const std::string &dirPattern)
{
  this->SetMitkTempDir(IOUtil::CreateTemporaryDirectory(dirPattern));
  m_InDir = IOUtil::CreateTemporaryDirectory("sam-in-XXXXXX", this->GetMitkTempDir());
  m_OutDir = IOUtil::CreateTemporaryDirectory("sam-out-XXXXXX", this->GetMitkTempDir());
}

void mitk::SegmentAnythingTool::StartAsyncProcess()
{
  if (nullptr != m_DaemonExec)
  {
    this->StopAsyncProcess();
  }
  if (this->m_MitkTempDir.empty())
  {
    this->CreateTempDirs(m_PARENT_TEMP_DIR_PATTERN);
  }
  std::stringstream controlStream;
  controlStream << SIGNALCONSTANTS::READY;
  this->WriteControlFile(controlStream);

  m_DaemonExec = ProcessExecutor::New();
  itk::CStyleCommand::Pointer spCommand = itk::CStyleCommand::New();
  spCommand->SetCallback(&mitk::SegmentAnythingTool::onPythonProcessEvent);
  m_DaemonExec->AddObserver(ExternalProcessOutputEvent(), spCommand);
  m_Future = std::async(std::launch::async, &mitk::SegmentAnythingTool::start_python_daemon, this);
}

void mitk::SegmentAnythingTool::WriteCSVFile(std::stringstream &csvStream)
{
  std::string triggerFilePath = m_InDir + IOUtil::GetDirectorySeparator() + m_TRIGGER_FILENAME;
  std::ofstream csvfile;
  csvfile.open(triggerFilePath, std::ofstream::out | std::ofstream::trunc);
  csvfile << csvStream.rdbuf();
  csvfile.close();
}

void mitk::SegmentAnythingTool::WriteControlFile(std::stringstream &statusStream)
{
  std::string controlFilePath = m_InDir + IOUtil::GetDirectorySeparator() + "control.txt";
  std::ofstream controlFile;
  controlFile.open(controlFilePath, std::ofstream::out | std::ofstream::trunc);
  controlFile << statusStream.rdbuf();
  controlFile.close();
}

void mitk::SegmentAnythingTool::start_python_daemon()
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
  args.push_back(this->GetModelType());

  args.push_back("--checkpoint");
  args.push_back(this->GetCheckpointPath());

  try
  {
    std::string cudaEnv = "CUDA_VISIBLE_DEVICES=" + std::to_string(this->GetGpuId());
    itksys::SystemTools::PutEnv(cudaEnv.c_str());

    std::stringstream logStream;
    for (const auto &arg : args)
      logStream << arg << " ";
    logStream << this->GetPythonPath();
    MITK_INFO << logStream.str();

    m_DaemonExec->Execute(this->GetPythonPath(), command, args);
  }
  catch (const mitk::Exception &e)
  {
    MITK_ERROR << e.GetDescription();
    return;
  }
  MITK_INFO << "ending python process.....";
}

std::stringstream mitk::SegmentAnythingTool::GetPointsAsCSVString(const mitk::BaseGeometry* baseGeometry)
{
  MITK_INFO << "No.of points: " << m_PointSetPositive->GetSize();
  std::stringstream pointsAndLabels;
  pointsAndLabels << "Point,Label\n";
  mitk::PointSet::PointsIterator pointSetItPos = m_PointSetPositive->Begin();
  mitk::PointSet::PointsIterator pointSetItNeg = m_PointSetNegative->Begin();
  const char SPACE = ' ';
  const char newLine = ' ';
  while (pointSetItPos != m_PointSetPositive->End() || pointSetItNeg != m_PointSetNegative->End())
  {
    if (pointSetItPos != m_PointSetPositive->End())
    {
      mitk::Point3D point = pointSetItPos.Value();
      if (baseGeometry->IsInside(point))
      {
        Point2D p2D = Get2DIndicesfrom3DWorld(baseGeometry, point);
        pointsAndLabels << (int)p2D[0] << SPACE << (int)p2D[1] << ",1" << std::endl;
      }
      ++pointSetItPos;
    }
    if (pointSetItNeg != m_PointSetNegative->End())
    {
      mitk::Point3D point = pointSetItNeg.Value();
      if (baseGeometry->IsInside(point))
      {
        Point2D p2D = Get2DIndicesfrom3DWorld(baseGeometry, point);
        pointsAndLabels << (int)p2D[0] << SPACE << (int)p2D[1] << ",0" << std::endl;
      }
      ++pointSetItNeg;
    }
  }
  return pointsAndLabels;
}

std::vector<std::pair<mitk::Point2D, std::string>> mitk::SegmentAnythingTool::GetPointsAsVector(const mitk::BaseGeometry *baseGeometry)
{
  std::vector<std::pair<mitk::Point2D, std::string>> clickVec;
  clickVec.reserve(m_PointSetPositive->GetSize()+m_PointSetNegative->GetSize());
  mitk::PointSet::PointsIterator pointSetItPos = m_PointSetPositive->Begin();
  mitk::PointSet::PointsIterator pointSetItNeg = m_PointSetNegative->Begin();
  while (pointSetItPos != m_PointSetPositive->End() || pointSetItNeg != m_PointSetNegative->End())
  {
    if (pointSetItPos != m_PointSetPositive->End())
    {
      mitk::Point3D point = pointSetItPos.Value();
      if (baseGeometry->IsInside(point))
      {
        Point2D p2D = Get2DIndicesfrom3DWorld(baseGeometry, point);
        clickVec.push_back(std::pair(p2D, "1"));
      }
      ++pointSetItPos;
    }
    if (pointSetItNeg != m_PointSetNegative->End())
    {
      mitk::Point3D point = pointSetItNeg.Value();
      if (baseGeometry->IsInside(point))
      {
        Point2D p2D = Get2DIndicesfrom3DWorld(baseGeometry, point);
        clickVec.push_back(std::pair(p2D, "0"));
      }
      ++pointSetItNeg;
    }
  }
  return clickVec;
}

mitk::Point2D mitk::SegmentAnythingTool::Get2DIndicesfrom3DWorld(const mitk::BaseGeometry* baseGeometry, mitk::Point3D& point3d)
{
  baseGeometry->WorldToIndex(point3d, point3d);
  MITK_INFO << point3d[0] << " " << point3d[1] << " " << point3d[2]; // remove
  Point2D point2D;
  point2D.SetElement(0, point3d[0]);
  point2D.SetElement(1, point3d[1]);
  return point2D;
}
