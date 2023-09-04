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
#include <memory>
#include <unordered_map>
#include <set>
#include <httplib.h>
#include <nlohmann/json.hpp>
#include "mitkPointSet.h"


namespace us
{
  class ModuleResource;
}

namespace mitk
{
  struct MonaiModelInfo
  {
    std::string name;
    std::string type;
    std::unordered_map<std::string, int> labels;
    int dimension;
    std::string description;
    std::unordered_map<bool, std::string> config; //TODO: find the full extent

    inline bool operator==(const MonaiModelInfo &rhs) const
    {
      if (this->name == rhs.name && this->type == rhs.type) // Comparing only name and type, for now.
      {
        return true;
      }
      return false;
    }
  };

  struct MonaiAppMetadata
  {
    std::string hostName;
    int port;
    std::string origin;
    std::string name;
    std::string description;
    std::string version;
    std::vector<std::string> labels;
    std::vector<MonaiModelInfo> models;
  };

  /**
  * Parameters that goes into infer POST
  */
  struct MonaiLabelRequest
  {
    MonaiModelInfo model;
    std::string hostName;
    std::string requestLabel;
    int port;

    inline bool operator==(const MonaiLabelRequest &rhs) const
    { 
      if (this->model == rhs.model && this->hostName == rhs.hostName && this->port == rhs.port && this->requestLabel == rhs.requestLabel)
      {
        return true;
      }
      return false;
    }
  };

  class MITKSEGMENTATION_EXPORT MonaiLabelTool : public SegWithPreviewTool
  {
  public:
    mitkClassMacro(MonaiLabelTool, SegWithPreviewTool);
    itkCloneMacro(Self);
    bool m_TEST = false; // cleanup later
    void Activated() override;
    void Deactivated() override;

    void GetOverallInfo(std::string&, int&);
    std::unique_ptr<MonaiAppMetadata> m_InfoParameters; //contains all parameters from Server to serve the GUI
    std::unique_ptr<MonaiLabelRequest> m_RequestParameters;
    std::vector<MonaiModelInfo> GetAutoSegmentationModels(int dim = -1);
    std::vector<MonaiModelInfo> GetInteractiveSegmentationModels(int dim = -1);
    std::vector<MonaiModelInfo> GetScribbleSegmentationModels(int dim = -1);
    void PostInferRequest(std::string &, int &, std::string &, std::string &, const mitk::BaseGeometry *);
    MonaiModelInfo GetModelInfoFromName(std::string&);

    itkSetMacro(ModelName, std::string);
    itkGetConstMacro(ModelName, std::string);
    itkSetMacro(URL, std::string);
    itkGetConstMacro(URL, std::string);
    itkSetMacro(MitkTempDir, std::string);
    itkGetConstMacro(MitkTempDir, std::string);
    itkSetMacro(IsLastSuccess, bool);
    itkGetConstMacro(IsLastSuccess, bool);

    /**
     * @brief  Clears all picks and updates the preview.
     */
    void ClearPicks();

    /**
     * @brief Checks if any point exists in the either
     * of the pointsets
     *
     * @return bool
     */
    bool HasPicks() const;

  protected:
    MonaiLabelTool();
    ~MonaiLabelTool();
    void DoUpdatePreview(const Image* inputAtTimeStep, const Image* oldSegAtTimeStep, LabelSetImage* previewImage, TimeStepType timeStep) override;
    void ConnectActionsAndFunctions() override;

    /*
     * @brief Add positive seed point action of StateMachine pattern
     */
    virtual void OnAddPositivePoint(StateMachineAction *, InteractionEvent *interactionEvent);

    /*
     * @brief Add negative seed point action of StateMachine pattern
     */
    virtual void OnAddNegativePoint(StateMachineAction *, InteractionEvent *interactionEvent);

    /*
     * @brief Delete action of StateMachine pattern
     */
    virtual void OnDelete(StateMachineAction *, InteractionEvent *);

    /*
     * @brief Clear all seed points and call UpdatePreview to reset the segmentation Preview
     */
    void ClearSeeds();

    std::stringstream GetPointsAsListString(const mitk::BaseGeometry*, PointSet::Pointer);

  private:
    std::string m_MitkTempDir;
    std::vector<std::string> getPartsBetweenBoundary(const std::string &, const std::string &);
    std::unique_ptr<MonaiAppMetadata> mitk::MonaiLabelTool::DataMapper(nlohmann::json&);
    void MapLabelsToSegmentation(const mitk::LabelSetImage*, mitk::LabelSetImage*, std::map<std::string, mitk::Label::PixelType>&);
    bool IsMonaiServerOn(std::string &, int &);
    bool m_IsLastSuccess = false;
    std::string m_ModelName;
    std::string m_URL;
    nlohmann::json m_ResultMetadata;
    PointSet::Pointer m_PointSetPositive;
    PointSet::Pointer m_PointSetNegative;
    DataNode::Pointer m_PointSetNodePositive;
    DataNode::Pointer m_PointSetNodeNegative;
    int m_PointSetCount = 0;
    const std::set<std::string> m_AUTO_SEG_TYPE_NAME = {"segmentation"};
    const std::set<std::string> m_SCRIBBLE_SEG_TYPE_NAME = {"scribbles"};
    const std::set<std::string> m_INTERACTIVE_SEG_TYPE_NAME = {"deepedit", "deepgrow"};
    const std::string m_TEMPLATE_FILENAME = "XXXXXX_000_0000.nii.gz";
    const std::string m_SERVER_503_ERROR_TEXT = "A connection to MonaiLabel server cannot be established.";
  };
}
#endif
