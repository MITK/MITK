/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#ifndef mitkLevelWindowManager_h
#define mitkLevelWindowManager_h

#include "mitkDataStorage.h"
#include "mitkLevelWindowProperty.h"
#include "mitkBaseProperty.h"
#include <map>
#include <utility>

namespace mitk
{

/**
  \brief Provides access to the LevelWindowProperty object and LevelWindow of the "current" image.

  - provides a LevelWindowProperty for purposes like GUI editors
  - this property comes from one of two possible sources
    - either something (e.g. the application) sets the property because of some user selection
    - OR the "Auto top-most" logic is used to search a DataStorage for the image with the highest "layer" property value

  Changes on Level/Window can be set with SetLevelWindow() and will affect either the topmost layer image,
  if isAutoTopMost() returns true, or an image which is set by SetLevelWindowProperty(LevelWindowProperty::Pointer levelWindowProperty).

  Changes to Level/Window, when another image gets active or by SetLevelWindow(const LevelWindow& levelWindow),
  will be sent to all listeners by Modified().

  DataStorageChanged() listens to the DataStorage for new or removed images. Depending on how m_AutoTopMost is set,
  the new image becomes active or not. If an image is removed from the DataStorage and m_AutoTopMost is false,
  there is a check to proof, if the active image is still available. If not, then m_AutoTopMost becomes true.

  Note that this class is not thread safe at the moment!
*/

  class MITKCORE_EXPORT LevelWindowManager : public itk::Object
  {
  public:

    mitkClassMacro(LevelWindowManager, itk::Object)
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    void SetDataStorage(DataStorage* ds);
    DataStorage* GetDataStorage();  ///< returns the datastorage

    /** @brief (Re-)Initializes the LevelWindowManager by setting the topmost image.
     *         Use the removedNode parameter if a node was removed...
     *  @param autoTopMost true: sets the topmost layer image to be affected by changes
     *  @param removedNode != NULL a node was removed from DataStorage */
    void SetAutoTopMostImage(bool autoTopMost, const DataNode* removedNode = NULL);

    void Update(const itk::EventObject& e);  ///< gets called if a visible property changes

    /** @brief Sets an specific LevelWindowProperty, all changes will affect the image belonging to this property.
     *  @throw mitk::Exception Throws an exception if the there is no image in the data storage which belongs to this property.*/
    void SetLevelWindowProperty(LevelWindowProperty::Pointer levelWindowProperty);

    /** @brief Sets new Level/Window values and informs all listeners about changes. */
    void SetLevelWindow(const LevelWindow& levelWindow);

    /** @return Returns Level/Window values for the current image.*/
    const LevelWindow& GetLevelWindow();

    /** @return Returns the current mitkLevelWindowProperty object from the image that is affected by changes.*/
    LevelWindowProperty::Pointer GetLevelWindowProperty();

    /** @return true if changes on slider or line-edits will affect always the topmost layer image. */
    bool isAutoTopMost();

    /** @brief This method is called when a node is added to the data storage.
      *        A listener on the data storage is used to call this method automatically after a node was added.
      * @throw mitk::Exception Throws an exception if something is wrong, e.g. if the number of observers differs from the number of nodes.
      */
    void DataStorageAddedNode(const DataNode* n = NULL);

    /** @brief This method is called when a node is removed to the data storage.
      *        A listener on the data storage is used to call this method automatically directly before a node will be removed.
      * @throw mitk::Exception Throws an exception if something is wrong, e.g. if the number of observers differs from the number of nodes.
      */
    void DataStorageRemovedNode(const DataNode* removedNode = NULL);

    /** @brief change notifications from mitkLevelWindowProperty */
    void OnPropertyModified(const itk::EventObject& e);

    Image* GetCurrentImage(); ///< return the currently active image

    /**
     * @return Returns the current number of observers which are registered in this object.
     * @throw mitk::Exception Throws an exception if the number of observers differs from
     *                        the number of relevant objects
     *                        which means that something is wrong.
     *
     */
    int GetNumberOfObservers();


    /**
    *  returns all nodes in the DataStorage that have the following properties:
    *  "visible" == true, "binary" == false, "levelwindow", and DataType == Image
    */
    DataStorage::SetOfObjects::ConstPointer GetRelevantNodes();

  protected:
    LevelWindowManager();
    ~LevelWindowManager();

    DataStorage::Pointer         m_DataStorage;
    LevelWindowProperty::Pointer m_LevelWindowProperty; ///< pointer to the LevelWindowProperty of the current image
    typedef std::pair<unsigned long, DataNode::Pointer> PropDataPair;
    typedef std::map<PropDataPair, BaseProperty::Pointer> ObserverToPropertyMap;

    ObserverToPropertyMap        m_PropObserverToNode;  ///< map to hold observer IDs to every visible property of DataNode�s BaseProperty
    ObserverToPropertyMap        m_PropObserverToNode2; ///< map to hold observer IDs to every layer property of DataNode�s BaseProperty
    ObserverToPropertyMap        m_PropObserverToNode3; ///< map to hold observer IDs to every Image Rendering.Mode property of DataNode�s BaseProperty
    void UpdateObservers();                             ///< updates the internal observer list. Ignores nodes which are marked to be deleted in the variable m_NodeMarkedToDelete
    void ClearPropObserverLists();                      ///< internal help method to clear both lists/maps.
    void CreatePropObserverLists();                     ///< internal help method to create both lists/maps.
    const mitk::DataNode*        m_NodeMarkedToDelete;  ///< this variable holds a data node which will be deleted from the datastorage immedeately (if there is one, NULL otherways)

    bool                         m_AutoTopMost;
    unsigned long                m_ObserverTag;
    bool                         m_IsObserverTagSet;
    unsigned long                m_PropertyModifiedTag;
    Image*                 m_CurrentImage;
    bool                         m_IsPropertyModifiedTagSet;
  };
}
#endif
