/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkAutoSegmentationWithPreviewTool_h_Included
#define mitkAutoSegmentationWithPreviewTool_h_Included

#include "mitkAutoSegmentationTool.h"
#include "mitkCommon.h"
#include "mitkDataNode.h"
#include "mitkToolCommand.h"
#include <MitkSegmentationExports.h>

namespace mitk
{
  /**
  \brief Base class for any auto segmentation tool that provides a preview of the new segmentation.

  This tool class implements a lot basic logic to handle auto segmentation tools with preview,
  Time point and ROI support. Derived classes will ask to update the segmentation preview if needed
  (e.g. because the ROI or the current time point has changed) or because derived tools
  indicated the need to update themselves.
  This class also takes care to properly transfer a confirmed preview into the segementation
  result.

  \ingroup ToolManagerEtAl
  \sa mitk::Tool
  \sa QmitkInteractiveSegmentation
  */
  class MITKSEGMENTATION_EXPORT AutoSegmentationWithPreviewTool : public AutoSegmentationTool
  {
  public:

    mitkClassMacro(AutoSegmentationWithPreviewTool, AutoSegmentationTool);

    void Activated() override;
    void Deactivated() override;

    void ConfirmSegmentation();

    itkSetMacro(CreateAllTimeSteps, bool);
    itkGetMacro(CreateAllTimeSteps, bool);
    itkBooleanMacro(CreateAllTimeSteps);

    itkSetMacro(KeepActiveAfterAccept, bool);
    itkGetMacro(KeepActiveAfterAccept, bool);
    itkBooleanMacro(KeepActiveAfterAccept);

    itkSetMacro(IsTimePointChangeAware, bool);
    itkGetMacro(IsTimePointChangeAware, bool);
    itkBooleanMacro(IsTimePointChangeAware);

    bool CanHandle(const BaseData* referenceData, const BaseData* workingData) const override;

    /** Triggers the actualization of the preview
     * @param ignoreLazyPreviewSetting If set true UpdatePreview will always
     * generate the preview for all time steps. If set to false, UpdatePreview
     * will regard the setting specified by the constructor.
     * To define the update generation for time steps implement DoUpdatePreview.
     * To alter what should be done directly before or after the update of the preview,
     * reimplement UpdatePrepare() or UpdateCleanUp().*/
    void UpdatePreview(bool ignoreLazyPreviewSetting = false);

    /** Indicate if currently UpdatePreview is triggered (true) or not (false).*/
    bool IsUpdating() const;

  protected:
    mitk::ToolCommand::Pointer m_ProgressCommand;

    /** Member is always called if GetSegmentationInput() has changed
     * (e.g. because a new ROI was defined, or on activation) to give derived
     * classes the posibility to initiate their state accordingly.
     * Reimplement this function to implement special behavior.
     */
    virtual void InitiateToolByInput();

    virtual void UpdatePrepare();
    virtual void UpdateCleanUp();

    /** This function does the real work. Here the preview for a given
     * input image should be computed and stored in the also passed
     * preview image at the passed time step.
     */
    virtual void DoUpdatePreview(const Image* inputAtTimeStep, Image* previewImage, TimeStepType timeStep) = 0;

    AutoSegmentationWithPreviewTool(bool lazyDynamicPreviews = false); // purposely hidden
    AutoSegmentationWithPreviewTool(bool lazyDynamicPreviews, const char* interactorType, const us::Module* interactorModule = nullptr); // purposely hidden

    ~AutoSegmentationWithPreviewTool() override;

    /** Returns the image that contains the preview of the current segmentation.
     * Returns null if the node is not set or does not contain an image.*/
    Image* GetPreviewSegmentation();
    DataNode* GetPreviewSegmentationNode();

    /** Returns the input that should be used for any segmentation/preview or tool update.
     * It is either the data of ReferenceDataNode itself or a part of it defined by a ROI mask
     * provided by the tool manager. Derived classes should regard this as the relevant
     * input data for any processing.
     * Returns null if the node is not set or does not contain an image.*/
    const Image* GetSegmentationInput() const;

    /** Returns the image that is provided by the ReferenceDataNode.
     * Returns null if the node is not set or does not contain an image.*/
    const Image* GetReferenceData() const;

    /** Resets the preview node so it is empty and ready to be filled by the tool*/
    void ResetPreviewNode();

    TimePointType GetLastTimePointOfUpdate() const;

    itkSetObjectMacro(WorkingPlaneGeometry, PlaneGeometry);
    itkGetConstObjectMacro(WorkingPlaneGeometry, PlaneGeometry);

  private:
    void TransferImageAtTimeStep(const Image* sourceImage, Image* destinationImage, const TimeStepType timeStep);

    void CreateResultSegmentationFromPreview();

    void OnRoiDataChanged();
    void OnTimePointChanged();

    /** Node that containes the preview data generated and managed by this class or derived ones.*/
    DataNode::Pointer m_PreviewSegmentationNode;
    /** The reference data recieved from ToolManager::GetReferenceData when tool was activated.*/
    DataNode::Pointer m_ReferenceDataNode;
    /** Node that containes the data that should be used as input for any auto segmentation. It might
     * be the same like m_ReferenceDataNode (if no ROI is set) or a sub region (if ROI is set).*/
    DataNode::Pointer m_SegmentationInputNode;

    /** Indicates if Accepting the threshold should transfer/create the segmentations
     of all time steps (true) or only of the currently selected timepoint (false).*/
    bool m_CreateAllTimeSteps = false;

    /** Indicates if the tool should kept active after accepting the segmentation or not.*/
    bool m_KeepActiveAfterAccept = false;

    /** Relevant if the working data / preview image has multiple time steps (dynamic segmentations).
     * This flag has to be set by derived classes accordingly to there way to generate dynamic previews.
     * If LazyDynamicPreview is true, the tool generates only the preview for the current time step.
     * Therefore it always has to update the preview if current time point has changed and it has to (re)compute
     * all timeframes if ConfirmSegmentation() is called.*/
    bool m_LazyDynamicPreviews = false;

    bool m_IsTimePointChangeAware = true;

    TimePointType m_LastTimePointOfUpdate = 0.;

    bool m_IsUpdating = false;

    /** This variable indicates if for the tool a working plane geometry is defined.
     * If a working plane is defined the tool will only work an the slice of the input
     * and the segmentation. Thus only the relevant input slice will be passed to
     * DoUpdatePreview(...) and only the relevant slice of the preview will be transfered when
     * ConfirmSegmentation() is called.*/
    PlaneGeometry::Pointer m_WorkingPlaneGeometry;
  };

} // namespace

#endif
