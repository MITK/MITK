/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkLabelSetImage.h"

#include <mitkImageAccessByItk.h>
#include <mitkImageCast.h>
#include <mitkImagePixelReadAccessor.h>
#include <mitkImagePixelWriteAccessor.h>
#include <mitkPadImageFilter.h>
#include <mitkDICOMSegmentationPropertyHelper.h>
#include <mitkDICOMQIPropertyHelper.h>
#include <mitkNodePredicateGeometry.h>
#include <mitkLabelSetImageHelper.h>
#include <mitkImageTimeSelector.h>
#include <itkLabelGeometryImageFilter.h>
#include <itkCommand.h>
#include <itkBinaryFunctorImageFilter.h>


namespace mitk
{
  template <typename ImageType>
  void ClearBufferProcessing(ImageType* itkImage)
  {
    itkImage->FillBuffer(0);
  }

  void ClearImageBuffer(mitk::Image* image)
  {
    if (image->GetDimension() == 4)
    { //remark: this extra branch was added, because MultiLabelSegmentation instances can be
      //dynamic (4D), but AccessByItk by support only supports 2D and 3D.
      //The option to change the CMake default dimensions for AccessByItk was
      //dropped (for details see discussion in T28756)
      AccessFixedDimensionByItk(image, ClearBufferProcessing, 4);
    }
    else
    {
      AccessByItk(image, ClearBufferProcessing);
    }
  }
}

const mitk::MultiLabelSegmentation::LabelValueType mitk::MultiLabelSegmentation::UNLABELED_VALUE = 0;

mitk::PixelType mitk::MultiLabelSegmentation::GetPixelType()
{
  return MakePixelType<LabelValueType, LabelValueType, 1>();
};


bool mitk::MultiLabelSegmentation::IsSliceSet(int s, int t, int n) const
{
  for (const auto& image : m_GroupContainer)
  {
    if (!image->IsSliceSet(s, t, n)) return false;
  }
  return true;
}

bool mitk::MultiLabelSegmentation::IsVolumeSet(int t, int n) const
{
  for (const auto& image : m_GroupContainer)
  {
    if (!image->IsVolumeSet(t, n)) return false;
  }
  return true;
}

bool mitk::MultiLabelSegmentation::IsChannelSet(int n) const
{
  for (const auto& image : m_GroupContainer)
  {
    if (!image->IsChannelSet(n)) return false;
  }
  return true;
}

unsigned int mitk::MultiLabelSegmentation::GetDimension() const
{
  return m_GroupImageDimensions.size();
};

const mitk::MultiLabelSegmentation::GroupImageDimensionVectorType& mitk::MultiLabelSegmentation::GetDimensions() const
{
  return m_GroupImageDimensions;
};

mitk::MultiLabelSegmentation::MultiLabelSegmentation()
  : mitk::SlicedData(), m_ActiveLabelValue(0), m_UnlabeledLabelLock(false)
{
  m_LookupTable = mitk::LookupTable::New();
  m_LookupTable->SetType(mitk::LookupTable::MULTILABEL);

  // Add some DICOM Tags as properties to segmentation image
  DICOMSegmentationPropertyHelper::DeriveDICOMSegmentationProperties(this);
}

mitk::MultiLabelSegmentation::MultiLabelSegmentation(const mitk::MultiLabelSegmentation &other)
  : SlicedData(other),
    m_ActiveLabelValue(other.m_ActiveLabelValue),
    m_LookupTable(other.m_LookupTable->Clone()),
    m_UnlabeledLabelLock(other.m_UnlabeledLabelLock),
    m_GroupImageDimensions(other.m_GroupImageDimensions)
{
  GroupIndexType i = 0;
  for (auto groupImage : other.m_GroupContainer)
  {
    this->AddGroup(groupImage->Clone(), other.GetConstLabelsByValue(other.GetLabelValuesByGroup(i)));
    i++;
  }
  m_Groups = other.m_Groups;

  // Add some DICOM Tags as properties to segmentation image
  DICOMSegmentationPropertyHelper::DeriveDICOMSegmentationProperties(this);
}

mitk::Image::Pointer mitk::MultiLabelSegmentation::GenerateNewGroupImage() const
{
  auto groupImage = Image::New();

  auto pixelType = this->GetPixelType();

  auto geometryDimensions = DetermineImageDimensionsFromTimeGeometry(this->GetTimeGeometry());
  if (geometryDimensions.size() == 2)
  {
    auto dimensions = std::array{ m_GroupImageDimensions[0], m_GroupImageDimensions[1], 1u };
    groupImage->Initialize(pixelType, 3, dimensions.data());
  }
  else
  {
    groupImage->Initialize(pixelType, *(this->GetTimeGeometry()));
  }
  groupImage->GetTimeGeometry()->UpdateBoundingBox();
  return groupImage;
}

void mitk::MultiLabelSegmentation::Initialize()
{
  this->Initialize(this->GetTimeGeometry(), true, false);
}

void mitk::MultiLabelSegmentation::Initialize(const mitk::Image * templateImage, bool resetLabels, bool ensure1stGroup)
{
  if (nullptr == templateImage) mitkThrow() << "Cannot initialize multi label segmentation instance. Passed template image is a nullptr.";

  auto originalGeometry = templateImage->GetTimeGeometry()->Clone();
  originalGeometry->UpdateBoundingBox();
  this->SetTimeGeometry(originalGeometry);
  m_GroupImageDimensions = GroupImageDimensionVectorType(templateImage->GetDimensions(), templateImage->GetDimensions() + templateImage->GetDimension());

  if (resetLabels)
  {
    while (this->GetNumberOfGroups() > 0)
    {
      this->RemoveGroup(0);
    }
  }
  else
  {
    for (auto& imagePtr : m_GroupContainer)
    {
      imagePtr = this->GenerateNewGroupImage();
      ClearImageBuffer(imagePtr);
    }
  }

  // Transfer some general DICOM properties from the source image to derived image (e.g. Patient information,...)
  DICOMQIPropertyHelper::DeriveDICOMSourceProperties(templateImage, this);

  // Add an initial LabelSet and corresponding image data to the stack
  if (ensure1stGroup && this->GetNumberOfGroups() == 0)
  {
    AddGroup();
  }
}

void mitk::MultiLabelSegmentation::Initialize(const mitk::TimeGeometry* geometry, bool resetLabels, bool ensure1stGroup)
{
  if (nullptr == geometry) mitkThrow() << "Cannot initialize multi label segmentation instance. Passed time geometry is a nullptr.";

  auto clonedGeometry = geometry->Clone();

  m_GroupImageDimensions = DetermineImageDimensionsFromTimeGeometry(clonedGeometry);

  // make sure the image geometry flag is properly set for all time steps
  for (TimeStepType step = 0; step < clonedGeometry->CountTimeSteps(); ++step)
  {
    if (!clonedGeometry->GetGeometryCloneForTimeStep(step)->GetImageGeometry())
    {
      MITK_WARN("Image.3DnT.Initialize") << " Attempt to initialize an image with a non-image geometry. "
        "Re-interpreting the initialization geometry for time step "
        << step << " as image geometry, the original geometry remains unchanged.";
      clonedGeometry->GetGeometryForTimeStep(step)->ImageGeometryOn();
    }
  }
  clonedGeometry->UpdateBoundingBox();
  this->SetTimeGeometry(clonedGeometry);

  if (resetLabels)
  {
    while (this->GetNumberOfGroups() > 0)
    {
      this->RemoveGroup(0);
    }
  }
  else
  {
    for (auto& imagePtr : m_GroupContainer)
    {
      imagePtr = this->GenerateNewGroupImage();
      ClearImageBuffer(imagePtr);
    }
  }

  // Add an initial LabelSet and corresponding image data to the stack
  if (ensure1stGroup && this->GetNumberOfGroups() == 0)
  {
    AddGroup();
  }
}


mitk::MultiLabelSegmentation::~MultiLabelSegmentation()
{
  for (auto [value, label] : m_LabelMap)
  {
    (void)value; // Prevent unused variable error in older compilers
    this->ReleaseLabel(label);
  }
  m_LabelMap.clear();
}

unsigned int mitk::MultiLabelSegmentation::GetActiveLayer() const
{
  if (m_GroupContainer.empty()) mitkThrow() << "Cannot return active group index. No group is available.";
  if (m_ActiveLabelValue == UNLABELED_VALUE) return 0;

  return this->GetGroupIndexOfLabel(m_ActiveLabelValue);
}

unsigned int mitk::MultiLabelSegmentation::GetNumberOfGroups() const
{
  return m_GroupContainer.size();
}

void mitk::MultiLabelSegmentation::RemoveGroup(GroupIndexType indexToDelete)
{
  if (!this->ExistGroup(indexToDelete)) mitkThrow() << "Cannot remove group. Group does not exist. Invalid group index: "<<indexToDelete;

  auto relevantLabels = m_GroupToLabelMap[indexToDelete];

  {
    std::lock_guard<std::shared_mutex> guard(m_LabelNGroupMapsMutex);
    // remove labels of group
    for (auto labelValue : relevantLabels)
    {
      auto label = m_LabelMap[labelValue];
      this->ReleaseLabel(label);
      m_LabelToGroupMap.erase(labelValue);
      m_LabelMap.erase(labelValue);
      this->InvokeEvent(LabelRemovedEvent(labelValue));
    }
    // remove the group entries in the maps and the image.
    m_Groups.erase(m_Groups.begin() + indexToDelete);
    m_GroupToLabelMap.erase(m_GroupToLabelMap.begin() + indexToDelete);
    m_GroupContainer.erase(m_GroupContainer.begin() + indexToDelete);
  }

  //update old indexes in m_LabelToGroupMap to new group indexes
  for (auto& element : m_LabelToGroupMap)
  {
    if (element.second > indexToDelete) element.second = element.second -1;
  }

  if (!this->ExistLabel(m_ActiveLabelValue))
  {
    //the current active label was removed with the group. Set active label to the first defined label
    if (!m_LabelMap.empty())
    {
      m_ActiveLabelValue = m_LabelMap.begin()->first;
    }
    else
    {
      m_ActiveLabelValue = MultiLabelSegmentation::UNLABELED_VALUE;
    }
  }

  this->InvokeEvent(LabelsChangedEvent(relevantLabels));
  this->InvokeEvent(GroupRemovedEvent(indexToDelete));
  this->Modified();
}

mitk::MultiLabelSegmentation::LabelValueVectorType mitk::MultiLabelSegmentation::ExtractLabelValuesFromLabelVector(const LabelVectorType& labels)
{
  LabelValueVectorType result;

  for (auto label : labels)
  {
    result.emplace_back(label->GetValue());
  }
  return result;
}

mitk::MultiLabelSegmentation::LabelValueVectorType mitk::MultiLabelSegmentation::ExtractLabelValuesFromLabelVector(const ConstLabelVectorType& labels)
{
  LabelValueVectorType result;

  for (auto label : labels)
  {
    result.emplace_back(label->GetValue());
  }
  return result;
}

mitk::MultiLabelSegmentation::ConstLabelVectorType mitk::MultiLabelSegmentation::ConvertLabelVectorConst(const LabelVectorType& labels)
{
  ConstLabelVectorType result(labels.begin(), labels.end());
  return result;
};

const mitk::MultiLabelSegmentation::LabelValueVectorType mitk::MultiLabelSegmentation::GetAllLabelValues() const
{
  LabelValueVectorType result;

  for (auto [value, label] : m_LabelMap)
  {
    (void)label; // Prevent unused variable error in older compilers
    result.emplace_back(value);
  }
  return result;
}

mitk::MultiLabelSegmentation::LabelValueVectorType mitk::MultiLabelSegmentation::GetUsedLabelValues() const
{
  LabelValueVectorType result = { UNLABELED_VALUE };

  for (auto [value, label] : m_LabelMap)
  {
    (void)label; // Prevent unused variable error in older compilers
    result.emplace_back(value);
  }

  return result;
}

mitk::MultiLabelSegmentation::GroupIndexType mitk::MultiLabelSegmentation::AddGroup(ConstLabelVector labels)
{
  auto newImage = this->GenerateNewGroupImage();
  ClearImageBuffer(newImage);

  return this->AddGroup(newImage, labels);
}

mitk::MultiLabelSegmentation::GroupIndexType mitk::MultiLabelSegmentation::AddGroup(mitk::Image* groupImage, ConstLabelVector labels)
{
  GroupIndexType newGroupID = m_Groups.size();

  if (nullptr == groupImage)
    mitkThrow() << "Cannot add group. Passed group image is nullptr.";

  bool equalGeometries = Equal(
    *(this->GetTimeGeometry()),
    *(groupImage->GetTimeGeometry()),
    NODE_PREDICATE_GEOMETRY_DEFAULT_CHECK_COORDINATE_PRECISION,
    NODE_PREDICATE_GEOMETRY_DEFAULT_CHECK_DIRECTION_PRECISION,
    false);

  if (!equalGeometries)
    mitkThrow() << "Cannot add group. Passed group image has not the same geometry like segmentation.";

  if (groupImage->GetPixelType() != MakePixelType<LabelValueType, LabelValueType, 1>())
    mitkThrow() << "Cannot add group. Passed group image has incorrect pixel type. Only LabelValueType is supported. Invalid pixel type: "<< groupImage->GetPixelType().GetTypeAsString();

  // push a new working image for the new group
  m_GroupContainer.push_back(groupImage);

  m_Groups.push_back("");
  m_GroupToLabelMap.push_back({});

  for (auto label : labels)
  {
    if (m_LabelMap.end() != m_LabelMap.find(label->GetValue()))
    {
      mitkThrow() << "Cannot add group. Labels that should be added with group use at least one label value that is already in use. Conflicted label value: " << label->GetValue();
    }

    auto labelClone = label->Clone();

    DICOMSegmentationPropertyHelper::SetDICOMSegmentProperties(labelClone);
    this->AddLabelToMap(labelClone->GetValue(), labelClone, newGroupID);
    this->RegisterLabel(labelClone);
  }

  this->Modified();
  this->InvokeEvent(GroupAddedEvent(newGroupID));

  return newGroupID;
}

void mitk::MultiLabelSegmentation::ReplaceGroupLabels(const GroupIndexType groupID, const ConstLabelVectorType& labelSet)
{
  if (m_GroupContainer.size() <= groupID)
  {
    mitkThrow() << "Trying to replace labels of non-existing group. Invalid group id: "<<groupID;
  }

  auto oldActiveLabel = m_ActiveLabelValue;

  LabelValueVectorType removedLabels;
  LabelValueVectorType addedLabels;
  LabelValueVectorType modifiedLabels;

  LabelValueVectorType oldLabels;
  {
    std::lock_guard<std::shared_mutex> guard(m_LabelNGroupMapsMutex);

    //remove old group labels
    oldLabels = this->m_GroupToLabelMap[groupID];
    for (auto labelID : oldLabels)
    {
      this->RemoveLabelFromMap(labelID);
      if (std::find_if(labelSet.cbegin(), labelSet.cend(), [labelID](const Label* label){return label->GetValue() == labelID;}) == labelSet.cend())
      { //label is not in the new set, so it will be effectively removed
        removedLabels.push_back(labelID);
      }
    }

    //add new labels to group
    for (auto label : labelSet)
    {
      if (m_LabelMap.find(label->GetValue()) != m_LabelMap.cend())
      {
        auto conflictingGroup = this->GetGroupIndexOfLabel(label->GetValue());
        mitkThrow() << "Error while replacing labels. Label value is already existing in another group. Invalid label: " << label->GetValue() << "; conflicting group: " << conflictingGroup;
      }

      // add DICOM information of the label
      auto clonedLabel = label->Clone();
      DICOMSegmentationPropertyHelper::SetDICOMSegmentProperties(clonedLabel);

      this->AddLabelToMap(clonedLabel->GetValue(), clonedLabel, groupID);
      this->RegisterLabel(clonedLabel);

      if (std::find(oldLabels.cbegin(), oldLabels.cend(), label->GetValue()) == oldLabels.cend())
      { //label was not in the old set, so it was effectively added
        addedLabels.push_back(clonedLabel->GetValue());
      }
      else
      {
        modifiedLabels.push_back(clonedLabel->GetValue());
      }
    }
  }

  //now after the manipulation operation send all events
  for (auto labelID : removedLabels)
  {
    this->InvokeEvent(LabelRemovedEvent(labelID));
  }
  for (auto labelID : addedLabels)
  {
    this->InvokeEvent(LabelAddedEvent(labelID));
  }
  for (auto labelID : modifiedLabels)
  {
    this->InvokeEvent(LabelModifiedEvent(labelID));
  }

  this->InvokeEvent(LabelsChangedEvent(oldLabels));
  this->InvokeEvent(GroupModifiedEvent(groupID));
  this->Modified();

  if (!this->ExistLabel(oldActiveLabel))
  { //Active label must be redefined, because it was removed by replacement.
    this->SetActiveLabel(m_LabelMap.empty() ? UNLABELED_VALUE : m_LabelMap.begin()->second->GetValue());
  }
}

void mitk::MultiLabelSegmentation::ReplaceGroupLabels(const GroupIndexType groupID, const LabelVectorType& labelSet)
{
  return ReplaceGroupLabels(groupID, ConvertLabelVectorConst(labelSet));
}

mitk::Image* mitk::MultiLabelSegmentation::GetGroupImage(GroupIndexType groupID)
{
  if (!this->ExistGroup(groupID)) mitkThrow() << "Error, cannot return group image. Group ID is invalid. Invalid ID: " << groupID;

  return m_GroupContainer[groupID];
}


const mitk::Image* mitk::MultiLabelSegmentation::GetGroupImage(GroupIndexType groupID) const
{
  if (!this->ExistGroup(groupID)) mitkThrow() << "Error, cannot return group image. Group ID is invalid. Invalid ID: " << groupID;

  return m_GroupContainer.at(groupID).GetPointer();
}

void mitk::MultiLabelSegmentation::UpdateGroupImage(GroupIndexType groupID, const mitk::Image* sourceImage, TimeStepType timestep, TimeStepType sourceTimestep)
{
  if (!this->ExistGroup(groupID)) mitkThrow() << "Error, cannot update group image. Group ID is invalid. Invalid ID: " << groupID;
  if (nullptr == sourceImage) mitkThrow() << "Error, cannot update group image. Passed sourceImage is invalid.";
  if (this->GetTimeSteps()<=timestep) mitkThrow() << "Error, cannot update group image. Assigned time step is not valid for segmentation. Invalid time step: " << timestep;
  if (sourceImage->GetTimeSteps() <= sourceTimestep) mitkThrow() << "Error, cannot update group image. Requested time step of source image is not valid. Invalid source time step: " << sourceTimestep;

  if (!mitk::Equal(*(m_GroupContainer[groupID]->GetGeometry(timestep)), *(sourceImage->GetGeometry(sourceTimestep)), mitk::NODE_PREDICATE_GEOMETRY_DEFAULT_CHECK_COORDINATE_PRECISION, mitk::NODE_PREDICATE_GEOMETRY_DEFAULT_CHECK_DIRECTION_PRECISION))
    mitkThrow() << "Error, cannot update group image. Passed sourceImage has not the same geometry then the MultiLabelSegmentationInstance.";

  auto imageTimeStep = SelectImageByTimeStep(sourceImage, sourceTimestep);
  mitk::ImageReadAccessor sourceImageAcc(imageTimeStep);
  m_GroupContainer[groupID]->SetVolume(sourceImageAcc.GetData(), timestep);
}


const std::string& mitk::MultiLabelSegmentation::GetGroupName(GroupIndexType groupID) const
{
  if (!this->ExistGroup(groupID))
    mitkThrow() << "Error, cannot return group name. Group ID is invalid. Invalid ID: " << groupID;

  return m_Groups[groupID];
}

void mitk::MultiLabelSegmentation::SetGroupName(GroupIndexType groupID, const std::string& name)
{
  if (!this->ExistGroup(groupID))
    mitkThrow() << "Error, cannot set group name. Group ID is invalid. Invalid ID: " << groupID;

  m_Groups[groupID] = name;
  this->InvokeEvent(GroupModifiedEvent(groupID));
}


void mitk::MultiLabelSegmentation::SetActiveLabel(LabelValueType label)
{
  if (m_ActiveLabelValue != label)
  {
    bool eventNeeded = false;
    if (UNLABELED_VALUE == m_ActiveLabelValue || UNLABELED_VALUE == label || !this->ExistLabel(m_ActiveLabelValue))
    {
      eventNeeded = true;
    }
    else if (this->GetGroupIndexOfLabel(m_ActiveLabelValue) != this->GetGroupIndexOfLabel(label))
    {
      eventNeeded = true;
    }

    m_ActiveLabelValue = label;
    Modified();

    if (eventNeeded)
    {
      AfterChangeLayerEvent.Send();
    }
  }
}

void mitk::MultiLabelSegmentation::ClearGroupImage(GroupIndexType groupID)
{
  if (!this->ExistGroup(groupID))
    mitkThrow() << "Error, cannot clear group image. Group ID is invalid. Invalid ID: " << groupID;

  try
  {
    auto groupImage = this->GetGroupImage(groupID);
    ClearImageBuffer(groupImage);
    groupImage->Modified();
    this->InvokeEvent(LabelsChangedEvent(this->GetLabelValuesByGroup(groupID)));
    this->InvokeEvent(GroupModifiedEvent(groupID));
  }
  catch (itk::ExceptionObject &e)
  {
    mitkThrow() << e.GetDescription();
  }
  this->Modified();
}

void mitk::MultiLabelSegmentation::ClearGroupImage(GroupIndexType groupID, TimeStepType timestep)
{
  if (!this->ExistGroup(groupID))
    mitkThrow() << "Error, cannot clear group image. Group ID is invalid. Invalid ID: " << groupID;

  if (timestep >= this->GetTimeSteps())
    mitkThrow() << "Error, cannot clear group image time step. Time step " << timestep << " is invalid. Number of time steps: " << this->GetTimeSteps();

  try
  {
    auto groupImage = this->GetGroupImage(groupID);
    auto tsImage = SelectImageByTimeStep(groupImage, timestep);
    ClearImageBuffer(tsImage);
    groupImage->Modified();
    this->InvokeEvent(LabelsChangedEvent(this->GetLabelValuesByGroup(groupID)));
    this->InvokeEvent(GroupModifiedEvent(groupID));
  }
  catch (itk::ExceptionObject& e)
  {
    mitkThrow() << e.GetDescription();
  }
  this->Modified();
}

void mitk::MultiLabelSegmentation::ClearGroupImages()
{
  for (GroupIndexType groupID = 0; groupID < m_GroupContainer.size(); ++groupID)
  {
    try
    {
      auto groupImage = this->GetGroupImage(groupID);
      ClearImageBuffer(groupImage);
      groupImage->Modified();
      this->InvokeEvent(LabelsChangedEvent(this->GetLabelValuesByGroup(groupID)));
      this->InvokeEvent(GroupModifiedEvent(groupID));
    }
    catch (itk::ExceptionObject& e)
    {
      mitkThrow() << e.GetDescription();
    }
  }
  this->Modified();
}

void mitk::MultiLabelSegmentation::ClearGroupImages(TimeStepType timestep)
{
  if (timestep >= this->GetTimeSteps())
    mitkThrow() << "Error, cannot clear group image time step. Time step " << timestep << " is invalid. Number of time steps: " << this->GetTimeSteps();

  for (GroupIndexType groupID = 0; groupID < m_GroupContainer.size(); ++groupID)
  {
    try
    {
      auto groupImage = this->GetGroupImage(groupID);
      auto tsImage = SelectImageByTimeStep(groupImage, timestep);
      ClearImageBuffer(tsImage);
      groupImage->Modified();
      this->InvokeEvent(LabelsChangedEvent(this->GetLabelValuesByGroup(groupID)));
      this->InvokeEvent(GroupModifiedEvent(groupID));
    }
    catch (itk::ExceptionObject& e)
    {
      mitkThrow() << e.GetDescription();
    }
  }
  this->Modified();
}

void mitk::MultiLabelSegmentation::MergeLabel(LabelValueType targetLabelValue, LabelValueType sourceLabelValue, OverwriteStyle overwriteStyle)
{
  if (!this->ExistLabel(sourceLabelValue)) mitkThrow() << "Cannot merge label. Source label value ("<<sourceLabelValue<<") does not exist.";
  if (!this->ExistLabel(targetLabelValue)) mitkThrow() << "Cannot merge label. Target label value (" << targetLabelValue << ") does not exist.";

  this->MergeLabels(targetLabelValue, { sourceLabelValue }, overwriteStyle);
}

void mitk::MultiLabelSegmentation::MergeLabels(LabelValueType targetLabelValue, const LabelValueVectorType& sourceLabelValues,
  OverwriteStyle overwriteStyle)
{
  for (const auto& value : sourceLabelValues)
  {
    if (!this->ExistLabel(value)) mitkThrow() << "Cannot merge label. Source label value (" << value << ") does not exist.";
  }

  if (!this->ExistLabel(targetLabelValue)) mitkThrow() << "Cannot merge label. Target label value (" << targetLabelValue << ") does not exist.";

  auto targetGroupID = this->GetGroupIndexOfLabel(targetLabelValue);
  auto sourceGroupMapping = LabelSetImageHelper::SplitLabelValuesByGroup(this, sourceLabelValues);

  ////////////////////////////////////////
  //prepare label information for transfer
  auto targetGroupLabelValues = this->GetLabelValuesByGroup(targetGroupID);

  //remove all source labels, if they are in labels, because we want to ensure that these are not locked for the transfer
  auto isSourceValueCheckLambda = [&](LabelValueType x)
    { return std::find(sourceLabelValues.begin(), sourceLabelValues.end(), x) != sourceLabelValues.end(); };
  targetGroupLabelValues.erase(std::remove_if(targetGroupLabelValues.begin(), targetGroupLabelValues.end(), isSourceValueCheckLambda),
    targetGroupLabelValues.end());

  auto labels = this->GetConstLabelsByValue(targetGroupLabelValues);
  for (const auto value : sourceLabelValues)
  {
    auto unlockedSourceLabel = this->GetLabel(value)->Clone();
    unlockedSourceLabel->SetLocked(false);
    labels.push_back(unlockedSourceLabel);
  }

  ////////////////////////////////////////////////////////////////////
  //iterate through all relevant source groups and merge there content
  for (const auto& [sourceGroupID, relevantSourceLabelValues] : sourceGroupMapping)
  {
    LabelValueMappingVector mapping;
    for (const auto value : relevantSourceLabelValues)
    {
      mapping.emplace_back(value, targetLabelValue);
    }

    //do the merge as a transfer operation
    mitk::TransferLabelContent(this->GetGroupImage(sourceGroupID), this->GetGroupImage(targetGroupID), labels,
      mitk::MultiLabelSegmentation::UNLABELED_VALUE, mitk::MultiLabelSegmentation::UNLABELED_VALUE, false, mapping,
      MergeStyle::Merge, overwriteStyle);
  }

  this->InvokeEvent(LabelModifiedEvent(targetLabelValue));
  for (const auto value : sourceLabelValues)
  {
    this->InvokeEvent(LabelModifiedEvent(value));
  }

  auto modifiedValues = sourceLabelValues;
  modifiedValues.push_back(targetLabelValue);
  this->InvokeEvent(LabelsChangedEvent(modifiedValues));

  Modified();
}

void mitk::MultiLabelSegmentation::RemoveLabel(LabelValueType pixelValue)
{
  GroupIndexType groupID = 0;
  {
    std::lock_guard<std::shared_mutex> guard(m_LabelNGroupMapsMutex);
    if (m_LabelMap.find(pixelValue) == m_LabelMap.end()) return;

    groupID = this->GetGroupIndexOfLabel(pixelValue);

    //first erase the pixel content (also triggers a LabelModified event)
    this->EraseLabel(pixelValue);
    this->RemoveLabelFromMap(pixelValue);


    if (m_ActiveLabelValue == pixelValue)
    {
      this->SetActiveLabel(0);
    }
  }

  this->InvokeEvent(LabelRemovedEvent(pixelValue));
  this->InvokeEvent(LabelsChangedEvent({ pixelValue }));
  this->InvokeEvent(GroupModifiedEvent(groupID));
}

void mitk::MultiLabelSegmentation::RemoveLabelFromMap(LabelValueType pixelValue)
{
  if (m_LabelMap.find(pixelValue) == m_LabelMap.end()) mitkThrow()<<"Invalid state of instance. RemoveLabelFromMap was called for unknown label id. invalid label id: "<<pixelValue;

  auto groupID = this->GetGroupIndexOfLabel(pixelValue);

  this->ReleaseLabel(m_LabelMap[pixelValue]);
  //now remove the label entry itself
  m_LabelMap.erase(pixelValue);
  m_LabelToGroupMap.erase(pixelValue);
  auto labelsInGroup = m_GroupToLabelMap[groupID];
  labelsInGroup.erase(std::remove(labelsInGroup.begin(), labelsInGroup.end(), pixelValue), labelsInGroup.end());
  m_GroupToLabelMap[groupID] = labelsInGroup;
}

void mitk::MultiLabelSegmentation::RemoveLabels(const LabelValueVectorType& vectorOfLabelPixelValues)
{
  for (const auto labelValue : vectorOfLabelPixelValues)
  {
    this->RemoveLabel(labelValue);
  }
  this->InvokeEvent(LabelsChangedEvent(vectorOfLabelPixelValues));
}

void mitk::MultiLabelSegmentation::EraseLabel(LabelValueType pixelValue)
{
  try
  {
    auto groupID = this->GetGroupIndexOfLabel(pixelValue);

    mitk::Image* groupImage = this->GetGroupImage(groupID);

    if (4 == this->GetDimension())
    {
      AccessFixedDimensionByItk_1(groupImage, EraseLabelProcessing, 4, pixelValue);
    }
    else
    {
      AccessByItk_1(groupImage, EraseLabelProcessing, pixelValue);
    }
    groupImage->Modified();
  }
  catch (const itk::ExceptionObject& e)
  {
    mitkThrow() << e.GetDescription();
  }

  this->InvokeEvent(LabelModifiedEvent(pixelValue));
  this->InvokeEvent(LabelsChangedEvent({ pixelValue }));
  Modified();
}

void mitk::MultiLabelSegmentation::EraseLabels(const LabelValueVectorType& labelValues)
{
  for (auto labelValue : labelValues)
  {
    this->EraseLabel(labelValue);
  }
}

mitk::MultiLabelSegmentation::LabelValueType mitk::MultiLabelSegmentation::GetUnusedLabelValue() const
{
  auto usedValues = this->GetUsedLabelValues();
  return usedValues.back() + 1;
}

mitk::Label* mitk::MultiLabelSegmentation::AddLabel(mitk::Label* label, GroupIndexType groupID, bool addAsClone, bool correctLabelValue)
{
  if (nullptr == label) mitkThrow() << "Invalid use of AddLabel. label is not valid.";

  mitk::Label::Pointer newLabel = label;

  {
    std::lock_guard<std::shared_mutex> guard(m_LabelNGroupMapsMutex);

    unsigned int max_size = mitk::Label::MAX_LABEL_VALUE + 1;
    if (m_GroupContainer.size() >= max_size)
      return nullptr;

    if (addAsClone) newLabel = label->Clone();

    auto pixelValue = newLabel->GetValue();
    auto usedValues = this->GetUsedLabelValues();
    auto finding = std::find(usedValues.begin(), usedValues.end(), pixelValue);

    if (!usedValues.empty() && usedValues.end() != finding)
    {
      if (correctLabelValue)
      {
        pixelValue = this->GetUnusedLabelValue();
        newLabel->SetValue(pixelValue);
      }
      else
      {
        mitkThrow() << "Cannot add label due to conflicting label value that already exists in the MultiLabelSegmentation. Conflicting label value: " << pixelValue;
      }
    }

    // add DICOM information of the label
    DICOMSegmentationPropertyHelper::SetDICOMSegmentProperties(newLabel);

    this->AddLabelToMap(pixelValue, newLabel, groupID);
    this->RegisterLabel(newLabel);
  }

  this->InvokeEvent(LabelAddedEvent(newLabel->GetValue()));
  m_ActiveLabelValue = newLabel->GetValue();
  this->Modified();

  return newLabel;
}

mitk::Label* mitk::MultiLabelSegmentation::AddLabelWithContent(Label* label, const Image* labelContent, GroupIndexType groupID, LabelValueType contentLabelValue, bool addAsClone, bool correctLabelValue)
{
  if (nullptr == labelContent) mitkThrow() << "Invalid use of AddLabel. labelContent is not valid.";
  if (!Equal(*(this->GetTimeGeometry()), *(labelContent->GetTimeGeometry()), mitk::NODE_PREDICATE_GEOMETRY_DEFAULT_CHECK_COORDINATE_PRECISION, mitk::NODE_PREDICATE_GEOMETRY_DEFAULT_CHECK_DIRECTION_PRECISION))
    mitkThrow() << "Invalid use of AddLabel. labelContent has not the same geometry like the segmentation.";

  auto newLabel = this->AddLabel(label, groupID, addAsClone, correctLabelValue);

  mitk::TransferLabelContent(labelContent, this->GetGroupImage(groupID), this->GetConstLabelsByValue(this->GetLabelValuesByGroup(groupID)),
    mitk::MultiLabelSegmentation::UNLABELED_VALUE, mitk::MultiLabelSegmentation::UNLABELED_VALUE, false, { {contentLabelValue, newLabel->GetValue()}},
    mitk::MultiLabelSegmentation::MergeStyle::Replace, mitk::MultiLabelSegmentation::OverwriteStyle::RegardLocks);

  this->Modified();

  return newLabel;
}

mitk::Label* mitk::MultiLabelSegmentation::AddLabel(const std::string& name, const mitk::Color& color, GroupIndexType groupID)
{
  mitk::Label::Pointer newLabel = mitk::Label::New();
  newLabel->SetName(name);
  newLabel->SetColor(color);
  return AddLabel(newLabel,groupID,false);
}

void mitk::MultiLabelSegmentation::RenameLabel(LabelValueType pixelValue, const std::string& name, const mitk::Color& color)
{
  std::shared_lock<std::shared_mutex> guard(m_LabelNGroupMapsMutex);

  auto label = GetLabel(pixelValue);
  if (label.IsNull()) mitkThrow() << "Cannot rename label. Unknown label value provided. Unknown label value:" << pixelValue;

  label->SetName(name);
  label->SetColor(color);

  this->UpdateLookupTable(pixelValue);
  m_LookupTable->Modified();

  // change DICOM information of the label
  DICOMSegmentationPropertyHelper::SetDICOMSegmentProperties(label);
}

mitk::Label *mitk::MultiLabelSegmentation::GetActiveLabel()
{
  if (m_ActiveLabelValue == UNLABELED_VALUE) return nullptr;

  auto finding = m_LabelMap.find(m_ActiveLabelValue);
  return finding == m_LabelMap.end() ? nullptr : finding->second;
}

const mitk::Label* mitk::MultiLabelSegmentation::GetActiveLabel() const
{
  if (m_ActiveLabelValue == UNLABELED_VALUE) return nullptr;

  auto finding = m_LabelMap.find(m_ActiveLabelValue);
  return finding == m_LabelMap.end() ? nullptr : finding->second;
}

void mitk::MultiLabelSegmentation::UpdateCenterOfMass(LabelValueType pixelValue)
{
  if (4 == this->GetDimension())
  {
    AccessFixedDimensionByItk_1(this->GetGroupImage(this->GetGroupIndexOfLabel(pixelValue)), CalculateCenterOfMassProcessing, 4, pixelValue);
  }
  else
  {
    AccessByItk_1(this->GetGroupImage(this->GetGroupIndexOfLabel(pixelValue)), CalculateCenterOfMassProcessing, pixelValue);
  }
}

bool mitk::MultiLabelSegmentation::IsEmpty(LabelValueType pixelValue, TimeStepType t) const
{
  Image::ConstPointer image = this->GetGroupImage(this->GetGroupIndexOfLabel(pixelValue));
  image = SelectImageByTimeStep(image, t);

  size_t numPixels = 1;

  for (int i = 0; i < 3; ++i)
    numPixels *= static_cast<size_t>(image->GetDimension(i));

  ImagePixelReadAccessor<LabelValueType, 3> accessor(image);
  auto pixels = accessor.GetData();

  for (size_t i = 0; i < numPixels; ++i)
  {
    if (pixels[i] == pixelValue)
      return false;
  }

  return true;
}

bool mitk::MultiLabelSegmentation::IsEmpty(const Label* label, TimeStepType t) const
{
  if (label == nullptr)
    mitkThrow() << "Cannot check if label is empty. Label is null.";

  return this->IsEmpty(label->GetValue(), t);
}

void mitk::MultiLabelSegmentation::SetLookupTable(mitk::LookupTable* lut)
{
  m_LookupTable = lut;
  this->Modified();
}

void mitk::MultiLabelSegmentation::UpdateLookupTable(LabelValueType pixelValue)
{
  auto label = this->GetLabel(pixelValue);
  if (label.IsNull()) mitkThrow() << "Cannot update lookup table. Unknown label value provided. Unknown label value:" << pixelValue;

  const mitk::Color& color = label->GetColor();

  double rgba[4];
  m_LookupTable->GetTableValue(static_cast<int>(pixelValue), rgba);
  rgba[0] = color.GetRed();
  rgba[1] = color.GetGreen();
  rgba[2] = color.GetBlue();
  if (label->GetVisible())
    rgba[3] = label->GetOpacity();
  else
    rgba[3] = 0.0;
  m_LookupTable->SetTableValue(static_cast<int>(pixelValue), rgba);
}

unsigned int mitk::MultiLabelSegmentation::GetNumberOfLabels(unsigned int group) const
{
  if (group >= m_Groups.size()) mitkThrow() << "Cannot get number of labels in group. Group is unknown. Invalid index:" << group;
  return m_GroupToLabelMap[group].size();
}

unsigned int mitk::MultiLabelSegmentation::GetTotalNumberOfLabels() const
{
  return m_LabelMap.size();
}

void mitk::MultiLabelSegmentation::InitializeByLabeledImage(const Image* image)
{
  if (nullptr == image || image->IsEmpty() || !image->IsInitialized())
    mitkThrow() << "Invalid labeled image.";

  try
  {
    this->Initialize(image, true);
    auto groupImage = this->GetGroupImage(0);

    if (groupImage->GetDimension() == 3)
    {
      AccessTwoImagesFixedDimensionByItk(groupImage, image, InitializeByLabeledImageProcessing, 3);
    }
    else if (groupImage->GetDimension() == 4)
    {
      AccessTwoImagesFixedDimensionByItk(groupImage, image, InitializeByLabeledImageProcessing, 4);
    }
    else
    {
      mitkThrow() << image->GetDimension() << "-dimensional label set images not yet supported";
    }
  }
  catch (Exception& e)
  {
    mitkReThrow(e) << "Could not initialize by provided labeled image.";
  }
  catch (...)
  {
    mitkThrow() << "Could not initialize by provided labeled image due to unknown error.";
  }
  this->Modified();
}

template <typename MultiLabelSegmentationType, typename ImageType>
void mitk::MultiLabelSegmentation::InitializeByLabeledImageProcessing(MultiLabelSegmentationType *labelSetImage, const ImageType *image)
{
  typedef itk::ImageRegionConstIteratorWithIndex<ImageType> SourceIteratorType;
  typedef itk::ImageRegionIterator<MultiLabelSegmentationType> TargetIteratorType;

  TargetIteratorType targetIter(labelSetImage, labelSetImage->GetRequestedRegion());
  targetIter.GoToBegin();

  SourceIteratorType sourceIter(image, image->GetRequestedRegion());
  sourceIter.GoToBegin();

  while (!sourceIter.IsAtEnd())
  {
    const auto originalSourceValue = sourceIter.Get();
    const auto sourceValue = static_cast<LabelValueType>(originalSourceValue);

    if (originalSourceValue > mitk::Label::MAX_LABEL_VALUE)
    {
      mitkThrow() << "Cannot initialize MultiLabelSegmentation by image. Image contains a pixel value that exceeds the label value range. Invalid pixel value:" << originalSourceValue;
    }

    targetIter.Set(sourceValue);

    if (MultiLabelSegmentation::UNLABELED_VALUE!=sourceValue && !this->ExistLabel(sourceValue))
    {
      if (this->GetTotalNumberOfLabels() >= mitk::Label::MAX_LABEL_VALUE)
      {
        mitkThrow() << "Cannot initialize MultiLabelSegmentation by image. Image contains to many labels.";
      }

      std::stringstream name;
      name << "object-" << sourceValue;

      double rgba[4];
      this->GetLookupTable()->GetTableValue(sourceValue, rgba);

      mitk::Color color;
      color.SetRed(rgba[0]);
      color.SetGreen(rgba[1]);
      color.SetBlue(rgba[2]);

      auto label = mitk::Label::New();
      label->SetName(name.str().c_str());
      label->SetColor(color);
      label->SetOpacity(rgba[3]);
      label->SetValue(sourceValue);

      this->AddLabel(label,0,false);
    }

    ++sourceIter;
    ++targetIter;
  }
}

itk::ModifiedTimeType mitk::MultiLabelSegmentation::GetMTime() const
{
  itk::ModifiedTimeType result = Superclass::GetMTime();

  for (const auto& groupImage : m_GroupContainer)
  {
    result = std::max(result, groupImage->GetMTime());
  }

  return result;
}

template <typename ImageType>
void mitk::MultiLabelSegmentation::CalculateCenterOfMassProcessing(ImageType *itkImage, LabelValueType pixelValue)
{
  if (ImageType::GetImageDimension() != 3)
  {
    return;
  }

  auto labelGeometryFilter = itk::LabelGeometryImageFilter<ImageType>::New();
  labelGeometryFilter->SetInput(itkImage);
  labelGeometryFilter->Update();
  auto centroid = labelGeometryFilter->GetCentroid(pixelValue);

  mitk::Point3D pos;
  pos[0] = centroid[0];
  pos[1] = centroid[1];
  pos[2] = centroid[2];

  auto label = this->GetLabel(pixelValue);
  if (label.IsNotNull())
  {
    label->SetCenterOfMassIndex(pos);
    this->GetSlicedGeometry()->IndexToWorld(pos, pos);
    label->SetCenterOfMassCoordinates(pos);
  }
}

template <typename ImageType>
void mitk::MultiLabelSegmentation::EraseLabelProcessing(ImageType *itkImage, LabelValueType pixelValue)
{
  typedef itk::ImageRegionIterator<ImageType> IteratorType;

  IteratorType iter(itkImage, itkImage->GetLargestPossibleRegion());
  iter.GoToBegin();

  while (!iter.IsAtEnd())
  {
    LabelValueType value = iter.Get();

    if (value == pixelValue)
    {
      iter.Set(0);
    }
    ++iter;
  }
}

void mitk::MultiLabelSegmentation::AddLabelToMap(LabelValueType labelValue, mitk::Label* label, GroupIndexType groupID)
{
  if (m_LabelMap.find(labelValue)!=m_LabelMap.end())
    mitkThrow() << "Segmentation is in an invalid state: Label value collision. A label was added with a LabelValue already in use. LabelValue: " << labelValue;

  if (!this->ExistGroup(groupID))
    mitkThrow() << "Cannot add label. Defined group is unknown. Invalid group index: " << groupID;

  m_LabelMap[labelValue] = label;
  m_LabelToGroupMap[labelValue] = groupID;
  auto groupFinding = std::find(m_GroupToLabelMap[groupID].begin(), m_GroupToLabelMap[groupID].end(), labelValue);
  if (groupFinding == m_GroupToLabelMap[groupID].end())
  {
    m_GroupToLabelMap[groupID].push_back(labelValue);
  }
}

void mitk::MultiLabelSegmentation::RegisterLabel(mitk::Label* label)
{
  if (nullptr == label) mitkThrow() << "Invalid call of RegisterLabel with a nullptr.";

  UpdateLookupTable(label->GetValue());
  m_LookupTable->Modified();

  auto command = itk::MemberCommand<MultiLabelSegmentation>::New();
  command->SetCallbackFunction(this, &MultiLabelSegmentation::OnLabelModified);
  m_LabelModEventGuardMap.emplace(label->GetValue(), ITKEventObserverGuard(label, itk::ModifiedEvent(), command));
}

void mitk::MultiLabelSegmentation::ReleaseLabel(Label* label)
{
  if (nullptr == label) mitkThrow() << "Invalid call of ReleaseLabel with a nullptr.";
  m_LabelModEventGuardMap.erase(label->GetValue());
}

void mitk::MultiLabelSegmentation::ApplyToLabels(const LabelValueVectorType& values, std::function<void(Label*)>&& lambda)
{
  auto labels = this->GetLabelsByValue(values);
  std::for_each(labels.begin(), labels.end(), lambda);
  this->InvokeEvent(LabelsChangedEvent(values));
}

void mitk::MultiLabelSegmentation::VisitLabels(const LabelValueVectorType& values, std::function<void(const Label*)>&& lambda) const
{
  auto labels = this->GetConstLabelsByValue(values);
  std::for_each(labels.begin(), labels.end(), lambda);
}


void mitk::MultiLabelSegmentation::OnLabelModified(const Object* sender, const itk::EventObject&)
{
  auto label = dynamic_cast<const Label*>(sender);
  if (nullptr == label)
    mitkThrow() << "LabelSet is in wrong state. LabelModified event is not send by a label instance.";

  Superclass::Modified();
  this->InvokeEvent(LabelModifiedEvent(label->GetValue()));
}

bool mitk::MultiLabelSegmentation::ExistLabel(LabelValueType value) const
{
  auto finding = m_LabelMap.find(value);
  return m_LabelMap.end() != finding;
}

bool mitk::MultiLabelSegmentation::ExistLabel(LabelValueType value, GroupIndexType groupIndex) const
{
  auto finding = m_LabelToGroupMap.find(value);
  if (m_LabelToGroupMap.end() != finding)
  {
    return finding->second == groupIndex;
  }
  return false;
}

bool mitk::MultiLabelSegmentation::ExistGroup(GroupIndexType index) const
{
  return index < m_GroupContainer.size();
}

mitk::MultiLabelSegmentation::GroupIndexType mitk::MultiLabelSegmentation::GetGroupIndexOfLabel(LabelValueType value) const
{
  auto finding = m_LabelToGroupMap.find(value);
  if (m_LabelToGroupMap.end() == finding)
  {
    mitkThrow()<< "Cannot deduce group index. Passed label value does not exist. Value: "<< value;
  }
  return finding->second;
}


mitk::Label::ConstPointer mitk::MultiLabelSegmentation::GetLabel(LabelValueType value) const
{
  auto finding = m_LabelMap.find(value);
  if (m_LabelMap.end() != finding)
  {
    return finding->second;
  }
  return nullptr;
};

mitk::Label::Pointer mitk::MultiLabelSegmentation::GetLabel(LabelValueType value)
{
  auto finding = m_LabelMap.find(value);
  if (m_LabelMap.end() != finding)
  {
    return finding->second;
  }
  return nullptr;
};

bool mitk::MultiLabelSegmentation::IsLabelLocked(LabelValueType value) const
{
  if (value == UNLABELED_VALUE)
  {
    return m_UnlabeledLabelLock;
  }

  const auto label = this->GetLabel(value);
  return label->GetLocked();
}

const mitk::MultiLabelSegmentation::ConstLabelVectorType mitk::MultiLabelSegmentation::GetLabels() const
{
  ConstLabelVectorType result;
  for (auto [value, label] : m_LabelMap)
  {
    (void)value; // Prevent unused variable error in older compilers
    result.emplace_back(label);
  }
  return result;
}

const mitk::MultiLabelSegmentation::LabelVectorType mitk::MultiLabelSegmentation::GetLabels()
{
  LabelVectorType result;
  for (auto [value, label] : m_LabelMap)
  {
    (void)value; // Prevent unused variable error in older compilers
    result.emplace_back(label);
  }
  return result;
}

const mitk::MultiLabelSegmentation::LabelVectorType mitk::MultiLabelSegmentation::GetLabelsByValue(const LabelValueVectorType& labelValues, bool ignoreMissing)
{
  LabelVectorType result;
  for (const auto& labelValue : labelValues)
  {
    Label::Pointer label = this->GetLabel(labelValue);

    if (label.IsNotNull())
    {
      result.emplace_back(label);
    }
    else if (!ignoreMissing) mitkThrow() << "Error cannot get labels by Value. At least one passed value is unknown. Unknown value: " << labelValue;
  }
  return result;
}

const mitk::MultiLabelSegmentation::ConstLabelVectorType mitk::MultiLabelSegmentation::GetConstLabelsByValue(const LabelValueVectorType& labelValues, bool ignoreMissing) const
{
  ConstLabelVectorType result;
  for (const auto& labelValue : labelValues)
  {
    Label::ConstPointer label = this->GetLabel(labelValue);

    if (label.IsNotNull())
    {
      result.emplace_back(label);
    }
    else if (!ignoreMissing) mitkThrow() << "Error cannot get labels by Value. At least one passed value is unknown. Unknown value: " << labelValue;
  }
  return result;
}

const mitk::MultiLabelSegmentation::LabelValueVectorType mitk::MultiLabelSegmentation::GetLabelValuesByGroup(GroupIndexType index) const
{
  if (!this->ExistGroup(index))
    mitkThrow() << "Cannot get labels of an invalid group. Invalid group index: " << index;

  return m_GroupToLabelMap[index];
}

const mitk::MultiLabelSegmentation::LabelValueVectorType mitk::MultiLabelSegmentation::GetLabelValuesByName(GroupIndexType index, const std::string_view name) const
{
  LabelValueVectorType result;

  auto searchName = [&result, name](const Label* l) { if(l->GetName() == name) result.push_back(l->GetValue()); };

  this->VisitLabels(this->GetLabelValuesByGroup(index), searchName);

  return result;
}

std::vector<std::string> mitk::MultiLabelSegmentation::GetLabelClassNames() const
{
  std::set<std::string> names;
  auto searchName = [&names](const Label* l) { names.emplace(l->GetName()); };
  this->VisitLabels(this->GetAllLabelValues(), searchName);

  return std::vector<std::string>(names.begin(), names.end());
}

std::vector<std::string> mitk::MultiLabelSegmentation::GetLabelClassNamesByGroup(GroupIndexType index) const
{
  std::set<std::string> names;
  auto searchName = [&names](const Label* l) { names.emplace(l->GetName()); };
  this->VisitLabels(this->GetLabelValuesByGroup(index), searchName);

  return std::vector<std::string>(names.begin(), names.end());
}

void mitk::MultiLabelSegmentation::SetAllLabelsVisible(bool visible)
{
  auto setVisibility = [visible,this](Label* l)
    {
      l->SetVisible(visible);
      this->UpdateLookupTable(l->GetValue());
    };

  this->ApplyToLabels(this->GetAllLabelValues(), setVisibility);
  this->m_LookupTable->Modified();
}

void mitk::MultiLabelSegmentation::SetAllLabelsVisibleByGroup(GroupIndexType group, bool visible)
{
  auto setVisibility = [visible, this](Label* l)
    {
      l->SetVisible(visible);
      this->UpdateLookupTable(l->GetValue());
    };

  this->ApplyToLabels(this->GetLabelValuesByGroup(group), setVisibility);
  this->m_LookupTable->Modified();
}

void mitk::MultiLabelSegmentation::SetAllLabelsVisibleByName(GroupIndexType group, const std::string_view name, bool visible)
{
  auto setVisibility = [visible, this](Label* l)
    {
      l->SetVisible(visible);
      this->UpdateLookupTable(l->GetValue());
    };

  this->ApplyToLabels(this->GetLabelValuesByName(group, name), setVisibility);
  this->m_LookupTable->Modified();
}

void mitk::MultiLabelSegmentation::SetAllLabelsLocked(bool locked)
{
  auto setLock = [locked](Label* l) { l->SetLocked(locked); };

  this->ApplyToLabels(this->GetAllLabelValues(), setLock);
}

void mitk::MultiLabelSegmentation::SetAllLabelsLockedByGroup(GroupIndexType group, bool locked)
{
  auto setLock = [locked](Label* l) { l->SetLocked(locked); };

  this->ApplyToLabels(this->GetLabelValuesByGroup(group), setLock);
}

void mitk::MultiLabelSegmentation::SetAllLabelsLockedByName(GroupIndexType group, const std::string_view name, bool locked)
{
  auto setLock = [locked](Label* l) { l->SetLocked(locked); };

  this->ApplyToLabels(this->GetLabelValuesByName(group, name), setLock);
}

bool mitk::Equal(const mitk::MultiLabelSegmentation &leftHandSide,
                 const mitk::MultiLabelSegmentation &rightHandSide,
                 ScalarType eps,
                 bool verbose)
{
  bool returnValue = true;

  /* MultiLabelSegmentation members */

  MITK_INFO(verbose) << "--- MultiLabelSegmentation Equal ---";

  // m_LookupTable;
  const mitk::LookupTable* lhsLUT = leftHandSide.GetLookupTable();
  const mitk::LookupTable* rhsLUT = rightHandSide.GetLookupTable();

  returnValue = *lhsLUT == *rhsLUT;
  if (!returnValue)
  {
    MITK_INFO(verbose) << "Lookup tables not equal.";
    return returnValue;
    ;
  }

  // number groups
  returnValue = leftHandSide.GetNumberOfGroups() == rightHandSide.GetNumberOfGroups();
  if (!returnValue)
  {
    MITK_INFO(verbose) << "Number of groups not equal.";
    return false;
  }

  // total number labels
  returnValue = leftHandSide.GetTotalNumberOfLabels() == rightHandSide.GetTotalNumberOfLabels();
  if (!returnValue)
  {
    MITK_INFO(verbose) << "Total number of labels not equal.";
    return false;
  }

  // active group
  returnValue = leftHandSide.GetActiveLayer() == rightHandSide.GetActiveLayer();
  if (!returnValue)
  {
    MITK_INFO(verbose) << "Active group not equal.";
    return false;
  }

  if (leftHandSide.GetTotalNumberOfLabels() != rightHandSide.GetTotalNumberOfLabels())
  {
    MITK_INFO(verbose) << "Number of labels are not equal.";
    return false;
  }

  for (unsigned int groupIndex = 0; groupIndex < leftHandSide.GetNumberOfGroups(); groupIndex++)
  {
    if (4 == leftHandSide.GetDimension())
    {
      MITK_INFO(verbose) << "Can not compare image data for 4D images - skipping check.";
    }
    else
    {
      // group image data
      returnValue =
        mitk::Equal(*leftHandSide.GetGroupImage(groupIndex), *rightHandSide.GetGroupImage(groupIndex), eps, verbose);
      if (!returnValue)
      {
        MITK_INFO(verbose) << "Group image data not equal.";
        return false;
      }
    }

    // label data
    auto leftLabelsInGroup = leftHandSide.GetLabelValuesByGroup(groupIndex);
    auto rightLabelsInGroup = rightHandSide.GetLabelValuesByGroup(groupIndex);

    if (leftLabelsInGroup.size()!=rightLabelsInGroup.size())
    {
      MITK_INFO(verbose) << "Number of group labels is not equal. Invalid group:" <<groupIndex;
      return false;
    }

    for (ConstLabelVector::size_type index = 0; index < leftLabelsInGroup.size(); ++index)
    {
      if (!mitk::Equal(*(leftHandSide.GetLabel(leftLabelsInGroup[index])), *(rightHandSide.GetLabel(rightLabelsInGroup[index])),eps,verbose))
      {
        MITK_INFO(verbose) << "At least one label in group is not equal. Invalid group:" << groupIndex;
        return false;
      }
    }
  }

  return returnValue;
}

bool mitk::Equal(const mitk::MultiLabelSegmentation::ConstLabelVectorType& leftHandSide,
  const mitk::MultiLabelSegmentation::ConstLabelVectorType& rightHandSide, ScalarType eps, bool verbose)
{
  bool returnValue = true;

  // container size;
  returnValue = leftHandSide.size() == rightHandSide.size();
  if (!returnValue)
  {
    MITK_INFO(verbose) << "Number of labels not equal.";
    return returnValue;
    ;
  }

  // m_LabelContainer;
  auto lhsit = leftHandSide.begin();
  auto rhsit = rightHandSide.begin();
  for (; lhsit != leftHandSide.end(); ++lhsit, ++rhsit)
  {
    returnValue = mitk::Equal(**rhsit, **lhsit,eps,verbose);
    if (!returnValue)
    {
      MITK_INFO(verbose) << "Label in label container not equal.";
      return returnValue;
      ;
    }
  }

  return returnValue;
}

bool mitk::Equal(const mitk::MultiLabelSegmentation::LabelValueVectorType& leftHandSide,
  const mitk::MultiLabelSegmentation::LabelValueVectorType& rightHandSide,
  bool orderIsRelevant)
{
  if (orderIsRelevant)
  {
    return leftHandSide == rightHandSide;
  }
  if (leftHandSide.size() == rightHandSide.size())
  {
    // lambda to compare node pointer inside both lists
    return std::is_permutation(leftHandSide.begin(), leftHandSide.end(), rightHandSide.begin());
  }

  return false;
}


/**Helper function to convert a vector of labels into a label map
 * @pre every label in the vector has a unique value.*/
using ConstLabelMapType = std::map<mitk::MultiLabelSegmentation::LabelValueType, mitk::Label::ConstPointer>;
ConstLabelMapType ConvertLabelVectorToMap(const mitk::ConstLabelVector& labelV)
{
  ConstLabelMapType result;
  for (auto label : labelV)
  {
    const auto value = label->GetValue();
    auto finding = result.find(value);
    if (finding != result.end()) mitkThrow() << "Operation failed. Cannot convert label vector into label map, because at least one label value is not unique. Violating label value: " << value;
    result.insert(std::make_pair(value, label));
  }

  return result;
}


/** Functor class that implements the label transfer and is used in conjunction with the itk::BinaryFunctorImageFilter.
* For details regarding the usage of the filter and the functor patterns, please see info of itk::BinaryFunctorImageFilter.
*/
template <class TDestinationPixel, class TSourcePixel, class TOutputpixel>
class LabelTransferFunctor
{

public:
  LabelTransferFunctor() {};

  LabelTransferFunctor(const ConstLabelMapType& destinationLabels, mitk::Label::PixelType sourceBackground,
    mitk::Label::PixelType destinationBackground, bool destinationBackgroundLocked,
    mitk::Label::PixelType sourceLabel, mitk::Label::PixelType newDestinationLabel, mitk::MultiLabelSegmentation::MergeStyle mergeStyle,
    mitk::MultiLabelSegmentation::OverwriteStyle overwriteStyle) :
    m_DestinationLabels(destinationLabels), m_SourceBackground(sourceBackground),
    m_DestinationBackground(destinationBackground), m_DestinationBackgroundLocked(destinationBackgroundLocked),
    m_SourceLabel(sourceLabel), m_NewDestinationLabel(newDestinationLabel), m_MergeStyle(mergeStyle), m_OverwriteStyle(overwriteStyle)
  {
  };

  ~LabelTransferFunctor() {};

  bool operator!=(const LabelTransferFunctor& other)const
  {
    return !(*this == other);
  }
  bool operator==(const LabelTransferFunctor& other) const
  {
    return this->m_SourceBackground == other.m_SourceBackground &&
      this->m_DestinationBackground == other.m_DestinationBackground &&
      this->m_DestinationBackgroundLocked == other.m_DestinationBackgroundLocked &&
      this->m_SourceLabel == other.m_SourceLabel &&
      this->m_NewDestinationLabel == other.m_NewDestinationLabel &&
      this->m_MergeStyle == other.m_MergeStyle &&
      this->m_OverwriteStyle == other.m_OverwriteStyle &&
      this->m_DestinationLabels == other.m_DestinationLabels;
  }

  LabelTransferFunctor& operator=(const LabelTransferFunctor& other)
  {
    this->m_DestinationLabels = other.m_DestinationLabels;
    this->m_SourceBackground = other.m_SourceBackground;
    this->m_DestinationBackground = other.m_DestinationBackground;
    this->m_DestinationBackgroundLocked = other.m_DestinationBackgroundLocked;
    this->m_SourceLabel = other.m_SourceLabel;
    this->m_NewDestinationLabel = other.m_NewDestinationLabel;
    this->m_MergeStyle = other.m_MergeStyle;
    this->m_OverwriteStyle = other.m_OverwriteStyle;

    return *this;
  }

  inline TOutputpixel operator()(const TDestinationPixel& existingDestinationValue, const TSourcePixel& existingSourceValue)
  {
    if (existingSourceValue == this->m_SourceLabel)
    {
      if (mitk::MultiLabelSegmentation::OverwriteStyle::IgnoreLocks == this->m_OverwriteStyle)
      {
        return this->m_NewDestinationLabel;
      }
      else
      {
        if (existingDestinationValue == m_DestinationBackground)
        {
          if (!m_DestinationBackgroundLocked)
          {
            return this->m_NewDestinationLabel;
          }
        }
        else
        {
          auto labelFinding = this->m_DestinationLabels.find(existingDestinationValue);
          if (labelFinding==this->m_DestinationLabels.end() || !labelFinding->second->GetLocked())
          {
            return this->m_NewDestinationLabel;
          }
        }
      }
    }
    else if (mitk::MultiLabelSegmentation::MergeStyle::Replace == this->m_MergeStyle
      && existingSourceValue == this->m_SourceBackground
      && existingDestinationValue == this->m_NewDestinationLabel
      && (mitk::MultiLabelSegmentation::OverwriteStyle::IgnoreLocks == this->m_OverwriteStyle
          || !this->m_DestinationBackgroundLocked))
    {
      return this->m_DestinationBackground;
    }

    return existingDestinationValue;
  }

private:
  ConstLabelMapType m_DestinationLabels;
  mitk::Label::PixelType m_SourceBackground = 0;
  mitk::Label::PixelType m_DestinationBackground = 0;
  bool m_DestinationBackgroundLocked = false;
  mitk::Label::PixelType m_SourceLabel = 1;
  mitk::Label::PixelType m_NewDestinationLabel = 1;
  mitk::MultiLabelSegmentation::MergeStyle m_MergeStyle = mitk::MultiLabelSegmentation::MergeStyle::Replace;
  mitk::MultiLabelSegmentation::OverwriteStyle m_OverwriteStyle = mitk::MultiLabelSegmentation::OverwriteStyle::RegardLocks;
};

/**Helper function used by TransferLabelContentAtTimeStep to allow the templating over different image dimensions in conjunction of AccessFixedPixelTypeByItk_n.*/
template<unsigned int VImageDimension>
void TransferLabelContentAtTimeStepHelper(const itk::Image<mitk::Label::PixelType, VImageDimension>* itkSourceImage, mitk::Image* destinationImage,
  const mitk::ConstLabelVector& destinationLabels, mitk::Label::PixelType sourceBackground, mitk::Label::PixelType destinationBackground,
  bool destinationBackgroundLocked, mitk::Label::PixelType sourceLabel, mitk::Label::PixelType newDestinationLabel, mitk::MultiLabelSegmentation::MergeStyle mergeStyle, mitk::MultiLabelSegmentation::OverwriteStyle overwriteStyle)
{
  typedef itk::Image<mitk::Label::PixelType, VImageDimension> ContentImageType;
  typename ContentImageType::Pointer itkDestinationImage;
  mitk::CastToItkImage(destinationImage, itkDestinationImage);

  auto sourceRegion = itkSourceImage->GetLargestPossibleRegion();
  auto relevantRegion = itkDestinationImage->GetLargestPossibleRegion();
  bool overlapping = relevantRegion.Crop(sourceRegion);

  if (!overlapping)
  {
    mitkThrow() << "Invalid call of TransferLabelContentAtTimeStep; sourceImage and destinationImage seem to have no overlapping image region.";
  }

  typedef LabelTransferFunctor <mitk::Label::PixelType, mitk::Label::PixelType, mitk::Label::PixelType> LabelTransferFunctorType;
  typedef itk::BinaryFunctorImageFilter<ContentImageType, ContentImageType, ContentImageType, LabelTransferFunctorType> FilterType;

  LabelTransferFunctorType transferFunctor(ConvertLabelVectorToMap(destinationLabels), sourceBackground, destinationBackground,
    destinationBackgroundLocked, sourceLabel, newDestinationLabel, mergeStyle, overwriteStyle);

  auto transferFilter = FilterType::New();

  transferFilter->SetFunctor(transferFunctor);
  transferFilter->InPlaceOn();
  transferFilter->SetInput1(itkDestinationImage);
  transferFilter->SetInput2(itkSourceImage);
  transferFilter->GetOutput()->SetRequestedRegion(relevantRegion);

  transferFilter->Update();
}

void mitk::TransferLabelContentAtTimeStep(
  const Image* sourceImage, Image* destinationImage, const mitk::ConstLabelVector& destinationLabels, const TimeStepType timeStep, mitk::Label::PixelType sourceBackground,
  mitk::Label::PixelType destinationBackground, bool destinationBackgroundLocked, LabelValueMappingVector labelMapping,
  MultiLabelSegmentation::MergeStyle mergeStyle, MultiLabelSegmentation::OverwriteStyle overwriteStlye)
{
  if (nullptr == sourceImage)
  {
    mitkThrow() << "Invalid call of TransferLabelContentAtTimeStep; sourceImage must not be null.";
  }
  if (nullptr == destinationImage)
  {
    mitkThrow() << "Invalid call of TransferLabelContentAtTimeStep; destinationImage must not be null.";
  }

  if (sourceImage == destinationImage && labelMapping.size() > 1)
  {
    MITK_DEBUG << "Warning. Using TransferLabelContentAtTimeStep or TransferLabelContent with equal source and destination and more then on label to transfer, can lead to wrong results. Please see documentation and verify that the usage is OK.";
  }

  Image::ConstPointer sourceImageAtTimeStep = SelectImageByTimeStep(sourceImage, timeStep);
  Image::Pointer destinationImageAtTimeStep = SelectImageByTimeStep(destinationImage, timeStep);

  if (nullptr == sourceImageAtTimeStep)
  {
    mitkThrow() << "Invalid call of TransferLabelContentAtTimeStep; sourceImage does not have the requested time step: " << timeStep;
  }

  if (nullptr == destinationImageAtTimeStep)
  {
    mitkThrow() << "Invalid call of TransferLabelContentAtTimeStep; destinationImage does not have the requested time step: " << timeStep;
  }

  if (!Equal(*(sourceImageAtTimeStep->GetGeometry()), *(destinationImageAtTimeStep->GetGeometry()), mitk::NODE_PREDICATE_GEOMETRY_DEFAULT_CHECK_COORDINATE_PRECISION, mitk::NODE_PREDICATE_GEOMETRY_DEFAULT_CHECK_DIRECTION_PRECISION))
  {
    if (IsSubGeometry(*(sourceImageAtTimeStep->GetGeometry()), *(destinationImageAtTimeStep->GetGeometry()), mitk::NODE_PREDICATE_GEOMETRY_DEFAULT_CHECK_COORDINATE_PRECISION, mitk::NODE_PREDICATE_GEOMETRY_DEFAULT_CHECK_DIRECTION_PRECISION, true))
    {
      //we have to pad the source image
      //because ImageToImageFilters always check for origin matching even if
      //the requested output region is fitting :(
      auto padFilter = mitk::PadImageFilter::New();
      padFilter->SetInput(0, sourceImageAtTimeStep);
      padFilter->SetInput(1, destinationImageAtTimeStep);
      padFilter->SetPadConstant(Label::UNLABELED_VALUE);
      padFilter->SetBinaryFilter(false);

      padFilter->Update();

      sourceImageAtTimeStep = padFilter->GetOutput();
    }
    else
    {
      mitkThrow() << "Invalid call of TransferLabelContentAtTimeStep; source image has neither the same geometry than destination image nor has the source image a sub geometry.";
    }
  }

  auto destLabelMap = ConvertLabelVectorToMap(destinationLabels);
  for (const auto& [sourceLabel, newDestinationLabel] : labelMapping)
  {
    if (MultiLabelSegmentation::UNLABELED_VALUE!=newDestinationLabel && destLabelMap.end() == destLabelMap.find(newDestinationLabel))
    {
      mitkThrow() << "Invalid call of TransferLabelContentAtTimeStep. Defined destination label does not exist in destinationImage. newDestinationLabel: " << newDestinationLabel;
    }

    AccessFixedPixelTypeByItk_n(sourceImageAtTimeStep, TransferLabelContentAtTimeStepHelper, (Label::PixelType), (destinationImageAtTimeStep, destinationLabels, sourceBackground, destinationBackground, destinationBackgroundLocked, sourceLabel, newDestinationLabel, mergeStyle, overwriteStlye));
  }
  destinationImage->Modified();
}

void mitk::TransferLabelContent(
  const Image* sourceImage, Image* destinationImage, const mitk::ConstLabelVector& destinationLabels, mitk::Label::PixelType sourceBackground,
  mitk::Label::PixelType destinationBackground, bool destinationBackgroundLocked, LabelValueMappingVector labelMapping,
  MultiLabelSegmentation::MergeStyle mergeStyle, MultiLabelSegmentation::OverwriteStyle overwriteStlye)
{
  if (nullptr == sourceImage)
  {
    mitkThrow() << "Invalid call of TransferLabelContent; sourceImage must not be null.";
  }
  if (nullptr == destinationImage)
  {
    mitkThrow() << "Invalid call of TransferLabelContent; destinationImage must not be null.";
  }

  const auto sourceTimeStepCount = sourceImage->GetTimeGeometry()->CountTimeSteps();
  if (sourceTimeStepCount != destinationImage->GetTimeGeometry()->CountTimeSteps())
  {
    mitkThrow() << "Invalid call of TransferLabelContent; mismatch between images in number of time steps.";
  }

  for (mitk::TimeStepType i = 0; i < sourceTimeStepCount; ++i)
  {
    TransferLabelContentAtTimeStep(sourceImage, destinationImage, destinationLabels, i, sourceBackground,
      destinationBackground, destinationBackgroundLocked, labelMapping, mergeStyle, overwriteStlye);
  }
}

void mitk::TransferLabelContentAtTimeStep(
  const MultiLabelSegmentation* sourceImage, MultiLabelSegmentation* destinationImage, const TimeStepType timeStep,
  LabelValueMappingVector labelMapping,
  MultiLabelSegmentation::MergeStyle mergeStyle, MultiLabelSegmentation::OverwriteStyle overwriteStlye)
{
  if (nullptr == sourceImage)
  {
    mitkThrow() << "Invalid call of TransferLabelContentAtTimeStep; sourceImage must not be null.";
  }
  if (nullptr == destinationImage)
  {
    mitkThrow() << "Invalid call of TransferLabelContentAtTimeStep; destinationImage must not be null.";
  }

  //split all label mappings by source group id
  auto groupLabelValueMappingSplits = LabelSetImageHelper::SplitLabelValueMappingBySourceAndTargetGroup(sourceImage, destinationImage, labelMapping);

  //start transfer by iterating over relevant source groups
  for (const auto& [sourceGroupID, destGroupLabelMapping] : groupLabelValueMappingSplits)
  {
    const auto sourceGroupImage = sourceImage->GetGroupImage(sourceGroupID);

    for (const auto& [destGroupID, relevantLabelMapping] : destGroupLabelMapping)
    {
      auto destGroupImage = destinationImage->GetGroupImage(destGroupID);
      auto destinationLabels = destinationImage->GetConstLabelsByValue(destinationImage->GetLabelValuesByGroup(destGroupID));
      TransferLabelContentAtTimeStep(sourceGroupImage, destGroupImage, destinationLabels, timeStep, MultiLabelSegmentation::UNLABELED_VALUE, MultiLabelSegmentation::UNLABELED_VALUE, destinationImage->GetUnlabeledLabelLock(),
        relevantLabelMapping, mergeStyle, overwriteStlye);
    }
  }
}

void mitk::TransferLabelContent(
  const MultiLabelSegmentation* sourceImage, MultiLabelSegmentation* destinationImage,
  LabelValueMappingVector labelMapping,
  MultiLabelSegmentation::MergeStyle mergeStyle, MultiLabelSegmentation::OverwriteStyle overwriteStlye)
{
  if (nullptr == sourceImage)
  {
    mitkThrow() << "Invalid call of TransferLabelContent; sourceImage must not be null.";
  }
  if (nullptr == destinationImage)
  {
    mitkThrow() << "Invalid call of TransferLabelContent; destinationImage must not be null.";
  }

  const auto sourceTimeStepCount = sourceImage->GetTimeGeometry()->CountTimeSteps();
  if (sourceTimeStepCount != destinationImage->GetTimeGeometry()->CountTimeSteps())
  {
    mitkThrow() << "Invalid call of TransferLabelContent; images have no equal number of time steps.";
  }

  for (mitk::TimeStepType i = 0; i < sourceTimeStepCount; ++i)
  {
    TransferLabelContentAtTimeStep(sourceImage, destinationImage, i, labelMapping, mergeStyle, overwriteStlye);
  }
}

