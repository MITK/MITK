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

#include <array>
#include <optional>
#include <utility>
#include <vector>

namespace mitk
{
  class MITKSEGMENTATION_EXPORT nnInteractiveTool : public SegWithPreviewTool
  {
  public:
    enum class Tool
    {
      Point,
      Box,
      Scribble,
      Lasso
    };

    enum class PromptType
    {
      Positive,
      Negative
    };

    enum class Intensity
    {
      Vibrant,
      Muted
    };

    mitkClassMacro(nnInteractiveTool, SegWithPreviewTool);
    itkFactorylessNewMacro(Self);

    const char* GetName() const override;
    const char** GetXPM() const override;
    us::ModuleResource GetIconResource() const override;

    void Activated() override;
    void Deactivated() override;

    const std::array<Tool, 4>& GetTools() const;

    void EnableInteraction(Tool tool, PromptType promptType);
    void DisableInteraction();
    void ResetInteractions();

  protected:
    static Color GetColor(PromptType promptType, Intensity intensity);

    nnInteractiveTool();
    ~nnInteractiveTool() override;

    void DoUpdatePreview(const Image* inputAtTimeStep, const Image* oldSegAtTimeStep, LabelSetImage* previewImage, TimeStepType timeStep) override;

    void BlockLMBDisplayInteraction();
    void UnblockLMBDisplayInteraction();

    DataNode* GetPointSetNode(PromptType promptType) const;
    std::vector<DataNode::Pointer>& GetBoxNodes(PromptType promptType);

    std::string CreateNodeName(const std::string& name, PromptType promptType) const;

  private:
    void AddNewBoxNode(PromptType promptType);
    void RemoveNewBoxNode();
    void OnBoxPlaced();

    std::vector<std::pair<us::ServiceReference<InteractionEventObserver>, EventConfig>> m_EventConfigBackup;

    PromptType m_PromptType;

    std::array<Tool, 4> m_Tools;
    std::optional<Tool> m_ActiveTool;

    DataNode::Pointer m_PositivePointsNode;
    DataNode::Pointer m_NegativePointsNode;

    std::vector<DataNode::Pointer> m_PositiveBoxNodes;
    std::vector<DataNode::Pointer> m_NegativeBoxNodes;
    std::pair<DataNode::Pointer, unsigned long> m_NewBoxNode;
  };
}

#endif
