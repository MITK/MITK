/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSegmentAnythingTool_h
#define mitkSegmentAnythingTool_h

#include "mitkSegWithPreviewTool.h"
#include "mitkPointSet.h"
#include "mitkProcessExecutor.h"
#include "mitkSegmentAnythingPythonService.h"
#include <MitkSegmentationExports.h>
#include <itkImage.h>
#include <mitkLevelWindow.h>

namespace us
{
  class ModuleResource;
}

namespace mitk
{
  /**
  \brief Segment Anything Model interactive 2D tool class.

  \ingroup ToolManagerEtAl
  \sa mitk::Tool
  \sa QmitkInteractiveSegmentation

  */
  class MITKSEGMENTATION_EXPORT SegmentAnythingTool : public SegWithPreviewTool
  {
  public:
    mitkClassMacro(SegmentAnythingTool, SegWithPreviewTool);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    const char **GetXPM() const override;
    const char *GetName() const override;
    us::ModuleResource GetIconResource() const override;

    void Activated() override;
    void Deactivated() override;

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

    itkSetMacro(MitkTempDir, std::string);
    itkGetConstMacro(MitkTempDir, std::string);

    itkSetMacro(PythonPath, std::string);
    itkGetConstMacro(PythonPath, std::string);

    itkSetMacro(ModelType, std::string);
    itkGetConstMacro(ModelType, std::string);

    itkSetMacro(CheckpointPath, std::string);
    itkGetConstMacro(CheckpointPath, std::string);

    itkSetMacro(GpuId, int);
    itkGetConstMacro(GpuId, int);
    
    itkSetMacro(TimeOutLimit, long);
    itkGetConstMacro(TimeOutLimit, long);

    itkSetMacro(IsReady, bool);
    itkGetConstMacro(IsReady, bool);
    itkBooleanMacro(IsReady);

    /**
     * @brief Initializes python service and
     * starts async python daemon of SegmentAnything model.
     * 
     */
    void InitSAMPythonProcess();

    /**
     * @brief Checks if Python daemon is ready to accept inputs.
     * 
     * @return bool 
     */
    bool IsPythonReady() const;

    Message1<const std::string&> SAMStatusMessageEvent;

  protected:
    SegmentAnythingTool();
    ~SegmentAnythingTool() = default;

    void ConnectActionsAndFunctions() override;

    /*
     * @brief Add positive seed point action of StateMachine pattern
     */
    virtual void OnAddPositivePoint(StateMachineAction*, InteractionEvent *interactionEvent);
    
    /*
     * @brief Add negative seed point action of StateMachine pattern
     */
    virtual void OnAddNegativePoint(StateMachineAction*, InteractionEvent *interactionEvent);

    /*
     * @brief Delete action of StateMachine pattern. The function deletes positive or negative points in 
       the reverse order of creation. This is done by finding & deleting the Point having the highest 
       PointIdentifier value from either of the PointSets m_PointSetPositive & m_PointSetNegative.
     */
    virtual void OnDelete(StateMachineAction*, InteractionEvent*);

    /*
     * @brief Clear all seed points and call UpdatePreview to reset the segmentation Preview
     */
    void ClearSeeds();

    /**
     * @brief Overriden method from the tool manager to execute the segmentation
     * Implementation:
     * 1. Creates Hash for input image from current plane geometry.
     * 2. Transfers image pointer to python service along with the hash code.
     * 3. Creates seed points as CSV string & transfers to python service
     * 3. Retrieves resulting segmentation Image pointer from python service and sets to previewImage.
     *
     * @param inputAtTimeStep
     * @param oldSegAtTimeStep
     * @param previewImage
     * @param timeStep
     */
    void DoUpdatePreview(const Image *inputAtTimeStep, const Image *oldSegAtTimeStep, LabelSetImage *previewImage, TimeStepType timeStep) override;

    /**
     * @brief Get the Points from positive and negative pointsets as std::vector.
     * 
     * @param baseGeometry of Image
     * @return std::vector<std::pair<mitk::Point2D, std::string>> 
     */
    std::vector<std::pair<mitk::Point2D, std::string>> GetPointsAsVector(const mitk::BaseGeometry*);

    /**
     * @brief Get the Points from positive and negative pointsets as csv string.
     * 
     * @param baseGeometry 
     * @return std::stringstream 
     */
    std::stringstream GetPointsAsCSVString(const mitk::BaseGeometry *baseGeometry);

    /**
     * @brief Get the Hash For Current Plane from current working plane geometry.
     * 
     * @return std::string 
     */
    std::string GetHashForCurrentPlane(const mitk::LevelWindow&);

    /**
     * @brief Emits message to connected Listnerers.
     * 
     */
    void EmitSAMStatusMessageEvent(const std::string&);

    /**
     * @brief Cleans up segmentation preview and clears all seeds.
     * 
     */
    void ConfirmCleanUp() override;

    /**
     * @brief Applies ITK IntensityWindowing Filter to input image;
     *
     */
    template <typename TPixel, unsigned int VImageDimension>
    void ITKWindowing(const itk::Image<TPixel, VImageDimension>*, mitk::Image*, ScalarType, ScalarType);

  private:
    /**
     * @brief Convert 3D world coordinates to 2D indices.
     * 
     * @param baseGeometry 
     * @param mitk::Point3D
     * @return mitk::Point2D 
     */
    static mitk::Point2D Get2DIndicesfrom3DWorld(const mitk::BaseGeometry*, const mitk::Point3D&);

    std::string m_MitkTempDir;
    std::string m_PythonPath;
    std::string m_ModelType;
    std::string m_CheckpointPath;
    int m_GpuId = 0;
    PointSet::Pointer m_PointSetPositive;
    PointSet::Pointer m_PointSetNegative;
    DataNode::Pointer m_PointSetNodePositive;
    DataNode::Pointer m_PointSetNodeNegative;
    bool m_IsGenerateEmbeddings = true;
    bool m_IsReady = false;
    int m_PointSetCount = 0;
    long m_TimeOutLimit = -1;
    std::unique_ptr<SegmentAnythingPythonService> m_PythonService;
    const Label::PixelType MASK_VALUE = 1;
  };
} // namespace

#endif
