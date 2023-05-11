/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSegWithPreviewTool_h
#define mitkSegWithPreviewTool_h

#include "mitkTool.h"
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
  class MITKSEGMENTATION_EXPORT SegWithPreviewTool : public Tool
  {
  public:

    mitkClassMacro(SegWithPreviewTool, Tool);

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

    itkSetMacro(ResetsToEmptyPreview, bool);
    itkGetMacro(ResetsToEmptyPreview, bool);
    itkBooleanMacro(ResetsToEmptyPreview);

    itkSetMacro(UseSpecialPreviewColor, bool);
    itkGetMacro(UseSpecialPreviewColor, bool);
    itkBooleanMacro(UseSpecialPreviewColor);

    /*itk macro was not used on purpose, to aviod the change of mtime.*/
    void SetMergeStyle(MultiLabelSegmentation::MergeStyle mergeStyle);
    itkGetMacro(MergeStyle, MultiLabelSegmentation::MergeStyle);

    /*itk macro was not used on purpose, to aviod the change of mtime.*/
    void SetOverwriteStyle(MultiLabelSegmentation::OverwriteStyle overwriteStyle);
    itkGetMacro(OverwriteStyle, MultiLabelSegmentation::OverwriteStyle);

    enum class LabelTransferScope
    {
      ActiveLabel, //Only the selected label will be transfered from the preview segmentation
                   //to the result segmentation.
                   //If this mode is selected the class expects that GetSelectedLabels indicate
                   //the label in the preview.
      SelectedLabels, //The labels defined as selected labels will be transfered.
      AllLabels //Transfer all labels of the preview
    };
    /*itk macro was not used on purpose, to aviod the change of mtime.*/
    void SetLabelTransferScope(LabelTransferScope labelTransferScope);
    itkGetMacro(LabelTransferScope, LabelTransferScope);

    using SelectedLabelVectorType = std::vector<Label::PixelType>;
    /** Specifies the labels that should be transfered form preview to the working image,
      if the segmentation is confirmed. The setting will be used, if LabelTransferScope is set to "ActiveLabel"
      or "SelectedLabels".
      @remark If the LabelTransferScope=="ActiveLabel", the class expects only one label to be selected.
      @remark The selected label IDs corespond to the labels of the preview image.*/
    void SetSelectedLabels(const SelectedLabelVectorType& labelsToTransfer);
    itkGetMacro(SelectedLabels, SelectedLabelVectorType);

    enum class LabelTransferMode
    {
      MapLabel, //Only the active label will be transfered from preview to segmentation.
      AddLabel //The labels defined as selected labels will be transfered.
    };
    /*itk macro was not used on purpose, to aviod the change of mtime.*/
    void SetLabelTransferMode(LabelTransferMode labelTransferMode);
    itkGetMacro(LabelTransferMode, LabelTransferMode);

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

    /**
   * @brief Gets the name of the currently selected segmentation node
   * @return the name of the segmentation node or an empty string if
   *         none is selected
   */
    std::string GetCurrentSegmentationName();

    /**
     * @brief Returns the currently selected segmentation node
     * @return a mitk::DataNode which contains a segmentation image
     */
    virtual DataNode* GetTargetSegmentationNode() const;
    LabelSetImage* GetTargetSegmentation() const;

    /** Returns the image that contains the preview of the current segmentation.
     * Returns null if the node is not set or does not contain an image.*/
    LabelSetImage* GetPreviewSegmentation();
    const LabelSetImage* GetPreviewSegmentation() const;
    DataNode* GetPreviewSegmentationNode();

  protected:
    ToolCommand::Pointer m_ProgressCommand;

    SegWithPreviewTool(bool lazyDynamicPreviews = false); // purposely hidden
    SegWithPreviewTool(bool lazyDynamicPreviews, const char* interactorType, const us::Module* interactorModule = nullptr); // purposely hidden

    ~SegWithPreviewTool() override;

    const char* GetGroup() const override;

    /** Helper that extracts the image for the passed timestep, if the image has multiple time steps.*/
    static Image::ConstPointer GetImageByTimeStep(const Image* image, TimeStepType timestep);
    /** Helper that extracts the image for the passed timestep, if the image has multiple time steps.*/
    static Image::Pointer GetImageByTimeStep(Image* image, TimeStepType timestep);
    /** Helper that extracts the image for the passed time point, if the image has multiple time steps.*/
    static Image::ConstPointer GetImageByTimePoint(const Image* image, TimePointType timePoint);

    void EnsureTargetSegmentationNodeInDataStorage() const;

    /** Member is always called if GetSegmentationInput() has changed
     * (e.g. because a new ROI was defined, or on activation) to give derived
     * classes the posibility to initiate their state accordingly.
     * Reimplement this function to implement special behavior.
     */
    virtual void InitiateToolByInput();

    /** This member function offers derived classes the possibility to alter what should
    happen directly before the update of the preview is performed. It is called by
    UpdatePreview. Default implementation does nothing.*/
    virtual void UpdatePrepare();

    /** This member function offers derived classes the possibility to alter what should
    happen directly after the update of the preview is performed. It is called by
    UpdatePreview. Default implementation does nothing.*/
    virtual void UpdateCleanUp();

    using LabelMappingType = std::vector<std::pair<Label::PixelType, Label::PixelType> >;

    /** This member function offers derived classes the possibility to alter what should
    happen directly before the content of the preview is transfered to the segmentation,
    when the segmentation is confirmed. It is called by CreateResultSegmentationFromPreview.
    Default implementation ensure that all labels that will be transfered, exist in the
    segmentation. If they are not existing before the transfer, the will be added by
    cloning the label information of the preview.
    @param labelMapping the mapping that should be used for transfering the labels.
    */
    virtual void PreparePreviewToResultTransfer(const LabelMappingType& labelMapping);

    static void TransferLabelInformation(const LabelMappingType& labelMapping,
      const mitk::LabelSetImage* source, mitk::LabelSetImage* target);

    /**Helper function that can be used to move the content of an LabelSetImage (the pixels of the active source layer and the labels).
     This is e.g. helpfull if you generate an LabelSetImage content in DoUpdatePreview and you want to transfer it into the preview image.*/
    static void TransferLabelSetImageContent(const LabelSetImage* source, LabelSetImage* target, TimeStepType timeStep);

    /** This function does the real work. Here the preview for a given
     * input image should be computed and stored in the also passed
     * preview image at the passed time step.
     * It also provides the current/old segmentation at the time point,
     * which can be used, if the preview depends on the the segmenation so far.
     */
    virtual void DoUpdatePreview(const Image* inputAtTimeStep, const Image* oldSegAtTimeStep, LabelSetImage* previewImage, TimeStepType timeStep) = 0;

    /** Returns the input that should be used for any segmentation/preview or tool update.
     * It is either the data of ReferenceDataNode itself or a part of it defined by a ROI mask
     * provided by the tool manager. Derived classes should regard this as the relevant
     * input data for any processing.
     * Returns null if the node is not set or does not contain an image.*/
    const Image* GetSegmentationInput() const;

    /** Returns the image that is provided by the ReferenceDataNode.
     * Returns null if the node is not set or does not contain an image.*/
    const Image* GetReferenceData() const;

    /** Resets the preview node so it is empty and ready to be filled by the tool
    @remark Calling this function will generate a new preview image, and the old
    might be invalidated. Therefore this function should not be used within the
    scope of UpdatePreview (m_IsUpdating == true).*/
    void ResetPreviewNode();

    /** Resets the complete content of the preview image. The instance of the preview image and its settings
    * stay the same.*/
    void ResetPreviewContent();

    /** Resets only the image content of the specified timeStep of the preview image. If the preview image or the specified
    time step does not exist, nothing happens.*/
    void ResetPreviewContentAtTimeStep(unsigned int timeStep);

    TimePointType GetLastTimePointOfUpdate() const;

    LabelSetImage::LabelValueType GetActiveLabelValueOfPreview() const;

    itkGetConstMacro(UserDefinedActiveLabel, Label::PixelType);

    itkSetObjectMacro(WorkingPlaneGeometry, PlaneGeometry);
    itkGetConstObjectMacro(WorkingPlaneGeometry, PlaneGeometry);

  private:
    void TransferImageAtTimeStep(const Image* sourceImage, Image* destinationImage, const TimeStepType timeStep, const LabelMappingType& labelMapping);

    void CreateResultSegmentationFromPreview();

    void OnRoiDataChanged();
    void OnTimePointChanged();

    /**Internal helper that ensures that the stored active label is up to date.
     This is a fix for T28131 / T28986. It should be refactored if T28524 is being worked on.
     On the long run, the active label will be communicated/set by the user/toolmanager as a
     state of the tool and the tool should react accordingly (like it does for other external
     state changes).
     @return indicates if the label has changed (true) or not.
     */
    bool EnsureUpToDateUserDefinedActiveLabel();

    /**Returns that label mapping between preview segmentation (first element of pair) and
     result segmentation (second element of pair).
     The content depends on the settings of LabelTransferMode and LabelTransferScope*/
    LabelMappingType GetLabelMapping() const;

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

    /** Controls if ResetPreviewNode generates an empty content (true) or clones the current
    segmentation (false).*/
    bool m_ResetsToEmptyPreview = false;

    /** Controls if for the preview of the active label a special preview color is used.
     * If set to false, coloring will stay in the preview like it is in the working image.*/
    bool m_UseSpecialPreviewColor = true;

    TimePointType m_LastTimePointOfUpdate = 0.;

    bool m_IsUpdating = false;

    Label::PixelType m_UserDefinedActiveLabel = 1;

    /** This variable indicates if for the tool a working plane geometry is defined.
     * If a working plane is defined the tool will only work an the slice of the input
     * and the segmentation. Thus only the relevant input slice will be passed to
     * DoUpdatePreview(...) and only the relevant slice of the preview will be transfered when
     * ConfirmSegmentation() is called.*/
    PlaneGeometry::Pointer m_WorkingPlaneGeometry;

    /** This variable controles how the label pixel content of the preview should be transfered into the
      segmentation- For more details of the behavior see documentation of MultiLabelSegmentation::MergeStyle. */
    MultiLabelSegmentation::MergeStyle m_MergeStyle = MultiLabelSegmentation::MergeStyle::Replace;
    /** This variable controles how the label pixel content of the preview should be transfered into the
      segmentation- For more details of the behavior see documentation of MultiLabelSegmentation::OverwriteStyle. */
    MultiLabelSegmentation::OverwriteStyle m_OverwriteStyle = MultiLabelSegmentation::OverwriteStyle::RegardLocks;

    LabelTransferScope m_LabelTransferScope = LabelTransferScope::ActiveLabel;
    SelectedLabelVectorType m_SelectedLabels = {};

    LabelTransferMode m_LabelTransferMode = LabelTransferMode::MapLabel;
  };

} // namespace

#endif
