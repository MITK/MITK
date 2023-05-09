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

#include <MitkSegmentationExports.h>

namespace us
{
  class ModuleResource;
}

namespace mitk
{
  /** CHANGE THIS --ashis
  \brief Segment Anything Model interactive 2D tool.

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
     * Clears all picks and updates the preview.
     */
    void ClearPicks();
    bool HasPicks() const;

    itkSetMacro(MitkTempDir, std::string);
    itkGetConstMacro(MitkTempDir, std::string);

    itkSetMacro(PythonPath, std::string);
    itkGetConstMacro(PythonPath, std::string);

    itkSetMacro(ModelType, std::string);
    itkGetConstMacro(ModelType, std::string);

    itkSetMacro(CheckpointPath, std::string);
    itkGetConstMacro(CheckpointPath, std::string);

    itkSetMacro(GpuId, unsigned int);
    itkGetConstMacro(GpuId, unsigned int);

    itkSetMacro(IsAuto, bool);
    itkGetConstMacro(IsAuto, bool);
    itkBooleanMacro(IsAuto);

    itkSetMacro(IsReady, bool);
    itkGetConstMacro(IsReady, bool);
    itkBooleanMacro(IsReady);

    /**
     * @brief Static function to print out everything from itk::EventObject.
     * Used as callback in mitk::ProcessExecutor object.
     *
     */
    static void onPythonProcessEvent(itk::Object *, const itk::EventObject &e, void *);

    bool run_download_model(std::string);

  protected:
    SegmentAnythingTool();
    ~SegmentAnythingTool() override;

    void ConnectActionsAndFunctions() override;

    /*
     * @brief Add point action of StateMachine pattern
     */
    virtual void OnAddPoint(StateMachineAction*, InteractionEvent* interactionEvent);
    virtual void OnAddNegativePoint(StateMachineAction*, InteractionEvent* interactionEvent);

    /*
     * @brief Delete action of StateMachine pattern
     */
    virtual void OnDelete(StateMachineAction*, InteractionEvent* interactionEvent);

    /*
     * @brief Clear all seed points and call UpdatePreview to reset the segmentation Preview
     */
    void ClearSeeds();

    void DoUpdatePreview(const Image* inputAtTimeStep, const Image* oldSegAtTimeStep, LabelSetImage* previewImage, TimeStepType timeStep) override;

    std::vector<std::pair<mitk::Point2D, std::string>> GetPointsAsCSVString(const mitk::BaseGeometry*);

  private:
    /**
     * @brief Runs SAM python process with desired arguments to generate embeddings for the input image
     *
     */
    void run_generate_embeddings(ProcessExecutor*, const std::string&, const std::string&, const std::string&, const std::string&, const unsigned int);
    
    void run_segmentation_from_points(ProcessExecutor*, const std::string&, const std::string&, const std::string&,
                                    const std::string&,
                                    const std::string&,
                                    const std::string&,
                                    const unsigned int);
    static mitk::Point2D Get2DIndicesfrom3DWorld(const mitk::BaseGeometry*, mitk::Point3D&);

    std::string m_MitkTempDir;
    std::string m_PythonPath;
    std::string m_ModelType;
    std::string m_CheckpointPath;
    std::string m_InDir, m_OutDir, inputImagePath, pickleFilePath, outputImagePath, token; //rename as per standards


    unsigned int m_GpuId = 0;

    PointSet::Pointer m_PointSetPositive;
    PointSet::Pointer m_PointSetNegative;
    DataNode::Pointer m_PointSetNode;
    DataNode::Pointer m_PointSetNodeNegative;
    bool m_IsGenerateEmbeddings = true;
    bool m_IsAuto = false;
    bool m_IsReady = false;
    const std::string TEMPLATE_FILENAME = "XXXXXX_000_0000.nii.gz";
    int m_PointSetCount = 0;

  };

} // namespace

#endif
