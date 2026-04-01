/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSegmentAnythingTool_h
#define mitkSegmentAnythingTool_h

#include <mitkSegWithPreviewTool.h>
#include <mitkPointSet.h>
#include <mitkProcessExecutor.h>
#include <mitkSegmentAnythingPythonService.h>
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

    const char *GetName() const override;
    us::ModuleResource GetIconResource() const override;

    void Activated() override;
    void Deactivated() override;

    /**
     * \brief Clears all seed picks and updates the preview.
     */
    virtual void ClearPicks();

    /**
     * \brief Checks if any seed point exists in either the positive or negative point set.
     * \return true if picks exist, false otherwise.
     */
    virtual bool HasPicks() const;

    itkSetMacro(MitkTempDir, std::string);
    itkGetConstMacro(MitkTempDir, std::string);

    itkSetMacro(PythonPath, std::string);
    itkGetConstMacro(PythonPath, std::string);

    itkSetMacro(ModelType, std::string);
    itkGetConstMacro(ModelType, std::string);

    itkSetMacro(CheckpointPath, std::string);
    itkGetConstMacro(CheckpointPath, std::string);

    itkSetMacro(Backend, std::string);
    itkGetConstMacro(Backend, std::string);

    itkSetMacro(GpuId, int);
    itkGetConstMacro(GpuId, int);
    
    itkSetMacro(TimeOutLimit, long);
    itkGetConstMacro(TimeOutLimit, long);

    itkSetMacro(IsReady, bool);
    itkGetConstMacro(IsReady, bool);
    itkBooleanMacro(IsReady);

    /**
     * \brief Initializes the Python service and starts the async SAM daemon.
     */
    void InitSAMPythonProcess();

    /**
     * \brief Checks if the Python daemon is ready to accept inputs.
     * \return true if the daemon is ready, false otherwise.
     */
    bool IsPythonReady() const;

    Message1<const std::string&> SAMStatusMessageEvent;

  protected:
    SegmentAnythingTool();
    ~SegmentAnythingTool() = default;

    void ConnectActionsAndFunctions() override;

    /**
     * \brief Add positive seed point action of StateMachine pattern.
     */
    virtual void OnAddPositivePoint(StateMachineAction*, InteractionEvent *interactionEvent);
    
    /**
     * \brief Add negative seed point action of StateMachine pattern.
     */
    virtual void OnAddNegativePoint(StateMachineAction*, InteractionEvent *interactionEvent);

    /**
     * \brief Delete action of StateMachine pattern.
     *
     * Deletes positive or negative points in reverse order of creation by finding and
     * deleting the point with the highest PointIdentifier value from either point set.
     */
    virtual void OnDelete(StateMachineAction*, InteractionEvent*);

    void OnMove(StateMachineAction*, InteractionEvent*);
    void OnRelease(StateMachineAction*, InteractionEvent*);
    void OnPrimaryButtonPressed(StateMachineAction *, InteractionEvent *);

    /**
     * \brief Clears all seed points and updates the preview.
     */
    void ClearSeeds();

    /**
     * \brief Executes the SAM segmentation by communicating with the Python daemon.
     *
     * Implementation: (1) creates a hash for the input image from the current plane geometry;
     * (2) transfers the image to the Python service; (3) creates seed points as CSV and transfers
     * them; (4) retrieves the resulting segmentation from the Python service and writes it to the preview.
     */
    void DoUpdatePreview(const Image *inputAtTimeStep, const Image *oldSegAtTimeStep, MultiLabelSegmentation *previewImage, TimeStepType timeStep) override;

    /**
     * \brief Returns all seed points from both positive and negative point sets as a vector.
     * \return Vector of 2D point/label-string pairs.
     */
    std::vector<std::pair<mitk::Point2D, std::string>> GetPointsAsVector(const mitk::BaseGeometry *baseGeometry) const;

    /**
     * \brief Returns all seed points from positive and negative point sets as a CSV string.
     * \return CSV-formatted string of point coordinates.
     */
    virtual std::string GetPointsAsCSVString(const mitk::BaseGeometry *baseGeometry) const;

    /**
     * \brief Computes a hash for the current working plane geometry.
     * \return A hash string identifying the current plane.
     */
    std::string GetHashForCurrentPlane(const mitk::LevelWindow &levelWindow) const;

    /**
     * \brief Emits a status message to connected listeners.
     */
    void EmitSAMStatusMessageEvent(const std::string &status);

    /**
     * \brief Cleans up segmentation preview and clears all seeds.
     */
    void ConfirmCleanUp() override;

    /**
     * \brief Applies ITK intensity windowing filter to the input image.
     */
    template <typename TPixel, unsigned int VImageDimension>
    void ITKWindowing(const itk::Image<TPixel, VImageDimension>*, mitk::Image*, ScalarType, ScalarType);

    /**
     * \brief Converts 3D world coordinates to 2D index coordinates.
     *
     * \param[in] baseGeometry Base geometry of the image.
     * \param[in] point3d The 3D world coordinates to convert.
     * \return The corresponding 2D index coordinates.
     */
    static mitk::Point2D Get2DIndicesfrom3DWorld(const mitk::BaseGeometry *baseGeometry, const mitk::Point3D &point3d);

    SegmentAnythingPythonService::Pointer m_PythonService;

  private:
    std::string m_MitkTempDir;
    std::string m_PythonPath;
    std::string m_ModelType;
    std::string m_CheckpointPath;
    std::string m_Backend;
    int m_GpuId = 0;
    PointSet::Pointer m_PointSetPositive;
    PointSet::Pointer m_PointSetNegative;
    DataNode::Pointer m_PointSetNodePositive;
    DataNode::Pointer m_PointSetNodeNegative;
    bool m_IsGenerateEmbeddings = true;
    bool m_IsReady = false;
    int m_PointSetCount = 0;
    long m_TimeOutLimit = -1;
    const Label::PixelType MASK_VALUE = 1;
  };
} // namespace

#endif
