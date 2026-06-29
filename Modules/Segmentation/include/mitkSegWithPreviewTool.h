/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSegWithPreviewTool_h
#define mitkSegWithPreviewTool_h

#include <mitkTool.h>
#include <mitkCommon.h>
#include <mitkDataNode.h>
#include <mitkToolCommand.h>
#include <MitkSegmentationExports.h>

namespace mitk
{
  /**
  \brief Base class for any auto segmentation tool that provides a preview of the new segmentation.

  This tool class implements a lot basic logic to handle auto segmentation tools with preview,
  Time point and ROI support. Derived classes will ask to update the segmentation preview if needed
  (e.g. because the ROI or the current time point has changed) or because derived tools
  indicated the need to update themselves.
  This class also takes care to properly transfer a confirmed preview into the segmentation
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

    /** \brief Preview tools are SEMIAUTOMATIC by default (the user supplies seeds/parameters).
     * Fully automatic preview tools (e.g. TotalSegmentator) override this again to AUTOMATIC. */
    mitk::Label::AlgorithmType GetAlgorithmType() const override;

    /**
     * \brief Confirms the current preview segmentation and transfers it to the working image.
     *
     * The transfer behavior is controlled by LabelTransferScope, LabelTransferMode,
     * MergeStyle, and OverwriteStyle.
     */
    void ConfirmSegmentation();

    /** \brief If true, the preview is computed for all time steps at once. */
    itkSetMacro(CreateAllTimeSteps, bool);
    itkGetMacro(CreateAllTimeSteps, bool);
    itkBooleanMacro(CreateAllTimeSteps);

    /** \brief If true, the tool stays active after the segmentation is accepted. */
    itkSetMacro(KeepActiveAfterAccept, bool);
    itkGetMacro(KeepActiveAfterAccept, bool);
    itkBooleanMacro(KeepActiveAfterAccept);

    /** \brief If true, the tool reacts to time point changes and updates the preview. */
    itkSetMacro(IsTimePointChangeAware, bool);
    itkGetMacro(IsTimePointChangeAware, bool);
    itkBooleanMacro(IsTimePointChangeAware);

    /** \brief If true, the preview is reset to empty on activation. */
    itkSetMacro(ResetsToEmptyPreview, bool);
    itkGetMacro(ResetsToEmptyPreview, bool);
    itkBooleanMacro(ResetsToEmptyPreview);

    /** \brief If true, a special color is used for the preview rendering. */
    itkSetMacro(UseSpecialPreviewColor, bool);
    itkGetMacro(UseSpecialPreviewColor, bool);
    itkBooleanMacro(UseSpecialPreviewColor);

    /** \brief If true, a confirmation dialog is shown before deactivating the tool. */
    itkSetMacro(RequestDeactivationConfirmation, bool);
    itkGetMacro(RequestDeactivationConfirmation, bool);
    itkBooleanMacro(RequestDeactivationConfirmation);

    /**
     * \brief Returns the special preview color.
     * \return The RGB color used for preview rendering.
     */
    Color GetSpecialPreviewColor() const;

    /**
     * \brief Sets how the preview content is merged into the working image upon confirmation.
     * \param[in] mergeStyle The merge style (Replace or Merge).
     * \note Does not modify MTime to avoid unnecessary preview updates.
     */
    void SetMergeStyle(MultiLabelSegmentation::MergeStyle mergeStyle);
    itkGetMacro(MergeStyle, MultiLabelSegmentation::MergeStyle);

    /**
     * \brief Sets whether locked labels should be overwritten during transfer.
     * \param[in] overwriteStyle The overwrite style (RegardLocks or IgnoreLocks).
     * \note Does not modify MTime to avoid unnecessary preview updates.
     */
    void SetOverwriteStyle(MultiLabelSegmentation::OverwriteStyle overwriteStyle);
    itkGetMacro(OverwriteStyle, MultiLabelSegmentation::OverwriteStyle);

    /**
     * \brief Defines which labels from the preview are transferred on confirmation.
     */
    enum class LabelTransferScope
    {
      ActiveLabel,    /**< Only the currently active/selected label is transferred. */
      SelectedLabels, /**< Only the labels in the SelectedLabels list are transferred. */
      AllLabels       /**< All labels of the preview segmentation are transferred. */
    };

    /**
     * \brief Sets which labels from the preview are transferred on confirmation.
     * \param[in] labelTransferScope The scope of labels to transfer.
     * \note Does not modify MTime to avoid unnecessary preview updates.
     */
    void SetLabelTransferScope(LabelTransferScope labelTransferScope);
    itkGetMacro(LabelTransferScope, LabelTransferScope);

    /** \brief Type for the vector of selected label values. */
    using SelectedLabelVectorType = std::vector<Label::PixelType>;

    /**
     * \brief Sets the labels to transfer from preview to working image on confirmation.
     *
     * Used when LabelTransferScope is ActiveLabel or SelectedLabels.
     *
     * \param[in] labelsToTransfer Vector of label values from the preview image.
     * \pre If LabelTransferScope is ActiveLabel, exactly one label must be provided.
     */
    void SetSelectedLabels(const SelectedLabelVectorType& labelsToTransfer);
    itkGetMacro(SelectedLabels, SelectedLabelVectorType);

    /**
     * \brief Defines how labels are transferred from preview to the working image.
     */
    enum class LabelTransferMode
    {
      MapLabel, /**< Preview labels are mapped to the active label in the working image. */
      AddLabel  /**< Preview labels are added as new labels to the working image. */
    };

    /**
     * \brief Sets how labels are transferred from preview to the working image.
     * \param[in] labelTransferMode The label transfer mode.
     * \note Does not modify MTime to avoid unnecessary preview updates.
     */
    void SetLabelTransferMode(LabelTransferMode labelTransferMode);
    itkGetMacro(LabelTransferMode, LabelTransferMode);

    bool CanHandle(const BaseData* referenceData, const BaseData* workingData) const override;

    /**
     * \brief Triggers the update of the preview segmentation.
     *
     * \param[in] ignoreLazyPreviewSetting If true, the preview is generated for all time steps.
     *            If false, the lazy preview setting from the constructor is respected.
     *
     * To define the update generation for time steps, implement DoUpdatePreview().
     * To alter behavior before/after the preview update, reimplement UpdatePrepare() or UpdateCleanUp().
     */
    void UpdatePreview(bool ignoreLazyPreviewSetting = false);

    /** Indicate if currently UpdatePreview is triggered (true) or not (false).*/
    bool IsUpdating() const;

    /**
     * \brief Returns the name of the currently selected segmentation node.
     * \return The name of the segmentation node or an empty string if none is selected.
     */
    std::string GetCurrentSegmentationName();

    /**
     * \brief Returns the currently selected segmentation node.
     * \return A mitk::DataNode which contains a segmentation image.
     */
    virtual DataNode* GetTargetSegmentationNode() const;
    MultiLabelSegmentation* GetTargetSegmentation() const;

    /** Returns the image that contains the preview of the current segmentation.
     * Returns null if the node is not set or does not contain an image.*/
    MultiLabelSegmentation* GetPreviewSegmentation();
    const MultiLabelSegmentation* GetPreviewSegmentation() const;
    DataNode* GetPreviewSegmentationNode();

    /** Sets the opacity (0.0 - 1.0) used to render the preview
        segmentation. The value persists across preview regenerations. */
    void SetPreviewOpacity(float opacity);
    itkGetConstMacro(PreviewOpacity, float);

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
     * classes the possibility to initiate their state accordingly.
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

    /** This member function offers derived classes the possibility to alter what should
    happen directly after the Confirmation of the preview is performed. It is called by
    ConfirmSegmentation. Default implementation does nothing.*/
    virtual void ConfirmCleanUp();

    using LabelMappingType = std::vector<std::pair<Label::PixelType, Label::PixelType> >;

    /**
     * \brief Called before the preview content is transferred to the segmentation on confirmation.
     *
     * Default implementation ensures that all labels to be transferred exist in the segmentation.
     * Missing labels are added by cloning label information from the preview.
     *
     * \param[in] labelMapping The mapping used for transferring labels from preview to result.
     */
    virtual void PreparePreviewToResultTransfer(const LabelMappingType& labelMapping);

    /**
     * \brief Copies label information (not pixel content) from one segmentation to another.
     *
     * \param[in] labelMapping Indicates which labels to copy and optional label value remapping.
     * \param[in] source The source segmentation containing the label information.
     * \param[in,out] target The target segmentation that receives the label information.
     */
    static void TransferLabelInformation(const LabelMappingType& labelMapping,
      const mitk::MultiLabelSegmentation* source, mitk::MultiLabelSegmentation* target);

    /** This function does the real work. Here the preview for a given
     * input image should be computed and stored in the also passed
     * preview image at the passed time step.
     * It also provides the current/old segmentation at the time point,
     * which can be used, if the preview depends on the the segmentation so far.
     */
    virtual void DoUpdatePreview(const Image* inputAtTimeStep, const Image* oldSegAtTimeStep, MultiLabelSegmentation* previewImage, TimeStepType timeStep) = 0;

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

    MultiLabelSegmentation::LabelValueType GetActiveLabelValueOfPreview() const;

    itkGetConstMacro(UserDefinedActiveLabel, Label::PixelType);

    itkSetObjectMacro(WorkingPlaneGeometry, PlaneGeometry);
    itkGetConstObjectMacro(WorkingPlaneGeometry, PlaneGeometry);

    bool ConfirmBeforeDeactivation() override;

    /** Called when the selected time point changes (subscribed to ToolManager::SelectedTimePointChanged
     * by the base class on Activated()). The default implementation calls UpdatePreview() if the tool
     * is in lazy-dynamic mode or has a static segmentation on a dynamic reference image. Derived tools
     * may override this hook to implement different time-point change semantics, for example to discard
     * unconfirmed state when the bound time step is no longer current.*/
    virtual void OnTimePointChanged();

  private:
    void TransferSegmentationsAtTimeStep(const MultiLabelSegmentation* sourceSeg, MultiLabelSegmentation* destinationSeg, const TimeStepType timeStep, const LabelMappingType& labelMapping);

    void CreateResultSegmentationFromPreview();

    void OnRoiDataChanged();

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

    /** Node that contains the preview data generated and managed by this class or derived ones.*/
    DataNode::Pointer m_PreviewSegmentationNode;
    /** The reference data received from ToolManager::GetReferenceData when tool was activated.*/
    DataNode::Pointer m_ReferenceDataNode;
    /** Node that contains the data that should be used as input for any auto segmentation. It might
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
     * DoUpdatePreview(...) and only the relevant slice of the preview will be transferred when
     * ConfirmSegmentation() is called.*/
    PlaneGeometry::Pointer m_WorkingPlaneGeometry;

    /** This variable controls how the label pixel content of the preview should be transferred into the
      segmentation- For more details of the behavior see documentation of MultiLabelSegmentation::MergeStyle. */
    MultiLabelSegmentation::MergeStyle m_MergeStyle = MultiLabelSegmentation::MergeStyle::Replace;
    /** This variable controls how the label pixel content of the preview should be transferred into the
      segmentation- For more details of the behavior see documentation of MultiLabelSegmentation::OverwriteStyle. */
    MultiLabelSegmentation::OverwriteStyle m_OverwriteStyle = MultiLabelSegmentation::OverwriteStyle::RegardLocks;

    LabelTransferScope m_LabelTransferScope = LabelTransferScope::ActiveLabel;
    SelectedLabelVectorType m_SelectedLabels = {};

    LabelTransferMode m_LabelTransferMode = LabelTransferMode::MapLabel;

    bool m_IsPreviewGenerated = false;

    /** Opacity (0.0 - 1.0) used when rendering the preview segmentation.
     * Kept as tool state so a user-defined value survives the opacity
     * reset that happens whenever the preview node is regenerated
     * (see ResetPreviewNode).*/
    float m_PreviewOpacity = 1.0f / 3.0f;

    /** This variable tracks if there should be a user-confirmation before a tool is deactivated or not.
     * Call RequestDeactivationConfirmationOn() in the tool class to avail this feature.
     */
    bool m_RequestDeactivationConfirmation = false;
  };

} // namespace

#endif
