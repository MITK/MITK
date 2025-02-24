/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkMultiLabelSegmentation_h
#define mitkMultiLabelSegmentation_h

#include <shared_mutex>
#include <mitkImage.h>
#include <mitkLabel.h>
#include <mitkLookupTable.h>
#include <mitkMultiLabelEvents.h>
#include <mitkMessage.h>
#include <mitkITKEventObserverGuard.h>

#include <MitkMultilabelExports.h>

namespace mitk
{
  /** @brief MultiLabelSegmentation class for handling labels and layers in a segmentation session.
  *
  * Events that are potentially send by the class in regard to groups or labels:
  * - LabelAddedEvent is emitted whenever a new label has been added.
  * - LabelModifiedEvent is emitted whenever a label has been modified.
  * - LabelRemovedEvent is emitted whenever a label has been removed.
  * - LabelsChangedEvent is emitted when labels are changed (added, removed, modified). In difference to the other label events LabelsChanged is send only *one time* after the modification of the
  * MultiLableImage instance is finished. So e.g. even if 4 labels are changed by a merge operation, this event will
  * only be sent once (compared to LabelRemoved or LabelModified).
  * - GroupAddedEvent is emitted whenever a new group has been added.
  * - GroupModifiedEvent is emitted whenever a group has been modified.
  * - GroupRemovedEvent is emitted whenever a label has been removed.
  *
  * @ingroup Data
  */
  class MITKMULTILABEL_EXPORT MultiLabelSegmentation : public SlicedData
  {
  public:
    /**
    * \brief BeforeChangeLayerEvent (e.g. used for GUI integration)
    * As soon as active labelset should be changed, the signal emits.
    * Emitted by SetActiveLayer(int layer);
    */
    Message<> BeforeChangeLayerEvent;

    /**
    * \brief AfterchangeLayerEvent (e.g. used for GUI integration)
    * As soon as active labelset was changed, the signal emits.
    * Emitted by SetActiveLayer(int layer);
    */
    Message<> AfterChangeLayerEvent;

    mitkClassMacro(MultiLabelSegmentation, SlicedData);
    itkNewMacro(Self);

    typedef mitk::Label::PixelType PixelType;

    using GroupIndexType = std::size_t;
    using LabelValueType = mitk::Label::PixelType;
    using ConstLabelVectorType = ConstLabelVector;
    using LabelVectorType = LabelVector;
    using LabelValueVectorType = std::vector<LabelValueType>;

    const static LabelValueType UNLABELED_VALUE;

    enum class MergeStyle
    {
      Replace, //The old label content of a label value will be replaced by its new label content.
      //Therefore pixels that are labeled might become unlabeled again.
      //(This means that a lock of the value is also ignored).
      Merge //The union of old and new label content will be generated.
    };

    enum class OverwriteStyle
    {
      RegardLocks, //Locked labels in the same spatial group will not be overwritten/changed.
      IgnoreLocks //Label locks in the same spatial group will be ignored, so these labels might be changed.
    };

    /**
      * @brief Check whether slice @a s at time @a t in channel @a n is set
      */
    bool IsSliceSet(int s = 0, int t = 0, int n = 0) const override;

    /**
      * @brief Check whether volume at time @a t in channel @a n is set
      */
    bool IsVolumeSet(int t = 0, int n = 0) const override;

    /**
      * @brief Check whether the channel @a n is set
      */
    bool IsChannelSet(int n = 0) const override;

    /**
      * @brief Get dimension of the MultiLabelSegmentation instance.
      */
    unsigned int GetDimension() const;
    using GroupImageDimensionVectorType = std::vector<unsigned int>;
    /**
      * @brief Returns a vector of that contains the size (in pixel) for all dimensions of the
      * MultiLabelSegmentation instance. It is simelar to mitk::Image::GetDimensions.
      */
    const GroupImageDimensionVectorType& GetDimensions() const;

    /** \brief Adds a label instance to a group of the multi label image.
    * @remark By default, if the pixel value of the label is already used in the image, the label
    * will get a new none conflicting value assigned. This can be controlled by correctLabelValue.
    * @param label Instance of an label that should be added or used as template
    * @param groupID The id of the group the label should be added to.
    * @param addAsClone Flag that controls, if the passed instance should be added (false; the image will then take ownership,
    * be aware that e.g. event observers will be added)
    * a clone of the instance (true).
    * @param correctLabelValue Flag that controls, if the value of the passed label should be correct, if this value is already used in
    * the multi label image. True: Conflicting values will be corrected, be assigning a none conflicting value. False: If the value is conflicting
    * an exception will be thrown.
    * @return Instance of the label as it was added to the label set.
    * @pre label must point to a valid instance.
    * @pre If correctLabelValue==false, label value must be non conflicting.
    * @pre groupID must indicate an existing group.
    */
    mitk::Label* AddLabel(Label* label, GroupIndexType groupID, bool addAsClone = true, bool correctLabelValue = true);

    /** \brief Adds a label instance to a group of the multi label image including its pixel content.
    * @remark By default, if the pixel value of the label is already used in the image, the label
    * will get a new none conflicting value assigned. This can be controlled by correctLabelValue.
    * @param label Instance of a label that should be added or used as template
    * @param groupID The id of the group the label should be added to.
    * @param labelContent Pointer to an image that contains the pixel content of the label that should be added.
    * @param contentLabelValue Pixel value in the content image that indicates the label (may not be the same like the label value
    * used in the segmentation after addition).
    * @param addAsClone Flag that controls, if the passed instance should be added (false; the image will then take ownership,
    * be aware that e.g. event observers will be added)
    * a clone of the instance (true).
    * @param correctLabelValue Flag that controls, if the value of the passed label should be corrected, if this value is already used in
    * the multi label image. True: Conflicting values will be corrected, by assigning a none conflicting value. False: If the value is conflicting
    * an exception will be thrown.
    * @return Instance of the label as it was added to the label set.
    * @pre label must point to a valid instance.
    * @pre If correctLabelValue==false, label value must be non conflicting.
    * @pre groupID must indicate an existing group.
    * @pre labelContent must point to a valid image that has the same geometry like the segmentation.
    */
    mitk::Label* AddLabelWithContent(Label* label, const Image* labelContent, GroupIndexType groupID, LabelValueType contentLabelValue, bool addAsClone = true, bool correctLabelValue = true);

    /** \brief Adds a new label to a group of the image by providing name and color.
    * @param name (Class) name of the label instance that should be added.
    * @param color Color of the new label instance.
    * @param groupID The id of the group the label should be added to.
    * @return Instance of the label as it was added to the label set.
    * @pre groupID must indicate an existing group.
    */
    mitk::Label* AddLabel(const std::string& name, const Color& color, GroupIndexType groupID);

    /** \brief allows to adapt name and color of a certain label
    * @param labelValue Value of the label that should be changed
    * @param name New name for the label
    * @param color New color for the label
    * @pre Indicated label value must exist.
    */
    void RenameLabel(LabelValueType labelValue, const std::string& name, const Color& color);

    /**
     * @brief Removes the label with the given value.
     * The label is removed from the labelset and
     * the pixel with the value of the label are set to UNLABELED_VALUE.
     * @param labelValue the pixel value of the label to be removed. If the label is unknown,
     * the method will return without doing anything.
     */
    void RemoveLabel(LabelValueType labelValue);

    /**
     * @brief Removes labels from the mitk::MultiLabelSegmentation.
     * The label is removed from the labelset and
     * the pixel with the value of the label are set to UNLABELED_VALUE.
     * If a label value does not exist, it will be ignored.
     * @param vectorOfLabelPixelValues a list of labels to be removed
     */
    void RemoveLabels(const LabelValueVectorType& vectorOfLabelPixelValues);

    /**
     * @brief Erases the label with the given value from the labelset image.
     * The label itself will not be erased from the respective mitk::LabelSet. In order to
     * remove the label itself use mitk::MultiLabelSegmentation::RemoveLabels()
     * @param labelValue the pixel value of the label that will be erased from the labelset image
     * @pre labelValue must exist.
     */
    void EraseLabel(LabelValueType labelValue);

    /**
     * @brief Erases a list of labels with the given values from the labelset image.
     * @param labelValues the list of pixel values of the labels
     * that will be erased from the labelset image
     * @pre label values must exist
     */
    void EraseLabels(const LabelValueVectorType& labelValues);

    /**
     * @brief Removes a whole group including all its labels.
     * @remark with removing a group all groups with greater index will be re-indexed to
     * close the gap. Hence externally stored spatial group indices may become invalid.
     * @param group Group index of the spatial group that should be removed. If the spatial group does not exist, an
     * exception will be raised.
     * @pre group index must be valid.
     */
    void RemoveGroup(GroupIndexType group);

    /**
     * @brief Merges the content of a source label into the target label.
     *
     * @param targetLabelValue The value of the label that should be the merged label.
     * @param sourceLabelValue The value of the label that should be merged into the specified one
     * @param overwriteStyle Controls if the merge operation should regard the label locks in the
     * group of the target label.
     * @pre targetLabelValue must exist.
     * @pre sourceLabelValue must exist.
     * @remark the group that is modified by the operation is defined by the targetLabelValue. So you
     * can do mergings across groups. This will result in copying the source contents. If you want to
     * remove the content of the source labels that must be done explicitly by calling EraseLabel(s) (if
     * you just want to remove the pixel content) or RemoveLabel(s) (if you want remove the label
     * completely)
     */
    void MergeLabel(LabelValueType targetLabelValue, LabelValueType sourceLabelValue,
      OverwriteStyle overwriteStyle = OverwriteStyle::RegardLocks);

    /**
     * @brief Merges the content of a source labels into the target label.
     *
     * @param targetLabelValue The value of the label that should be the merged label.
     * @param sourceLabelValues The values of the labels that should be merged into the target label
     * with merge style "Merge".
     * @param overwriteStyle Controls if/how the merge operation should regard the label locks in the
     * group of the target label.
     * @pre targetLabelValue must exist.
     * @pre sourceLabelValues must exist.
     * @remark If a spatial group is empty after the operation, it might be removed. The removal of an spatial group
     * might invalidate any other spatial group index (due to new sorting/ordering).
     */
    void MergeLabels(LabelValueType targetLabelValue, const LabelValueVectorType& sourceLabelValues,
      OverwriteStyle overwriteStyle = OverwriteStyle::RegardLocks);

    /** \brief  Returns true if the value exists in the MultiLabelSegmentation instance*/
    bool ExistLabel(LabelValueType value) const;

    /**
     * @brief Checks if a label belongs in a certain spatial group
     * @param value the label value
     * @param groupIndex Index of the spatial group which should be checked for the label
     * @return true if the label exists otherwise false
     */
    bool ExistLabel(LabelValueType value, GroupIndexType groupIndex) const;

    /**
      * @brief  Returns true if the spatial group exists in the MultiLabelSegmentation instance.
      *
      * @param index Group index of the group that should be checked for existence.
      */
    bool ExistGroup(GroupIndexType index) const;

    /** Returns the group id of the based label value.
    * @pre label value must exists.
    */
    GroupIndexType GetGroupIndexOfLabel(LabelValueType value) const;

    /**
     * @brief Returns the mitk::Label with the given value.
     * @param value the pixel value of the label
     * @return smart pointer to the label instance if defined in the segmentation, otherwise nullptr.
     * @remark The label is returned as smart pointer, because the MultiLabelSegmentation instance
     * gives no guarantee how long the label instance will be a valid label of the segmentation.
     * If you hold the label instance for a longer time, you must expect that it is not valid anymore
     * (Either because the label id was removed or the label instance was replaced). It is valid as long
     * it points to the same label instance like a recent GetLabel() call.
     */
    mitk::Label::ConstPointer GetLabel(LabelValueType value) const;
    mitk::Label::Pointer GetLabel(LabelValueType value);

    /** Returns a vector with pointers to all labels currently defined in the MultiLabelSegmentation
    instance.*/
    const ConstLabelVectorType GetLabels() const;
    const LabelVectorType GetLabels();

    /** Returns a vector of all label values currently defined in the MultiLabelSegmentation
    instance.*/
    const LabelValueVectorType GetAllLabelValues() const;

    /** @brief Returns a vector with pointers to all labels in the MultiLabelSegmentation indicated
    * by the passed label value vector.
    * @param labelValues Vector of values of labels that should be returned.
    * @param ignoreMissing If true (default), unknown labels Will be skipped in the result. If false,
    * an exception will be raised, if a label is requested.
    */
    const LabelVectorType GetLabelsByValue(const LabelValueVectorType& labelValues, bool ignoreMissing = true);

    /** @brief Returns a vector with const pointers to all labels in the MultiLabelSegmentation indicated
    * by the passed label value vector.
    * For details see GetLabelsByValue();
    */
    const ConstLabelVectorType GetConstLabelsByValue(const LabelValueVectorType& labelValues, bool ignoreMissing = false) const;

    /** Helper function that can be used to extract a vector of label values of a vector of label instance pointers.*/
    static LabelValueVectorType ExtractLabelValuesFromLabelVector(const ConstLabelVectorType& labels);
    /** Helper function that can be used to extract a vector of label values are vector of label instances.*/
    static LabelValueVectorType ExtractLabelValuesFromLabelVector(const LabelVectorType& labels);

    /** Helper function that converts a given vector of label instance pointers into a vector of const pointers.*/
    static ConstLabelVectorType ConvertLabelVectorConst(const LabelVectorType& labels);

    /**
     * @brief Returns a vector of all label values located on the specified group.
     * @param index the index of the group for which the vector of labels should be retrieved.
     * If an invalid index is passed an exception will be raised.
     * @return the respective vector of label values.
     * @pre group index must exist.
     */
    const LabelValueVectorType GetLabelValuesByGroup(GroupIndexType index) const;

    /**
     * @brief Returns a vector of all label values located on the specified group having a certain name.
     * @param index the index of the group for which the vector of labels should be retrieved.
     * If an invalid index is passed an exception will be raised.
     * @param name Name of the label instances one is looking for.
     * @return the respective vector of label values.
     * @pre group index must exist.
     */
    const LabelValueVectorType GetLabelValuesByName(GroupIndexType index, const std::string_view name) const;

    /**
    * Returns a vector with (class) names of all label instances used in the segmentation (over all groups)
    */
    std::vector<std::string> GetLabelClassNames() const;

    /**
    * Returns a vector with (class) names of all label instances present in a certain group.
    * @param index ID of the group, for which the label class names should be returned
    * @pre Indicated group must exist. */
    std::vector<std::string> GetLabelClassNamesByGroup(GroupIndexType index) const;

    /** Helper that returns an unused label value, that could be used e.g. if one wants to define a label externally
    * before adding it.
    * @return A label value currently not in use.
    * @remark is no unused label value can be provided an exception will be thrown.*/
    LabelValueType GetUnusedLabelValue() const;

    itkGetConstMacro(UnlabeledLabelLock, bool);
    itkSetMacro(UnlabeledLabelLock, bool);
    itkBooleanMacro(UnlabeledLabelLock);

    /** Set the visibility of all label instances accordingly to the passed state.
    */
    void SetAllLabelsVisible(bool visible);

    /** Set the visibility of all label instances in a group accordingly to the passed state.
    * @pre The specified group must exist.
    */
    void SetAllLabelsVisibleByGroup(GroupIndexType group, bool visible);

    /** Set the visibility of all label instances In a group with a given class name
    * accordingly to the passed state.
    * @pre The specified group must exist.
    */
    void SetAllLabelsVisibleByName(GroupIndexType group, const std::string_view name, bool visible);

    /** Returns the lock state of the label (including UnlabeledLabel value).
     @pre Requested label does exist.*/
    bool IsLabelLocked(LabelValueType value) const;

    /** Set the lock state of all label instances accordingly to the passed state.
    */
    void SetAllLabelsLocked(bool locked);

    /** Set the lock state of all label instances in a group accordingly to the passed state.
    * @pre The specified group must exist.
    */
    void SetAllLabelsLockedByGroup(GroupIndexType group, bool locked);

    /** Set the lock state of all label instances In a group with a given class name
    * accordingly to the passed state.
    * @pre The specified group must exist.
    */
    void SetAllLabelsLockedByName(GroupIndexType group, const std::string_view name, bool locked);

    /**
    * \brief Replaces the labels of a group with a given vector of labels.
    *
    * @remark The passed label instances will be cloned before added to ensure clear ownership
    * of the new labels.
    * @remark The pixel content of the old labels will not be removed.
    * @param groupID The index of the group that should have its labels replaced
    * @param newLabels The vector of new labels
    * @pre Group that should be replaced must exist.
    * @pre new label values must not be used in other groups.
    */
    void ReplaceGroupLabels(const GroupIndexType groupID, const ConstLabelVectorType& newLabels);

    void ReplaceGroupLabels(const GroupIndexType groupID, const LabelVectorType& newLabels);

    /** Returns the pointer to the image that contains the labeling of the indicate group.
     *@pre groupID must reference an existing group.*/
    mitk::Image* GetGroupImage(GroupIndexType groupID);

    /** Returns the pointer to the image that contains the labeling of the indicate group.
     *@pre groupID must reference an existing group.*/
    const mitk::Image* GetGroupImage(GroupIndexType groupID) const;

    /** Updates a group image by copying a given source image content.
    * @remark the pixel content of the sourceImage will be simply copied. It won't
    * be checked if the source only contains valid label values for the group.
    * This must be ensured by the callee of the function. If only some labels should be
    * transfered/updated, TransferLabelContent(...) is a better option.
    * @pre sourceImage must point to a valid image
    * @pre groupID must indicate an existing group
    * @pre sourceImage and this instance must have the same time geometry.
    */
    void UpdateGroupImage(GroupIndexType groupID, const mitk::Image* sourceImage, TimeStepType timestep, TimeStepType sourceTimestep = 0);

    /**
     * @brief clears all label pixel content from the indicated group.
     * @per groupID must point to a valid group.
     */
    void ClearGroupImage(GroupIndexType groupID);
    /**
     * @brief clears all label pixel content from the indicated group.
     * @per groupID must point to a valid group.
     * @per timestep must be a valid.
     */
     void ClearGroupImage(GroupIndexType groupID, TimeStepType timestep);

    /**
     * @brief clears all label pixel content of all groups.
     * @per groupID must point to a valid group.
     */
    void ClearGroupImages();
    /**
     * @brief clears all label pixel content of all groups in one time step.
     * @per timestep must be a valid.
     */
    void ClearGroupImages(TimeStepType timestep);

    /** Returns the name of the indicated group. String may be empty if no name was defined.
     * Remark: The name neither is guaranteed to be defined nor that it is unique. Use the index
     * to uniquely refer to a group.
     *@pre groupID must reference an existing group.*/
    const std::string& GetGroupName(GroupIndexType groupID) const;

    /** Set the name of a group.
     *@pre groupID must reference an existing group.*/
    void SetGroupName(GroupIndexType groupID, const std::string& name);

    itkGetModifiableObjectMacro(LookupTable, mitk::LookupTable);
    void SetLookupTable(LookupTable* lut);
    /** Updates the lookup table for a label indicated by the passed label value using the color of the label.
    * @pre labelValue must exist.
    */
    void UpdateLookupTable(PixelType pixelValue);

    /**
    * @brief Initialize new (or re-initialize) the segmentation based on the properties
    * and geometric information of a passed image. The pixel content will be reseted.
    *
    * @param templateImage Template for the initialization
    * @param resetLabels Indicate if the labels should be reseted on initialization.
    * True (default): all label and group informations will be removed. False: label
    * information and groups are kept, but all pixel information will be erased
    * (cf EraseLabel(...)).
    * @param ensure1stGroup Indicates if the initialization guarantees that the first
    * group always exists afterwards (true) or if after initialization no group will be
    * added even if no group is defined (false; e.g if labels are reseted or instance has
    * no groups so far)
    */
    void Initialize(const mitk::Image* templateImage, bool resetLabels = true, bool ensure1stGroup = true);

    /**
    * @brief Initialize new (or re-initialize) the segmentation based on the passed time geometry
    * information. The pixel content will be reseted.
    *
    * @param geometry Time geometry that should be used.
    * @param resetLabels Indicate if the labels should be reseted on initialization.
    * True (default): all label and group informations will be removed. False: label
    * information and groups are kept, but all pixel information will be erased
    * (cf EraseLabel(...)).
    * @param ensure1stGroup Indicates if the initialization guarantees that the first
    * group always exists afterwards (true) or if after initialization no group will be
    * added even if no group is defined (false; e.g if labels are reseted or instance has
    * no groups so far)
    */
    void Initialize(const mitk::TimeGeometry* geometry, bool resetLabels = true, bool ensure1stGroup = true);

    /** @brief Initialize a new mitk::MultiLabelSegmentation by a given image.
     * For all distinct pixel values of the parameter image new labels will
     * be created. If the number of distinct pixel values exceeds mitk::Label::MAX_LABEL_VALUE
     * an exception will be raised.
     * @param image the image which is used for initialization
     */
    void InitializeByLabeledImage(const mitk::Image* image);

    protected:

      void OnLabelModified(const Object* sender, const itk::EventObject&);

      /** Helper to ensure that the maps are correctly populated for a new label instance.*/
      void AddLabelToMap(LabelValueType labelValue, Label* label, GroupIndexType groupID);
      void RemoveLabelFromMap(LabelValueType labelValue);
      /** Helper to ensure label events are correctly connected and lookup table is updated for a new label instance.*/
      void RegisterLabel(Label* label);
      /** Helper to ensure label events are unregistered.*/
      void ReleaseLabel(Label* label);

      /** Helper class used internally to apply lambda functions to the labels specified by the passed label value vector.
      */
      void ApplyToLabels(const LabelValueVectorType& values, std::function<void(Label*)>&& lambda);
      /** Helper class used internally to for visiting the labels specified by the passed label value vector
      * with the lambda function.
      */
      void VisitLabels(const LabelValueVectorType& values, std::function<void(const Label*)>&& lambda) const;

      LabelValueType m_ActiveLabelValue;

    private:

      /** Generates a new group image that fits to the geometry of the current instance.
       @remark The pixel values are not initialized. E.g. use clear Image buffer for that.*/
      Image::Pointer GenerateNewGroupImage() const;

      using LabelMapType = std::map<LabelValueType, Label::Pointer>;
      /** Dictionary that holds all known labels (label value is the key).*/
      LabelMapType m_LabelMap;

      using GroupNameVectorType = std::vector<std::string>;
      /** Vector storing the names of all groups. If a group has no user name defined, string is empty.*/
      GroupNameVectorType m_Groups;

      /**This type is internally used to track which label is currently
       * associated with which layer.*/
      using GroupToLabelMapType = std::vector<LabelValueVectorType>;
      /* Dictionary that maps between group id (key) and label values in the group (vector of label value).*/
      GroupToLabelMapType m_GroupToLabelMap;
      using LabelToGroupMapType = std::map<LabelValueType, GroupIndexType>;
      /* Dictionary that maps between label value (key) and group id (value)*/
      LabelToGroupMapType m_LabelToGroupMap;

      using LabelEventGuardMapType = std::map<LabelValueType, ITKEventObserverGuard>;
      LabelEventGuardMapType m_LabelModEventGuardMap;

      LookupTable::Pointer m_LookupTable;

      /** Indicates if the MultiLabelSegmentation allows to overwrite unlabeled pixels in normal pixel manipulation operations (e.g. TransferLabelConent).*/
      bool m_UnlabeledLabelLock;

      /** Mutex used to secure manipulations of the internal state of label and group maps.*/
      std::shared_mutex m_LabelNGroupMapsMutex;

      /** This variable stores the dimensions of the multi label segmentation, in order to make it available even
      when no group image is available.*/
      GroupImageDimensionVectorType m_GroupImageDimensions;

    public:


    ///////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////
    // END FUTURE MultiLabelSegmentation
    ///////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////

    /**
      * \brief  */
      void UpdateCenterOfMass(PixelType pixelValue);

    using BaseData::IsEmpty;

    /** \brief Checks if a label is empty at a given time step (does not caintain any pixels).
      */
    bool IsEmpty(const Label* label, TimeStepType t = 0) const;
    bool IsEmpty(PixelType pixelValue, TimeStepType t = 0) const;

    /**
     * @brief Gets the ID of the currently active layer
     * @return the ID of the active layer
     * @pre at least on group must exist.
     */
    unsigned int GetActiveLayer() const;

    Label* GetActiveLabel();

    const Label* GetActiveLabel() const;

    /**
     * @brief Get the number of all existing mitk::Labels for a given layer
     * @param layer the layer ID for which the active mitk::Labels should be retrieved
     * @return the number of all existing mitk::Labels for the given layer
     */
    unsigned int GetNumberOfLabels(unsigned int layer) const;

    /**
     * @brief Returns the number of all labels summed up across all layers
     * @return the overall number of labels across all layers
     */
    unsigned int GetTotalNumberOfLabels() const;

    void SetActiveLayer(unsigned int layer);
    void SetActiveLabel(LabelValueType label);

    unsigned int GetNumberOfLayers() const;

    /**
     * \brief Adds a new layer to the MultiLabelSegmentation. The new layer will be set as the active one.
     * \param labels Labels that will be added to the new layer if provided
     * \return the layer ID of the new layer
     */
    GroupIndexType AddLayer(ConstLabelVector labels = {});

   /**
    * \brief Adds a layer based on a provided mitk::Image.
    * \param layerImage is added to the vector of label images
    * \param labels labels that will be cloned and added to the new layer if provided
    * \return the layer ID of the new layer
    * \pre layerImage must be valid instance
    * \pre layerImage needs to have the same geometry then the segmentation
    * \pre layerImage must have the pixel value equal to LabelValueType.
    */
    GroupIndexType AddLayer(mitk::Image* layerImage, ConstLabelVector labels = {});

  protected:
    mitkCloneMacro(Self);

      MultiLabelSegmentation();
    MultiLabelSegmentation(const MultiLabelSegmentation &other);
    ~MultiLabelSegmentation() override;

    template <typename TPixel, unsigned int VImageDimension>
    void LayerContainerToImageProcessing(itk::Image<TPixel, VImageDimension> *source, unsigned int layer);

    template <typename TPixel, unsigned int VImageDimension>
    void ImageToLayerContainerProcessing(const itk::Image<TPixel, VImageDimension> *source, unsigned int layer) const;

    template <typename ImageType>
    void CalculateCenterOfMassProcessing(ImageType *input, LabelValueType index);

    template <typename ImageType>
    void EraseLabelProcessing(ImageType *input, PixelType index);

    template <typename MultiLabelSegmentationType, typename ImageType>
    void InitializeByLabeledImageProcessing(MultiLabelSegmentationType *input, const ImageType *other);

    /** helper needed for ensuring unique values.
      returns a sorted list of all labels (including the value for Unlabeled pixels..*/
    LabelValueVectorType GetUsedLabelValues() const;

    std::vector<Image::Pointer> m_LayerContainer;

    int m_ActiveLayer;
    bool m_activeLayerInvalid;
  };

  /**
  * @brief Equal A function comparing two label set images for being equal in meta- and imagedata
  *
  * @ingroup MITKTestingAPI
  *
  * Following aspects are tested for equality:
  *  - MultiLabelSegmentation members
  *  - working image data
  *  - layer image data
  *  - labels in label set
  *
  * @param rightHandSide An image to be compared
  * @param leftHandSide An image to be compared
  * @param eps Tolerance for comparison. You can use mitk::eps in most cases.
  * @param verbose Flag indicating if the user wants detailed console output or not.
  * @return true, if all subsequent comparisons are true, false otherwise
  */
  MITKMULTILABEL_EXPORT bool Equal(const mitk::MultiLabelSegmentation &leftHandSide,
                                   const mitk::MultiLabelSegmentation &rightHandSide,
                                   ScalarType eps,
                                   bool verbose);

  /**
  * @brief Equal A function comparing two vectors of labels for being equal in data
  *
  * @ingroup MITKTestingAPI
  *
  * Following aspects are tested for equality:
  *  - Labels in vector
  *
  * @param rightHandSide An vector of labels to be compared
  * @param leftHandSide An vector of labels to be compared
  * @param eps Tolerance for comparison. You can use mitk::eps in most cases.
  * @param verbose Flag indicating if the user wants detailed console output or not.
  * @return true, if all subsequent comparisons are true, false otherwise
  */
  MITKMULTILABEL_EXPORT bool Equal(const mitk::MultiLabelSegmentation::ConstLabelVectorType& leftHandSide,
    const mitk::MultiLabelSegmentation::ConstLabelVectorType& rightHandSide,
    ScalarType eps,
    bool verbose);

  /**
  * @brief Equal A function comparing two vectors of label values for being equal in data
  *
  * @ingroup MITKTestingAPI
  *
  * Following aspects are tested for equality:
  *  - Label values in vector
  *
  * @param rightHandSide A vector of label values to be compared
  * @param leftHandSide A vector of label values to be compared
  * @param orderIsRelevant Indicating if only the presence of the same values
  * in the vectors are relevant or also their ordering.Tolerance for comparison.
  * @return true, if all subsequent comparisons are true, false otherwise
  */
  MITKMULTILABEL_EXPORT bool Equal(const mitk::LabelSetImage::LabelValueVectorType& leftHandSide,
    const mitk::LabelSetImage::LabelValueVectorType& rightHandSide,
    bool orderIsRelevant = false);


  using LabelValueMappingVector = std::vector < std::pair<Label::PixelType, Label::PixelType> >;

  /**Helper function that transfers pixels of the specified source label from source image to the destination image by using
  a specified destination label for a specific time step. Function processes the whole image volume of the specified time step.
  @remark in its current implementation the function only transfers contents of the active layer of the passed MultiLabelSegmentations.
  @remark the function assumes that it is only called with source and destination image of same geometry.
  @remark CAUTION: The function is not save if sourceImage and destinationImage are the same instance and more than one label is transferred,
  because the changes are made in-place for performance reasons in multiple passes. If a mapped value A equals an "old value"
  that occurs later in the mapping, one ends up with a wrong transfer, as a pixel would be first mapped to A and then later again, because
  it is also an "old" value in the mapping table.
  @param sourceImage Pointer to the MultiLabelSegmentation which active layer should be used as source for the transfer.
  @param destinationImage Pointer to the MultiLabelSegmentation which active layer should be used as destination for the transfer.
  @param labelMapping Map that encodes the mappings of all label pixel transfers that should be done. First element is the
  label in the source image. The second element is the label that transferred pixels should become in the destination image.
  The order in which the labels will be transferred is the same order of elements in the labelMapping.
  If you use a heterogeneous label mapping (e.g. (1,2); so changing the label while transferring), keep in mind that
  for the MergeStyle and OverwriteStyle only the destination label (second element) is relevant (e.g. what should be
  altered with MergeStyle Replace).
  @param mergeStyle indicates how the transfer should be done (merge or replace). For more details see documentation of
  MultiLabelSegmentation::MergeStyle.
  @param overwriteStlye indicates if label locks in the destination image should be regarded or not. For more details see
  documentation of MultiLabelSegmentation::OverwriteStyle.
  @param timeStep indicate the time step that should be transferred.
  @pre sourceImage and destinationImage must be valid
  @pre sourceImage and destinationImage must contain the indicated timeStep
  @pre sourceImage must contain all indicated sourceLabels in its active layer.
  @pre destinationImage must contain all indicated destinationLabels in its active layer.*/
  MITKMULTILABEL_EXPORT void TransferLabelContentAtTimeStep(const MultiLabelSegmentation* sourceImage, MultiLabelSegmentation* destinationImage,
    const TimeStepType timeStep, LabelValueMappingVector labelMapping = { {1,1} },
    MultiLabelSegmentation::MergeStyle mergeStyle = MultiLabelSegmentation::MergeStyle::Replace,
    MultiLabelSegmentation::OverwriteStyle overwriteStlye = MultiLabelSegmentation::OverwriteStyle::RegardLocks);

  /**Helper function that transfers pixels of the specified source label from source image to the destination image by using
  a specified destination label. Function processes the whole image volume for all time steps.
  For more details please see TransferLabelContentAtTimeStep for MultiLabelSegmentations.
  @sa TransferLabelContentAtTimeStep*/
  MITKMULTILABEL_EXPORT void TransferLabelContent(const MultiLabelSegmentation* sourceImage, MultiLabelSegmentation* destinationImage, LabelValueMappingVector labelMapping = { {1,1} },
    MultiLabelSegmentation::MergeStyle mergeStyle = MultiLabelSegmentation::MergeStyle::Replace,
    MultiLabelSegmentation::OverwriteStyle overwriteStlye = MultiLabelSegmentation::OverwriteStyle::RegardLocks);


  /**Helper function that transfers pixels of the specified source label from source image to the destination image by using
  a specified destination label for a specific time step. Function processes the whole image volume of the specified time step.
  @remark the function assumes that it is only called with source and destination image of same geometry.
  @remark CAUTION: The function is not save, if sourceImage and destinationImage are the same instance and you transfer more then one
  label, because the changes are made in-place for performance reasons but not in one pass. If a mapped value A equals a "old value"
  that is later in the mapping, one ends up with a wrong transfer, as a pixel would be first mapped to A and then latter again, because
  it is also an "old" value in the mapping table.
  @param sourceImage Pointer to the image that should be used as source for the transfer.
  @param destinationImage Pointer to the image that should be used as destination for the transfer.
  @param destinationLabelVector Reference to the vector of labels (incl. lock states) in the destination image. Unknown pixel
  values in the destinationImage will be assumed to be unlocked.
  @param sourceBackground Value indicating the background in the source image.
  @param destinationBackground Value indicating the background in the destination image.
  @param destinationBackgroundLocked Value indicating the lock state of the background in the destination image.
  @param labelMapping Map that encodes the mappings of all label pixel transfers that should be done. First element is the
  label in the source image. The second element is the label that transferred pixels should become in the destination image.
  The order in which the labels will be transferred is the same order of elements in the labelMapping.
  If you use a heterogeneous label mapping (e.g. (1,2); so changing the label while transferring), keep in mind that
  for the MergeStyle and OverwriteStyle only the destination label (second element) is relevant (e.g. what should be
  altered with MergeStyle Replace).
  @param mergeStyle indicates how the transfer should be done (merge or replace). For more details see documentation of
  MultiLabelSegmentation::MergeStyle.
  @param overwriteStlye indicates if label locks in the destination image should be regarded or not. For more details see
  documentation of MultiLabelSegmentation::OverwriteStyle.
  @param timeStep indicate the time step that should be transferred.
  @pre sourceImage, destinationImage and destinationLabelVector must be valid
  @pre sourceImage and destinationImage must contain the indicated timeStep
  @pre destinationLabelVector must contain all indicated destinationLabels for mapping.*/
  MITKMULTILABEL_EXPORT void TransferLabelContentAtTimeStep(const Image* sourceImage, Image* destinationImage, const mitk::ConstLabelVector& destinationLabelVector,
    const TimeStepType timeStep, mitk::Label::PixelType sourceBackground = MultiLabelSegmentation::UNLABELED_VALUE,
    mitk::Label::PixelType destinationBackground = MultiLabelSegmentation::UNLABELED_VALUE,
    bool destinationBackgroundLocked = false,
    LabelValueMappingVector labelMapping = { {1,1} },
    MultiLabelSegmentation::MergeStyle mergeStyle = MultiLabelSegmentation::MergeStyle::Replace,
    MultiLabelSegmentation::OverwriteStyle overwriteStlye = MultiLabelSegmentation::OverwriteStyle::RegardLocks);

  /**Helper function that transfers pixels of the specified source label from source image to the destination image by using
  a specified destination label. Function processes the whole image volume for all time steps.
  For more details please see TransferLabelContentAtTimeStep.
  @sa TransferLabelContentAtTimeStep*/
  MITKMULTILABEL_EXPORT void TransferLabelContent(const Image* sourceImage, Image* destinationImage, const mitk::ConstLabelVector& destinationLabelVector,
    mitk::Label::PixelType sourceBackground = MultiLabelSegmentation::UNLABELED_VALUE,
    mitk::Label::PixelType destinationBackground = MultiLabelSegmentation::UNLABELED_VALUE,
    bool destinationBackgroundLocked = false,
    LabelValueMappingVector labelMapping = { {1,1} },
    MultiLabelSegmentation::MergeStyle mergeStyle = MultiLabelSegmentation::MergeStyle::Replace,
    MultiLabelSegmentation::OverwriteStyle overwriteStlye = MultiLabelSegmentation::OverwriteStyle::RegardLocks);

} // namespace mitk

#endif
