/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkLevelWindowManager_h
#define mitkLevelWindowManager_h

// mitk core
#include <mitkBaseProperty.h>
#include <mitkDataStorage.h>
#include <mitkLevelWindowProperty.h>

//  c++
#include <map>
#include <utility>
#include <vector>

namespace mitk
{
  /**
    @brief Provides access to the LevelWindowProperty object and LevelWindow of the "current" image.

    The manager observes a DataStorage and decides which image node is the current one. The
    LevelWindowProperty of that node is exposed for GUI editors and SetLevelWindow() writes
    back to it. Only nodes returned by GetRelevantNodes() that are visible and rendered with a
    level-window based "Image Rendering.Mode" are candidates.

    The current node is chosen according to the Mode:
    - Mode::TopMostImage: the candidate with the highest "layer" property.
    - Mode::SelectedImage: the candidate with the highest "layer" property among the nodes whose
      "selected" property is true. Without a selected candidate there is no current node. If
      GetApplyToAllSelectedImages() is true, SetLevelWindow() additionally writes to all other
      selected candidates.
    - Mode::ExplicitImage: the node pinned by SetLevelWindowProperty(). While the pinned node is
      not a candidate (e.g. invisible), the top-most candidate is shown instead. If the pinned
      node is removed from the DataStorage, the mode that was active before the pick is restored.

    The mode is kept across node additions and removals. The bool property "imageForLevelWindow"
    is written as an informational marker that is true only on the current node; the manager does
    not read it.

    Changes of the current node or its level window are sent to all listeners by Modified().

    Note that this class is not thread safe at the moment!
  */
  class MITKCORE_EXPORT LevelWindowManager : public itk::Object
  {
  public:

    mitkClassMacroItkParent(LevelWindowManager, itk::Object);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /** \brief Strategy for choosing the current image node. */
    enum class Mode
    {
      TopMostImage,
      SelectedImage,
      ExplicitImage
    };

    /**
     * \brief Set the DataStorage to observe for image nodes.
     *
     * Registers listeners for node add/remove events on the new DataStorage.
     * If a previous DataStorage was set, its listeners are removed first.
     *
     * \param[in] dataStorage The DataStorage to observe. Ignored if nullptr.
     */
    void SetDataStorage(DataStorage* dataStorage);

    /**
     * \brief Get the currently observed DataStorage.
     * \return Pointer to the DataStorage.
     */
    DataStorage *GetDataStorage();

    /**
     * \brief Set the strategy for choosing the current image node and apply it.
     *
     * Switching to Mode::ExplicitImage pins the node that is current at that moment.
     * Without a DataStorage the mode is only stored and applied by SetDataStorage().
     */
    void SetMode(Mode mode);
    Mode GetMode() const;

    /**
     * \brief In Mode::SelectedImage, write level window changes to all visible selected images
     *        instead of only the displayed one.
     */
    void SetApplyToAllSelectedImages(bool apply);
    bool GetApplyToAllSelectedImages() const;

    /**
     * \brief Recalculate the level window for the currently displayed component of selected images.
     *
     * Called when the "Image.Displayed Component" property changes.
     * Uses SetAuto() to recompute optimal contrast for the new component.
     *
     * The event parameter is unused.
     */
    void RecalculateLevelWindowForSelectedComponent(const itk::EventObject&);

    /**
     * \brief Determine the current node again according to the mode.
     *        This function is called if a relevant property of a data node is changed.
     */
    void Update(const itk::EventObject&);

    /**
     * \brief Pin the image belonging to the given property and switch to Mode::ExplicitImage.
     * \throw mitk::Exception if no DataStorage is set or no relevant node owns the property.
     */
    void SetLevelWindowProperty(LevelWindowProperty::Pointer levelWindowProperty);

    /**
     * \brief Set new Level/Window values and inform all listeners about changes.
     */
    void SetLevelWindow(const LevelWindow &levelWindow);

    /**
     * \brief Return the LevelWindowProperty of the current node, or nullptr if there is none.
     */
    LevelWindowProperty::Pointer GetLevelWindowProperty() const;

    /**
     * \brief Return Level/Window values of the current node.
     * \throw mitk::Exception if there is no current node.
     */
    const LevelWindow &GetLevelWindow() const;

    /**
     * \brief This method is called when a node is added to the data storage.
     *        A listener on the data storage is used to call this method automatically after a node was added.
     * \throw mitk::Exception Throws an exception if something is wrong, e.g. if the number of observers differs from
     *        the number of nodes.
     */
    void DataStorageAddedNode(const DataNode *dataNode = nullptr);

    /**
     * \brief This method is called when a node is removed from the data storage.
     *        A listener on the data storage is used to call this method automatically before a node will be removed.
     * \throw mitk::Exception Throws an exception if something is wrong, e.g. if the number of observers differs from
     *        the number of nodes.
     */
    void DataStorageRemovedNode(const DataNode *dataNode = nullptr);

    /**
     * \brief Change notifications from mitkLevelWindowProperty.
     */
    void OnPropertyModified(const itk::EventObject&);

    /**
     * \brief Return the node whose level window is displayed, or nullptr if there is none.
     */
    DataNode *GetCurrentNode() const;

    /**
     * \brief Return the image of the current node, or nullptr if there is none.
     */
    Image *GetCurrentImage() const;

    /**
     * \brief Return the number of observers for data node's "visible" property.
     *        This basically returns the number of relevant nodes to observe.
     */
    int GetNumberOfObservers() const;

    /**
     * \brief Return all nodes in the data storage that have the following properties:
     *   - "binary" == false
     *   - "levelwindow"
     *   - DataType == Image / DiffusionImage / TensorImage / OdfImage / ShImage
     */
    DataStorage::SetOfObjects::ConstPointer GetRelevantNodes() const;

  private:
    LevelWindowManager();
    ~LevelWindowManager() override;

    using PropDataPair = std::pair<unsigned long, DataNode::Pointer>;
    using ObserverToPropertyValueMap = std::map<PropDataPair, BaseProperty::Pointer>;

    void Recompute(const DataNode *removedNode = nullptr);
    DataNode *FindTopMostVisibleNode(bool selectedOnly, const DataNode *excludedNode) const;
    void SetCurrentNode(DataNode *node);

    void OnSelectedPropertyModified(const itk::EventObject&);
    void OnNodeNameModified(const itk::EventObject&);

    void UpdateObservers(const DataNode *excludedNode = nullptr);
    void ClearPropertyObserverMaps();
    void CreatePropertyObserverMaps(const DataNode *excludedNode);
    void ObserveProperty(DataNode *node,
                         const char *propertyKey,
                         void (LevelWindowManager::*callback)(const itk::EventObject &),
                         ObserverToPropertyValueMap &observerMap);

    DataStorage::Pointer m_DataStorage;
    LevelWindowProperty::Pointer m_LevelWindowProperty;
    DataNode::Pointer m_CurrentNode;
    DataNode::Pointer m_ExplicitNode;
    std::vector<DataNode::Pointer> m_DataNodesForLevelWindow;

    Mode m_Mode = Mode::TopMostImage;
    Mode m_ModeBeforeExplicitImage = Mode::TopMostImage;
    bool m_ApplyToAllSelectedImages = false;
    unsigned long m_PropertyModifiedTag = 0;
    bool m_IsPropertyModifiedTagSet = false;

    ObserverToPropertyValueMap m_ObserverToVisibleProperty;
    ObserverToPropertyValueMap m_ObserverToLayerProperty;
    ObserverToPropertyValueMap m_ObserverToRenderingModeProperty;
    ObserverToPropertyValueMap m_ObserverToDisplayedComponentProperty;
    ObserverToPropertyValueMap m_ObserverToSelectedProperty;
    ObserverToPropertyValueMap m_ObserverToNameProperty;
  };
}

#endif
