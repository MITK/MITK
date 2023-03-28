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
#include "mitkBaseProperty.h"
#include "mitkDataStorage.h"
#include "mitkLevelWindowProperty.h"

//  c++
#include <map>
#include <utility>

namespace mitk
{
  /**
    @brief Provides access to the LevelWindowProperty object and LevelWindow of the "current" image.

    - provides a LevelWindowProperty for purposes like GUI editors
    - this property comes from one of two possible sources
      - either something (e.g. the application) sets the property because of some user selection
      - OR the "Auto top-most" logic is used to search a DataStorage for the image with the highest "layer" property
        value

    Changes on Level/Window can be set with SetLevelWindow() and will affect either the topmost layer image,
    if IsAutoTopMost() returns true, or an image which is set by SetLevelWindowProperty(LevelWindowProperty::Pointer
    levelWindowProperty).
    Additionally the changes on Level/Window will affect one or multiple selected images, if IsSelectedImages() returns true.
    Only one of the two different modes can be enabled at the same time.

    Changes to Level/Window, when another image gets active or by SetLevelWindow(const LevelWindow& levelWindow),
    will be sent to all listeners by Modified().

    DataStorageChanged() listens to the DataStorage for new or removed images. Depending on the currently enabled mode,
    the new image becomes active or not. If an image is removed from the DataStorage and m_AutoTopMost is false,
    there is a check to proof, if the active image is still available. If not, then m_AutoTopMost becomes true.

    Note that this class is not thread safe at the moment!
  */
  class MITKCORE_EXPORT LevelWindowManager : public itk::Object
  {
  public:

    mitkClassMacroItkParent(LevelWindowManager, itk::Object);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    void SetDataStorage(DataStorage* dataStorage);
    DataStorage *GetDataStorage();

    /**
    * @brief (Re-)Initialize the LevelWindowManager by setting the topmost image.
    *        Use the removedNode parameter if a node was removed.
    *
    * @param autoTopMost    Set the topmost layer image to be affected by changes, if true.
    * @param removedNode    A node was removed from the data storage if != nullptr.
    */
    void SetAutoTopMostImage(bool autoTopMost, const DataNode *removedNode = nullptr);
    /**
    * @brief (Re-)Initialize the LevelWindowManager by setting the selected images.
    *         Use the removedNode parameter if a node was removed.
    *
    * @param selectedImagesMode  Set the selected images to be affected by changes, if true.
    * @param removedNode         A node was removed from the data storage if != nullptr.
    */
    void SetSelectedImages(bool selectedImagesMode, const DataNode *removedNode = nullptr);

    void RecalculateLevelWindowForSelectedComponent(const itk::EventObject&);
    /**
    * @brief Update the level window.
    *        This function is called if a property of a data node is changed.
    *        Relevant properties are defined in the protected 'ObserverToPropertyValueMap'-members.
    */
    void Update(const itk::EventObject&);
    /**
    * @brief Update the level window.
    *        This function is only called if the 'selected' property of a data node is changed.
    *        This is done in order to avoid finding the correct image each time a node is selected but
    *        the 'm_SelectedImages' bool value is set to false (as the normal 'Update'-function would do).
    *        Changes of the 'selected' property happen quite a lot so this should not slow down the application.
    */
    void UpdateSelected(const itk::EventObject&);
    /**
     * @brief Set a specific LevelWindowProperty; all changes will affect the image belonging to this property.
     * @throw mitk::Exception Throw an exception if the there is no image in the data storage which belongs to this
     *        property.
     */
    void SetLevelWindowProperty(LevelWindowProperty::Pointer levelWindowProperty);
    /**
    *   @brief Set new Level/Window values and inform all listeners about changes.
    */
    void SetLevelWindow(const LevelWindow &levelWindow);
    /**
     * @brief Return the current LevelWindowProperty object from the image that is affected by changes.
     *
     * @return The current LevelWindowProperty
     */
    LevelWindowProperty::Pointer GetLevelWindowProperty() const;
    /**
    * @brief Return Level/Window values for the current image
    *
    * @return The LevelWindow value for the current image.
    */
    const LevelWindow &GetLevelWindow() const;
    /**
    * @brief Return true, if level window changes will affect the topmost layer image.
    *
    * @return Return the member value that denotes the auto-topmost mode.
    */
    bool IsAutoTopMost() const;
    /**
    * @brief Return true, if level window changes will affect the currently selected images.
    *
    * @return Return the member value that denotes the selected-images mode.
    */
    bool IsSelectedImages() const;
    /**
     * @brief This method is called when a node is added to the data storage.
     *        A listener on the data storage is used to call this method automatically after a node was added.
     * @throw mitk::Exception Throws an exception if something is wrong, e.g. if the number of observers differs from
     *        the number of nodes.
     */
    void DataStorageAddedNode(const DataNode *dataNode = nullptr);
    /**
     * @brief This method is called when a node is removed from the data storage.
     *        A listener on the data storage is used to call this method automatically before a node will be removed.
     * @throw mitk::Exception Throws an exception if something is wrong, e.g. if the number of observers differs from
     *        the number of nodes.
     */
    void DataStorageRemovedNode(const DataNode *dataNode = nullptr);
    /**
    * @brief Change notifications from mitkLevelWindowProperty.
    */
    void OnPropertyModified(const itk::EventObject&);
    /**
    * @brief Return the currently active image.
    *
    * @return The member variable holding the currently active image.
    */
    Image *GetCurrentImage() const;
    /**
     * @brief Return the number of observers for data node's "visible" property.
     *        This basically returns the number of relevant nodes to observe.
     *
     * @return The current number of observers which are registered in this object.
     */
    int GetNumberOfObservers() const;
    /**
    * @brief Return all nodes in the data storage that have the following properties:
    *   - "binary" == false
    *   - "levelwindow"
    *   - DataType == Image / DiffusionImage / TensorImage / OdfImage / ShImage
    *
    @ return The filtered list of relevant nodes in the data storage
    */
    DataStorage::SetOfObjects::ConstPointer GetRelevantNodes() const;

  private:
    LevelWindowManager();
    ~LevelWindowManager() override;

    DataStorage::Pointer m_DataStorage;
    LevelWindowProperty::Pointer m_LevelWindowProperty;

    typedef std::pair<unsigned long, DataNode::Pointer> PropDataPair;
    typedef std::map<PropDataPair, BaseProperty::Pointer> ObserverToPropertyValueMap;

    ObserverToPropertyValueMap m_ObserverToVisibleProperty;
    ObserverToPropertyValueMap m_ObserverToLayerProperty;
    ObserverToPropertyValueMap m_ObserverToRenderingModeProperty;
    ObserverToPropertyValueMap m_ObserverToDisplayedComponentProperty;
    ObserverToPropertyValueMap m_ObserverToLevelWindowImageProperty;
    ObserverToPropertyValueMap m_ObserverToSelectedProperty;

    void UpdateObservers();
    void ClearPropertyObserverMaps();
    void CreatePropertyObserverMaps();

    bool HasLevelWindowRenderingMode(DataNode *dataNode) const;

    // This variable holds a data node which will be deleted from the datastorage immediately.
    const DataNode *m_NodeMarkedToDelete;

    bool m_AutoTopMost;
    bool m_SelectedImagesMode;
    unsigned long m_PropertyModifiedTag;
    Image *m_CurrentImage;
    std::vector<DataNode::Pointer> m_DataNodesForLevelWindow;
    bool m_IsPropertyModifiedTagSet;
    bool m_LevelWindowMutex;
  };
}

#endif
