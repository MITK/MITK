/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef MITKMONAILABELTOOL_H
#define MITKMONAILABELTOOL_H

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "mitkSegWithPreviewTool.h"
#include <MitkSegmentationExports.h>
#include <memory>
#include <unordered_map>
#include <set>
#include <httplib.h>
#include <nlohmann/json.hpp>
#include "mitkPointSet.h"
#include "mitkInteractionPositionEvent.h"


namespace us
{
  class ModuleResource;
}

namespace mitk
{
  /**
   * @brief Struct to hold featured models individual info
   * 
   */
  struct MonaiModelInfo
  {
    std::string name;
    std::string type;
    std::unordered_map<std::string, int> labels;
    int dimension;
    std::string description;
    std::unordered_map<bool, std::string> config;

    inline bool operator==(const MonaiModelInfo &rhs) const
    {
      if (this->name == rhs.name && this->type == rhs.type) // Comparing only name and type, for now.
      {
        return true;
      }
      return false;
    }

    inline bool IsInteractive() const
    {
      if ("deepgrow" == type || "deepedit" == type)
      {
        return true;
      }
      return false;
    }
  };

  /**
   * @brief Struct to store MonaiLabel server metadata including all model infos
   * 
   */
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
   * @brief Request class to pack model and other necessary server information 
   * from GUI.
   * 
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

  /**
    \brief MonaiLabel segmentation tool base class.

    \ingroup Interaction
    \ingroup ToolManagerEtAl

    \warning Only to be instantiated by mitk::ToolManager.
  */
  class MITKSEGMENTATION_EXPORT MonaiLabelTool : public SegWithPreviewTool
  {
  public:
    mitkClassMacro(MonaiLabelTool, SegWithPreviewTool);
    itkCloneMacro(Self);
    void Activated() override;
    void Deactivated() override;
    void UpdatePrepare() override;

    /**
     * @brief Writes image to disk as the tool desires.
     * Default implementation does nothing.
     */
    virtual void WriteImage(const Image*, std::string&);

    /**
     * @brief Method does the GET Rest call to fetch MonaiLabel
     * server metadata including all models' info.
     */
    void GetOverallInfo(std::string&, int&);

    /**
     * @brief Holds all parameters of the server to serve the UI
     * 
     */
    std::unique_ptr<MonaiAppMetadata> m_InfoParameters;
    
    /**
     * @brief Variable to set selected model's and other data needed
     * for the POST call.
     */
    std::unique_ptr<MonaiLabelRequest> m_RequestParameters; 

    /**
     * @brief Get the Auto Segmentation Models info for the given 
     * dimension. 
     * 
     * @param dim 
     * @return std::vector<MonaiModelInfo> 
     */
    std::vector<MonaiModelInfo> GetAutoSegmentationModels(int dim = -1);

    /**
     * @brief Get the Interactive Segmentation Models info for the given 
     * dimension.
     * 
     * @param dim 
     * @return std::vector<MonaiModelInfo> 
     */
    std::vector<MonaiModelInfo> GetInteractiveSegmentationModels(int dim = -1);

    /**
     * @brief Get the Scribble Segmentation Models info for the given 
     * dimension. 
     * 
     * @param dim 
     * @return std::vector<MonaiModelInfo> 
     */
    std::vector<MonaiModelInfo> GetScribbleSegmentationModels(int dim = -1);

    /**
     * @brief Function to prepare the Rest request and does the POST call.
     * Writes the POST responses back to disk.
     */
    void PostInferRequest(std::string &, int &, std::string &, std::string &, const mitk::BaseGeometry *);

    /**
     * @brief Helper function to get full model info object from model name.
     * 
     * @return MonaiModelInfo 
     */
    MonaiModelInfo GetModelInfoFromName(std::string&);

    itkSetMacro(ModelName, std::string);
    itkGetConstMacro(ModelName, std::string);
    itkSetMacro(URL, std::string);
    itkGetConstMacro(URL, std::string);
    itkSetMacro(MitkTempDir, std::string);
    itkGetConstMacro(MitkTempDir, std::string);

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

    Message1<const bool> MonaiStatusEvent;

  protected:
    MonaiLabelTool();
    ~MonaiLabelTool();
    void DoUpdatePreview(const Image* inputAtTimeStep, const Image* oldSegAtTimeStep, LabelSetImage* previewImage, TimeStepType timeStep) override;
    void ConnectActionsAndFunctions() override;

    /*
     * @brief Add positive seed point action of StateMachine pattern. 
     */
    virtual void OnAddPositivePoint(StateMachineAction *, InteractionEvent *interactionEvent) = 0;

    /*
     * @brief Add negative seed point action of StateMachine pattern
     */
    virtual void OnAddNegativePoint(StateMachineAction *, InteractionEvent *interactionEvent) = 0;

    /*
     * @brief Delete action of StateMachine pattern
     */
    virtual void OnDelete(StateMachineAction *, InteractionEvent *);

    /*
     * @brief Clear all seed points and call UpdatePreview to reset the segmentation Preview
     */
    void ClearSeeds();

    /**
     * @brief Get the Points from given pointset as csv string.
     * 
     * @param baseGeometry
     * @param pointSet
     * @return std::stringstream
     */
    virtual std::stringstream GetPointsAsListString(const mitk::BaseGeometry*, PointSet::Pointer) = 0;

    /**
     * @brief Writes back segmentation results in 3D or 2D shape to preview LabelSetImage.
     * 
     */
    virtual void WriteBackResults(LabelSetImage *, LabelSetImage *, TimeStepType) = 0;

    PointSet::Pointer m_PointSetPositive;
    PointSet::Pointer m_PointSetNegative;
    DataNode::Pointer m_PointSetNodePositive;
    DataNode::Pointer m_PointSetNodeNegative;
    int m_PointSetCount = 0;

  private:
    std::string m_MitkTempDir;

    /**
     * @brief Helper function to get the Parts of the POST response
     * 
     * @return std::vector<std::string> 
     */
    std::vector<std::string> getPartsBetweenBoundary(const std::string &, const std::string &);

    /**
     * @brief Converts the json GET response from the MonaiLabel server to MonaiAppMetadata object
     * 
     * @return std::unique_ptr<MonaiAppMetadata> 
     */
    std::unique_ptr<MonaiAppMetadata> DataMapper(nlohmann::json&);
    /**
     * @brief Applies the give std::map lookup table on the preview segmentation LabelSetImage.
     * 
     */
    void MapLabelsToSegmentation(const mitk::LabelSetImage*, mitk::LabelSetImage*, std::map<std::string, mitk::Label::PixelType>&);

    /**
     * @brief Checks if MonaiLabel server is alive
     * 
     * @return bool 
     */
    bool IsMonaiServerOn(std::string &, int &);
    std::string m_ModelName;
    std::string m_URL;
    nlohmann::json m_ResultMetadata;
    const std::set<std::string> m_AUTO_SEG_TYPE_NAME = {"segmentation"};
    const std::set<std::string> m_SCRIBBLE_SEG_TYPE_NAME = {"scribbles"};
    const std::set<std::string> m_INTERACTIVE_SEG_TYPE_NAME = {"deepgrow"}; // deepedit not supported yet 
    const std::string m_TEMPLATE_FILENAME = "XXXXXX_000_0000.nii.gz";
    const std::string m_SERVER_503_ERROR_TEXT = "A connection to MonaiLabel server cannot be established.";
    const Label::PixelType MASK_VALUE = 1;
  };
}
#endif
