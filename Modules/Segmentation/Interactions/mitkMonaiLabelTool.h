/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef MITKMONAILABELTOOL_H
#define MITKMONAILABELTOOL_H

#include "mitkSegWithPreviewTool.h"
#include <MitkSegmentationExports.h>
#include <mitkIRESTManager.h>
#include <memory>
#include <unordered_map>
#include <set>

namespace us
{
  class ModuleResource;
}

namespace mitk
{
  // class Image;

  struct MonaiModelInfo
  {
    std::string name;
    std::string type;
    std::unordered_map<std::string, int> labels;
    int dimension;
    std::string description;
    std::unordered_map<bool, std::string> config; //TODO: find the full extent
  };

  struct MonaiAppMetadata
  {
    std::string URL;
    unsigned short port;
    std::string origin;
    std::string name;
    std::string description;
    std::string version;
    std::vector<std::string> labels;
    std::vector<MonaiModelInfo> models;
  };

  class MITKSEGMENTATION_EXPORT MonaiLabelTool : public SegWithPreviewTool
  {
  public:
    mitkClassMacro(MonaiLabelTool, SegWithPreviewTool);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    const char *GetName() const override;
    const char **GetXPM() const override;
    us::ModuleResource GetIconResource() const override;

    void Activated() override;
    void GetOverallInfo(std::string);
    std::unique_ptr<MonaiAppMetadata> m_Parameters; //contains all parameters from Server to serve the GUI
    std::vector<MonaiModelInfo> GetAutoSegmentationModels();
    std::vector<MonaiModelInfo> GetInteractiveSegmentationModels();
    std::vector<MonaiModelInfo> GetScribbleSegmentationModels();
    void PostSegmentationRequest();

    itkSetMacro(ModelName, std::string);
    itkGetConstMacro(ModelName, std::string);
    itkSetMacro(URL, std::string);
    itkGetConstMacro(URL, std::string);



  protected:
    MonaiLabelTool();
    ~MonaiLabelTool() = default;

    void DoUpdatePreview(const Image* inputAtTimeStep, const Image* oldSegAtTimeStep, LabelSetImage* previewImage, TimeStepType timeStep) override;
   
  
  private:
    void InitializeRESTManager();
    std::unique_ptr<MonaiAppMetadata> DataMapper(web::json::value&);
    mitk::IRESTManager *m_RESTManager;
    const std::set<std::string> m_AUTO_SEG_TYPE_NAME = {"segmentation"};
    const std::set<std::string> m_SCRIBBLE_SEG_TYPE_NAME = {"scribbles"};
    const std::set<std::string> m_INTERACTIVE_SEG_TYPE_NAME = {"deepedit", "deepgrow"};
    std::string m_ModelName;
    std::string m_URL;

  }; // class
} // namespace
#endif
