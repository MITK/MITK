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


#include "mitkLevelWindowManager.h"
#include <itkCommand.h>
#include <mitkRenderingManager.h>

// constructor
mitk::LevelWindowManager::LevelWindowManager()
{
  m_LevelWindowProperty = NULL;
  m_IsObserverTagSet = false;
  m_IsPropertyModifiedTagSet = false;
  m_DataTree = NULL;
  m_AutoTopMost = true;
}

// destructor
mitk::LevelWindowManager::~LevelWindowManager()
{
  if (m_IsObserverTagSet && m_DataTree.IsNotNull())
  {
    m_DataTree->RemoveObserver(m_ObserverTag);
    m_IsObserverTagSet = false;
  }
  if (m_IsPropertyModifiedTagSet && m_LevelWindowProperty.IsNotNull())
  {
    m_LevelWindowProperty->RemoveObserver(m_PropertyModifiedTag);
    m_IsPropertyModifiedTagSet = false;
  }
}

// sets the DataTree which holds all image-nodes
void mitk::LevelWindowManager::SetDataTree(DataTree* tree)
{
  if (tree)
  {
    if (m_IsObserverTagSet)
    {
      m_DataTree->RemoveObserver(m_ObserverTag);
      m_IsObserverTagSet = false;
    }
    m_DataTree = tree;
    itk::ReceptorMemberCommand<LevelWindowManager>::Pointer command = itk::ReceptorMemberCommand<LevelWindowManager>::New();
    command->SetCallbackFunction(this, &LevelWindowManager::TreeChanged);
    m_ObserverTag = m_DataTree->AddObserver(itk::TreeChangeEvent<mitk::DataTreeBase>(), command);
    m_IsObserverTagSet = true;
  }
}

// change notifications from mitkLevelWindowProperty
void mitk::LevelWindowManager::OnPropertyModified(const itk::EventObject& )
{
  Modified();
}

// if autoTopMost == true: sets the topmost layer image to be affected by changes
void mitk::LevelWindowManager::SetAutoTopMostImage(bool autoTopMost)
{
  m_AutoTopMost = autoTopMost;
  if (m_AutoTopMost)
  {
    if (m_IsPropertyModifiedTagSet)
    {
      m_LevelWindowProperty->RemoveObserver(m_PropertyModifiedTag);
      m_IsPropertyModifiedTagSet = false;
    }

    if (m_DataTree.IsNotNull())
    {
      DataTreeIteratorClone iter = m_DataTree->GetIteratorToNode( m_DataTree, NULL );
      iter->GoToBegin();

      int maxLayer = itk::NumericTraits<int>::min();
      m_LevelWindowProperty = NULL;
      while ( !iter->IsAtEnd() )
      {
        if ( (iter->Get().IsNotNull()) && (iter->Get()->IsVisible(NULL)) )
        {
          int layer = 0;
          iter->Get()->GetIntProperty("layer", layer);
          if ( layer >= maxLayer )
          {
            bool binary = false;
            iter->Get()->GetBoolProperty("binary", binary);
            if( binary == false)
            {
              mitk::LevelWindowProperty::Pointer levelWindowProperty = dynamic_cast<mitk::LevelWindowProperty*>(iter->Get()->GetProperty("levelwindow").GetPointer());
              if (levelWindowProperty.IsNotNull())
              { 
                m_LevelWindowProperty = levelWindowProperty;
                maxLayer = layer;
              }
            }
          }
        }
        ++iter;
      }
      if (m_LevelWindowProperty.IsNotNull())
      {
        itk::ReceptorMemberCommand<LevelWindowManager>::Pointer command = itk::ReceptorMemberCommand<LevelWindowManager>::New();
        command->SetCallbackFunction(this, &LevelWindowManager::OnPropertyModified);
        m_PropertyModifiedTag = m_LevelWindowProperty->AddObserver( itk::ModifiedEvent(), command );
        m_IsPropertyModifiedTagSet = true;
      }
      Modified();
    }
  }
}

// sets an specific LevelWindowProperty, all changes will affect the image belonging to this property.
// sets m_AutoTopMost to false
void mitk::LevelWindowManager::SetLevelWindowProperty(LevelWindowProperty::Pointer levelWindowProperty)
{
  if (levelWindowProperty.IsNotNull())
  {
    if (m_IsPropertyModifiedTagSet)
    {
      m_LevelWindowProperty->RemoveObserver(m_PropertyModifiedTag);
      m_IsPropertyModifiedTagSet = false;
    }
    m_LevelWindowProperty = levelWindowProperty;
    itk::ReceptorMemberCommand<LevelWindowManager>::Pointer command = itk::ReceptorMemberCommand<LevelWindowManager>::New();
    command->SetCallbackFunction(this, &LevelWindowManager::OnPropertyModified);
    m_PropertyModifiedTag = m_LevelWindowProperty->AddObserver( itk::ModifiedEvent(), command );
    m_IsPropertyModifiedTagSet = true;
    m_AutoTopMost = false;
    Modified();
  }
}

// returns the current mitkLevelWindowProperty object from the image that is affected by changes
mitk::LevelWindowProperty::Pointer mitk::LevelWindowManager::GetLevelWindowProperty()
{
  return m_LevelWindowProperty;
}

// returns Level/Window values for the current image
const mitk::LevelWindow& mitk::LevelWindowManager::GetLevelWindow()
{
  if (m_LevelWindowProperty.IsNotNull())
    return m_LevelWindowProperty->GetLevelWindow();
  else
    throw "No LevelWindow available!";
}

// sets new Level/Window values and informs all listeners about changes
void mitk::LevelWindowManager::SetLevelWindow(const mitk::LevelWindow& levelWindow)
{
  if (m_LevelWindowProperty.IsNotNull())
  {
    m_LevelWindowProperty->SetLevelWindow(levelWindow);
    RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

// change notifications from the DataTree
/*Listens to the DataTree for new or removed images. Depending on how m_AutoTopMost is set,
  the new image becomes active or not. If an image is removed from the tree and m_AutoTopMost is false,
  there is a check to proof, if the active image is still available. If not, then m_AutoTopMost becomes true.*/
void mitk::LevelWindowManager::TreeChanged(const itk::EventObject&)
{
  bool stillExists = false;

  if (m_LevelWindowProperty.IsNotNull() && !m_AutoTopMost && m_DataTree.IsNotNull())
  {
    DataTreeIteratorClone iter = m_DataTree->GetIteratorToNode( m_DataTree, NULL );
    iter->GoToBegin();

    while ( !iter->IsAtEnd() )
    {
      if ( (iter->Get().IsNotNull()) && (iter->Get()->IsVisible(NULL)) )
      {
        bool binary = false;
        iter->Get()->GetBoolProperty("binary", binary);
        if( binary == false)
        { 
          mitk::LevelWindowProperty* levelWindowProperty = dynamic_cast<mitk::LevelWindowProperty*>(iter->Get()->GetProperty("levelwindow").GetPointer());
          if (levelWindowProperty)
          {
            if (m_LevelWindowProperty.GetPointer() == levelWindowProperty)
              stillExists = true;
          }
        }
      }
      ++iter;
    }
  }
  if (stillExists == false && m_DataTree.IsNotNull())
  {
    SetAutoTopMostImage(true);
  }
  RenderingManager::GetInstance()->RequestUpdateAll();
}

// returns the DataTree
mitk::DataTree* mitk::LevelWindowManager::GetDataTree()
{
  return m_DataTree.GetPointer();
}

// true if changes on slider or line-edits will affect always the topmost layer image
bool mitk::LevelWindowManager::isAutoTopMost()
{
  return m_AutoTopMost;
}
