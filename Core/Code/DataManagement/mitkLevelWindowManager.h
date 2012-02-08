/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef mitkLevelWindowManager_h
#define mitkLevelWindowManager_h

#include "mitkDataStorage.h"
#include "mitkLevelWindowProperty.h"
#include "mitkBaseProperty.h"
#include <map>

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
*/

  class MITK_CORE_EXPORT LevelWindowManager : public itk::Object 
  {
  public:

    mitkClassMacro(LevelWindowManager, itk::Object)
    itkNewMacro(Self); 

    void SetDataStorage(DataStorage* ds);
    DataStorage* GetDataStorage();  ///< returns the datastorage
    
    /// if autoTopMost == true: sets the topmost layer image to be affected by changes
    /// if removedNode != NULL a node was removed from DataStorage
    void SetAutoTopMostImage(bool autoTopMost, const DataNode* removedNode = NULL);

    void Update(const itk::EventObject& e);  ///< gets called if a visible property changes

    /**
    * Sets an specific LevelWindowProperty, all changes will affect the image belonging to this property.
    */
    void SetLevelWindowProperty(LevelWindowProperty::Pointer levelWindowProperty);

    /// sets new Level/Window values and informs all listeners about changes
    void SetLevelWindow(const LevelWindow& levelWindow);

    /// returns Level/Window values for the current image
    const LevelWindow& GetLevelWindow();

    /// returns the current mitkLevelWindowProperty object from the image that is affected by changes
    LevelWindowProperty::Pointer GetLevelWindowProperty();
  
    /// true if changes on slider or line-edits will affect always the topmost layer image
    bool isAutoTopMost();
  
    /// Change notifications from DataStorage
    void DataStorageChanged(const DataNode* n = NULL);

    /// Node removal notifications from DataStorage
    void DataStorageRemovedNode(const DataNode* removedNode = NULL);

    /// change notifications from mitkLevelWindowProperty
    void OnPropertyModified(const itk::EventObject& e);

    Image* GetCurrentImage(); ///< return the currently active image

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
    typedef std::map<unsigned long, BaseProperty::Pointer> ObserverToPropertyMap;
    ObserverToPropertyMap        m_PropObserverToNode; ///< map to hold observer ID큦 to every visible property of DataNode큦 BaseProperty
    ObserverToPropertyMap        m_PropObserverToNode2; ///< map to hold observer ID큦 to every layer property of DataNode큦 BaseProperty
    bool                         m_AutoTopMost;
    unsigned long                m_ObserverTag;
    bool                         m_IsObserverTagSet;
    unsigned long                m_PropertyModifiedTag;
    Image*                 m_CurrentImage;
    bool                         m_IsPropertyModifiedTagSet;
  };
}
#endif

