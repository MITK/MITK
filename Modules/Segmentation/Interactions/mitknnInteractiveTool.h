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
#include "mitkPythonContext.h"
#include <array>
#include <optional>
#include <utility>
#include <vector>

namespace mitk
{
  class nnInteractiveLassoTool;
  class nnInteractiveScribbleTool;
  class PlanarFigureInteractor;
  class PlanarRectangle;
  class PointSetDataInteractor;
  class ToolManager;

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

    static std::string GetToolString(Tool tool);
    static std::string GetPromptTypeString(PromptType promptType);

    mitkClassMacro(nnInteractiveTool, SegWithPreviewTool)
    itkFactorylessNewMacro(Self)

    const char* GetName() const override;
    const char** GetXPM() const override;
    us::ModuleResource GetIconResource() const override;

    void Activated() override;
    void Deactivated() override;
    void InitializeBackend();
    const std::array<Tool, 4>& GetTools() const;

    void EnableInteraction(Tool tool, PromptType promptType);
    void DisableInteraction();
    void ResetInteractions();
    bool HasInteractions() const;
    void SetImageInSession();

    void AddInitialSegInteraction(Image* mask);

    bool GetAutoZoom() const;
    void SetAutoZoom(bool autoZoom);

    bool IsSessionReady() const;
    bool CanHandle(const BaseData* referenceData, const BaseData* workingData) const override;

    mitk::Message1<bool> nnInterConfirmMessageEvent;

  protected:
    enum class Intensity
    {
      Vibrant,
      Muted
    };

    static const Color& GetColor(PromptType promptType, Intensity intensity);

    nnInteractiveTool();
    ~nnInteractiveTool() override;

    void SetToolManager(ToolManager* toolManager) override;
    void DoUpdatePreview(const Image* inputAtTimeStep, const Image* oldSegAtTimeStep, LabelSetImage* previewImage, TimeStepType timeStep) override;
    void Notify(InteractionEvent* interactionEvent, bool isHandled) override;
    void ConfirmCleanUp() override;

    void BlockLMBDisplayInteraction();
    void UnblockLMBDisplayInteraction();

    std::string CreateNodeName(const std::string& name, std::optional<PromptType> promptType = {}) const;

  private:
    void CreatePointInteractor();
    void CreateBoxInteractor();

    DataNode::Pointer CreatePointSetNode(PromptType promptType);
    void OnPointEvent();
    DataNode* GetPointSetNode(PromptType promptType) const;

    void AddNewBoxNode(PromptType promptType);
    void OnBoxPlaced();
    const std::vector<DataNode::Pointer>& GetBoxNodes(PromptType promptType) const;
    std::vector<DataNode::Pointer>& GetBoxNodes(PromptType promptType);
    void RemoveNewBoxNode();

    void AddScribbleNode();
    void AddScribbleLabel(PromptType promptType);
    void SetActiveScribbleLabel(PromptType promptType);
    void OnScribbleEvent(itk::Object* caller, const itk::EventObject& event);
    void RemoveScribbleNode();

    void AddLassoMaskNode(PromptType promptType);
    void OnLassoEvent(itk::Object* caller, const itk::EventObject& event);
    const std::vector<DataNode::Pointer>& GetLassoNodes(PromptType promptType) const;
    std::vector<DataNode::Pointer>& GetLassoNodes(PromptType promptType);
    void RemoveLassoMaskNode();

    void AddPointInteraction(const Point3D& point);
    void AddBoxInteraction(const PlanarRectangle* box);
    void AddScribbleInteraction(const Image* mask);
    void AddLassoInteraction(const Image* mask);

    void CleanUpSession();
    void SetSessionReady(bool isReady);

    std::pair<std::string, bool> GetPointAsListString(const mitk::BaseGeometry *baseGeometry) const;
    std::pair<std::string, bool> GetBBoxAsListString(const mitk::BaseGeometry *baseGeometry) const;

    std::vector<std::pair<us::ServiceReference<InteractionEventObserver>, EventConfig>> m_EventConfigBackup;

    bool m_AutoZoom;
    PromptType m_PromptType;

    std::array<Tool, 4> m_Tools;
    std::optional<Tool> m_ActiveTool;

    DataNode::Pointer m_PositivePointsNode;
    DataNode::Pointer m_NegativePointsNode;
    itk::SmartPointer<PointSetDataInteractor> m_PointInteractor;

    std::vector<DataNode::Pointer> m_PositiveBoxNodes;
    std::vector<DataNode::Pointer> m_NegativeBoxNodes;
    std::pair<DataNode::Pointer, unsigned long> m_NewBoxNode;
    itk::SmartPointer<PlanarFigureInteractor> m_BoxInteractor;

    itk::SmartPointer<ToolManager> m_ToolManager;

    itk::SmartPointer<nnInteractiveScribbleTool> m_ScribbleTool;
    DataNode::Pointer m_ScribbleNode;
    std::unordered_map<PromptType, Label::PixelType> m_ScribbleLabels;

    itk::SmartPointer<nnInteractiveLassoTool> m_LassoTool;
    std::vector<DataNode::Pointer> m_PositiveLassoNodes;
    std::vector<DataNode::Pointer> m_NegativeLassoNodes;
    DataNode::Pointer m_LassoMaskNode;

    mitk::Image::Pointer m_MaskImage;

    mitk::LabelSetImage::Pointer m_OutputBuffer;
    mitk::PythonContext::Pointer m_PythonContext;
    bool m_IsSessionReady;
  };
}

#endif
