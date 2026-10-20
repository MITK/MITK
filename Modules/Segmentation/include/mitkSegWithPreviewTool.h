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

#include <exception>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <thread>

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
     *
     * If the preview has to be updated first, for further time steps or a changed
     * active label, the transfer follows the update: at once, or once the work of
     * the update has run off the GUI thread (see RunOffGUIThread()). A transfer
     * that follows such work reports a failure through ErrorMessage. If the
     * update is cancelled or fails, nothing is transferred.
     *
     * \throw mitk::Exception if an update of the preview or other work off the
     *        GUI thread is running.
     */
    void ConfirmSegmentation();

    /** \brief Posts a function to the GUI thread, to run there after the current event.
     *
     * Must be callable from any thread.
     */
    using GUIThreadPoster = std::function<void(std::function<void()>)>;

    /**
     * \brief Lets the tool run work off the GUI thread, see RunOffGUIThread().
     *
     * A GUI installs it, as the tool does not know the GUI toolkit. Without it,
     * work runs on the calling thread, which is what tests and command-line
     * tools need.
     */
    void SetGUIThreadPoster(GUIThreadPoster poster);

    /** \brief If true, the preview is computed for all time steps at once. */
    itkSetMacro(CreateAllTimeSteps, bool);
    itkGetMacro(CreateAllTimeSteps, bool);
    itkBooleanMacro(CreateAllTimeSteps);

    /** \brief If true, the tool stays active after the segmentation is accepted. */
    itkSetMacro(KeepActiveAfterAccept, bool);
    itkGetMacro(KeepActiveAfterAccept, bool);
    itkBooleanMacro(KeepActiveAfterAccept);

    /** \brief If true, the tool reacts to time point changes and updates the preview.
     *
     * If false, the preview of a tool with lazy dynamic previews stays at the time point of
     * its last update, and ConfirmSegmentation() transfers that time point.
     */
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

    /**
     * \brief Whether the labels of all groups of the preview are transferred, or only those of its active group.
     *
     * If true, a label of the preview goes into the group of the segmentation
     * that has the index of its group in the preview, and groups that the
     * segmentation lacks are added to it. Otherwise the labels of the active
     * group of the preview go into the active group of the segmentation.
     */
    itkGetConstMacro(TransfersAllPreviewGroups, bool);

    bool CanHandle(const BaseData* referenceData, const BaseData* workingData) const override;

    /**
     * \brief Triggers the update of the preview segmentation.
     *
     * \param[in] ignoreLazyPreviewSetting If true, the preview is generated for all time steps.
     *            If false, the lazy preview setting from the constructor is respected.
     *
     * To define the update generation for time steps, implement DoUpdatePreview().
     * To alter behavior before/after the preview update, reimplement UpdatePrepare() or UpdateCleanUp().
     *
     * An update whose DoUpdatePreview() runs work off the GUI thread (see
     * RunOffGUIThread()) returns before it is done. It ends with
     * CurrentlyBusy reporting false. A call while an update or other work off
     * the GUI thread runs does nothing.
     */
    void UpdatePreview(bool ignoreLazyPreviewSetting = false);

    /** Indicate if currently UpdatePreview is triggered (true) or not (false).*/
    bool IsUpdating() const;

    /**
     * \brief Indicate whether an update of the preview can be stopped again.
     *
     * ToolCommand turns a cancel request into
     * itk::ProcessObject::SetAbortGenerateData() on the filter it observes, so
     * only a tool whose filter reads that flag, whether directly or through
     * itk::ProgressReporter, can honour one. Say so by overriding this, and the
     * user is offered a cancel button while the preview is being computed.
     *
     * Claiming it without being able to act on it is worse than not offering
     * it: the request is read by nothing, and the notification it belongs to
     * cannot be dismissed once it has been made.
     *
     * \return False, unless a derived tool overrides this.
     */
    virtual bool IsCancelable() const;

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

    /** Sets whether the preview segmentation is rendered. The value persists
        across preview regenerations. */
    void SetPreviewVisibility(bool visible);
    itkGetConstMacro(PreviewVisibility, bool);

  protected:
    ToolCommand::Pointer m_ProgressCommand;

    SegWithPreviewTool(bool lazyDynamicPreviews = false); // purposely hidden
    SegWithPreviewTool(bool lazyDynamicPreviews, const char* interactorType, const us::Module* interactorModule = nullptr); // purposely hidden

    ~SegWithPreviewTool() override;

    const char* GetGroup() const override;

    /** \brief If true (default), CanHandle() rejects segmentations without labels.
     *
     * Tools that only add their own result labels and never write into an
     * existing one switch this off, so hosts can offer them on segmentations
     * that do not contain any labels yet. A segmentation without groups is
     * rejected regardless, as the preview needs a target group.
     */
    itkSetMacro(RequiresExistingLabels, bool);
    itkGetConstMacro(RequiresExistingLabels, bool);
    itkBooleanMacro(RequiresExistingLabels);

    /** \brief If true (default), CanHandle() rejects reference images that are not
     * single-component scalar images (see Tool::IsSingleComponentScalarImage()).
     *
     * Tools that evaluate the intensities of the reference image cannot process
     * RGB, RGBA or vector images. Tools that only work on the segmentation
     * (e.g. PickingTool) switch this off.
     */
    itkSetMacro(RequiresScalarReference, bool);
    itkGetConstMacro(RequiresScalarReference, bool);
    itkBooleanMacro(RequiresScalarReference);

    /** \brief If true, CanHandle() rejects reference images with less than three
     * dimensions.
     *
     * For tools whose algorithm is fixed to volumes. A 2D+t image qualifies,
     * as each of its time steps is a single-slice volume, a static 2D image
     * does not.
     */
    itkSetMacro(RequiresVolumetricReference, bool);
    itkGetConstMacro(RequiresVolumetricReference, bool);
    itkBooleanMacro(RequiresVolumetricReference);

    /** \brief If true, CanHandle() rejects segmentations whose geometry differs from that
     * of the reference image, such as a segmentation of a part of the image.
     *
     * For tools that compute their result on the grid of the reference image.
     */
    itkSetMacro(RequiresReferenceGeometry, bool);
    itkGetConstMacro(RequiresReferenceGeometry, bool);
    itkBooleanMacro(RequiresReferenceGeometry);

    /** \brief See GetTransfersAllPreviewGroups(). False by default.
     *
     * Only for tools whose preview holds nothing but their results, as the
     * preview starts as a clone of the segmentation with all its labels.
     */
    itkSetMacro(TransfersAllPreviewGroups, bool);
    itkBooleanMacro(TransfersAllPreviewGroups);

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
    UpdatePreview. Default implementation does nothing.
    It is also called for an update whose work off the GUI thread outlived a
    deactivation of the tool (see RunOffGUIThread()).*/
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
     * Missing labels are added by cloning label information from the preview, into the groups
     * that GetTransfersAllPreviewGroups() describes.
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
     * \param[in] intoSourceGroups If true, a label goes into the group of the target that has the
     *            index of its group in the source, and missing groups are added to the target.
     *            Otherwise all labels go into the active group of the target.
     */
    static void TransferLabelInformation(const LabelMappingType& labelMapping,
      const mitk::MultiLabelSegmentation* source, mitk::MultiLabelSegmentation* target, bool intoSourceGroups = false);

    /** This function does the real work. Here the preview for a given
     * input image should be computed and stored in the also passed
     * preview image at the passed time step.
     * It also provides the current/old segmentation at the time point,
     * which can be used, if the preview depends on the the segmentation so far.
     *
     * Work that takes long can run off the GUI thread: prepare it here, and
     * call RunOffGUIThread() with it and with what writes its result into the
     * preview.
     */
    virtual void DoUpdatePreview(const Image* inputAtTimeStep, const Image* oldSegAtTimeStep, MultiLabelSegmentation* previewImage, TimeStepType timeStep) = 0;

    /**
     * \brief Runs work on a thread of its own and continues on the GUI thread.
     *
     * For work that takes long, like the calls of a Python worker, so that the
     * GUI thread keeps processing events meanwhile instead of waiting. \p work
     * must not touch data that the GUI thread uses, like images on display or
     * the preview. \p done runs on the GUI thread once \p work has finished,
     * with what \p work threw, or null, and can throw like DoUpdatePreview().
     *
     * Called from DoUpdatePreview(), \p work starts once DoUpdatePreview() has
     * returned, and the update goes on with the next time step, or ends, once
     * \p done has run. IsUpdating() stays true until then. Otherwise \p work
     * starts at once. What \p done throws, and what the end of the update
     * throws, is reported through ErrorMessage.
     *
     * The tool can be deactivated, and even activated anew, before \p done
     * runs. \p done still runs, and so does UpdateCleanUp(); the update then
     * ends without further time steps and without a confirmation following
     * it. \p done therefore works on what DoUpdatePreview() was given or what
     * it captured, not on GetPreviewSegmentation() or the data of the
     * ToolManager, which may belong to the new activation.
     *
     * Work runs one at a time. Without a GUI thread poster (see
     * SetGUIThreadPoster()), \p work and \p done run at once on the calling
     * thread.
     *
     * \throw mitk::Exception if other work has not finished yet, or if
     *        DoUpdatePreview() calls this more than once. Called from
     *        DoUpdatePreview(), the first cannot happen, as no update starts
     *        while other work runs.
     */
    void RunOffGUIThread(std::function<void()> work, std::function<void(std::exception_ptr)> done);

    /**
     * \brief Runs a function on the GUI thread, for example to report the progress of work.
     *
     * Callable from any thread. Without a GUI thread poster, \p function runs at once.
     */
    void PostToGUIThread(std::function<void()> function);

    /**
     * \brief Whether work started with RunOffGUIThread() has not reached its \p done yet.
     *
     * False while \p done runs, so \p done can start further work.
     */
    bool IsRunningOffGUIThread() const;

    /**
     * \brief Waits until work started with RunOffGUIThread() has finished.
     *
     * For the destructor of a derived tool whose work uses members of the tool:
     * make the work stop, then wait. Once the tool is destroyed, the \p done of
     * the work does not run.
     */
    void WaitForWorkOffGUIThread();

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

    /** Removes all labels of the preview image together with their pixel content.
    * For a tool that replaces the labels of the preview on every update, as
    * opposed to writing into the labels it inherits from the segmentation.
    * Nothing happens without a preview image.*/
    void RemoveAllPreviewLabels();

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
     * unconfirmed state when the bound time step is no longer current.
     * A change while an update or other work runs off the GUI thread is not applied by the default
     * implementation, as UpdatePreview() does nothing then; such a tool remembers the change and
     * applies it once the work has ended.*/
    virtual void OnTimePointChanged();

  private:
    struct UpdatePass;

    struct DeferredWork
    {
      std::function<void()> Work;
      std::function<void(std::exception_ptr)> Done;
    };

    /** Starts an update of the preview and runs \p then once it has ended. */
    void StartUpdate(bool ignoreLazyPreviewSetting, std::function<void()> then);

    /** Updates the remaining time steps of the running update, until one of them runs work off the GUI thread. */
    void ContinueUpdate();

    /** Reports what a time step of the running update threw, as the base class reports it. */
    void HandleUpdateError(std::exception_ptr error);

    void EndUpdate();

    void StartWork(std::function<void()> work, std::function<void(std::exception_ptr)> done);

    GUIThreadPoster GetGUIThreadPoster() const;

    /**
     * The part of ConfirmSegmentation() that follows the update of the preview.
     *
     * \param timePoint The time point that the update computed, if the transfer follows one.
     */
    void FinishConfirmation(std::optional<TimePointType> timePoint = {});

    void TransferSegmentationsAtTimeStep(const MultiLabelSegmentation* sourceSeg, MultiLabelSegmentation* destinationSeg, const TimeStepType timeStep, const LabelMappingType& labelMapping);

    /** \param timePoint The time point to transfer instead of the one the tool would choose. */
    void CreateResultSegmentationFromPreview(std::optional<TimePointType> timePoint = {});

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
     * Therefore it always has to update the preview if current time point has changed and it has to compute
     * all timeframes if ConfirmSegmentation() is called while all time steps are to be created.*/
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

    /** The running update, from UpdatePreview() until it has ended. */
    std::unique_ptr<UpdatePass> m_UpdatePass;

    /** Set while DoUpdatePreview() runs, whose work RunOffGUIThread() defers. */
    bool m_DefersWork = false;
    std::optional<DeferredWork> m_DeferredWork;

    mutable std::mutex m_GUIThreadPosterMutex;
    GUIThreadPoster m_GUIThreadPoster;

    std::thread m_WorkThread;
    bool m_WorkRunning = false;

    /** Expires with the tool, so that what is posted to the GUI thread afterwards does nothing. */
    std::shared_ptr<int> m_Alive = std::make_shared<int>(0);

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

    /** Indicates if the preview holds content that has not been transferred into the
     * working segmentation yet. Consulted by ConfirmBeforeDeactivation().*/
    bool m_HasUnconfirmedPreview = false;

    /** Opacity (0.0 - 1.0) used when rendering the preview segmentation.
     * Kept as tool state so a user-defined value survives the opacity
     * reset that happens whenever the preview node is regenerated
     * (see ResetPreviewNode).*/
    float m_PreviewOpacity = 1.0f / 3.0f;

    /** Whether the preview segmentation is rendered. Kept as tool state, like
     * m_PreviewOpacity, so it survives preview regeneration (see
     * ResetPreviewNode).*/
    bool m_PreviewVisibility = true;

    /** This variable tracks if there should be a user-confirmation before a tool is deactivated or not.
     * Call RequestDeactivationConfirmationOn() in the tool class to avail this feature.
     */
    bool m_RequestDeactivationConfirmation = false;

    bool m_RequiresExistingLabels = true;
    bool m_RequiresScalarReference = true;
    bool m_RequiresVolumetricReference = false;
    bool m_RequiresReferenceGeometry = false;
    bool m_TransfersAllPreviewGroups = false;
  };

} // namespace

#endif
