/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSegGroupOperationApplier_h
#define mitkSegGroupOperationApplier_h

#include "mitkCommon.h"
#include <mitkOperationActor.h>
#include <MitkSegmentationExports.h>

namespace mitk
{
  /** \brief Executes a SegGroupModifyOperation.
    \sa SegGroupModifyOperation
  */
  class MITKSEGMENTATION_EXPORT SegGroupOperationApplier : public OperationActor
  {
  public:
    mitkClassMacroNoParent(SegGroupOperationApplier)

      /** \brief Returns an instance of the class */
      static SegGroupOperationApplier *GetInstance();

    /** \brief Executes a SegGroupModifyOperation.
      \sa SegGroupModifyOperation
      Note:
        Only SegGroupModifyOperation is supported.
    */
    void ExecuteOperation(Operation *op) override;

  protected:
    SegGroupOperationApplier();

    ~SegGroupOperationApplier() override;
  };


  class SegChangeOperationBase;

  class MITKSEGMENTATION_EXPORT SegGroupModifyUndoRedoHelper
  {
  public:

    using GroupIndexSetType = std::set<MultiLabelSegmentation::GroupIndexType>;

    /** \brief */
    SegGroupModifyUndoRedoHelper(MultiLabelSegmentation* segmentation,
      const GroupIndexSetType& relevantGroupIDs, bool coverAllTimeSteps, TimeStepType timeStep = 0,
      bool noLabels = false, bool noGroupImages = false, bool noNames = false);

    ~SegGroupModifyUndoRedoHelper();

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

  class MITKSEGMENTATION_EXPORT SegGroupInsertUndoRedoHelper
  {
  public:

    using GroupIndexSetType = std::set<MultiLabelSegmentation::GroupIndexType>;

    /** \brief */
    SegGroupInsertUndoRedoHelper(MultiLabelSegmentation* segmentation,
      const GroupIndexSetType& relevantGroupIDs, bool noLabels = false, bool noGroupImages = false);

    ~SegGroupInsertUndoRedoHelper();

    void RegisterUndoRedoOperationEvent(const std::string& description);

  protected:
    MultiLabelSegmentation::Pointer m_Segmentation;
    GroupIndexSetType m_RelevantGroupIDs;
    bool m_NoLabels;
    bool m_NoGroupImages;

    SegChangeOperationBase* m_UndoOperation;
  };

  class MITKSEGMENTATION_EXPORT SegGroupRemoveUndoRedoHelper
  {
  public:

    using GroupIndexSetType = std::set<MultiLabelSegmentation::GroupIndexType>;

    /** \brief */
    SegGroupRemoveUndoRedoHelper(MultiLabelSegmentation* segmentation,
      const GroupIndexSetType& relevantGroupIDs, bool noLabels = false, bool noGroupImages = false);

    ~SegGroupRemoveUndoRedoHelper();

    void RegisterUndoRedoOperationEvent(const std::string& description);

  protected:
    MultiLabelSegmentation::Pointer m_Segmentation;
    GroupIndexSetType m_RelevantGroupIDs;


    SegChangeOperationBase* m_UndoOperation;
  };

}
#endif
