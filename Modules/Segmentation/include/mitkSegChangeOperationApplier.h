/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSegChangeOperationApplier_h
#define mitkSegChangeOperationApplier_h

#include <mitkCommon.h>
#include <mitkOperationActor.h>
#include <MitkSegmentationExports.h>

namespace mitk
{
  /** \brief Executes all operations derived from SegChangeOperationBase.

    This singleton OperationActor dispatches to the appropriate handler
    based on the dynamic type of the operation: SegGroupModifyOperation,
    SegGroupInsertOperation, SegGroupRemoveOperation, SegSliceOperation,
    or SegLabelPropModifyOperation.

    \sa SegChangeOperationBase
    \sa SegGroupModifyOperation
    \sa SegGroupInsertOperation
    \sa SegGroupRemoveOperation
    \sa SegSliceOperation
    \sa SegLabelPropModifyOperation
  */
  class MITKSEGMENTATION_EXPORT SegChangeOperationApplier : public OperationActor
  {
  public:
    mitkClassMacroNoParent(SegChangeOperationApplier)

      /** \brief Return the singleton instance of the applier. */
      static SegChangeOperationApplier *GetInstance();

    /** \brief Execute a SegChangeOperationBase-derived operation on its target segmentation.
      \param op The operation to execute. Must be a valid SegChangeOperationBase subclass instance.
    */
    void ExecuteOperation(Operation *op) override;

  protected:
    SegChangeOperationApplier();

    ~SegChangeOperationApplier() override;
  };


  class SegChangeOperationBase;

  /** \brief RAII helper that captures the state of segmentation groups for undo/redo support.

    On construction, a snapshot of the specified groups is taken (the undo operation).
    After the modification is performed, call RegisterUndoRedoOperationEvent() to capture
    the redo state and register both operations on the undo stack.

    \sa SegGroupModifyOperation
    \sa SegChangeOperationApplier
  */
  class MITKSEGMENTATION_EXPORT SegGroupModifyUndoRedoHelper
  {
  public:

    using GroupIndexSetType = std::set<MultiLabelSegmentation::GroupIndexType>;

    /** \brief Capture the current state of the given groups as the undo operation.
      \param segmentation The segmentation to operate on.
      \param relevantGroupIDs The group indices to capture.
      \param coverAllTimeSteps If true, all time steps are captured; otherwise only the specified one.
      \param timeStep The time step to capture (only used if coverAllTimeSteps is false).
      \param noLabels If true, label information is not captured.
      \param noGroupImages If true, group image data is not captured.
      \param noNames If true, group names are not captured.
    */
    SegGroupModifyUndoRedoHelper(MultiLabelSegmentation* segmentation,
      const GroupIndexSetType& relevantGroupIDs, bool coverAllTimeSteps, TimeStepType timeStep = 0,
      bool noLabels = false, bool noGroupImages = false, bool noNames = false);

    ~SegGroupModifyUndoRedoHelper();

    /** \brief Capture the current (modified) state as the redo operation and register the undo/redo pair.
      \param description A human-readable description of the operation for the undo stack.
    */
    void RegisterUndoRedoOperationEvent(const std::string& description);

  protected:
    MultiLabelSegmentation::Pointer m_Segmentation;
    GroupIndexSetType m_RelevantGroupIDs;
    bool m_CoverAllTimeSteps;
    TimeStepType m_TimeStep;
    bool m_NoLabels;
    bool m_NoGroupImages;
    bool m_NoNames;

    SegChangeOperationBase* m_UndoOperation;
  };

  /** \brief RAII helper that captures state for undoing a group insert operation.

    On construction, a SegGroupRemoveOperation is prepared as the undo operation.
    After the insert is performed, call RegisterUndoRedoOperationEvent() to capture
    the redo (insert) state and register both on the undo stack.

    \sa SegGroupInsertOperation
    \sa SegChangeOperationApplier
  */
  class MITKSEGMENTATION_EXPORT SegGroupInsertUndoRedoHelper
  {
  public:

    using GroupIndexSetType = std::set<MultiLabelSegmentation::GroupIndexType>;

    /** \brief Prepare the undo operation (removal) for the groups that will be inserted.
      \param segmentation The segmentation to operate on.
      \param relevantGroupIDs The group indices that will be inserted.
      \param noLabels If true, label information is not captured in the redo operation.
      \param noGroupImages If true, group image data is not captured in the redo operation.
    */
    SegGroupInsertUndoRedoHelper(MultiLabelSegmentation* segmentation,
      const GroupIndexSetType& relevantGroupIDs, bool noLabels = false, bool noGroupImages = false);

    ~SegGroupInsertUndoRedoHelper();

    /** \brief Capture the current (inserted) state as the redo operation and register the undo/redo pair.
      \param description A human-readable description of the operation for the undo stack.
    */
    void RegisterUndoRedoOperationEvent(const std::string& description);

  protected:
    MultiLabelSegmentation::Pointer m_Segmentation;
    GroupIndexSetType m_RelevantGroupIDs;
    bool m_NoLabels;
    bool m_NoGroupImages;

    SegChangeOperationBase* m_UndoOperation;
  };

  /** \brief RAII helper that captures state for undoing a group remove operation.

    On construction, a SegGroupInsertOperation is prepared as the undo operation
    (capturing the groups that are about to be removed). After the removal is
    performed, call RegisterUndoRedoOperationEvent() to register the undo/redo pair.

    \sa SegGroupRemoveOperation
    \sa SegChangeOperationApplier
  */
  class MITKSEGMENTATION_EXPORT SegGroupRemoveUndoRedoHelper
  {
  public:

    using GroupIndexSetType = std::set<MultiLabelSegmentation::GroupIndexType>;

    /** \brief Capture the current state of the groups that will be removed (for undo).
      \param segmentation The segmentation to operate on.
      \param relevantGroupIDs The group indices that will be removed.
      \param noLabels If true, label information is not captured.
      \param noGroupImages If true, group image data is not captured.
    */
    SegGroupRemoveUndoRedoHelper(MultiLabelSegmentation* segmentation,
      const GroupIndexSetType& relevantGroupIDs, bool noLabels = false, bool noGroupImages = false);

    ~SegGroupRemoveUndoRedoHelper();

    /** \brief Register the undo (insert) and redo (remove) operations on the undo stack.
      \param description A human-readable description of the operation for the undo stack.
    */
    void RegisterUndoRedoOperationEvent(const std::string& description);

  protected:
    MultiLabelSegmentation::Pointer m_Segmentation;
    GroupIndexSetType m_RelevantGroupIDs;


    SegChangeOperationBase* m_UndoOperation;
  };

  /** \brief RAII helper that captures state for undoing label property modifications.

    On construction, the current label properties are captured as the undo operation.
    After the label properties are modified, call RegisterUndoRedoOperationEvent() to
    capture the new state and register both on the undo stack.

    \sa SegLabelPropModifyOperation
    \sa SegChangeOperationApplier
  */
  class MITKSEGMENTATION_EXPORT SegLabelPropModifyUndoRedoHelper
  {
  public:

    /** \brief Capture the current properties of the specified labels for undo.
      \param segmentation The segmentation to operate on.
      \param relevantLabels The label values whose properties will be modified.
    */
    SegLabelPropModifyUndoRedoHelper(MultiLabelSegmentation* segmentation,
      const MultiLabelSegmentation::LabelValueVectorType& relevantLabels);

    ~SegLabelPropModifyUndoRedoHelper();

    /** \brief Capture the modified label properties as the redo operation and register the pair.
      \param description A human-readable description of the operation for the undo stack.
    */
    void RegisterUndoRedoOperationEvent(const std::string& description);

  protected:
    MultiLabelSegmentation::Pointer m_Segmentation;
    MultiLabelSegmentation::LabelValueVectorType m_RelevantLabels;


    SegChangeOperationBase* m_UndoOperation;
  };
}
#endif
