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


#include "mitkLevelWindowManager.h"
#include <itkCommand.h>
#include "mitkDataStorage.h"
#include "mitkNodePredicateBase.h"
#include "mitkNodePredicateProperty.h"
#include "mitkNodePredicateDataType.h"
#include "mitkNodePredicateAND.h"
#include "mitkNodePredicateOR.h"
#include "mitkNodePredicateNOT.h"
#include "mitkProperties.h"
#include "mitkMessage.h"


mitk::LevelWindowManager::LevelWindowManager()
: m_DataStorage(NULL), m_LevelWindowProperty(NULL), m_AutoTopMost(true),
  m_IsObserverTagSet(false), m_CurrentImage(NULL),m_IsPropertyModifiedTagSet(false)
{
}


mitk::LevelWindowManager::~LevelWindowManager()
{
  if (m_DataStorage.IsNotNull())
  {
    m_DataStorage->AddNodeEvent.RemoveListener(MessageDelegate1<LevelWindowManager, const mitk::DataTreeNode*>( this, &LevelWindowManager::DataStorageChanged ));
    m_DataStorage->RemoveNodeEvent.RemoveListener(MessageDelegate1<LevelWindowManager, const mitk::DataTreeNode*>( this, &LevelWindowManager::DataStorageRemovedNode ));
    m_DataStorage = NULL;
  }
  if (m_IsPropertyModifiedTagSet && m_LevelWindowProperty.IsNotNull())
  {
    m_LevelWindowProperty->RemoveObserver(m_PropertyModifiedTag);
    m_IsPropertyModifiedTagSet = false;
  }
  for( std::map<unsigned long, mitk::BaseProperty::Pointer>::iterator iter = m_PropObserverToNode.begin(); iter != m_PropObserverToNode.end(); iter++ ) {
    (*iter).second->RemoveObserver((*iter).first);
  }
  for( std::map<unsigned long, mitk::BaseProperty::Pointer>::iterator iter = m_PropObserverToNode2.begin(); iter != m_PropObserverToNode2.end(); iter++ ) {
    (*iter).second->RemoveObserver((*iter).first);
  }
}


void mitk::LevelWindowManager::SetDataStorage( mitk::DataStorage* ds )
{
  if (ds == NULL)
    return;

  /* remove listeners of old DataStorage */
  if (m_DataStorage.IsNotNull())
  {
    m_DataStorage->AddNodeEvent.RemoveListener(MessageDelegate1<LevelWindowManager, const mitk::DataTreeNode*>( this, &LevelWindowManager::DataStorageChanged ));
    m_DataStorage->RemoveNodeEvent.RemoveListener(MessageDelegate1<LevelWindowManager, const mitk::DataTreeNode*>( this, &LevelWindowManager::DataStorageRemovedNode ));
  }
  /* register listener for new DataStorage */
  m_DataStorage = ds;  // register 
  m_DataStorage->AddNodeEvent.AddListener(MessageDelegate1<LevelWindowManager, const mitk::DataTreeNode*>( this, &LevelWindowManager::DataStorageChanged ));
  m_DataStorage->RemoveNodeEvent.AddListener(MessageDelegate1<LevelWindowManager, const mitk::DataTreeNode*>( this, &LevelWindowManager::DataStorageRemovedNode ));
  this->DataStorageChanged(); // update us with new DataStorage
}


void mitk::LevelWindowManager::OnPropertyModified(const itk::EventObject& )
{
  Modified();
}


void mitk::LevelWindowManager::SetAutoTopMostImage(bool autoTopMost, const mitk::DataTreeNode* removedNode)
{
  m_AutoTopMost = autoTopMost;
  if (m_AutoTopMost == false)
    return;

  if (m_IsPropertyModifiedTagSet && m_LevelWindowProperty.IsNotNull())
  {
    m_LevelWindowProperty->RemoveObserver(m_PropertyModifiedTag);
    m_IsPropertyModifiedTagSet = false;
  }
  /* search topmost image */
  if (m_DataStorage.IsNull())
  {
    itkExceptionMacro("DataStorage not set");
  }
  int maxLayer = itk::NumericTraits<int>::min();
  m_LevelWindowProperty = NULL;

  mitk::DataStorage::SetOfObjects::ConstPointer all = this->GetRelevantNodes();
  for (mitk::DataStorage::SetOfObjects::ConstIterator it = all->Begin(); it != all->End(); ++it)
  {
    mitk::DataTreeNode::Pointer node = it->Value();
    if (node.IsNull() || (removedNode != NULL && node == removedNode))
      continue;
    if (node->IsVisible(NULL) == false)
      continue;
    int layer = 0;
    node->GetIntProperty("layer", layer);
    if ( layer < maxLayer )
      continue;
    mitk::LevelWindowProperty::Pointer levelWindowProperty = dynamic_cast<mitk::LevelWindowProperty*>(node->GetProperty("levelwindow"));
    if (levelWindowProperty.IsNull())
      continue;
    m_LevelWindowProperty = levelWindowProperty;
    m_CurrentImage = dynamic_cast<mitk::Image*>(node->GetData());
    maxLayer = layer;
  }
  Modified();
}

// sets an specific LevelWindowProperty, all changes will affect the image belonging to this property.
// sets m_AutoTopMost to false
void mitk::LevelWindowManager::SetLevelWindowProperty(LevelWindowProperty::Pointer levelWindowProperty)
{
  if (levelWindowProperty.IsNull())
    return;

  if (m_IsPropertyModifiedTagSet)  // remove listener for old property
  {
    m_LevelWindowProperty->RemoveObserver(m_PropertyModifiedTag);
    m_IsPropertyModifiedTagSet = false;
  }
  m_LevelWindowProperty = levelWindowProperty;

  itk::ReceptorMemberCommand<LevelWindowManager>::Pointer command = itk::ReceptorMemberCommand<LevelWindowManager>::New();  // register listener for new property
  command->SetCallbackFunction(this, &LevelWindowManager::OnPropertyModified);
  m_PropertyModifiedTag = m_LevelWindowProperty->AddObserver( itk::ModifiedEvent(), command );
  m_IsPropertyModifiedTagSet = true;
  m_AutoTopMost = false;

  /* search image than belongs to the property */
  mitk::NodePredicateProperty::Pointer p = mitk::NodePredicateProperty::New("levelwindow", m_LevelWindowProperty);
  mitk::DataTreeNode* n = m_DataStorage->GetNode(p);
  if (n == NULL)
  {
    itkExceptionMacro("No Image in DataStorage that belongs to LevelWindow property " << m_LevelWindowProperty);
  }
  m_CurrentImage = dynamic_cast<mitk::Image*>(n->GetData());
  this->Modified();
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
  {
    itkExceptionMacro("No LevelWindow available!");
  }
}

// sets new Level/Window values and informs all listeners about changes
void mitk::LevelWindowManager::SetLevelWindow(const mitk::LevelWindow& levelWindow)
{
  if (m_LevelWindowProperty.IsNotNull())
  {
    m_LevelWindowProperty->SetLevelWindow(levelWindow);
  }
  this->Modified();
}

void mitk::LevelWindowManager::DataStorageChanged( const mitk::DataTreeNode* )
{
  this->DataStorageRemovedNode();
}
void mitk::LevelWindowManager::DataStorageRemovedNode( const mitk::DataTreeNode* removedNode )
{
  /* remove old observers */
  for (ObserverToPropertyMap::iterator iter = m_PropObserverToNode.begin(); iter != m_PropObserverToNode.end(); iter++ ) 
    (*iter).second->RemoveObserver((*iter).first);
  m_PropObserverToNode.clear();

  for (ObserverToPropertyMap::iterator iter = m_PropObserverToNode2.begin(); iter != m_PropObserverToNode2.end(); iter++ ) 
    (*iter).second->RemoveObserver((*iter).first);
  m_PropObserverToNode2.clear();

  if (m_DataStorage.IsNull())
  {
    itkExceptionMacro("DataStorage not set");
  }

  /* listen to changes  in visible property of all images */

  mitk::DataStorage::SetOfObjects::ConstPointer all = this->GetRelevantNodes();
  for (mitk::DataStorage::SetOfObjects::ConstIterator it = all->Begin(); it != all->End(); ++it)
  {
    if (it->Value().IsNull())
      continue;
    /* register listener for changes in visible property */
    itk::ReceptorMemberCommand<LevelWindowManager>::Pointer command = itk::ReceptorMemberCommand<LevelWindowManager>::New();
    command->SetCallbackFunction(this, &LevelWindowManager::Update);
    m_PropObserverToNode[it->Value()->GetProperty("visible")->AddObserver( itk::ModifiedEvent(), command )] = it->Value()->GetProperty("visible");
  }

  /* listen to changes  in layer property of all images */

  for (mitk::DataStorage::SetOfObjects::ConstIterator it = all->Begin(); it != all->End(); ++it)
  {
    if (it->Value().IsNull())
      continue;
    /* register listener for changes in layer property */
    itk::ReceptorMemberCommand<LevelWindowManager>::Pointer command2 = itk::ReceptorMemberCommand<LevelWindowManager>::New();
    command2->SetCallbackFunction(this, &LevelWindowManager::Update);
    m_PropObserverToNode2[it->Value()->GetProperty("layer")->AddObserver( itk::ModifiedEvent(), command2 )] = it->Value()->GetProperty("layer");
  }

  /* search image than belongs to the property */
  if (m_LevelWindowProperty.IsNull())
    SetAutoTopMostImage(true, removedNode);
  else
  {
    mitk::NodePredicateProperty::Pointer p2 = mitk::NodePredicateProperty::New("levelwindow", m_LevelWindowProperty);
    mitk::DataTreeNode* n = m_DataStorage->GetNode(p2);
    if (n == NULL || m_AutoTopMost) // if node was deleted, change our behaviour to AutoTopMost, if AutoTopMost is true change level window to topmost node
      SetAutoTopMostImage(true, removedNode);
   }
}


mitk::DataStorage* mitk::LevelWindowManager::GetDataStorage()
{
  return m_DataStorage.GetPointer();
}


// true if changes on slider or line-edits will affect always the topmost layer image
bool mitk::LevelWindowManager::isAutoTopMost()
{
  return m_AutoTopMost;
}


void mitk::LevelWindowManager::Update(const itk::EventObject&)  // visible property of a image has changed
{
  if (m_AutoTopMost)
  {
    SetAutoTopMostImage(true);
    return;
  }
  mitk::DataStorage::SetOfObjects::ConstPointer all = this->GetRelevantNodes();
  for (mitk::DataStorage::SetOfObjects::ConstIterator it = all->Begin(); it != all->End(); ++it)
  {
    mitk::DataTreeNode::Pointer node = it->Value();
    if (node.IsNull())
      continue;
    if (node->IsVisible(NULL) == false)
      continue;
    mitk::LevelWindowProperty::Pointer levelWindowProperty = dynamic_cast<mitk::LevelWindowProperty*>(node->GetProperty("levelwindow"));
    if (levelWindowProperty.IsNull())
      continue;
    m_LevelWindowProperty = levelWindowProperty;
    m_CurrentImage = dynamic_cast<mitk::Image*>(node->GetData());
    if (m_LevelWindowProperty.IsNull() && m_LevelWindowProperty.GetPointer() == levelWindowProperty) // we found our m_LevelWindowProperty
    {
      return;
    }
  }
  Modified();
}


mitk::DataStorage::SetOfObjects::ConstPointer mitk::LevelWindowManager::GetRelevantNodes()
{
  if (m_DataStorage.IsNull())
    return mitk::DataStorage::SetOfObjects::ConstPointer(mitk::DataStorage::SetOfObjects::New());  // return empty set

  mitk::BoolProperty::Pointer trueProp = mitk::BoolProperty::New(true);
  mitk::NodePredicateProperty::Pointer isVisible = mitk::NodePredicateProperty::New("visible", mitk::BoolProperty::New(true));
  mitk::NodePredicateProperty::Pointer notBinary = mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(false));
  mitk::NodePredicateProperty::Pointer hasLevelWindow = mitk::NodePredicateProperty::New("levelwindow", NULL);
  
  mitk::NodePredicateDataType::Pointer isImage = mitk::NodePredicateDataType::New("Image");
  mitk::NodePredicateDataType::Pointer isDImage = mitk::NodePredicateDataType::New("DiffusionImage");
  mitk::NodePredicateDataType::Pointer isTImage = mitk::NodePredicateDataType::New("TensorImage");
  mitk::NodePredicateDataType::Pointer isQImage = mitk::NodePredicateDataType::New("QBallImage");
  mitk::NodePredicateOR::Pointer predicateTypes = mitk::NodePredicateOR::New();
  predicateTypes->AddPredicate(isImage);
  predicateTypes->AddPredicate(isDImage);
  predicateTypes->AddPredicate(isTImage);
  predicateTypes->AddPredicate(isQImage);
  
  mitk::NodePredicateAND::Pointer predicate = mitk::NodePredicateAND::New();
  predicate->AddPredicate(isVisible);
  predicate->AddPredicate(notBinary);
  predicate->AddPredicate(hasLevelWindow); 
  predicate->AddPredicate(predicateTypes);

  mitk::DataStorage::SetOfObjects::ConstPointer relevantNodes = m_DataStorage->GetSubset( predicate );
  return relevantNodes; 
}


mitk::Image* mitk::LevelWindowManager::GetCurrentImage()
{
  return m_CurrentImage;
}
