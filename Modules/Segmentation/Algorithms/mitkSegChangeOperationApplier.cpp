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


namespace
{
  void ApplyGroupModification(mitk::SegGroupModifyOperation* modOperation, mitk::MultiLabelSegmentation* segmentation)
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

  void ApplyGroupInsert(mitk::SegGroupInsertOperation* insertOperation, mitk::MultiLabelSegmentation* segmentation)
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

  void ApplyGroupRemove(mitk::SegGroupRemoveOperation* removeOperation, mitk::MultiLabelSegmentation* segmentation)
  {
    for (auto groupID : removeOperation->GetGroupIDs())
    {
      segmentation->RemoveGroup(groupID);
    }
  }

  void ApplySliceOperation(mitk::SegSliceOperation* sliceOperation, mitk::MultiLabelSegmentation* segmentation)
  {
    auto relevantGroupImage = segmentation->GetGroupImage(sliceOperation->GetGroupID());
    mitk::SegTool2D::WriteSliceToVolume(relevantGroupImage, sliceOperation->GetSlicePlaneGeometry(), sliceOperation->GetSlice(), sliceOperation->GetTimeStep());
    mitk::SegTool2D::UpdateAllSurfaceInterpolations(segmentation, sliceOperation->GetTimeStep(), sliceOperation->GetSlicePlaneGeometry(), true);
  }

  void ApplyPropertyModification(mitk::SegLabelPropModifyOperation* labelPropModOperation, mitk::MultiLabelSegmentation* segmentation)
  {
    segmentation->ReplaceLabels(labelPropModOperation->GetModifiedLabels());
  }
}


mitk::SegChangeOperationApplier::SegChangeOperationApplier()
{
}

mitk::SegChangeOperationApplier::~SegChangeOperationApplier()
{
}

void mitk::SegChangeOperationApplier::ExecuteOperation(Operation *operation)
{
  auto segOperation = dynamic_cast<SegChangeOperationBase*>(operation);

  // check if the operation is valid
  if (nullptr == segOperation || !segOperation->IsValid())
    return;

  auto segmentation = segOperation->GetSegmentation();

  if (auto modOperation = dynamic_cast<SegGroupModifyOperation*>(operation); nullptr != modOperation)
  {
    ApplyGroupModification(modOperation, segmentation);
  }
  else if (auto insertOperation = dynamic_cast<SegGroupInsertOperation*>(operation); nullptr != insertOperation)
  {
    ApplyGroupInsert(insertOperation, segmentation);
  }
  else if (auto removeOperation = dynamic_cast<SegGroupRemoveOperation*>(operation); nullptr != removeOperation)
  {
    ApplyGroupRemove(removeOperation, segmentation);
  }
  else if (auto sliceOperation = dynamic_cast<SegSliceOperation*>(operation); nullptr != sliceOperation)
  {
    ApplySliceOperation(sliceOperation, segmentation);
  }
  else if (auto labelPropModOperation = dynamic_cast<SegLabelPropModifyOperation*>(operation); nullptr != labelPropModOperation)
  {
    ApplyPropertyModification(labelPropModOperation, segmentation);
  }
}

mitk::SegChangeOperationApplier *mitk::SegChangeOperationApplier::GetInstance()
{
  static SegChangeOperationApplier s_Instance;
  return &s_Instance;
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
  delete m_UndoOperation;
};

void mitk::SegGroupInsertUndoRedoHelper::RegisterUndoRedoOperationEvent(const std::string& description)
{
  if (nullptr == m_UndoOperation)
    mitkThrow() << "Invalid usage of SegGroupInsertUndoRedoHelper. You can only call RegisterUndoRedoOperationEvent once.";

  UndoStackItem::IncCurrGroupEventId();
  UndoStackItem::IncCurrObjectEventId();

  auto redoOperation =
    SegGroupInsertOperation::CreateFromSegmentation(m_Segmentation, m_RelevantGroupIDs, m_NoLabels, m_NoGroupImages);

  OperationEvent* undoStackItem =
    new OperationEvent(SegChangeOperationApplier::GetInstance(), redoOperation, m_UndoOperation, description);
  m_UndoOperation = nullptr; //Ownership is now at undoStackItem

  UndoController::GetCurrentUndoModel()->SetOperationEvent(undoStackItem);
}


mitk::SegGroupRemoveUndoRedoHelper::SegGroupRemoveUndoRedoHelper(MultiLabelSegmentation* segmentation,
  const GroupIndexSetType& relevantGroupIDs, bool noLabels, bool noGroupImages) :
  m_Segmentation(segmentation), m_RelevantGroupIDs(relevantGroupIDs), m_UndoOperation(nullptr)
{
  m_UndoOperation = SegGroupInsertOperation::CreateFromSegmentation(m_Segmentation, m_RelevantGroupIDs, noLabels, noGroupImages);
}

mitk::SegGroupRemoveUndoRedoHelper::~SegGroupRemoveUndoRedoHelper()
{
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
