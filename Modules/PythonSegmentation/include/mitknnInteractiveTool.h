/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitknnInteractiveTool_h
#define mitknnInteractiveTool_h

#include <mitkSegWithPreviewTool.h>
#include <mitknnInteractiveEnums.h>

#include <optional>

namespace mitk::nnInteractive
{
  class Interactor;
}

namespace mitk
{
  class PythonContext;

  class MITKPYTHONSEGMENTATION_EXPORT nnInteractiveTool : public SegWithPreviewTool
  {
  public:
    mitkClassMacro(nnInteractiveTool, SegWithPreviewTool)
    itkFactorylessNewMacro(Self)

    const char* GetName() const override;
    const char** GetXPM() const override;
    us::ModuleResource GetIconResource() const override;
    bool CanHandle(const BaseData* referenceData, const BaseData* workingData) const override;
    void Deactivated() override;

    using InteractorMap = std::unordered_map<nnInteractive::InteractionType, std::unique_ptr<nnInteractive::Interactor>>;
    const InteractorMap& GetInteractors() const;
    const nnInteractive::Interactor* GetInteractor(nnInteractive::InteractionType interactionType) const;

    void EnableInteractor(nnInteractive::InteractionType nextInteractionType, nnInteractive::PromptType promptType);
    void DisableInteractor(std::optional<nnInteractive::InteractionType> interactionType = std::nullopt);
    void ResetInteractions();
    bool HasInteractions() const;

    bool GetAutoZoom() const;
    void SetAutoZoom(bool autoZoom);

    bool GetAutoRefine() const;
    void SetAutoRefine(bool autoRefine);

    std::optional<nnInteractive::Backend> GetBackend() const;

    bool CreatePythonContext(const std::string& pythonExecutable);
    bool IsInstalled();
    void StartSession();
    void EndSession();
    bool IsSessionRunning() const;
    void InitializeSessionWithMask(Image* mask);

    Message1<bool> ConfirmCleanUpEvent;

  protected:
    nnInteractiveTool();
    ~nnInteractiveTool() override;

    void SetToolManager(ToolManager* toolManager) override;
    void DoUpdatePreview(const Image* inputAtTimeStep, const Image* oldSegAtTimeStep, MultiLabelSegmentation* previewImage, TimeStepType timeStep) override;
    void Notify(InteractionEvent* event, bool isHandled) override;
    void ConfirmCleanUp() override;

  private:
    class Impl;
    std::unique_ptr<Impl> m_Impl;
  };
}

#endif
