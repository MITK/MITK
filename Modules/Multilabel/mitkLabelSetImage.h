/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkLabelSetImage_h
#define mitkLabelSetImage_h

#include <mitkImage.h>
#include <mitkLabel.h>
#include <mitkMessage.h>
#include <mitkLookupTable.h>

#include <MitkMultilabelExports.h>

namespace mitk
{
  //##Documentation
  //## @brief LabelSetImage class for handling labels and layers in a segmentation session.
  //##
  //## Handles operations for adding, removing, erasing and editing labels and layers.
  //## @ingroup Data

  class MITKMULTILABEL_EXPORT LabelSetImage : public Image
  {
  public:
    mitkClassMacro(LabelSetImage, Image);
    itkNewMacro(Self);

    typedef mitk::Label::PixelType PixelType;

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

    ///////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////
    // FUTURE MultiLabelSegmentation:
    // Section that already contains declarations used in the new class.
    // So this part of the interface will stay after refactoring towards
    // the new MultiLabelSegmentation class (see T28524). This section was introduced
    // because some of the planned features are already urgently needed.
    ///////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////

    using GroupIndexType = std::size_t;
    using LabelValueType = mitk::Label::PixelType;
    const static LabelValueType UnlabeledValue = 0;
    using ConstLabelVectorType = ConstLabelVector;
    using LabelVectorType = LabelVector;
    using LabelValueVectorType = std::vector<LabelValueType>;

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
    mitk::Label* AddLabel(mitk::Label* label, GroupIndexType groupID, bool addAsClone = true, bool correctLabelValue = true);

    /** \brief Adds a new label to a group of the image by providing name and color.
    * @param name (Class) name of the label instance that should be added.
    * @param color Color of the new label sinstance.
    * @param groupID The id of the group the label should be added to.
    * @return Instance of the label as it was added to the label set.
    * @pre groupID must indicate an existing group.
    */
    mitk::Label* AddLabel(const std::string& name, const Color& color, GroupIndexType groupID);

    /** \brief
    */
    void RenameLabel(LabelValueType labelValue, const std::string&, const Color&);

    /**
     * @brief Removes the label with the given value.
     *        The label is removed from the labelset and
     *        the pixel with the value of the label are set to UnlabeledValue.
     * @param labelValue the pixel value of the label to be removed
     */
    void RemoveLabel(LabelValueType labelValue);

    /**
     * @brief Removes labels from the mitk::MultiLabelSegmentation.
     * If a label value does not exist, it will be ignored.
     * @param vectorOfLabelPixelValues a list of labels to be removed
     */
    void RemoveLabels(const LabelValueVectorType& vectorOfLabelPixelValues);

    /**
     * @brief Removes a whole group including all its labels.
     * @remark with removing a group all groups with greater index will be reindexed to
     * close the gap. Hence externaly stored spatial group indices may become invalid.
     * @param group Group index of the spatial group that should be removed. If the spatial group does not exist, an
     * exception will be raised.
     * @pre group index must be valid.
     */
    void RemoveGroup(GroupIndexType group);

    /** \brief  Returns true if the value exists in the MultiLabelSegmentation instance*/
    bool ExistLabel(LabelValueType value) const;

    /**
     * @brief Checks if a label belongs in a certain spatial group
     * @param value the label value
     * @param groupIndex Indexp of the spacial group which should be checked for the label
     * @return true if the label exists otherwise false
     */
    bool ExistLabel(LabelValueType value, GroupIndexType groupIndex) const;

    /**
      * @brief  Returns true if the spatial group exists in the MultiLabelSegmentation instance.
      *
      * @param index Group index of the group that should be checked for existance.
      */
    bool ExistGroup(GroupIndexType index) const;

    bool IsLabelInGroup(LabelValueType value) const;
    bool IsLabelInGroup(LabelValueType value, GroupIndexType& groupIndex) const;

    /** Returns the group id of the based label value.
    * @pre label value must exists.
    */
    GroupIndexType GetGroupIndexOfLabel(LabelValueType value) const;

    /**
     * @brief Returns the mitk::Label with the given value.
     * @param value the pixel value of the label
     * @return the mitk::Label if available otherwise nullptr
     */
    const mitk::Label* GetLabel(LabelValueType value) const;
    mitk::Label* GetLabel(LabelValueType value);

    /** Returns the lock state of the label (including UnlabeledLabel value).
     @pre Requested label does exist.*/
    bool IsLabelLocked(LabelValueType value) const;

    /** Returns a vector with all labels currently defined in the MultiLabelSegmentation
    instance.*/
    const ConstLabelVectorType GetLabels() const;
    const LabelVectorType GetLabels();

    static LabelValueVectorType ExtractLabelValuesFromLabelVector(const ConstLabelVectorType& labels);
    static LabelValueVectorType ExtractLabelValuesFromLabelVector(const LabelVectorType& labels);

    static ConstLabelVectorType ConvertLabelVectorConst(const LabelVectorType& labels);

    /**
     * @brief Returns a vector of all labels located on the specified group.
     * @param index the index of the group for which the vector of labels should be retrieved.
     * If an invalid index is passed an exception will be raised.
     * @return the respective vector of const labels.
     * @pre group index must exist.
     */
    const ConstLabelVectorType GetConstLabelsInGroup(GroupIndexType index) const;
    /**
     * @brief Returns a vector of all labels located on the specified group.
     * @param index the index of the group for which the vector of labels should be retrieved.
     * If an invalid index is passed an exception will be raised.
     * @return the respective vector of labels.
     * @pre group index must exist.
     */
    const LabelVectorType GetLabelsInGroup(GroupIndexType index);

    itkGetConstMacro(UnlabeledLabelLock, bool);
    itkSetMacro(UnlabeledLabelLock, bool);
    itkBooleanMacro(UnlabeledLabelLock);

    /**
    * \brief Replaces the labels of a group with a given vector of labels.
    *
    * @remark The passed label instances will be cloned before added to ensure clear ownership
    * of the new labels.
    * @remark The pixel content of the old labels will not be removed.
    * \param groupID The index of the group that should have its labels replaced
    * \param newLabels The vector of new labels
    * @pre Group that shuold be replaced must exist.
    * Qpre new label values must not be used in other groups.
    */
    void ReplaceGroupLabels(const GroupIndexType groupID, const ConstLabelVectorType& newLabels);
    void ReplaceGroupLabels(const GroupIndexType groupID, const LabelVectorType& newLabels);

    /** Returns the pointer to the image that containes the labeling of the indicate group.
     *@pre groupID must reference an existing group.*/
    mitk::Image* GetGroupImage(GroupIndexType groupID);
    /** Returns the pointer to the image that containes the labeling of the indicate group.
     *@pre groupID must reference an existing group.*/
    const mitk::Image* GetGroupImage(GroupIndexType groupID) const;

    ////////////////////////////////////////////////////////////////////
    //Message slots that allow to react to changes in an instance

    using LabelEventType = Message1<LabelValueType>;
    using LabelsEventType = Message1<LabelValueVectorType>;
    using GroupEventType = Message1<GroupIndexType>;

    /**
    * \brief LabelAdded is emitted whenever a new label has been added.
    *
    * Observers should register to this event by calling this->AddLabelAddedListener(myObject,
    * MyObject::MyMethod).
    * After registering, myObject->MyMethod() will be called every time a new label has been added to the MultiLabelSegmentation.
    * Observers should unregister by calling this->RemoveLabelAddedListener(myObject, MyObject::MyMethod).
    * The registered method will be called with the label value of the added label.
    * @remark the usage of the message object is thread safe.
    */
    mitkNewMessage1Macro(LabelAdded, LabelValueType);

    /**
    * \brief LabelModified is emitted whenever a label has been modified.
    *
    * A label is modified if either its pixel content was changed, its spatial group or the label instance
    * information.
    * If you just want to get notified at the end of a MultiLabelSegmentation instance manipulation in the
    * case that at least one label was modified (e.g. to avoid getting a signal for each label
    * individually), use LabelsChanged instead.
    * Observers should register to this event by calling this->AddLabelModifiedListener(myObject,
    * MyObject::MyMethod).
    * After registering, myObject->MyMethod() will be called every time a new label has been added to the MultiLabelSegmentation.
    * Observers should unregister by calling this->RemoveLabelModifiedListener(myObject, MyObject::MyMethod).
    * The registered method will be called with the label value of the modified label.
    * @remark the usage of the message object is thread safe.
    */
    mitkNewMessage1Macro(LabelModified, LabelValueType);

    /**
    * \brief LabelRemoved is emitted whenever a label has been removed.
    *
    * Observers should register to this event by calling this->AddLabelRemovedListener(myObject,
    * MyObject::MyMethod).
    * After registering, myObject->MyMethod() will be called every time a new label has been added to the MultiLabelSegmentation.
    * Observers should unregister by calling this->RemoveLabelRemovedListener(myObject, MyObject::MyMethod).
    * The registered method will be called with the label value of the removed label.*
    * @remark the usage of the message object is thread safe.
    */
    mitkNewMessage1Macro(LabelRemoved, LabelValueType);

    /**
    * \brief LabelsChanged is emitted when labels are changed (added, removed, modified).
    *
    * In difference to the other label events LabelsChanged is send only *one time* after the modification of the
    * MultiLableImage instance is finished. So e.g. even if 4 labels are changed by a merge operation, this event will
    * only be sent once (compared to LabelRemoved or LabelModified).
    * Observers should register to this event by calling myMultiLabelSegmentation->AddLabelsChangedListener(myObject,
    * MyObject::MyMethod).
    * After registering, myObject->MyMethod() will be called every time a new label has been removed from the MultiLabelSegmentation.
    * Observers should unregister by calling myMultiLabelSegmentation->RemoveLabelsChangedListener(myObject,
    * MyObject::MyMethod).
    * The registered method will be called with the vector of label values of the modified labels.*
    * @remark the usage of the message object is thread safe.
    */
    mitkNewMessage1Macro(LabelsChanged, LabelValueVectorType);

    /**
    * \brief GroupAdded is emitted whenever a new group has been added.
    *
    * Observers should register to this event by calling this->AddGroupAddedListener(myObject,
    * MyObject::MyMethod).
    * After registering, myObject->MyMethod() will be called every time a new group has been added to the MultiLabelSegmentation.
    * Observers should unregister by calling this->RemoveGroupAddedListener(myObject, MyObject::MyMethod).
    * The registered method will be called with the group index of the added group.
    * @remark the usage of the message object is thread safe.
    */
    mitkNewMessage1Macro(GroupAdded, GroupIndexType);

    /**
    * \brief GroupModified is emitted whenever a group has been modified.
    *
    * A group is modified if the set of labels associated with it are changed or the group's meta data.
    * Observers should register to this event by calling this->AddGroupModifiedListener(myObject,
    * MyObject::MyMethod).
    * After registering, myObject->MyMethod() will be called every time a new label has been added to the MultiLabelSegmentation.
    * Observers should unregister by calling this->RemoveGroupModifiedListener(myObject, MyObject::MyMethod).
    * The registered method will be called with the group index of the added group.
    * @remark the usage of the message object is thread safe.
    */
    mitkNewMessage1Macro(GroupModified, GroupIndexType);

    /**
    * \brief GroupRemoved is emitted whenever a label has been removed.
    *
    * Observers should register to this event by calling this->AddGroupRemovedListener(myObject,
    * MyObject::MyMethod).
    * After registering, myObject->MyMethod() will be called every time a new label has been added to the MultiLabelSegmentation.
    * Observers should unregister by calling this->RemoveGroupRemovedListener(myObject, MyObject::MyMethod).
    * The registered method will be called with the group index of the removed group.*
    * @remark the usage of the message object is thread safe.
    */
    mitkNewMessage1Macro(GroupRemoved, GroupIndexType);

    protected:

      void OnLabelModified(const Object* sender, const itk::EventObject&);

      /** Helper to ensure that the maps are correctly populated for a new label instance.*/
      void AddLabelToMap(LabelValueType labelValue, Label* label, GroupIndexType groupID);
      void RemoveLabelFromMap(LabelValueType labelValue);
      /** Helper to ensure label events are correctly connected and lookup table is updated for a new label instance.*/
      void RegisterLabel(Label* label);
      /** Helper to ensure label events are unregistered.*/
      void ReleaseLabel(Label* label);

      using LabelMapType = std::map<LabelValueType, Label::Pointer>;
      LabelMapType m_LabelMap;

      using GroupNameVectorType = std::vector<std::string>;
      GroupNameVectorType m_Groups;

      /**This type is internally used to track which label is currently
       * associated with which layer.*/
      using GroupToLabelMapType = std::vector<LabelValueVectorType>;
      GroupToLabelMapType m_GroupToLabelMap;
      using LabelToGroupMapType = std::map<LabelValueType, GroupIndexType>;
      LabelToGroupMapType m_LabelToGroupMap;

      LookupTable::Pointer m_LookupTable;

      LabelValueType m_ActiveLabelValue;

    private:
      /** Indicates if the MultiLabelSegmentation allows to overwrite unlabeled pixels in normal pixel manipulation operations (e.g. TransferLabelConent).*/
      bool m_UnlabeledLabelLock;

    public:

      /**
        * \brief  */
      void UpdateCenterOfMass(PixelType pixelValue);

      itkGetModifiableObjectMacro(LookupTable, mitk::LookupTable);
      /** \brief
      */
      void SetLookupTable(LookupTable* lut);
      void UpdateLookupTable(PixelType pixelValue);


    ///////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////
    // END FUTURE MultiLabelSegmentation
    ///////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////

    /**
     * @brief Initialize an empty mitk::LabelSetImage using the information
     *        of an mitk::Image
     * @param image the image which is used for initializing the mitk::LabelSetImage
     */
    using mitk::Image::Initialize;
    void Initialize(const mitk::Image *image) override;

    /**
      * \brief  */
    void ClearBuffer();

    /**
     * @brief Merges the mitk::Label with a given target value with the active label
     *
     * @param pixelValue          the value of the label that should be the new merged label
     * @param sourcePixelValue    the value of the label that should be merged into the specified one
     * @param layer               the layer in which the merge should be performed
     */
    void MergeLabel(PixelType pixelValue, PixelType sourcePixelValue, unsigned int layer = 0);

    /**
     * @brief Merges a list of mitk::Labels with the mitk::Label that has a specific value
     *
     * @param pixelValue                  the value of the label that should be the new merged label
     * @param vectorOfSourcePixelValues   the list of label values that should be merge into the specified one
     * @param layer                       the layer in which the merge should be performed
     */
    void MergeLabels(PixelType pixelValue, const std::vector<PixelType>& vectorOfSourcePixelValues, unsigned int layer = 0);

    /**
      * \brief  */
    void UpdateCenterOfMass(PixelType pixelValue, unsigned int layer);

    /**
     * @brief Erases the label with the given value from the labelset image.
     *        The label itself will not be erased from the respective mitk::LabelSet. In order to
     *        remove the label itself use mitk::LabelSetImage::RemoveLabels()
     * @param pixelValue the pixel value of the label that will be erased from the labelset image
     */
    void EraseLabel(PixelType pixelValue);

    /**
     * @brief Erases a list of labels with the given values from the labelset image.
     * @param VectorOfLabelPixelValues the list of pixel values of the labels
     *                                 that will be erased from the labelset image
     */
    void EraseLabels(const std::vector<PixelType> &VectorOfLabelPixelValues);

    /**
      * \brief  Returns true if the labelset exists*/
    //[[deprecated("Will be removed with T28524")]]
    DEPRECATED(bool ExistLabelSet(unsigned int layer) const);

    /**
     * @brief Gets the ID of the currently active layer
     * @return the ID of the active layer
     */
    unsigned int GetActiveLayer() const;

    /** \brief
*/
    Label* GetActiveLabel();
    /** \brief
    */
    const Label* GetActiveLabel() const;

    /**
     * @brief Get the number of all existing mitk::Labels for a given layer
     * @param layer the layer ID for which the active mitk::Labels should be retrieved
     * @return the number of all existing mitk::Labels for the given layer
     */
    unsigned int GetNumberOfLabels(unsigned int layer = 0) const;

    /**
     * @brief Returns the number of all labels summed up across all layers
     * @return the overall number of labels across all layers
     */
    unsigned int GetTotalNumberOfLabels() const;

    /**
      * \brief  */
    mitk::Image::Pointer CreateLabelMask(PixelType index);

    /**
     * @brief Initialize a new mitk::LabelSetImage by an given image.
     * For all distinct pixel values of the parameter image new labels will
     * be created. If the number of distinct pixel values exceeds mitk::Label::MAX_LABEL_VALUE
     * a new layer will be created
     * @param image the image which is used for initialization
     */
    void InitializeByLabeledImage(mitk::Image::Pointer image);

    /**
      * \brief  */
    void MaskStamp(mitk::Image *mask, bool forceOverwrite);

    /**
      * \brief  */
    void SetActiveLayer(unsigned int layer);
    void SetActiveLabel(LabelValueType label);

    /**
      * \brief  */
    unsigned int GetNumberOfLayers() const;

    /**
     * \brief Adds a new layer to the LabelSetImage. The new layer will be set as the active one.
     * \param labelSet a labelset that will be added to the new layer if provided
     * \return the layer ID of the new layer
     */
    GroupIndexType AddLayer(ConstLabelVector labels = {});

    /**
    * \brief Adds a layer based on a provided mitk::Image.
    * \param layerImage is added to the vector of label images
    * \param labels labels that will be cloned and added to the new layer if provided
    * \return the layer ID of the new layer
    */
    GroupIndexType AddLayer(mitk::Image::Pointer layerImage, ConstLabelVector labels = {});

    /**
      * \brief  */
    mitk::Image *GetLayerImage(unsigned int layer);
    const mitk::Image *GetLayerImage(unsigned int layer) const;

  protected:
    mitkCloneMacro(Self);

      LabelSetImage();
    LabelSetImage(const LabelSetImage &other);
    ~LabelSetImage() override;

    template <typename TPixel, unsigned int VImageDimension>
    void LayerContainerToImageProcessing(itk::Image<TPixel, VImageDimension> *source, unsigned int layer);

    template <typename TPixel, unsigned int VImageDimension>
    void ImageToLayerContainerProcessing(itk::Image<TPixel, VImageDimension> *source, unsigned int layer) const;

    template <typename ImageType>
    void CalculateCenterOfMassProcessing(ImageType *input, LabelValueType index);

    template <typename ImageType>
    void EraseLabelProcessing(ImageType *input, PixelType index);

    template <typename ImageType>
    void MergeLabelProcessing(ImageType *input, PixelType pixelValue, PixelType index);

    template <typename ImageType>
    void MaskStampProcessing(ImageType *input, mitk::Image *mask, bool forceOverwrite);

    template <typename LabelSetImageType, typename ImageType>
    void InitializeByLabeledImageProcessing(LabelSetImageType *input, ImageType *other);

    /** helper needed for ensuring unique values in all layers until the refactoring is done.
      returns a sorted list of all labels.*/
    LabelValueVectorType GetUsedLabelValues() const;

    std::vector<Image::Pointer> m_LayerContainer;

    int m_ActiveLayer;
    LabelValueType m_ActiveLabel;

    bool m_activeLayerInvalid;
  };

  /**
  * @brief Equal A function comparing two label set images for beeing equal in meta- and imagedata
  *
  * @ingroup MITKTestingAPI
  *
  * Following aspects are tested for equality:
  *  - LabelSetImage members
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
  MITKMULTILABEL_EXPORT bool Equal(const mitk::LabelSetImage &leftHandSide,
                                   const mitk::LabelSetImage &rightHandSide,
                                   ScalarType eps,
                                   bool verbose);


  /** temporery namespace that is used until the new class MultiLabelSegmentation is
    introduced. It allows to already introduce/use some upcoming definitions, while
    refactoring code.*/
  namespace MultiLabelSegmentation
  {
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
  }

  using LabelValueMappingVector = std::vector < std::pair<Label::PixelType, Label::PixelType> >;

  /**Helper function that transfers pixels of the specified source label from source image to the destination image by using
  a specified destination label for a specific timestep. Function processes the whole image volume of the specified time step.
  @remark in its current implementation the function only transfers contents of the active layer of the passed LabelSetImages.
  @remark the function assumes that it is only called with source and destination image of same geometry.
  @remark CAUTION: The function is not save if sourceImage and destinationImage are the same instance and more than one label is transferred,
  because the changes are made in-place for performance reasons in multiple passes. If a mapped value A equals an "old value"
  that occurs later in the mapping, one ends up with a wrong transfer, as a pixel would be first mapped to A and then later again, because
  it is also an "old" value in the mapping table.
  @param sourceImage Pointer to the LabelSetImage which active layer should be used as source for the transfer.
  @param destinationImage Pointer to the LabelSetImage which active layer should be used as destination for the transfer.
  @param labelMapping Map that encodes the mappings of all label pixel transfers that should be done. First element is the
  label in the source image. The second element is the label that transferred pixels should become in the destination image.
  The order in which the labels will be transfered is the same order of elements in the labelMapping.
  If you use a heterogeneous label mapping (e.g. (1,2); so changing the label while transfering), keep in mind that
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
  MITKMULTILABEL_EXPORT void TransferLabelContentAtTimeStep(const LabelSetImage* sourceImage, LabelSetImage* destinationImage,
    const TimeStepType timeStep, LabelValueMappingVector labelMapping = { {1,1} },
    MultiLabelSegmentation::MergeStyle mergeStyle = MultiLabelSegmentation::MergeStyle::Replace,
    MultiLabelSegmentation::OverwriteStyle overwriteStlye = MultiLabelSegmentation::OverwriteStyle::RegardLocks);

  /**Helper function that transfers pixels of the specified source label from source image to the destination image by using
  a specified destination label. Function processes the whole image volume for all time steps.
  For more details please see TransferLabelContentAtTimeStep for LabelSetImages.
  @sa TransferLabelContentAtTimeStep*/
  MITKMULTILABEL_EXPORT void TransferLabelContent(const LabelSetImage* sourceImage, LabelSetImage* destinationImage, LabelValueMappingVector labelMapping = { {1,1} },
    MultiLabelSegmentation::MergeStyle mergeStyle = MultiLabelSegmentation::MergeStyle::Replace,
    MultiLabelSegmentation::OverwriteStyle overwriteStlye = MultiLabelSegmentation::OverwriteStyle::RegardLocks);


  /**Helper function that transfers pixels of the specified source label from source image to the destination image by using
  a specified destination label for a specific timestep. Function processes the whole image volume of the specified time step.
  @remark the function assumes that it is only called with source and destination image of same geometry.
  @remark CAUTION: The function is not save, if sourceImage and destinationImage are the same instance and you transfer more then one
  label, because the changes are made inplace for performance reasons but not in one pass. If a mapped value A equals a "old value"
  that is later in the mapping, one ends up with a wrong transfer, as a pixel would be first mapped to A and then latter again, because
  it is also an "old" value in the mapping table.
  @param sourceImage Pointer to the image that should be used as source for the transfer.
  @param destinationImage Pointer to the image that should be used as destination for the transfer.
  @param destinationLabelVector Reference to the vector of labels (incl. lock states) in the destination image. Unkown pixel
  values in the destinationImage will be assumed to be unlocked.
  @param sourceBackground Value indicating the background in the source image.
  @param destinationBackground Value indicating the background in the destination image.
  @param destinationBackgroundLocked Value indicating the lock state of the background in the destination image.
  @param labelMapping Map that encodes the mappings of all label pixel transfers that should be done. First element is the
  label in the source image. The second element is the label that transferred pixels should become in the destination image.
  The order in which the labels will be transfered is the same order of elements in the labelMapping.
  If you use a heterogeneous label mapping (e.g. (1,2); so changing the label while transfering), keep in mind that
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
    const TimeStepType timeStep, mitk::Label::PixelType sourceBackground = LabelSetImage::UnlabeledValue,
    mitk::Label::PixelType destinationBackground = LabelSetImage::UnlabeledValue,
    bool destinationBackgroundLocked = false,
    LabelValueMappingVector labelMapping = { {1,1} },
    MultiLabelSegmentation::MergeStyle mergeStyle = MultiLabelSegmentation::MergeStyle::Replace,
    MultiLabelSegmentation::OverwriteStyle overwriteStlye = MultiLabelSegmentation::OverwriteStyle::RegardLocks);

  /**Helper function that transfers pixels of the specified source label from source image to the destination image by using
  a specified destination label. Function processes the whole image volume for all time steps.
  For more details please see TransferLabelContentAtTimeStep.
  @sa TransferLabelContentAtTimeStep*/
  MITKMULTILABEL_EXPORT void TransferLabelContent(const Image* sourceImage, Image* destinationImage, const mitk::ConstLabelVector& destinationLabelVector,
    mitk::Label::PixelType sourceBackground = LabelSetImage::UnlabeledValue,
    mitk::Label::PixelType destinationBackground = LabelSetImage::UnlabeledValue,
    bool destinationBackgroundLocked = false,
    LabelValueMappingVector labelMapping = { {1,1} },
    MultiLabelSegmentation::MergeStyle mergeStyle = MultiLabelSegmentation::MergeStyle::Replace,
    MultiLabelSegmentation::OverwriteStyle overwriteStlye = MultiLabelSegmentation::OverwriteStyle::RegardLocks);

} // namespace mitk

#endif
