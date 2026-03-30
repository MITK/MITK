/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkMonaiLabelTool_h
#define mitkMonaiLabelTool_h

#include <mitkSegWithPreviewTool.h>
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
   * \brief Struct holding metadata for a single MonaiLabel model.
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
   * \brief Struct storing MonaiLabel server metadata including all model information.
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
   * \brief Request class encapsulating model and server information for a MonaiLabel inference request.
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
     * \brief Fetches MonaiLabel server metadata via GET REST call.
     */
    void FetchOverallInfo(const std::string &hostName, const int &port);

    /**
     * \brief Holds the selected model and server data for the POST inference call.
     */
    std::unique_ptr<MonaiLabelRequest> m_RequestParameters; 

    /**
     * \brief Returns the auto-segmentation model information for the given dimension.
     */
    const std::vector<MonaiModelInfo> GetAutoSegmentationModels(const int dim = -1) const;

    /**
     * \brief Returns the interactive segmentation model information for the given dimension.
     */
    const std::vector<MonaiModelInfo> GetInteractiveSegmentationModels(const int dim = -1) const;

    /**
     * \brief Returns the scribble segmentation model information for the given dimension.
     */
    const std::vector<MonaiModelInfo> GetScribbleSegmentationModels(const int dim = -1) const;

    /**
     * \brief Returns full model info for the given model name.
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
     * \brief Clears all seed picks and updates the preview.
     */
    void ClearPicks();

    /**
     * \brief Checks if any seed point exists in either the positive or negative point set.
     */
    bool HasPicks() const;

    Message1<const bool> MonaiStatusEvent;

  protected:
    MonaiLabelTool();
    ~MonaiLabelTool();
    void DoUpdatePreview(const Image* inputAtTimeStep, const Image* oldSegAtTimeStep, MultiLabelSegmentation* previewImage, TimeStepType timeStep) override;
    void ConnectActionsAndFunctions() override;

    /**
     * \brief Writes the input image to disk in a format required by the tool.
     *
     * Must be implemented by derived classes.
     */
    virtual void WriteImage(const Image *, const std::string &) const = 0;

    /**
     * \brief Add positive seed point action of StateMachine pattern.
     */
    virtual void OnAddPositivePoint(StateMachineAction *, InteractionEvent *interactionEvent);

    /**
     * \brief Add negative seed point action of StateMachine pattern.
     */
    virtual void OnAddNegativePoint(StateMachineAction *, InteractionEvent *interactionEvent);

    /**
     * \brief Delete action of StateMachine pattern.
     */
    virtual void OnDelete(StateMachineAction *, InteractionEvent *);

    void OnMove(StateMachineAction *, InteractionEvent *);
    void OnRelease(StateMachineAction *, InteractionEvent *);
    void OnPrimaryButtonPressed(StateMachineAction *, InteractionEvent *);
    
    /**
     * \brief Clears all seed points and updates the preview to reset the segmentation.
     */
    void ClearSeeds();

    /**
     * \brief Converts point coordinates from a given point set to a CSV string.
     */
    virtual std::string ConvertPointsAsListString(const mitk::BaseGeometry *baseGeometry,
                                                  const PointSet::Pointer pointSet) const;

    /**
     * \brief Writes back segmentation results in 3D or 2D shape to the preview image.
     */
    virtual void WriteBackResults(MultiLabelSegmentation *, MultiLabelSegmentation *, TimeStepType) const = 0;

    PointSet::Pointer m_PointSetPositive;
    PointSet::Pointer m_PointSetNegative;
    DataNode::Pointer m_PointSetNodePositive;
    DataNode::Pointer m_PointSetNodeNegative;
    int m_PointSetCount = 0;

  private:

    /**
     * \brief Holds all server parameters to serve the UI.
     */
    std::unique_ptr<MonaiAppMetadata> m_InfoParameters;

    /**
     * \brief Creates temporary directories for writing/reading images.
     * \return Pair of input and output file paths.
     */
    std::pair<std::string, std::string> CreateTempDirs(const std::string &filePattern) const;

    /**
     * \brief Checks if the MonaiLabel server is reachable.
     */
    bool IsMonaiServerOn(const std::string &hostName, const int &port) const;
     
    /**
     * \brief Applies level window filter on the input image using current level window bounds.
     */
    mitk::Image::Pointer ApplyLevelWindowEffect(const Image *inputAtTimeStep) const;

    /**
     * \brief Prepares and executes the POST REST call for inference, writing responses to disk.
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
