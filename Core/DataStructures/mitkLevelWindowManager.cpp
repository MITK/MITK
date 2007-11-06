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
#include "mitkDataStorage.h"
#include "mitkNodePredicateBase.h"
#include "mitkNodePredicateProperty.h"
#include "mitkNodePredicateDataType.h"
#include "mitkNodePredicateAND.h"
#include "mitkNodePredicateNOT.h"
#include "mitkProperties.h"

// constructor
mitk::LevelWindowManager::LevelWindowManager()
{
  m_LevelWindowProperty = NULL;
  m_CurrentImage = NULL;
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
  for( std::map<unsigned long, mitk::BaseProperty::Pointer>::iterator iter = m_PropObserverToNode.begin(); iter != m_PropObserverToNode.end(); iter++ ) {
    (*iter).second->RemoveObserver((*iter).first);
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
    LevelWindowManager::TreeChanged(itk::TreeChangeEvent<mitk::DataTreeBase>());
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
      int maxLayer = itk::NumericTraits<int>::min();
      m_LevelWindowProperty = NULL;
      std::vector<mitk::DataTreeNode::Pointer> allObjects = GetAllNodes();
      for ( std::vector<mitk::DataTreeNode::Pointer>::const_iterator objectIter = allObjects.begin();
        objectIter != allObjects.end();
        ++objectIter)
      {
        mitk::DataTreeNode* node = (*objectIter).GetPointer();
        if (node)
        {
          if (node->IsVisible(NULL))
          {
            int layer = 0;
            node->GetIntProperty("layer", layer);
            if ( layer >= maxLayer )
            {
              mitk::LevelWindowProperty::Pointer levelWindowProperty = dynamic_cast<mitk::LevelWindowProperty*>(node->GetProperty("levelwindow").GetPointer());
              if (levelWindowProperty.IsNotNull())
              {  
                m_LevelWindowProperty = levelWindowProperty;
                m_CurrentImage = dynamic_cast<mitk::Image*>(node->GetData());
                maxLayer = layer;
              }
            }
          }
        }
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
    std::vector<mitk::DataTreeNode::Pointer> allObjects = GetAllNodes();
    for ( std::vector<mitk::DataTreeNode::Pointer>::const_iterator objectIter = allObjects.begin();
        objectIter != allObjects.end();
        ++objectIter)
    {
      mitk::DataTreeNode* node = (*objectIter).GetPointer();
      if (node)
      {
        if (node->IsVisible(NULL))
        {
          mitk::LevelWindowProperty* levWinProp = dynamic_cast<mitk::LevelWindowProperty*>(node->GetProperty("levelwindow").GetPointer());
          if (levWinProp == levelWindowProperty)
          {
            m_CurrentImage = dynamic_cast<mitk::Image*>(node->GetData());
          }
        }
      }
    }
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
  }
}

// change notifications from the DataTree
/*Listens to the DataTree for new or removed images. Depending on how m_AutoTopMost is set,
  the new image becomes active or not. If an image is removed from the tree and m_AutoTopMost is false,
  there is a check to proof, if the active image is still available. If not, then m_AutoTopMost becomes true.*/
void mitk::LevelWindowManager::TreeChanged(const itk::EventObject&)
{
  for( std::map<unsigned long, mitk::BaseProperty::Pointer>::iterator iter = m_PropObserverToNode.begin(); iter != m_PropObserverToNode.end(); iter++ ) {
    (*iter).second->RemoveObserver((*iter).first);
  }
  m_PropObserverToNode.clear();

  bool stillExists = false;

  if (m_DataTree.IsNotNull())
  {
    DataTreeIteratorClone iter = m_DataTree->GetIteratorToNode( m_DataTree, NULL );
    iter->GoToBegin();
    while ( !iter->IsAtEnd() )
    {
      if ( (iter->Get().IsNotNull()) && (iter->Get()->GetProperty("visible")) )
      {
        itk::ReceptorMemberCommand<LevelWindowManager>::Pointer command = itk::ReceptorMemberCommand<LevelWindowManager>::New();
        command->SetCallbackFunction(this, &LevelWindowManager::Update);
        m_PropObserverToNode[iter->Get()->GetProperty("visible")->AddObserver( itk::ModifiedEvent(), command )] = iter->Get()->GetProperty("visible");
      }
      ++iter;
    }
    if (m_LevelWindowProperty.IsNotNull() && !m_AutoTopMost && m_DataTree.IsNotNull())
    {
      std::vector<mitk::DataTreeNode::Pointer> allObjects = GetAllNodes();
      for ( std::vector<mitk::DataTreeNode::Pointer>::const_iterator objectIter = allObjects.begin();
        objectIter != allObjects.end();
        ++objectIter)
      {
        mitk::DataTreeNode* node = (*objectIter).GetPointer();
        if (node)
        {
          if (node->IsVisible(NULL))
          {
            mitk::LevelWindowProperty* levelWindowProperty = dynamic_cast<mitk::LevelWindowProperty*>(node->GetProperty("levelwindow").GetPointer());
            if (levelWindowProperty)
            {
              if (m_LevelWindowProperty.GetPointer() == levelWindowProperty)
                stillExists = true;
            }
          }
        }
      }
    }
  }
  if (stillExists == false && m_DataTree.IsNotNull())
  {
    SetAutoTopMostImage(true);
  }
}

// returns the DataTree
mitk::DataTree::Pointer mitk::LevelWindowManager::GetDataTree()
{
  return m_DataTree.GetPointer();
}

// true if changes on slider or line-edits will affect always the topmost layer image
bool mitk::LevelWindowManager::isAutoTopMost()
{
  return m_AutoTopMost;
}

void mitk::LevelWindowManager::Update(const itk::EventObject&)
{
  bool stillExists = false;

  if (m_LevelWindowProperty.IsNotNull() && !m_AutoTopMost && m_DataTree.IsNotNull())
  {
    std::vector<mitk::DataTreeNode::Pointer> allObjects = GetAllNodes();
    for ( std::vector<mitk::DataTreeNode::Pointer>::const_iterator objectIter = allObjects.begin();
        objectIter != allObjects.end();
        ++objectIter)
    {
      mitk::DataTreeNode* node = (*objectIter).GetPointer();
      if (node)
      {
        mitk::LevelWindowProperty* levelWindowProperty = dynamic_cast<mitk::LevelWindowProperty*>(node->GetProperty("levelwindow").GetPointer());
        if (levelWindowProperty)
        {
          if (m_LevelWindowProperty.GetPointer() == levelWindowProperty)
            stillExists = true;
        }
      }
    }
  }
  if (stillExists == false && m_DataTree.IsNotNull())
  {
    SetAutoTopMostImage(true);
  }
}

std::vector<mitk::DataTreeNode::Pointer> mitk::LevelWindowManager::GetAllNodes()
{
  mitk::BoolProperty* myProp = new mitk::BoolProperty(true);
  mitk::DataStorage* dataStorage = mitk::DataStorage::GetInstance();
  mitk::NodePredicateProperty isVisible("visible", myProp);
  mitk::NodePredicateProperty isBinary("binary", myProp);
  mitk::NodePredicateNOT notBinary(isBinary);
  mitk::NodePredicateProperty hasLevelWindow("levelwindow", NULL);
  mitk::NodePredicateDataType isImage("Image");
  mitk::NodePredicateAND predicate( isVisible, isImage );
  mitk::NodePredicateAND predicateNotBinary( predicate, notBinary );
  mitk::NodePredicateAND predicateLevelWindow( predicateNotBinary, hasLevelWindow );

  mitk::DataStorage::SetOfObjects::ConstPointer allObjects = dataStorage->GetSubset( predicateLevelWindow );

  std::vector<DataTreeNode::Pointer> resultVector;

  for ( mitk::DataStorage::SetOfObjects::const_iterator objectIter = allObjects->begin();
        objectIter != allObjects->end();
        ++objectIter )
  {
    mitk::DataTreeNode* node = (*objectIter).GetPointer();
    resultVector.push_back( node );
  }
  delete myProp;
  return resultVector; 
}

mitk::Image* mitk::LevelWindowManager::GetCurrentImage()
{
  return m_CurrentImage;
}
