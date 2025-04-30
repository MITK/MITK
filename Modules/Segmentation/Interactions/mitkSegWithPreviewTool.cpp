/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkSegWithPreviewTool.h"

#include "mitkToolManager.h"

#include "mitkColorProperty.h"
#include "mitkProperties.h"

#include "mitkDataStorage.h"
#include "mitkRenderingManager.h"
#include <mitkTimeNavigationController.h>

#include "mitkImageAccessByItk.h"
#include "mitkImageCast.h"
#include "mitkLabelSetImage.h"
#include "mitkLabelSetImageHelper.h"
#include "mitkMaskAndCutRoiImageFilter.h"
#include "mitkPadImageFilter.h"
#include "mitkNodePredicateGeometry.h"
#include "mitkSegTool2D.h"

mitk::SegWithPreviewTool::SegWithPreviewTool(bool lazyDynamicPreviews): Tool("dummy"), m_LazyDynamicPreviews(lazyDynamicPreviews)
{
  m_ProgressCommand = ToolCommand::New();
}

mitk::SegWithPreviewTool::SegWithPreviewTool(bool lazyDynamicPreviews, const char* interactorType, const us::Module* interactorModule) : Tool(interactorType, interactorModule), m_LazyDynamicPreviews(lazyDynamicPreviews)
{
  m_ProgressCommand = ToolCommand::New();
}

mitk::SegWithPreviewTool::~SegWithPreviewTool()
{
}

void mitk::SegWithPreviewTool::SetMergeStyle(MultiLabelSegmentation::MergeStyle mergeStyle)
{
  m_MergeStyle = mergeStyle;
  this->Modified();
}

void mitk::SegWithPreviewTool::SetOverwriteStyle(MultiLabelSegmentation::OverwriteStyle overwriteStyle)
{
  m_OverwriteStyle = overwriteStyle;
  this->Modified();
}

void mitk::SegWithPreviewTool::SetLabelTransferScope(LabelTransferScope labelTransferScope)
{
  m_LabelTransferScope = labelTransferScope;
  this->Modified();
}

void mitk::SegWithPreviewTool::SetLabelTransferMode(LabelTransferMode labelTransferMode)
{
  m_LabelTransferMode = labelTransferMode;
  this->Modified();
}

void mitk::SegWithPreviewTool::SetSelectedLabels(const SelectedLabelVectorType& labelsToTransfer)
{
  m_SelectedLabels = labelsToTransfer;
  this->Modified();
}

bool mitk::SegWithPreviewTool::CanHandle(const BaseData* referenceData, const BaseData* workingData) const
{
  if (!Superclass::CanHandle(referenceData, workingData))
    return false;

  if (workingData == nullptr)
    return false;

  auto* referenceImage = dynamic_cast<const Image*>(referenceData);
  if (referenceImage == nullptr)
    return false;

  auto* labelSet = dynamic_cast<const MultiLabelSegmentation*>(workingData);
  if (labelSet == nullptr)
    return false;

  return true;
}

void mitk::SegWithPreviewTool::Activated()
{
  Superclass::Activated();

  this->GetToolManager()->RoiDataChanged +=
    MessageDelegate<SegWithPreviewTool>(this, &SegWithPreviewTool::OnRoiDataChanged);

  this->GetToolManager()->SelectedTimePointChanged +=
    MessageDelegate<SegWithPreviewTool>(this, &SegWithPreviewTool::OnTimePointChanged);

  m_ReferenceDataNode = this->GetToolManager()->GetReferenceData(0);
  m_SegmentationInputNode = m_ReferenceDataNode;

  m_LastTimePointOfUpdate = RenderingManager::GetInstance()->GetTimeNavigationController()->GetSelectedTimePoint();

  if (m_PreviewSegmentationNode.IsNull())
  {
    m_PreviewSegmentationNode = DataNode::New();
    m_PreviewSegmentationNode->SetProperty("color", ColorProperty::New(0.0, 1.0, 0.0));
    m_PreviewSegmentationNode->SetProperty("name", StringProperty::New(std::string(this->GetName())+" preview"));
    m_PreviewSegmentationNode->SetProperty("opacity", FloatProperty::New(0.3));
    m_PreviewSegmentationNode->SetProperty("binary", BoolProperty::New(true));
    m_PreviewSegmentationNode->SetProperty("helper object", BoolProperty::New(true));
  }

  if (m_SegmentationInputNode.IsNotNull())
  {
    this->ResetPreviewNode();
    this->InitiateToolByInput();
  }
  else
  {
    this->GetToolManager()->ActivateTool(-1);
  }
  m_IsPreviewGenerated = false;
}

void mitk::SegWithPreviewTool::Deactivated()
{
  this->GetToolManager()->RoiDataChanged -=
    MessageDelegate<SegWithPreviewTool>(this, &SegWithPreviewTool::OnRoiDataChanged);

  this->GetToolManager()->SelectedTimePointChanged -=
    MessageDelegate<SegWithPreviewTool>(this, &SegWithPreviewTool::OnTimePointChanged);

  m_SegmentationInputNode = nullptr;
  m_ReferenceDataNode = nullptr;
  m_WorkingPlaneGeometry = nullptr;

  try
  {
    if (DataStorage *storage = this->GetToolManager()->GetDataStorage())
    {
      storage->Remove(m_PreviewSegmentationNode);
      RenderingManager::GetInstance()->RequestUpdateAll();
    }
  }
  catch (...)
  {
    // don't care
  }

  if (m_PreviewSegmentationNode.IsNotNull())
  {
    m_PreviewSegmentationNode->SetData(nullptr);
  }

  Superclass::Deactivated();
}

void mitk::SegWithPreviewTool::ConfirmSegmentation()
{
  bool labelChanged = this->EnsureUpToDateUserDefinedActiveLabel();
  if ((m_LazyDynamicPreviews && m_CreateAllTimeSteps) || labelChanged)
  { // The tool should create all time steps but is currently in lazy mode,
    // thus ensure that a preview for all time steps is available.
    this->UpdatePreview(true);
  }

  CreateResultSegmentationFromPreview();

  RenderingManager::GetInstance()->RequestUpdateAll();

  if (!m_KeepActiveAfterAccept)
  {
    this->GetToolManager()->ActivateTool(-1);
  }
  this->ConfirmCleanUp();
}

void  mitk::SegWithPreviewTool::InitiateToolByInput()
{
  //default implementation does nothing.
  //implement in derived classes to change behavior
}

mitk::MultiLabelSegmentation* mitk::SegWithPreviewTool::GetPreviewSegmentation()
{
  if (m_PreviewSegmentationNode.IsNull())
  {
    return nullptr;
  }

  return dynamic_cast<MultiLabelSegmentation*>(m_PreviewSegmentationNode->GetData());
}

const mitk::MultiLabelSegmentation* mitk::SegWithPreviewTool::GetPreviewSegmentation() const
{
  if (m_PreviewSegmentationNode.IsNull())
  {
    return nullptr;
  }

  return dynamic_cast<MultiLabelSegmentation*>(m_PreviewSegmentationNode->GetData());
}

mitk::DataNode* mitk::SegWithPreviewTool::GetPreviewSegmentationNode()
{
  return m_PreviewSegmentationNode;
}

const mitk::Image* mitk::SegWithPreviewTool::GetSegmentationInput() const
{
  if (m_SegmentationInputNode.IsNull())
  {
    return nullptr;
  }

  return dynamic_cast<const Image*>(m_SegmentationInputNode->GetData());
}

const mitk::Image* mitk::SegWithPreviewTool::GetReferenceData() const
{
  if (m_ReferenceDataNode.IsNull())
  {
    return nullptr;
  }

  return dynamic_cast<const Image*>(m_ReferenceDataNode->GetData());
}

void mitk::SegWithPreviewTool::ResetPreviewContentAtTimeStep(unsigned int timeStep)
{
  auto previewImage = this->GetPreviewSegmentation();
  if (nullptr != previewImage)
  {
    previewImage->ClearGroupImages(timeStep);
  }
}

void mitk::SegWithPreviewTool::ResetPreviewContent()
{
  auto previewImage = this->GetPreviewSegmentation();
  if (nullptr != previewImage)
  {
    previewImage->ClearGroupImages();
  }
}

void mitk::SegWithPreviewTool::ResetPreviewNode()
{
  if (m_IsUpdating)
  {
    mitkThrow() << "Used tool is implemented incorrectly. ResetPreviewNode is called while preview update is ongoing. Check implementation!";
  }

  itk::RGBPixel<float> previewColor;
  previewColor[0] = 0.0f;
  previewColor[1] = 1.0f;
  previewColor[2] = 0.0f;

  const auto image = this->GetSegmentationInput();
  if (nullptr != image)
  {
    MultiLabelSegmentation::ConstPointer workingImage =
      dynamic_cast<const MultiLabelSegmentation *>(this->GetToolManager()->GetWorkingData(0)->GetData());

    if (workingImage.IsNotNull())
    {
      auto newPreviewImage = workingImage->Clone();
      if (this->GetResetsToEmptyPreview())
      {
        newPreviewImage->ClearGroupImages();
      }

      if (newPreviewImage.IsNull())
      {
        MITK_ERROR << "Cannot create preview helper objects. Unable to clone working image";
        return;
      }

      m_PreviewSegmentationNode->SetData(newPreviewImage);

      if (newPreviewImage->GetNumberOfGroups() == 0)
      {
        newPreviewImage->AddLayer();
      }

      auto* activeLabel = newPreviewImage->GetActiveLabel();
      if (nullptr == activeLabel)
      {
        activeLabel = newPreviewImage->AddLabel("toolresult", previewColor, newPreviewImage->GetActiveLayer());
        newPreviewImage->UpdateLookupTable(activeLabel->GetValue());
      }
      else if (m_UseSpecialPreviewColor)
      {
        // Let's paint the feedback node green...
        activeLabel->SetColor(previewColor);
        newPreviewImage->UpdateLookupTable(activeLabel->GetValue());
      }
      newPreviewImage->GetLookupTable()->Modified();
      activeLabel->SetVisible(true);
    }
    else
    {
      mitkThrow() << "Tool is an invalid state. Cannot setup preview node. Working data is an unsupported class and should have not been accepted by CanHandle().";
    }

    m_PreviewSegmentationNode->SetColor(previewColor);
    m_PreviewSegmentationNode->SetOpacity(0.5);

    int layer(50);
    m_ReferenceDataNode->GetIntProperty("layer", layer);
    m_PreviewSegmentationNode->SetIntProperty("layer", layer + 1);

    if (DataStorage *ds = this->GetToolManager()->GetDataStorage())
    {
      if (!ds->Exists(m_PreviewSegmentationNode))
        ds->Add(m_PreviewSegmentationNode, m_ReferenceDataNode);
    }
  }
}

mitk::SegWithPreviewTool::LabelMappingType mitk::SegWithPreviewTool::GetLabelMapping() const
{
  MultiLabelSegmentation::LabelValueType offset = 0;
  
  if (LabelTransferMode::AddLabel == m_LabelTransferMode && LabelTransferScope::ActiveLabel!=m_LabelTransferScope)
  {
    //If we are not just working on active label and transfer mode is add, we need to compute an offset for adding the
    //preview labels instat of just mapping them to existing segmentation labels.
    const auto segmentation = this->GetTargetSegmentation();
    if (nullptr == segmentation)
      mitkThrow() << "Invalid state of SegWithPreviewTool. Cannot GetLabelMapping if no target segmentation is set.";

    auto labels = segmentation->GetLabels();
    auto maxLabelIter = std::max_element(std::begin(labels), std::end(labels), [](const Label::Pointer& a, const Label::Pointer& b) {
      return a->GetValue() < b->GetValue();
    });

    if (maxLabelIter != labels.end())
    {
      offset = maxLabelIter->GetPointer()->GetValue();
    }
  }

  LabelMappingType labelMapping = {};

  switch (this->m_LabelTransferScope)
  {
    case LabelTransferScope::SelectedLabels:
      {
        for (auto label : this->m_SelectedLabels)
        {
          labelMapping.push_back({label, label + offset});
        }
      }
      break;
    case LabelTransferScope::AllLabels:
      {
        const auto labelValues = this->GetPreviewSegmentation()->GetLabelValuesByGroup(this->GetPreviewSegmentation()->GetActiveLayer());
        for (auto labelValue : labelValues)
        {
        labelMapping.push_back({ labelValue, labelValue + offset});
        }
      }
      break;
    default:
      {
        if (m_SelectedLabels.empty())
          mitkThrow() << "Failed to generate label transfer mapping. Tool is in an invalid state, as "
                         "LabelTransferScope==ActiveLabel but no label is indicated as selected label. Check "
                         "implementation of derived tool class.";
        if (m_SelectedLabels.size() > 1)
        mitkThrow() << "Failed to generate label transfer mapping. Tool is in an invalid state, as "
                       "LabelTransferScope==ActiveLabel but more then one selected label is indicated."
                       "Should be only one. Check implementation of derived tool class.";
        labelMapping.push_back({m_SelectedLabels.front(), this->GetUserDefinedActiveLabel()});
      }
      break;
  }

  return labelMapping;
}

void mitk::SegWithPreviewTool::TransferSegmentationsAtTimeStep(const MultiLabelSegmentation* sourceSeg, MultiLabelSegmentation* destinationSeg, const TimeStepType timeStep, const LabelMappingType& labelMapping)
{
  try
  {
    if (!Equal(*(sourceSeg->GetGeometry(timeStep)), *(destinationSeg->GetGeometry(timeStep)), NODE_PREDICATE_GEOMETRY_DEFAULT_CHECK_COORDINATE_PRECISION, NODE_PREDICATE_GEOMETRY_DEFAULT_CHECK_DIRECTION_PRECISION, false))
    {
      mitkThrow() << "Cannot transfer segmentations. Tool is in an invalid state, source and destination segmentations do not have the same geometry.";
    }

    if (nullptr != this->GetWorkingPlaneGeometry())
    {
      //split all label mappings by source group id
      auto groupLabelValueMappingSplits = LabelSetImageHelper::SplitLabelValueMappingBySourceAndTargetGroup(sourceSeg, destinationSeg, labelMapping);

      for (const auto& [sourceGroupID, destGroupLabelMapping] : groupLabelValueMappingSplits)
      {
        auto sourceSlice = SegTool2D::GetAffectedImageSliceAs2DImage(this->GetWorkingPlaneGeometry(), sourceSeg->GetGroupImage(sourceGroupID), timeStep);

        for (const auto& [destGroupID, relevantLabelMapping] : destGroupLabelMapping)
        {
          auto resultSlice =
            SegTool2D::GetAffectedImageSliceAs2DImage(this->GetWorkingPlaneGeometry(), destinationSeg->GetGroupImage(destGroupID), timeStep)->Clone();

          //We need to transfer explicitly to a copy of the current working image to ensure that labelMapping is done and things
          //like merge style, overwrite style and locks are regarded.
          TransferLabelContentAtTimeStep(sourceSlice,
            resultSlice,
            destinationSeg->GetConstLabelsByValue(destinationSeg->GetLabelValuesByGroup(destGroupID)),
            timeStep,
            0,
            0,
            destinationSeg->GetUnlabeledLabelLock(),
            labelMapping,
            m_MergeStyle,
            m_OverwriteStyle);
          //We use WriteBackSegmentationResult to ensure undo/redo is supported also by derived tools of this class.
          SegTool2D::WriteBackSegmentationResult(this->GetTargetSegmentationNode(), m_WorkingPlaneGeometry, resultSlice, timeStep);
        }
      }
    }
    else
    { //take care of the full segmentation volume
      TransferLabelContentAtTimeStep(sourceSeg, destinationSeg, timeStep, labelMapping, m_MergeStyle, m_OverwriteStyle);
    }
  }
  catch (mitk::Exception& e)
  {
    Tool::ErrorMessage(e.GetDescription());
    mitkReThrow(e);
  }
}

void mitk::SegWithPreviewTool::CreateResultSegmentationFromPreview()
{
  const auto segInput = this->GetSegmentationInput();
  auto previewImage = this->GetPreviewSegmentation();
  if (nullptr != segInput && nullptr != previewImage)
  {
    DataNode::Pointer resultSegmentationNode = GetTargetSegmentationNode();

    if (resultSegmentationNode.IsNotNull())
    {
      const TimePointType timePoint = RenderingManager::GetInstance()->GetTimeNavigationController()->GetSelectedTimePoint();
      auto resultSegmentation = dynamic_cast<MultiLabelSegmentation*>(resultSegmentationNode->GetData());
      if (nullptr == resultSegmentation)
      {
        mitkThrow() << "Cannot confirm/transfer segmentation. Internal tool state is invalid."
          << " target segmentation node does not contain a MultiLabelSegmentation instance.";
      }

      // REMARK: the following code in this scope assumes that previewImage and resultSegmentation
      // are clones of the working referenceImage (segmentation provided to the tool). Therefore they have
      // the same time geometry.
      if (previewImage->GetTimeSteps() != resultSegmentation->GetTimeSteps())
      {
        mitkThrow() << "Cannot confirm/transfer segmentation. Internal tool state is invalid."
          << " Preview segmentation and segmentation result image have different time geometries.";
      }

      auto labelMapping = this->GetLabelMapping();
      this->PreparePreviewToResultTransfer(labelMapping);
      if (m_CreateAllTimeSteps)
      {
        for (unsigned int timeStep = 0; timeStep < previewImage->GetTimeSteps(); ++timeStep)
        {
          this->TransferSegmentationsAtTimeStep(previewImage, resultSegmentation, timeStep, labelMapping);
        }
      }
      else
      {
        const auto timeStep = resultSegmentation->GetTimeGeometry()->TimePointToTimeStep(timePoint);
        this->TransferSegmentationsAtTimeStep(previewImage, resultSegmentation, timeStep, labelMapping);
      }

      // since we are maybe working on a smaller referenceImage, pad it to the size of the original referenceImage
      if (m_ReferenceDataNode.GetPointer() != m_SegmentationInputNode.GetPointer())
      {
        PadImageFilter::Pointer padFilter = PadImageFilter::New();

        padFilter->SetInput(0, resultSegmentation);
        padFilter->SetInput(1, dynamic_cast<Image*>(m_ReferenceDataNode->GetData()));
        padFilter->SetBinaryFilter(true);
        padFilter->SetUpperThreshold(1);
        padFilter->SetLowerThreshold(1);
        padFilter->Update();

        resultSegmentationNode->SetData(padFilter->GetOutput());
      }
      this->EnsureTargetSegmentationNodeInDataStorage();
    }
  }
}

void mitk::SegWithPreviewTool::OnRoiDataChanged()
{
  DataNode::ConstPointer node = this->GetToolManager()->GetRoiData(0);

  if (node.IsNotNull())
  {
    MaskAndCutRoiImageFilter::Pointer roiFilter = MaskAndCutRoiImageFilter::New();
    Image::Pointer image = dynamic_cast<Image *>(m_SegmentationInputNode->GetData());

    if (image.IsNull())
      return;

    roiFilter->SetInput(image);
    roiFilter->SetRegionOfInterest(node->GetData());
    roiFilter->Update();

    DataNode::Pointer tmpNode = DataNode::New();
    tmpNode->SetData(roiFilter->GetOutput());

    m_SegmentationInputNode = tmpNode;
  }
  else
    m_SegmentationInputNode = m_ReferenceDataNode;

  this->ResetPreviewNode();
  this->InitiateToolByInput();
  this->UpdatePreview();
}

void mitk::SegWithPreviewTool::OnTimePointChanged()
{
  if (m_IsTimePointChangeAware && m_PreviewSegmentationNode.IsNotNull() && m_SegmentationInputNode.IsNotNull())
  {
    const TimePointType timePoint = RenderingManager::GetInstance()->GetTimeNavigationController()->GetSelectedTimePoint();

    const bool isStaticSegOnDynamicImage = m_PreviewSegmentationNode->GetData()->GetTimeSteps() == 1 && m_SegmentationInputNode->GetData()->GetTimeSteps() > 1;
    if (timePoint!=m_LastTimePointOfUpdate && (isStaticSegOnDynamicImage || m_LazyDynamicPreviews))
    { //we only need to update either because we are lazzy
      //or because we have a static segmentation with a dynamic referenceImage 
      this->UpdatePreview();
    }
  }
}

bool mitk::SegWithPreviewTool::EnsureUpToDateUserDefinedActiveLabel()
{
  bool labelChanged = true;

  const auto workingImage = dynamic_cast<const MultiLabelSegmentation*>(this->GetToolManager()->GetWorkingData(0)->GetData());
  if (nullptr != workingImage)
  {
    // this is a fix for T28131 / T28986, which should be refactored if T28524 is being worked on
    auto newLabel = workingImage->GetActiveLabel()->GetValue();
    labelChanged = newLabel != m_UserDefinedActiveLabel;
    m_UserDefinedActiveLabel = newLabel;
  }
  else
  {
    mitkThrow() << "Cannot transfer EnsureUpToDateUserDefinedActiveLabel. Tool manager is in an invalid state, working data is null or no MultiLabelSegmentation instance.";
  }
  return labelChanged;
}

void mitk::SegWithPreviewTool::UpdatePreview(bool ignoreLazyPreviewSetting)
{
  const auto inputImage = this->GetSegmentationInput();
  auto previewImage = this->GetPreviewSegmentation();
  int progress_steps = 200;
  this->EnsureUpToDateUserDefinedActiveLabel();

  const auto workingSegmentation = this->GetTargetSegmentation();
  const auto workingImage = workingSegmentation->GetGroupImage(workingSegmentation->GetActiveLayer());

  this->CurrentlyBusy.Send(true);
  m_IsUpdating = true;
  m_IsPreviewGenerated = false;
  this->UpdatePrepare();

  const TimePointType timePoint = RenderingManager::GetInstance()->GetTimeNavigationController()->GetSelectedTimePoint();

  try
  {
    if (nullptr != inputImage && nullptr != previewImage)
    {
      m_ProgressCommand->AddStepsToDo(progress_steps);

      if (previewImage->GetTimeSteps() > 1 && (ignoreLazyPreviewSetting || !m_LazyDynamicPreviews))
      {
        for (unsigned int timeStep = 0; timeStep < previewImage->GetTimeSteps(); ++timeStep)
        {
          Image::ConstPointer feedBackImage;
          Image::ConstPointer currentSegImage;

          auto previewTimePoint = previewImage->GetTimeGeometry()->TimeStepToTimePoint(timeStep);
          auto inputTimeStep = inputImage->GetTimeGeometry()->TimePointToTimeStep(previewTimePoint);

          if (nullptr != this->GetWorkingPlaneGeometry())
          { //only extract a specific slice defined by the working plane as feedback referenceImage.
            feedBackImage = SegTool2D::GetAffectedImageSliceAs2DImage(this->GetWorkingPlaneGeometry(), inputImage, inputTimeStep);
            currentSegImage = SegTool2D::GetAffectedImageSliceAs2DImageByTimePoint(this->GetWorkingPlaneGeometry(), workingImage, previewTimePoint);
          }
          else
          { //work on the whole feedback referenceImage
            feedBackImage = this->GetImageByTimeStep(inputImage, inputTimeStep);
            currentSegImage = this->GetImageByTimePoint(workingImage, previewTimePoint);
          }

          this->DoUpdatePreview(feedBackImage, currentSegImage, previewImage, timeStep);
        }
      }
      else
      {
        Image::ConstPointer feedBackImage;
        Image::ConstPointer currentSegImage;

        if (nullptr != this->GetWorkingPlaneGeometry())
        {
          feedBackImage = SegTool2D::GetAffectedImageSliceAs2DImageByTimePoint(this->GetWorkingPlaneGeometry(), inputImage, timePoint);
          currentSegImage = SegTool2D::GetAffectedImageSliceAs2DImageByTimePoint(this->GetWorkingPlaneGeometry(), workingImage, timePoint);
        }
        else
        {
          feedBackImage = this->GetImageByTimePoint(inputImage, timePoint);
          currentSegImage = this->GetImageByTimePoint(workingImage, timePoint);
        }

        auto timeStep = previewImage->GetTimeGeometry()->TimePointToTimeStep(timePoint);

        this->DoUpdatePreview(feedBackImage, currentSegImage, previewImage, timeStep);
      }
      RenderingManager::GetInstance()->RequestUpdateAll();
      if (!previewImage->GetAllLabelValues().empty())
      { // check if labels exits for the preview
        m_IsPreviewGenerated = true;
      }
    }
  }
  catch (itk::ExceptionObject & excep)
  {
    MITK_ERROR << "Exception caught: " << excep.GetDescription();

    m_ProgressCommand->SetProgress(progress_steps);

    std::string msg = excep.GetDescription();
    ErrorMessage.Send(msg);
  }
  catch (...)
  {
    m_ProgressCommand->SetProgress(progress_steps);
    m_IsUpdating = false;
    CurrentlyBusy.Send(false);
    throw;
  }

  this->UpdateCleanUp();
  m_LastTimePointOfUpdate = timePoint;
  m_ProgressCommand->SetProgress(progress_steps);
  m_IsUpdating = false;
  CurrentlyBusy.Send(false);
}

bool mitk::SegWithPreviewTool::IsUpdating() const
{
  return m_IsUpdating;
}

void mitk::SegWithPreviewTool::UpdatePrepare()
{
  // default implementation does nothing
  //reimplement in derived classes for special behavior
}

void mitk::SegWithPreviewTool::UpdateCleanUp()
{
  // default implementation does nothing
  //reimplement in derived classes for special behavior
}

void mitk::SegWithPreviewTool::ConfirmCleanUp()
{
  // default implementation does nothing
  // reimplement in derived classes for special behavior
}

void mitk::SegWithPreviewTool::TransferLabelInformation(const LabelMappingType& labelMapping,
  const mitk::MultiLabelSegmentation* source, mitk::MultiLabelSegmentation* target)
{
  if (nullptr == source)
  {
    mitkThrow() << "Cannot transfer label information. Passed source segmentation pointer points to NULL.";
  }
  if (nullptr == target)
  {
    mitkThrow() << "Cannot transfer label information. Passed target segmentation pointer points to NULL.";
  }

  for (const auto& [sourceLabel, targetLabel] : labelMapping)
  {
    if (MultiLabelSegmentation::UNLABELED_VALUE != sourceLabel &&
        MultiLabelSegmentation::UNLABELED_VALUE != targetLabel &&
        !target->ExistLabel(targetLabel, target->GetActiveLayer()))
    {
      if (!source->ExistLabel(sourceLabel))
      {
        mitkThrow() << "Cannot prepare segmentation for preview transfer. Preview seems invalid as label is missing. Missing label: " << sourceLabel;
      }

      auto clonedLabel = source->GetLabel(sourceLabel)->Clone();
      clonedLabel->SetValue(targetLabel);
      target->AddLabel(clonedLabel,target->GetActiveLayer(), false, false);
    }
  }
}

void mitk::SegWithPreviewTool::PreparePreviewToResultTransfer(const LabelMappingType& labelMapping)
{
  DataNode::Pointer resultSegmentationNode = GetTargetSegmentationNode();

  if (resultSegmentationNode.IsNotNull())
  {
    auto resultSegmentation = resultSegmentationNode->GetDataAs<MultiLabelSegmentation>();

    if (nullptr == resultSegmentation)
    {
      mitkThrow() << "Cannot prepare segmentation for preview transfer. Tool is in invalid state as segmentation is not existing or of right type";
    }

    auto preview = this->GetPreviewSegmentation();
    TransferLabelInformation(labelMapping, preview, resultSegmentation);
  }
}

mitk::TimePointType mitk::SegWithPreviewTool::GetLastTimePointOfUpdate() const
{
  return m_LastTimePointOfUpdate;
}

mitk::MultiLabelSegmentation::LabelValueType mitk::SegWithPreviewTool::GetActiveLabelValueOfPreview() const
{
  const auto previewImage = this->GetPreviewSegmentation();
  const auto activeLabel = previewImage->GetActiveLabel();
  if (nullptr == activeLabel)
    mitkThrow() << this->GetNameOfClass() <<" is in an invalid state, as "
                   "preview has no active label indicated. Check "
                   "implementation of the class.";

  return activeLabel->GetValue();
}

const char* mitk::SegWithPreviewTool::GetGroup() const
{
  return "autoSegmentation";
}

mitk::Image::ConstPointer mitk::SegWithPreviewTool::GetImageByTimeStep(const mitk::Image* image, TimeStepType timestep)
{
  return SelectImageByTimeStep(image, timestep);
}

mitk::Image::Pointer mitk::SegWithPreviewTool::GetImageByTimeStep(mitk::Image* image, TimeStepType timestep)
{
  return SelectImageByTimeStep(image, timestep);
}

mitk::Image::ConstPointer mitk::SegWithPreviewTool::GetImageByTimePoint(const mitk::Image* image, TimePointType timePoint)
{
  return SelectImageByTimePoint(image, timePoint);
}

void mitk::SegWithPreviewTool::EnsureTargetSegmentationNodeInDataStorage() const
{
  auto targetNode = this->GetTargetSegmentationNode();
  auto dataStorage = this->GetToolManager()->GetDataStorage();
  if (!dataStorage->Exists(targetNode))
  {
    dataStorage->Add(targetNode, this->GetToolManager()->GetReferenceData(0));
  }
}

std::string mitk::SegWithPreviewTool::GetCurrentSegmentationName()
{
  auto workingData = this->GetToolManager()->GetWorkingData(0);

  return nullptr != workingData
    ? workingData->GetName()
    : "";
}

mitk::DataNode* mitk::SegWithPreviewTool::GetTargetSegmentationNode() const
{
  return this->GetToolManager()->GetWorkingData(0);
}

mitk::MultiLabelSegmentation* mitk::SegWithPreviewTool::GetTargetSegmentation() const
{
  auto node = this->GetTargetSegmentationNode();

  if (nullptr == node)
    return nullptr;

  return node->GetDataAs<MultiLabelSegmentation>();
}

bool mitk::SegWithPreviewTool::ConfirmBeforeDeactivation()
{
  return m_IsPreviewGenerated && m_RequestDeactivationConfirmation;
}
