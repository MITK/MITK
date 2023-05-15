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


namespace mitk
{
  MITK_TOOL_MACRO(MITKSEGMENTATION_EXPORT, SegmentAnythingTool, "SegmentAnythingTool");
}

mitk::SegmentAnythingTool::SegmentAnythingTool() : SegWithPreviewTool(false, "PressMoveReleaseAndPointSetting")
{
  this->ResetsToEmptyPreviewOn();
  this->IsTimePointChangeAwareOff();
}

mitk::SegmentAnythingTool::~SegmentAnythingTool()
{
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
      this->SetMitkTempDir(IOUtil::CreateTemporaryDirectory("mitk-XXXXXX"));
      m_InDir = IOUtil::CreateTemporaryDirectory("sam-in-XXXXXX", this->GetMitkTempDir());
      std::ofstream tmpStream;
      inputImagePath = IOUtil::CreateTemporaryFile(tmpStream, TEMPLATE_FILENAME, m_InDir + IOUtil::GetDirectorySeparator());
      tmpStream.close();
      std::size_t found = inputImagePath.find_last_of(IOUtil::GetDirectorySeparator());
      std::string fileName = inputImagePath.substr(found + 1);
      token = fileName.substr(0, fileName.find("_"));
      m_OutDir = IOUtil::CreateTemporaryDirectory("sam-out-XXXXXX", this->GetMitkTempDir());
    }
    if (this->HasPicks())
    {
      ProcessExecutor::Pointer spExec = ProcessExecutor::New();
      itk::CStyleCommand::Pointer spCommand = itk::CStyleCommand::New();
      spCommand->SetCallback(&onPythonProcessEvent);
      spExec->AddObserver(ExternalProcessOutputEvent(), spCommand);

      pickleFilePath = m_OutDir + IOUtil::GetDirectorySeparator() + "dump.pkl";
      outputImagePath = m_OutDir + IOUtil::GetDirectorySeparator() + token + "_000.nii.gz";

      this->SetPythonPath("C:\\DKFZ\\SAM_work\\sam_env\\Scripts");
      this->SetModelType("vit_b");
      this->SetCheckpointPath("C:\\DKFZ\\SAM_work\\sam_vit_b_01ec64.pth");

      if (m_IsGenerateEmbeddings)
      {auto startWrite = std::chrono::system_clock::now();
        IOUtil::Save(inputAtTimeStep, inputImagePath);
      auto endWrite = std::chrono::system_clock::now();
      MITK_INFO << "UnLoaded image from MITK. Elapsed: "
                  << std::chrono::duration_cast<std::chrono::milliseconds>(endWrite - startWrite).count();
        this->run_generate_embeddings(
          spExec, inputImagePath, m_OutDir, this->GetModelType(), this->GetCheckpointPath(), this->GetGpuId());
      }
      auto pointsVec = GetPointsAsCSVString(inputAtTimeStep->GetGeometry());
      std::string pointsCSV;
      std::string labelsCSV;
      pointsCSV.reserve((m_PointSetPositive->GetSize() + m_PointSetNegative->GetSize()) * 6);
      labelsCSV.reserve((m_PointSetPositive->GetSize() + m_PointSetNegative->GetSize()) * 2);
      for (const std::pair<mitk::Point2D, std::string>& pointPair : pointsVec)
      {
        auto& p2D = pointPair.first;
        pointsCSV.append(std::to_string(static_cast<int>(p2D[0])));
        pointsCSV.append(",");
        pointsCSV.append(std::to_string(static_cast<int>(p2D[1])));
        pointsCSV.append(";");

        auto &label = pointPair.second;
        labelsCSV.append(label);
        labelsCSV.append(",");
      }
      pointsCSV.pop_back(); labelsCSV.pop_back();
      MITK_INFO << " pointsCSV " << pointsCSV;
      MITK_INFO << " labelsCSV " << labelsCSV;
      auto start = std::chrono::system_clock::now();
      run_segmentation_from_points(
        spExec, pickleFilePath, outputImagePath, this->GetModelType(), this->GetCheckpointPath(), pointsCSV, labelsCSV, this->GetGpuId());
    auto endPython = std::chrono::system_clock::now();
      MITK_INFO << "Back in MITK. Elapsed: "
                << std::chrono::duration_cast<std::chrono::milliseconds>(endPython - start).count();
      //outputImagePath = "C:\\DKFZ\\SAM_work\\test_seg_3d.nii.gz";
      Image::Pointer outputImage = IOUtil::Load<Image>(outputImagePath);
      auto endloading = std::chrono::system_clock::now();
      MITK_INFO << "Loaded image in MITK. Elapsed: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(endloading- endPython).count();
      //mitk::SegTool2D::WriteSliceToVolume(previewImage, this->GetWorkingPlaneGeometry(), outputImage, timeStep, true);
      previewImage->InitializeByLabeledImage(outputImage);
      previewImage->SetGeometry(this->GetWorkingPlaneGeometry()->Clone());
    }
    else
    {
      previewImage->SetGeometry(this->GetWorkingPlaneGeometry()->Clone());
      this->ResetPreviewContentAtTimeStep(timeStep);
    }
  }
}

void mitk::SegmentAnythingTool::run_generate_embeddings(ProcessExecutor* spExec,
                                                        const std::string& inputImagePath,
                                                        const std::string& outputPicklePath,
                                                        const std::string& modelType,
                                                        const std::string& checkpointPath,
                                                        const unsigned int gpuId)
{
  
  ProcessExecutor::ArgumentListType args;
  std::string command = "python";

  args.push_back("C:\\DKFZ\\SAM_work\\sam-playground\\endpoints\\generate_embedding.py");

  args.push_back("--input");
  args.push_back(inputImagePath);

  args.push_back("--output");
  args.push_back(outputPicklePath);

  args.push_back("--model-type");
  args.push_back(modelType);

  args.push_back("--checkpoint");
  args.push_back(checkpointPath);

  try
  {
    std::string cudaEnv = "CUDA_VISIBLE_DEVICES=" + std::to_string(gpuId);
    itksys::SystemTools::PutEnv(cudaEnv.c_str());

    std::stringstream logStream;
    for (const auto &arg : args)
      logStream << arg << " ";
    logStream << this->GetPythonPath();
    MITK_INFO << logStream.str();

    spExec->Execute(this->GetPythonPath(), command, args);
  }
  catch (const mitk::Exception &e)
  {
    MITK_ERROR << e.GetDescription();
    return;
  }
}

void mitk::SegmentAnythingTool::run_segmentation_from_points(ProcessExecutor *spExec,
                                                             const std::string &pickleFilePath,
                                                             const std::string &outputImagePath,
                                                             const std::string &modelType,
                                                             const std::string &checkpointPath,
                                                             const std::string &points,
                                                             const std::string &labels,
                                                             const unsigned int gpuId)
{
  ProcessExecutor::ArgumentListType args;
  std::string command = "python";

  args.push_back("C:\\DKFZ\\SAM_work\\sam-playground\\endpoints\\generate_masks.py");

  args.push_back("--embedding");
  args.push_back(pickleFilePath);

  args.push_back("--output");
  args.push_back(outputImagePath);

  args.push_back("--model-type");
  args.push_back(modelType);

  args.push_back("--checkpoint");
  args.push_back(checkpointPath);
  
  args.push_back("--input-points");
  args.push_back(points);

  args.push_back("--input-labels");
  args.push_back(labels);

  try
  {
    std::string cudaEnv = "CUDA_VISIBLE_DEVICES=" + std::to_string(gpuId);
    itksys::SystemTools::PutEnv(cudaEnv.c_str());

    std::stringstream logStream;
    for (const auto &arg : args)
      logStream << arg << " ";
    logStream << this->GetPythonPath();
    MITK_INFO << logStream.str();

    spExec->Execute(this->GetPythonPath(), command, args);
  }
  catch (const mitk::Exception &e)
  {
    MITK_ERROR << e.GetDescription();
    return;
  }
}

bool mitk::SegmentAnythingTool::run_download_model(std::string targetDir)
{
  MITK_INFO << "model will be downloading to " << targetDir;
  return true;
}

std::vector<std::pair<mitk::Point2D, std::string>> mitk::SegmentAnythingTool::GetPointsAsCSVString(const mitk::BaseGeometry *baseGeometry)
{
  MITK_INFO << "No.of points: " << m_PointSetPositive->GetSize();
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
