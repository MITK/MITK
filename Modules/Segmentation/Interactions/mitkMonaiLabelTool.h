/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkMonaiLabelTool_h
#define mitkMonaiLabelTool_h

#include "mitkSegWithPreviewTool.h"
#include <MitkSegmentationExports.h>
#include <memory>
#include <unordered_map>
#include <set>
#include <nlohmann/json.hpp>
#include <mitkPointSet.h>
#include <mitkInteractionPositionEvent.h>


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
      return (this->name == rhs.name && this->type == rhs.type); // Comparing only name and type, for now.
    }

    inline bool IsInteractive() const
    {
      return ("deepgrow" == type || "deepedit" == type);
    }
    
    inline bool Is2D() const 
    { 
      return dimension == 2; 
    }
  };

  /**
   * @brief Struct to store MonaiLabel server metadata including all model infos
   * 
   */
  struct MonaiAppMetadata
  {
    std::string name;
    std::string description;
    std::vector<std::string> labels;
    std::string version;
    std::string hostName;
    int port;
    std::string origin;
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
      return (this->model == rhs.model && this->hostName == rhs.hostName && this->port == rhs.port &&
              this->requestLabel == rhs.requestLabel);
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
     * @brief Method does the GET Rest call to fetch MonaiLabel
     * server metadata including all models' info.
     */
    void FetchOverallInfo(const std::string &hostName, const int &port);

    /**
     * @brief Variable to set selected model's and other data needed
     * for the POST call.
     */
    std::unique_ptr<MonaiLabelRequest> m_RequestParameters; 

    /**
     * @brief Get the Auto Segmentation Models info for the given 
     * dimension.
     */
    const std::vector<MonaiModelInfo> GetAutoSegmentationModels(const int dim = -1) const;

    /**
     * @brief Get the Interactive Segmentation Models info for the given 
     * dimension.
     */
    const std::vector<MonaiModelInfo> GetInteractiveSegmentationModels(const int dim = -1) const;

    /**
     * @brief Get the Scribble Segmentation Models info for the given 
     * dimension. 
     */
    const std::vector<MonaiModelInfo> GetScribbleSegmentationModels(const int dim = -1) const;

    /**
     * @brief Helper function to get full model info object from model name. 
     */
    MonaiModelInfo GetModelInfoFromName(const std::string) const;

    itkSetMacro(ModelName, std::string);
    itkGetConstMacro(ModelName, std::string);
    itkSetMacro(URL, std::string);
    itkGetConstMacro(URL, std::string);
    itkSetMacro(TempDir, std::string);
    itkGetConstMacro(TempDir, std::string);
    itkSetMacro(Timeout, unsigned int);
    itkGetConstMacro(Timeout, unsigned int);

    const MonaiAppMetadata *GetInfoParameters() const;


    /**
     * @brief  Clears all picks and updates the preview.
     */
    void ClearPicks();

    /**
     * @brief Checks if any point exists in the either of the PointSetPositive
     * or PointSetNegative.
     */
    bool HasPicks() const;

    Message1<const bool> MonaiStatusEvent;

  protected:
    MonaiLabelTool();
    ~MonaiLabelTool();
    void DoUpdatePreview(const Image* inputAtTimeStep, const Image* oldSegAtTimeStep, LabelSetImage* previewImage, TimeStepType timeStep) override;
    void ConnectActionsAndFunctions() override;

    /**
     * @brief Writes image to disk as the tool desires.
     * Default implementation does nothing.
     */
    virtual void WriteImage(const Image *, const std::string &) const = 0;

    /*
     * @brief Add positive seed point action of StateMachine pattern. 
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

    void OnMove(StateMachineAction *, InteractionEvent *);
    void OnRelease(StateMachineAction *, InteractionEvent *);
    void OnPrimaryButtonPressed(StateMachineAction *, InteractionEvent *);
    
    /*
     * @brief Clear all seed points and call UpdatePreview to reset the segmentation Preview
     */
    void ClearSeeds();

    /**
     * @brief Get the Points from given pointset as csv string.
     * 
     */
    virtual std::string ConvertPointsAsListString(const mitk::BaseGeometry *baseGeometry,
                                                  const PointSet::Pointer pointSet) const;

    /**
     * @brief Writes back segmentation results in 3D or 2D shape to preview LabelSetImage.
     */
    virtual void WriteBackResults(LabelSetImage *, LabelSetImage *, TimeStepType) const = 0;

    PointSet::Pointer m_PointSetPositive;
    PointSet::Pointer m_PointSetNegative;
    DataNode::Pointer m_PointSetNodePositive;
    DataNode::Pointer m_PointSetNodeNegative;
    int m_PointSetCount = 0;

  private:

    /**
     * @brief Holds all parameters of the server to serve the UI
     *
     */
    std::unique_ptr<MonaiAppMetadata> m_InfoParameters;

    /**
     * @brief Helper function to create temp directory for writing/reading images
     * Returns input and output file names expected as a pair.
     */
    std::pair<std::string, std::string> CreateTempDirs(const std::string &filePattern) const;

    /**
     * @brief Checks if MonaiLabel server is alive.
     */
    bool IsMonaiServerOn(const std::string &hostName, const int &port) const;
     
    /**
     * @brief Applies level window filter on the input image. Current Level window bounds 
     * are captured from the tool manager.
     */
    mitk::Image::Pointer ApplyLevelWindowEffect(const Image *inputAtTimeStep) const;

    /**
     * @brief Function to prepare the Rest request and does the POST call.
     * Writes the POST responses back to disk.
     */
    void PostInferRequest(const std::string &hostName, const int &port, const std::string &filePath, const std::string &outFile,
                          const mitk::BaseGeometry *baseGeometry);
    
    std::string m_TempDir;
    std::string m_ModelName;
    std::string m_URL;
    nlohmann::json m_ResultMetadata;
    unsigned int m_Timeout = 60; //seconds
    const std::set<std::string> m_AUTO_SEG_TYPE_NAME = {"segmentation"};
    const std::set<std::string> m_SCRIBBLE_SEG_TYPE_NAME = {"scribbles"};
    const std::set<std::string> m_INTERACTIVE_SEG_TYPE_NAME = {"deepgrow"}; // deepedit not supported yet 
    const std::string m_TEMPLATE_FILENAME = "XXXXXX_000_0000.nii.gz";
    const std::string m_SERVER_503_ERROR_TEXT = "A connection to MonaiLabel server cannot be established.";
    const Label::PixelType MASK_VALUE = 1;
  };
}
#endif
