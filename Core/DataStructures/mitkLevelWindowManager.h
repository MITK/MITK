/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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

#include <mitkDataTree.h>
#include <mitkLevelWindowProperty.h>

/**
  \class mitk::LevelWindowManager mitkLevelWindowManager.h mitkLevelWindowManager.h

  \brief Provides access to the mitkLevelWindowProperty object and mitkLevelWindow to the current image.

  Changes on Level/Window can be set with SetLevelWindow() and will affect either the topmost Layer image,
  if isAutoTopMost() returns true, or an image which is set by SetLevWinProp(LevelWindowProperty::Pointer levWinProp).
  Changes on Level/Window, when another image gets active or by SetLevelWindow(const mitk::LevelWindow& levWin),
  will be sent to all listeners by Modified().

  TreeChanged() listens to the DataTree for new or removed images. Depending on how m_AutoTopMost is set,
  the new image becomes active or not. If an image is removed from the tree and m_AutoTopMost is false,
  there is a check to proof, if the active image is still available. If not, then m_AutoTopMost becomes true.

  */

namespace mitk
{
  class LevelWindowManager : public itk::Object {
  public:

    mitkClassMacro(LevelWindowManager, itk::Object)
    itkNewMacro(Self); 

    /// sets the DataTree which holds all image-nodes
    void SetDataTree(mitk::DataTree* tree);

    /// returns the DataTree
    DataTree* GetDataTree();
  
    /// sets the topmost layer image to be affected by changes
    void SetAutoTopMostImage();

    /// sets an specific LevelWindowProperty, all changes will affect the image belonging to this property.
    /// sets m_AutoTopMost to false
    void SetLevelWindowProperty(LevelWindowProperty::Pointer levelWindowProperty);

    /// sets new Level/Window values and informs all listeners about changes
    void SetLevelWindow(const mitk::LevelWindow& levelWindow);

    /// returns Level/Window values for the current image
    const LevelWindow& GetLevelWindow();

    /// returns the current mitkLevelWindowProperty object from the image that is affected by changes
    LevelWindowProperty::Pointer GetLevelWindowProperty();
  
    /// true if changes on slider or line-edits will affect always the topmost layer image
    bool isAutoTopMost();
  
    /// change notifications from the DataTree
    void TreeChanged(const itk::EventObject& treeChangedEvent);

    /// change notifications from mitkLevelWindowProperty
    void OnPropertyModified(const itk::EventObject& e);

  protected:

    /// constructor
    LevelWindowManager();

    /// destructor
    ~LevelWindowManager();

  private:

    /// the DataTree with all image-nodes
    DataTree::Pointer m_DataTree;

    /// pointer to the LevelWindowProperty of the current image
    LevelWindowProperty::Pointer m_LevelWindowProperty;

    bool m_AutoTopMost;
    unsigned long m_ObserverTag;
    bool m_IsObserverTagSet;
    unsigned long m_PropertyModifiedTag;
    bool m_IsPropertyModifiedTagSet;
  };
}

#endif
