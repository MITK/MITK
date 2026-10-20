/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkSegWithPreviewTool.h>

#include <mitkToolManager.h>

#include <mitkColorProperty.h>
#include <mitkProperties.h>

#include <mitkDataStorage.h>
#include <mitkRenderingManager.h>
#include <mitkTimeNavigationController.h>

#include <mitkImageAccessByItk.h>
#include <mitkImageCast.h>
#include <mitkLabelSetImage.h>
#include <mitkLabelSetImageHelper.h>
#include <mitkMaskAndCutRoiImageFilter.h>
#include <mitkPadImageFilter.h>
#include <mitkNodePredicateGeometry.h>
#include <mitkSegTool2D.h>

#include <mitkProgressTask.h>
#include <mitkScopedProgressTask.h>
#include <mitkSegChangeOperationApplier.h>

#include <algorithm>
#include <map>
#include <optional>
#include <utility>

struct mitk::SegWithPreviewTool::UpdatePass
{
  // Held, not just referenced: the work of an update can run off the GUI
  // thread, during which the tool can be deactivated, and Deactivated()
  // releases the input and the data of the preview node. The user can also
  // remove the images from the data storage meanwhile.
  Image::ConstPointer Input;
  MultiLabelSegmentation::Pointer Preview;
  Image::Pointer WorkingImage;

  TimePointType TimePoint = 0.0;

  /** Every time step of the preview, or the one at TimePoint. */
  bool AllTimeSteps = false;
  std::vector<TimeStepType> TimeSteps;
  std::size_t NextTimeStep = 0;

  bool Cancelled = false;
  bool Failed = false;
  bool Computed = false;

  /** Set by Deactivated(): the update ends once its running work has, and
      leaves the state of the tool alone, which may be activated anew. */
  bool Abandoned = false;

  /** What follows the update, like the transfer of a confirmation. */
  std::function<void()> Then;

  // Declared in this order, so that the task is taken from the command before it ends.
  std::unique_ptr<ProgressTask> Task;
  std::optional<ScopedProgressTask<ToolCommand>> ScopedTask;
};

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
  // Work reads the token when it posts to the GUI thread.
  this->WaitForWorkOffGUIThread();
  m_Alive.reset();
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

  if (m_RequiresScalarReference && !IsSingleComponentScalarImage(referenceImage))
    return false;

  if (m_RequiresVolumetricReference && referenceImage->GetDimension() < 3)
    return false;

  auto* labelSet = dynamic_cast<const MultiLabelSegmentation*>(workingData);
  if (labelSet == nullptr)
    return false;

  if (labelSet->GetNumberOfGroups() == 0)
    return false;

  if (m_RequiresExistingLabels && labelSet->GetTotalNumberOfLabels() == 0)
    return false;

  // The same tolerances as MultiLabelSegmentation::UpdateGroupImage(), which
  // rejects a result of another geometry.
  if (m_RequiresReferenceGeometry && !Equal(*referenceImage->GetGeometry(), *labelSet->GetGeometry(),
        NODE_PREDICATE_GEOMETRY_DEFAULT_CHECK_COORDINATE_PRECISION, NODE_PREDICATE_GEOMETRY_DEFAULT_CHECK_DIRECTION_PRECISION))
    return false;

  return true;
}

mitk::Label::AlgorithmType mitk::SegWithPreviewTool::GetAlgorithmType() const
{
  return mitk::Label::AlgorithmType::SEMIAUTOMATIC;
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
    m_PreviewSegmentationNode->SetProperty("opacity", FloatProperty::New(m_PreviewOpacity));
    m_PreviewSegmentationNode->SetProperty("visible", BoolProperty::New(m_PreviewVisibility));
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
  m_HasUnconfirmedPreview = false;
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

  // An update whose work still runs ends once the work has, without further
  // time steps and without a confirmation following it. IsUpdating() stays
  // true until then, which tells derived tools that the work still uses what
  // they would otherwise release now.
  if (m_UpdatePass != nullptr)
  {
    m_UpdatePass->Abandoned = true;
    m_UpdatePass->Then = nullptr;
  }

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
  if (m_IsUpdating || m_WorkRunning)
    mitkThrow() << this->GetName() << ": the preview cannot be confirmed while the tool is busy.";

  bool labelChanged = this->EnsureUpToDateUserDefinedActiveLabel();

  // A lazy tool has computed the current time step only, which is all that is
  // transferred unless all time steps are to be created. A changed active label
  // needs what is transferred to be computed again, but not more than that.
  const bool updateAllTimeSteps = m_LazyDynamicPreviews && m_CreateAllTimeSteps;

  if (updateAllTimeSteps || labelChanged)
  {
    // The user can select another time point while the update computes off
    // the GUI thread. The preview holds the one the update computed, which
    // EndUpdate() has recorded once this runs.
    this->StartUpdate(updateAllTimeSteps, [this]() { this->FinishConfirmation(m_LastTimePointOfUpdate); });
    return;
  }

  this->FinishConfirmation();
}

void mitk::SegWithPreviewTool::FinishConfirmation(std::optional<TimePointType> timePoint)
{
  this->CreateResultSegmentationFromPreview(timePoint);

  // Cleared before ConfirmCleanUp() so a tool that regenerates a preview in that
  // hook ends up flagged again.
  m_HasUnconfirmedPreview = false;

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

void mitk::SegWithPreviewTool::SetPreviewOpacity(float opacity)
{
  m_PreviewOpacity = std::clamp(opacity, 0.0f, 1.0f);

  if (m_PreviewSegmentationNode.IsNotNull())
  {
    m_PreviewSegmentationNode->SetOpacity(m_PreviewOpacity);
    RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void mitk::SegWithPreviewTool::SetPreviewVisibility(bool visible)
{
  m_PreviewVisibility = visible;

  if (m_PreviewSegmentationNode.IsNotNull())
  {
    m_PreviewSegmentationNode->SetVisibility(visible);
    RenderingManager::GetInstance()->RequestUpdateAll();
  }
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

void mitk::SegWithPreviewTool::RemoveAllPreviewLabels()
{
  auto* previewImage = this->GetPreviewSegmentation();

  if (nullptr == previewImage)
    return;

  // RemoveLabels() erases the pixels of each label in a pass of its own over
  // the whole preview. Dropping the labels first and clearing the group images
  // afterwards takes a single pass, whatever the number of labels.
  std::map<MultiLabelSegmentation::GroupIndexType, MultiLabelSegmentation::ConstLabelVectorType> emptyGroups;

  for (MultiLabelSegmentation::GroupIndexType groupID = 0; groupID < previewImage->GetNumberOfGroups(); ++groupID)
    emptyGroups[groupID] = {};

  previewImage->ReplaceGroupLabels(emptyGroups);
  previewImage->ClearGroupImages();
}

mitk::Color mitk::SegWithPreviewTool::GetSpecialPreviewColor() const
{
  Color color;
  color.Set(0.0f, 1.0f, 0.0);
  return color;
}

void mitk::SegWithPreviewTool::ResetPreviewNode()
{
  // An update abandoned by a deactivation holds its own preview, so an
  // activation while its work ends can set up a new one.
  if (m_IsUpdating && !(m_UpdatePass != nullptr && m_UpdatePass->Abandoned))
  {
    mitkThrow() << "Used tool is implemented incorrectly. ResetPreviewNode is called while preview update is ongoing. Check implementation!";
  }

  const auto image = this->GetSegmentationInput();
  if (nullptr != image)
  {
    MultiLabelSegmentation::ConstPointer workingImage =
      dynamic_cast<const MultiLabelSegmentation *>(this->GetToolManager()->GetWorkingData(0)->GetData());

    const auto previewColor = this->GetSpecialPreviewColor();

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
        newPreviewImage->AddGroup();
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
    m_PreviewSegmentationNode->SetOpacity(m_PreviewOpacity);
    m_PreviewSegmentationNode->SetVisibility(m_PreviewVisibility);

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
        const auto* preview = this->GetPreviewSegmentation();
        const auto labelValues = m_TransfersAllPreviewGroups
          ? preview->GetAllLabelValues()
          : preview->GetLabelValuesByGroup(preview->GetActiveLayer());

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
          auto destSlice = SegTool2D::GetAffectedImageSliceAs2DImage(this->GetWorkingPlaneGeometry(), destinationSeg->GetGroupImage(destGroupID), timeStep);
          if (destSlice.IsNull())
          {
            MITK_ERROR << "Unable to extract destination slice.";
            continue;
          }
          auto resultSlice = destSlice->Clone();

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
          SegTool2D::WriteBackSegmentationResult(this->GetTargetSegmentationNode(), m_WorkingPlaneGeometry, resultSlice, timeStep,this->GetName());
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

void mitk::SegWithPreviewTool::CreateResultSegmentationFromPreview(std::optional<TimePointType> timePoint)
{
  const auto segInput = this->GetSegmentationInput();
  auto previewImage = this->GetPreviewSegmentation();
  if (nullptr != segInput && nullptr != previewImage)
  {
    DataNode::Pointer resultSegmentationNode = GetTargetSegmentationNode();

    if (resultSegmentationNode.IsNotNull())
    {
      // A lazy tool that ignores time point changes holds the preview of the
      // time point of its last update only, wherever the user has gone since.
      if (!timePoint.has_value())
      {
        timePoint = m_LazyDynamicPreviews && !m_IsTimePointChangeAware
          ? m_LastTimePointOfUpdate
          : RenderingManager::GetInstance()->GetTimeNavigationController()->GetSelectedTimePoint();
      }

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

      const auto timeStep = resultSegmentation->GetTimeGeometry()->TimePointToTimeStep(*timePoint);
      const auto oldGroupCount = resultSegmentation->GetNumberOfGroups();

      // The existing groups that receive labels. Groups that the transfer adds
      // are covered by an undo operation of their own.
      SegGroupModifyUndoRedoHelper::GroupIndexSetType modifiedGroups;

      if (m_TransfersAllPreviewGroups)
      {
        for (const auto& [sourceLabel, targetLabel] : labelMapping)
        {
          if (previewImage->ExistLabel(sourceLabel))
          {
            const auto group = previewImage->GetGroupIndexOfLabel(sourceLabel);

            if (group < oldGroupCount)
              modifiedGroups.insert(group);
          }
        }
      }
      else
      {
        modifiedGroups.insert(resultSegmentation->GetActiveLayer());
      }

      std::optional<SegGroupModifyUndoRedoHelper> undoRedoGenerator;

      if (!modifiedGroups.empty())
        undoRedoGenerator.emplace(resultSegmentation, modifiedGroups, m_CreateAllTimeSteps, timeStep, false, false, true);

      this->PreparePreviewToResultTransfer(labelMapping);

      const auto newGroupCount = resultSegmentation->GetNumberOfGroups();

      // REMARK: the following code in this scope assumes that PreparePreviewToResultTransfer only adds
      // groups if the labels of all preview groups are transferred.
      if (newGroupCount < oldGroupCount || (newGroupCount > oldGroupCount && !m_TransfersAllPreviewGroups))
      {
        mitkThrow() << "Cannot confirm/transfer segmentation. Internal tool state is invalid."
          << " Tool has changed the number of groups. Current base implementation expects that"
          << " this does not happen.";
      }

      if (m_CreateAllTimeSteps)
      {
        for (unsigned int timeStep = 0; timeStep < previewImage->GetTimeSteps(); ++timeStep)
        {
          this->TransferSegmentationsAtTimeStep(previewImage, resultSegmentation, timeStep, labelMapping);
        }
      }
      else
      {
        this->TransferSegmentationsAtTimeStep(previewImage, resultSegmentation, timeStep, labelMapping);
      }

      // Stamp provenance on every affected result label (the target values of the label mapping).
      // Covers BOTH the 3D-volume and 2D-working-plane branches of TransferSegmentationsAtTimeStep,
      // regardless of that branch re-entering the (provenance-free) static WriteBackSegmentationResult.
      // Must run before RegisterUndoRedoOperationEvent so the redo snapshot captures the stamp.
      // AddToolUse is idempotent, so iterating duplicate target values is safe.
      for (const auto& mapping : labelMapping)
      {
        if (auto label = resultSegmentation->GetLabel(mapping.second)) // GetLabel returns mitk::Label::Pointer
          label->AddToolUse(this->GetAlgorithmType(), this->GetName());
      }

      const auto undoDescription = "Segmentation " + std::string(this->GetName());

      if (undoRedoGenerator.has_value())
        undoRedoGenerator->RegisterUndoRedoOperationEvent(undoDescription);

      if (newGroupCount > oldGroupCount)
      {
        SegGroupInsertUndoRedoHelper::GroupIndexSetType insertedGroups;

        for (auto group = oldGroupCount; group < newGroupCount; ++group)
          insertedGroups.insert(group);

        SegGroupInsertUndoRedoHelper insertUndoRedoGenerator(resultSegmentation, insertedGroups);
        insertUndoRedoGenerator.RegisterUndoRedoOperationEvent(undoDescription, undoRedoGenerator.has_value());
      }

      // since we are maybe working on a smaller referenceImage, pad it to the size of the original referenceImage
      if (m_ReferenceDataNode.GetPointer() != m_SegmentationInputNode.GetPointer())
      {
        mitkThrow() << "Feature of having a smaller referenceImage then actual workingImage is currently"
          " not supported for new MultiLabelSegmentation class. Evaluation is pending if this feature"
          " should be supported in the future.";
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
    auto activeLabel = workingImage->GetActiveLabel();
    auto newLabel = activeLabel != nullptr
      ? activeLabel->GetValue()
      : Label::UNLABELED_VALUE;
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
  // An update whose work runs off the GUI thread computes what the tool was
  // asked for when it started. Other work, like starting a model, uses the
  // resources an update needs, and only one work runs at a time.
  if (m_IsUpdating || m_WorkRunning)
  {
    MITK_WARN << this->GetName() << ": the tool is busy, the preview is not updated.";
    return;
  }

  this->StartUpdate(ignoreLazyPreviewSetting, {});
}

void mitk::SegWithPreviewTool::StartUpdate(bool ignoreLazyPreviewSetting, std::function<void()> then)
{
  auto pass = std::make_unique<UpdatePass>();
  pass->Input = this->GetSegmentationInput();
  pass->Preview = this->GetPreviewSegmentation();
  this->EnsureUpToDateUserDefinedActiveLabel();

  pass->Task = std::make_unique<ProgressTask>(this->GetName(), 100, this->IsCancelable());
  pass->ScopedTask.emplace(m_ProgressCommand.GetPointer(), pass->Task.get());

  const auto workingSegmentation = this->GetTargetSegmentation();
  pass->WorkingImage = workingSegmentation->GetGroupImage(workingSegmentation->GetActiveLayer());
  pass->TimePoint = RenderingManager::GetInstance()->GetTimeNavigationController()->GetSelectedTimePoint();
  pass->Then = std::move(then);

  if (nullptr != pass->Input && nullptr != pass->Preview)
  {
    if (pass->Preview->GetTimeSteps() > 1 && (ignoreLazyPreviewSetting || !m_LazyDynamicPreviews))
    {
      pass->AllTimeSteps = true;

      for (TimeStepType timeStep = 0; timeStep < pass->Preview->GetTimeSteps(); ++timeStep)
        pass->TimeSteps.push_back(timeStep);
    }
    else
    {
      pass->TimeSteps.push_back(pass->Preview->GetTimeGeometry()->TimePointToTimeStep(pass->TimePoint));
    }
  }

  m_UpdatePass = std::move(pass);

  this->CurrentlyBusy.Send(true);
  m_IsUpdating = true;
  m_HasUnconfirmedPreview = false;

  try
  {
    this->UpdatePrepare();
  }
  catch (...)
  {
    // A tool left busy keeps the tools of the host disabled.
    m_UpdatePass.reset();
    m_IsUpdating = false;
    this->CurrentlyBusy.Send(false);
    throw;
  }

  this->ContinueUpdate();
}

void mitk::SegWithPreviewTool::ContinueUpdate()
{
  auto& pass = *m_UpdatePass;

  while (!pass.Abandoned && pass.NextTimeStep < pass.TimeSteps.size())
  {
    const auto index = pass.NextTimeStep++;
    const auto timeStep = pass.TimeSteps[index];

    Image::ConstPointer feedBackImage;
    Image::ConstPointer currentSegImage;

    if (pass.AllTimeSteps)
    {
      // One run of the filter per time step, each reporting its own
      // fraction, so each of them gets its own share of the task.
      m_ProgressCommand->SetShare(static_cast<unsigned int>(index), static_cast<unsigned int>(pass.TimeSteps.size()));

      const auto previewTimePoint = pass.Preview->GetTimeGeometry()->TimeStepToTimePoint(timeStep);
      const auto inputTimeStep = pass.Input->GetTimeGeometry()->TimePointToTimeStep(previewTimePoint);

      if (nullptr != this->GetWorkingPlaneGeometry())
      { //only extract a specific slice defined by the working plane as feedback referenceImage.
        feedBackImage = SegTool2D::GetAffectedImageSliceAs2DImage(this->GetWorkingPlaneGeometry(), pass.Input, inputTimeStep);
        currentSegImage = SegTool2D::GetAffectedImageSliceAs2DImageByTimePoint(this->GetWorkingPlaneGeometry(), pass.WorkingImage, previewTimePoint);
      }
      else
      { //work on the whole feedback referenceImage
        feedBackImage = this->GetImageByTimeStep(pass.Input, inputTimeStep);
        currentSegImage = this->GetImageByTimePoint(pass.WorkingImage, previewTimePoint);
      }
    }
    else if (nullptr != this->GetWorkingPlaneGeometry())
    {
      feedBackImage = SegTool2D::GetAffectedImageSliceAs2DImageByTimePoint(this->GetWorkingPlaneGeometry(), pass.Input, pass.TimePoint);
      currentSegImage = SegTool2D::GetAffectedImageSliceAs2DImageByTimePoint(this->GetWorkingPlaneGeometry(), pass.WorkingImage, pass.TimePoint);
    }
    else
    {
      feedBackImage = this->GetImageByTimePoint(pass.Input, pass.TimePoint);
      currentSegImage = this->GetImageByTimePoint(pass.WorkingImage, pass.TimePoint);
    }

    m_DefersWork = true;

    try
    {
      this->DoUpdatePreview(feedBackImage, currentSegImage, pass.Preview, timeStep);
    }
    catch (...)
    {
      m_DefersWork = false;
      m_DeferredWork.reset();
      this->HandleUpdateError(std::current_exception());
      this->EndUpdate();
      return;
    }

    m_DefersWork = false;

    if (m_DeferredWork.has_value())
    {
      auto deferred = std::move(*m_DeferredWork);
      m_DeferredWork.reset();

      // Neither the work nor its done runs if the work cannot start, so the
      // update ends here, or the tool would stay busy.
      try
      {
        this->StartWork(std::move(deferred.Work), [this, done = std::move(deferred.Done)](std::exception_ptr error) {
          try
          {
            done(error);
          }
          catch (...)
          {
            this->HandleUpdateError(std::current_exception());
            this->EndUpdate();
            return;
          }

          this->ContinueUpdate();
        });
      }
      catch (...)
      {
        this->HandleUpdateError(std::current_exception());
        this->EndUpdate();
      }

      return;
    }
  }

  if (!pass.Abandoned && !pass.TimeSteps.empty())
  {
    RenderingManager::GetInstance()->RequestUpdateAll();
    pass.Computed = true;
  }

  this->EndUpdate();
}

void mitk::SegWithPreviewTool::HandleUpdateError(std::exception_ptr error)
{
  // Deactivated meanwhile: nobody waits for the outcome any more, and the
  // preview on display is not the one of this update.
  const bool abandoned = m_UpdatePass->Abandoned;

  // Whether cancelled or failed, there is no result to transfer.
  m_UpdatePass->Failed = true;

  try
  {
    std::rethrow_exception(error);
  }
  catch (const itk::ExceptionObject& e)
  {
    // A cancelled preview aborts the filter, which reports the abort as an
    // exception. Reporting that back as an error would turn the user's own
    // decision into a failure message.
    m_UpdatePass->Cancelled = m_UpdatePass->Task->IsCancelRequested();

    if (!m_UpdatePass->Cancelled)
    {
      MITK_ERROR << "Exception caught: " << e.GetDescription();

      if (!abandoned)
        ErrorMessage.Send(e.GetDescription());
    }
    else if (!abandoned)
    {
      // Whatever the filter managed to write before it stopped is not a
      // result. Left on display it offers the user half a segmentation that
      // m_HasUnconfirmedPreview will not let them confirm.
      this->ResetPreviewContent();
      RenderingManager::GetInstance()->RequestUpdateAll();
    }
  }
  catch (const std::exception& e)
  {
    // Covers mitk::AccessByItkException, which is not an ITK exception, so an
    // unsupported pixel type ends up in the error message as well instead of
    // unwinding through the tool activation.
    MITK_ERROR << "Exception caught: " << e.what();

    if (!abandoned)
      ErrorMessage.Send(e.what());
  }
  catch (...)
  {
    MITK_ERROR << "Unknown exception caught.";

    if (!abandoned)
      ErrorMessage.Send("An unknown error occurred.");
  }
}

void mitk::SegWithPreviewTool::EndUpdate()
{
  std::exception_ptr cleanUpError;

  try
  {
    this->UpdateCleanUp();
  }
  catch (...)
  {
    cleanUpError = std::current_exception();
  }

  auto pass = std::move(m_UpdatePass);

  if (cleanUpError == nullptr && !pass->Abandoned)
  {
    // Decided only now, as UpdateCleanUp() can still change the labels of the
    // preview, removing the ones that turned out empty for example.
    if (pass->Computed && !pass->Preview->GetAllLabelValues().empty())
      m_HasUnconfirmedPreview = true;

    // A cancelled update leaves the time point unrecorded on purpose: recording
    // it would tell OnTimePointChanged() that this time point is up to date, so
    // the discarded preview would never be recomputed for it.
    if (!pass->Cancelled)
      m_LastTimePointOfUpdate = pass->TimePoint;
  }

  // A confirmation of a cancelled or failed update would transfer a preview
  // that is empty or half written.
  std::function<void()> then;

  if (!pass->Failed)
    then = std::move(pass->Then);

  pass.reset();

  // However the update ends, the tool is not busy afterwards. A tool left busy
  // keeps the tools of the host disabled.
  m_IsUpdating = false;
  this->CurrentlyBusy.Send(false);

  if (cleanUpError != nullptr)
    std::rethrow_exception(cleanUpError);

  if (then)
    then();
}

void mitk::SegWithPreviewTool::SetGUIThreadPoster(GUIThreadPoster poster)
{
  std::lock_guard lock(m_GUIThreadPosterMutex);
  m_GUIThreadPoster = std::move(poster);
}

mitk::SegWithPreviewTool::GUIThreadPoster mitk::SegWithPreviewTool::GetGUIThreadPoster() const
{
  std::lock_guard lock(m_GUIThreadPosterMutex);
  return m_GUIThreadPoster;
}

void mitk::SegWithPreviewTool::RunOffGUIThread(std::function<void()> work, std::function<void(std::exception_ptr)> done)
{
  if (m_DefersWork)
  {
    if (m_DeferredWork.has_value())
      mitkThrow() << this->GetName() << ": a time step of an update can run one work off the GUI thread only.";

    m_DeferredWork = DeferredWork{ std::move(work), std::move(done) };
    return;
  }

  this->StartWork(std::move(work), std::move(done));
}

void mitk::SegWithPreviewTool::StartWork(std::function<void()> work, std::function<void(std::exception_ptr)> done)
{
  if (m_WorkRunning)
    mitkThrow() << this->GetName() << ": other work off the GUI thread has not finished yet.";

  // Runs the done of the work, which is the end of the work for the caller.
  // What it throws, like a failed transfer that followed an update, has no
  // caller left to go to, so it goes to the user.
  const auto finish = [this](const std::function<void(std::exception_ptr)>& done, std::exception_ptr error) {
    try
    {
      done(error);
    }
    catch (const itk::ExceptionObject& e)
    {
      MITK_ERROR << this->GetName() << ": " << e.GetDescription();
      this->ErrorMessage.Send(e.GetDescription());
    }
    catch (const std::exception& e)
    {
      MITK_ERROR << this->GetName() << ": " << e.what();
      this->ErrorMessage.Send(e.what());
    }
    catch (...)
    {
      MITK_ERROR << this->GetName() << ": unknown exception after work off the GUI thread.";
      this->ErrorMessage.Send("An unknown error occurred.");
    }
  };

  const auto poster = this->GetGUIThreadPoster();

  if (!poster)
  {
    std::exception_ptr error;

    try
    {
      work();
    }
    catch (...)
    {
      error = std::current_exception();
    }

    finish(done, error);
    return;
  }

  // Set once the thread exists, which may fail. Its done cannot reset it
  // before: that runs on this thread, after this function has returned.
  m_WorkThread = std::thread([this, poster, finish, alive = std::weak_ptr<int>(m_Alive), work = std::move(work), done = std::move(done)]() mutable {
    std::exception_ptr error;

    try
    {
      work();
    }
    catch (...)
    {
      error = std::current_exception();
    }

    // Moved, not copied: what done holds, like widgets of a GUI, must be
    // destroyed on the GUI thread, not when this thread ends.
    poster([this, finish, alive, done = std::move(done), error]() {
      // The tool is gone, and with it what done would work on.
      if (alive.expired())
        return;

      if (m_WorkThread.joinable())
        m_WorkThread.join();

      m_WorkRunning = false;
      finish(done, error);
    });
  });

  m_WorkRunning = true;
}

void mitk::SegWithPreviewTool::PostToGUIThread(std::function<void()> function)
{
  const auto poster = this->GetGUIThreadPoster();

  if (!poster)
  {
    function();
    return;
  }

  poster([alive = std::weak_ptr<int>(m_Alive), function = std::move(function)]() {
    if (!alive.expired())
      function();
  });
}

bool mitk::SegWithPreviewTool::IsRunningOffGUIThread() const
{
  return m_WorkRunning;
}

void mitk::SegWithPreviewTool::WaitForWorkOffGUIThread()
{
  if (m_WorkThread.joinable())
    m_WorkThread.join();
}

bool mitk::SegWithPreviewTool::IsCancelable() const
{
  // No filter that a tool currently hands to ToolCommand reads the abort
  // flag, so nothing in the tree overrides this yet.
  return false;
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
  const mitk::MultiLabelSegmentation* source, mitk::MultiLabelSegmentation* target, bool intoSourceGroups)
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
    if (MultiLabelSegmentation::UNLABELED_VALUE == sourceLabel || MultiLabelSegmentation::UNLABELED_VALUE == targetLabel)
      continue;

    const auto targetGroup = intoSourceGroups && source->ExistLabel(sourceLabel)
      ? source->GetGroupIndexOfLabel(sourceLabel)
      : target->GetActiveLayer();

    if (target->ExistLabel(targetLabel, targetGroup))
      continue;

    if (!source->ExistLabel(sourceLabel))
    {
      mitkThrow() << "Cannot prepare segmentation for preview transfer. Preview seems invalid as label is missing. Missing label: " << sourceLabel;
    }

    while (target->GetNumberOfGroups() <= targetGroup)
      target->AddGroup();

    auto clonedLabel = source->GetLabel(sourceLabel)->Clone();
    clonedLabel->SetValue(targetLabel);
    target->AddLabel(clonedLabel, targetGroup, false, false);
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
    TransferLabelInformation(labelMapping, preview, resultSegmentation, m_TransfersAllPreviewGroups);
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
  return m_HasUnconfirmedPreview && m_RequestDeactivationConfirmation;
}
