/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitknnInteractiveTool_h
#define mitknnInteractiveTool_h

#include "mitkSegWithPreviewTool.h"

#include <mitkDataNode.h>
#include <mitkPointSet.h>

namespace mitk
{
  class MITKSEGMENTATION_EXPORT nnInteractiveTool : public SegWithPreviewTool
  {
  public:
    enum PromptType
    {
      Positive,
      Negative
    };

    mitkClassMacro(nnInteractiveTool, SegWithPreviewTool);
    itkFactorylessNewMacro(Self);

    const char* GetName() const override;
    const char** GetXPM() const override;
    us::ModuleResource GetIconResource() const override;

    void Activated() override;
    void Deactivated() override;

    DataNode* GetPointSetNode(PromptType promptType) const;
  protected:
    nnInteractiveTool();
    ~nnInteractiveTool() override;

    void DoUpdatePreview(const Image* inputAtTimeStep, const Image* oldSegAtTimeStep, LabelSetImage* previewImage, TimeStepType timeStep) override;

  private:
    PointSet::Pointer m_PositivePoints;
    DataNode::Pointer m_PositivePointsNode;
    PointSet::Pointer m_NegativePoints;
    DataNode::Pointer m_NegativePointsNode;
  };
}

#endif
