/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkSegChangeOperationApplier.h"

#include <mitkSegLabelPropModifyOperation.h>
#include <mitkSegGroupModifyOperation.h>
#include <mitkSegGroupInsertOperation.h>
#include <mitkSegGroupRemoveOperation.h>
#include <mitkSegSliceOperation.h>

#include <mitkSegTool2D.h>
#include <mitkRenderingManager.h>

#include <mitkUndoController.h>

// VTK
#include <vtkSmartPointer.h>

mitk::SegChangeOperationApplier::SegChangeOperationApplier()
{
}

mitk::SegChangeOperationApplier::~SegChangeOperationApplier()
{
}

void mitk::SegChangeOperationApplier::ExecuteOperation(Operation *operation)
{
  auto* segOperation = dynamic_cast<SegChangeOperationBase*>(operation);
  auto* modOperation = dynamic_cast<SegGroupModifyOperation *>(operation);
  auto* insertOperation = dynamic_cast<SegGroupInsertOperation*>(operation);
  auto* removeOperation = dynamic_cast<SegGroupRemoveOperation*>(operation);
  auto* sliceOperation = dynamic_cast<SegSliceOperation*>(operation);
  auto* labelPropModOperation = dynamic_cast<SegLabelPropModifyOperation*>(operation);

  // check if the operation is valid
  if (nullptr == segOperation || !segOperation->IsValid())
    return;

  auto segmentation = segOperation->GetSegmentation();
  if (nullptr != modOperation)
  {
    bool modified = false;

    for (auto modifiedImageGroupID : modOperation->GetImageGroupIDs())
    {
      for (auto timeStep : modOperation->GetImageTimeSteps(modifiedImageGroupID))
      {
        auto modifiedImage = modOperation->GetModifiedGroupImage(modifiedImageGroupID, timeStep);
        segmentation->UpdateGroupImage(modifiedImageGroupID, modifiedImage, timeStep);
        modified = true;
      }
    }
    if (modified)
    {
      segmentation->Modified();
    }

    for (auto modifiedGroupID : modOperation->GetLabelGroupIDs())
    {
        segmentation->ReplaceGroupLabels(modifiedGroupID, modOperation->GetModifiedLabels(modifiedGroupID));
    }
    for (auto modifiedGroupID : modOperation->GetNameGroupIDs())
    {
      segmentation->SetGroupName(modifiedGroupID, modOperation->GetModifiedName(modifiedGroupID));
    }
  }
  else if (nullptr != insertOperation)
  {
    for (auto groupID : insertOperation->GetGroupIDs())
    {
      auto groupImage = insertOperation->GetGroupImage(groupID);
      if (groupImage.IsNull())
      {
        segmentation->InsertGroup(groupID, insertOperation->GetGroupLabels(groupID));
      }
      else
      {
        segmentation->InsertGroup(groupID, insertOperation->GetGroupImage(groupID), insertOperation->GetGroupLabels(groupID));
      }
    }
  }
  else if (nullptr != removeOperation)
  {
    for (auto groupID : removeOperation->GetGroupIDs())
    {
      segmentation->RemoveGroup(groupID);
    }
  }
  else if (nullptr != sliceOperation)
  {
    auto relevantGroupImage = segmentation->GetGroupImage(sliceOperation->GetGroupID());
    SegTool2D::WriteSliceToVolume(relevantGroupImage, sliceOperation->GetSlicePlaneGeometry(), sliceOperation->GetSlice(), sliceOperation->GetTimeStep());
    SegTool2D::UpdateAllSurfaceInterpolations(segmentation, sliceOperation->GetTimeStep(), sliceOperation->GetSlicePlaneGeometry(), true);
  }
  else if (nullptr != labelPropModOperation)
  {
    auto segmentation = labelPropModOperation->GetSegmentation();
    segmentation->ReplaceLabels(labelPropModOperation->GetModifiedLabels());
  }
}

mitk::SegChangeOperationApplier *mitk::SegChangeOperationApplier::GetInstance()
{
  static auto *s_Instance = new SegChangeOperationApplier();
  return s_Instance;
}


mitk::SegGroupModifyUndoRedoHelper::SegGroupModifyUndoRedoHelper(MultiLabelSegmentation* segmentation,
  const GroupIndexSetType& relevantGroupIDs, bool coverAllTimeSteps, TimeStepType timeStep,
  bool noLabels, bool noGroupImages, bool noNames) :
  m_Segmentation(segmentation), m_RelevantGroupIDs(relevantGroupIDs),
  m_CoverAllTimeSteps(coverAllTimeSteps), m_TimeStep(timeStep), m_NoLabels(noLabels), m_NoGroupImages(noGroupImages),
  m_NoNames(noNames), m_UndoOperation(nullptr)
{
  m_UndoOperation = SegGroupModifyOperation::CreatFromSegmentation(m_Segmentation, m_RelevantGroupIDs, m_CoverAllTimeSteps, m_TimeStep, m_NoLabels, m_NoGroupImages, m_NoNames);
}

mitk::SegGroupModifyUndoRedoHelper::~SegGroupModifyUndoRedoHelper()
{
  if (nullptr != m_UndoOperation)
    delete m_UndoOperation;
};

void mitk::SegGroupModifyUndoRedoHelper::RegisterUndoRedoOperationEvent(const std::string& description)
{
  if (nullptr == m_UndoOperation)
    mitkThrow() << "Invalid usage of SegGroupModifyUndoRedoHelper. You can only call RegisterUndoRedoOperationEvent once.";

  UndoStackItem::IncCurrGroupEventId();
  UndoStackItem::IncCurrObjectEventId();

  auto redoOperation =
    SegGroupModifyOperation::CreatFromSegmentation(m_Segmentation, m_RelevantGroupIDs, m_CoverAllTimeSteps, m_TimeStep,
      m_NoLabels, m_NoGroupImages, m_NoNames);

  OperationEvent* undoStackItem =
    new OperationEvent(SegChangeOperationApplier::GetInstance(), redoOperation, m_UndoOperation, description);
  m_UndoOperation = nullptr; //Ownership is now at undoStackItem

  UndoController::GetCurrentUndoModel()->SetOperationEvent(undoStackItem);
}


mitk::SegGroupInsertUndoRedoHelper::SegGroupInsertUndoRedoHelper(MultiLabelSegmentation* segmentation,
  const GroupIndexSetType& relevantGroupIDs, bool noLabels, bool noGroupImages) :
  m_Segmentation(segmentation), m_RelevantGroupIDs(relevantGroupIDs), m_NoLabels(noLabels), m_NoGroupImages(noGroupImages), m_UndoOperation(nullptr)
{
  m_UndoOperation = new SegGroupRemoveOperation(m_Segmentation, m_RelevantGroupIDs);
}

mitk::SegGroupInsertUndoRedoHelper::~SegGroupInsertUndoRedoHelper()
{
  if (nullptr != m_UndoOperation)
    delete m_UndoOperation;
};

void mitk::SegGroupInsertUndoRedoHelper::RegisterUndoRedoOperationEvent(const std::string& description)
{
  if (nullptr == m_UndoOperation)
    mitkThrow() << "Invalid usage of SegGroupInsertUndoRedoHelper. You can only call RegisterUndoRedoOperationEvent once.";

  UndoStackItem::IncCurrGroupEventId();
  UndoStackItem::IncCurrObjectEventId();

  auto redoOperation =
    SegGroupInsertOperation::CreatFromSegmentation(m_Segmentation, m_RelevantGroupIDs, m_NoLabels, m_NoGroupImages);

  OperationEvent* undoStackItem =
    new OperationEvent(SegChangeOperationApplier::GetInstance(), redoOperation, m_UndoOperation, description);
  m_UndoOperation = nullptr; //Ownership is now at undoStackItem

  UndoController::GetCurrentUndoModel()->SetOperationEvent(undoStackItem);
}


mitk::SegGroupRemoveUndoRedoHelper::SegGroupRemoveUndoRedoHelper(MultiLabelSegmentation* segmentation,
  const GroupIndexSetType& relevantGroupIDs, bool noLabels, bool noGroupImages) :
  m_Segmentation(segmentation), m_RelevantGroupIDs(relevantGroupIDs), m_UndoOperation(nullptr)
{
  m_UndoOperation = SegGroupInsertOperation::CreatFromSegmentation(m_Segmentation, m_RelevantGroupIDs, noLabels, noGroupImages);
}

mitk::SegGroupRemoveUndoRedoHelper::~SegGroupRemoveUndoRedoHelper()
{
  if (nullptr != m_UndoOperation)
    delete m_UndoOperation;
};

void mitk::SegGroupRemoveUndoRedoHelper::RegisterUndoRedoOperationEvent(const std::string& description)
{
  if (nullptr == m_UndoOperation)
    mitkThrow() << "Invalid usage of SegGroupRemoveUndoRedoHelper. You can only call RegisterUndoRedoOperationEvent once.";

  UndoStackItem::IncCurrGroupEventId();
  UndoStackItem::IncCurrObjectEventId();

  auto redoOperation = new SegGroupRemoveOperation(m_Segmentation, m_RelevantGroupIDs);

  OperationEvent* undoStackItem =
    new OperationEvent(SegChangeOperationApplier::GetInstance(), redoOperation, m_UndoOperation, description);
  m_UndoOperation = nullptr; //Ownership is now at undoStackItem

  UndoController::GetCurrentUndoModel()->SetOperationEvent(undoStackItem);
}

mitk::SegLabelPropModifyUndoRedoHelper::SegLabelPropModifyUndoRedoHelper(MultiLabelSegmentation* segmentation,
  const MultiLabelSegmentation::LabelValueVectorType& relevantLabels) :
  m_Segmentation(segmentation), m_RelevantLabels(relevantLabels), m_UndoOperation(nullptr)
{
  m_UndoOperation = SegLabelPropModifyOperation::CreatFromSegmentation(m_Segmentation, m_RelevantLabels);
}

mitk::SegLabelPropModifyUndoRedoHelper::~SegLabelPropModifyUndoRedoHelper()
{
  if (nullptr != m_UndoOperation)
    delete m_UndoOperation;
};

void mitk::SegLabelPropModifyUndoRedoHelper::RegisterUndoRedoOperationEvent(const std::string& description)
{
  if (nullptr == m_UndoOperation)
    mitkThrow() << "Invalid usage of SegLabelPropModifyUndoRedoHelper. You can only call RegisterUndoRedoOperationEvent once.";

  UndoStackItem::IncCurrGroupEventId();
  UndoStackItem::IncCurrObjectEventId();

  auto redoOperation = SegLabelPropModifyOperation::CreatFromSegmentation(m_Segmentation, m_RelevantLabels);

  OperationEvent* undoStackItem =
    new OperationEvent(SegChangeOperationApplier::GetInstance(), redoOperation, m_UndoOperation, description);
  m_UndoOperation = nullptr; //Ownership is now at undoStackItem

  UndoController::GetCurrentUndoModel()->SetOperationEvent(undoStackItem);
}
