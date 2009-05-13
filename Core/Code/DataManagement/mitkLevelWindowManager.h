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


#ifndef MITKLEVELWINDOWMANAGER_H
#define MITKLEVELWINDOWMANAGER_H

#include <mitkDataStorage.h>
#include <mitkLevelWindowProperty.h>
#include <mitkBaseProperty.h>
#include <map>

/**Documentation
  \class mitk::LevelWindowManager mitkLevelWindowManager.h mitkLevelWindowManager.h

  \brief Provides access to the mitkLevelWindowProperty object and mitkLevelWindow to the current image.

  Changes on Level/Window can be set with SetLevelWindow() and will affect either the topmost layer image,
  if isAutoTopMost() returns true, or an image which is set by SetLevelWindowProperty(LevelWindowProperty::Pointer levelWindowProperty).
  Changes on Level/Window, when another image gets active or by SetLevelWindow(const mitk::LevelWindow& levelWindow),
  will be sent to all listeners by Modified().

  DataStorageChanged() listens to the DataStorage for new or removed images. Depending on how m_AutoTopMost is set,
  the new image becomes active or not. If an image is removed from the DataStorage and m_AutoTopMost is false,
  there is a check to proof, if the active image is still available. If not, then m_AutoTopMost becomes true.
*/

namespace mitk
{
  class MITK_CORE_EXPORT LevelWindowManager : public itk::Object 
  {
  public:

    mitkClassMacro(LevelWindowManager, itk::Object)
    itkNewMacro(Self); 

    void SetDataStorage(mitk::DataStorage* ds);
    mitk::DataStorage* GetDataStorage();  ///< returns the datastorage
    
    /// if autoTopMost == true: sets the topmost layer image to be affected by changes
    void SetAutoTopMostImage(bool autoTopMost);

    void Update(const itk::EventObject& e);  ///< gets called if a visible property changes

    /**Documentation
    * Sets an specific LevelWindowProperty, all changes will affect the image belonging to this property.
    *
    * Sets m_AutoTopMost to false
    */
    void SetLevelWindowProperty(LevelWindowProperty::Pointer levelWindowProperty);

    /// sets new Level/Window values and informs all listeners about changes
    void SetLevelWindow(const mitk::LevelWindow& levelWindow);

    /// returns Level/Window values for the current image
    const LevelWindow& GetLevelWindow();

    /// returns the current mitkLevelWindowProperty object from the image that is affected by changes
    LevelWindowProperty::Pointer GetLevelWindowProperty();
  
    /// true if changes on slider or line-edits will affect always the topmost layer image
    bool isAutoTopMost();
  
    /// Change notifications from DataStorage
    void DataStorageChanged(const mitk::DataTreeNode* n = NULL);

    /// change notifications from mitkLevelWindowProperty
    void OnPropertyModified(const itk::EventObject& e);

    mitk::Image* GetCurrentImage(); ///< return the currently active image
  protected:
    LevelWindowManager();
    ~LevelWindowManager();

    /**
    *  returns all nodes in the DataStorage that have the following properties: 
    *  "visible" == true, "binary" == false, "levelwindow", and DataType == Image
    */
    mitk::DataStorage::SetOfObjects::ConstPointer GetRelevantNodes();


    DataStorage::Pointer m_DataStorage;
    LevelWindowProperty::Pointer m_LevelWindowProperty; ///< pointer to the LevelWindowProperty of the current image
    typedef std::map<unsigned long, mitk::BaseProperty::Pointer> ObserverToPropertyMap;
    ObserverToPropertyMap m_PropObserverToNode; ///< map to hold observer ID´s to every visible property of DataTreeNode´s BaseProperty
    bool m_AutoTopMost;
    unsigned long m_ObserverTag;
    bool m_IsObserverTagSet;
    unsigned long m_PropertyModifiedTag;
    bool m_IsPropertyModifiedTagSet;
    mitk::Image* m_CurrentImage;
  };
}
#endif
